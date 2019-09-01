//
// file:        matlab_emulator_application.cpp
// created on:  2019 Jun 12
//

//#define USE_QDIR_SEARCH_ENGINE

#include "matlab_emulator_application_c.hpp"
#include <QRegExp>
#include <iostream>
#include <QFileInfo>
#include <daq_root_reader.hpp>
#include <QFile>
#include <wchar.h>
#include <QDir>
#include <QFileInfo>
#include <common/matlabemulator_compiler_internal.h>
#include <signal.h>
#include <matlab/emulator/extendbylib.hpp>
#include <common/system/handlelib.hpp>
#include <utility>
#include <matrix.h> // from MATLAB

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#define pipe(_pfds) _pipe((_pfds),256,O_BINARY)
#define gclose _close
#define gread  _read
#define gwrite _write
#define gdup   _dup
#define gdup2  _dup2
// todo: DK, modify these macroses with propers
#define STDERR_FILENO   2
#else
#include <unistd.h>
#define SleepEx(_x,_isAlertable) usleep(1000*(_x))
#define gclose close
#define gread  read
#define gwrite write
#define gdup   dup
#define gdup2  dup2
#endif


#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_memory,...)
#endif

int CreateProcessToDevNullAndWait2(const char* a_cpcExecuteLine);
static size_t ParseInputArgumentsLine( const QString& a_inputArgumentsLine, ::std::vector<QString>* a_pOutArgs);

using namespace matlab;

#define MATLAB_START_COMMAND    nullptr

#define CHECK_MATLAB_ENGINE_AND_DO_CNT_RAW(_context,_function,...)   \
    if((_context)->m_pEngine){ \
        int nReturn=_function(__VA_ARGS__); \
        if(nReturn){ \
            Engine* pTmpEngine = (_context)->m_pEngine; \
            (_context)->m_pEngine = nullptr; \
            engClose(pTmpEngine);\
            (_context)->OpenOrReopenMatEngine(); \
            if((_context)->m_pEngine){ \
                _function(__VA_ARGS__); \
            } \
        } \
    }

#define CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,...)   CHECK_MATLAB_ENGINE_AND_DO_CNT_RAW(this,_function,__VA_ARGS__)
#define CHECK_MATLAB_ENGINE_AND_DO_RAW_LAMBDA(_function,...)   CHECK_MATLAB_ENGINE_AND_DO_CNT_RAW(a_this,_function,__VA_ARGS__)

#define CHECK_MATLAB_ENGINE_AND_DO(_function,...)   CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,m_pEngine,__VA_ARGS__)
#define CHECK_MATLAB_ENGINE_AND_DO_NO_ARGS(_function)   CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,m_pEngine)

#define CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(_function,...)   CHECK_MATLAB_ENGINE_AND_DO_RAW_LAMBDA(_function,a_this->m_pEngine,__VA_ARGS__)
#define CHECK_MATLAB_ENGINE_AND_DO_NO_ARGS_LAMBDA(_function)   CHECK_MATLAB_ENGINE_AND_DO_RAW_LAMBDA(_function,a_this->m_pEngine)

#define INDX_TO_DISPLAY 2

void noMessageOutputStatic(QtMsgType a_type, const QMessageLogContext & a_ctx,const QString &a_message);

static const char* s_emulExtensions[] ={
    ".scr"
};

static const size_t s_nNumberOfExtensions = sizeof(s_emulExtensions) / sizeof(const char*);


emulator::Application::Application(int& a_argc, char** a_argv)
    :
      QApplication (a_argc,a_argv)
{
    m_nNumberUnknownExtend1Functions = 0;
    m_pPrcHandle = nullptr;
    m_isAllowedToUse = 0;
    m_isCommandRunning2 = 0;
    m_numberOfUsers = 0;
    m_first = m_last = NEWNULLPTR;
    m_vErrorPipes[0]=m_vErrorPipes[1]=0;
    //
    m_originalMessageeHandler = qInstallMessageHandler(&noMessageOutputStatic);

    QCoreApplication::setOrganizationName("DESY");
    QCoreApplication::setApplicationName("matlabemulator");
        // use ini files on all platforms
    QSettings::setDefaultFormat(QSettings::IniFormat);

    m_pSettings = new QSettings();

    QSettings::Format fmt = m_pSettings->format();
    QSettings::Scope scp = m_pSettings->scope();
    QString filePath = m_pSettings->fileName();
    qDebug()<<fmt<<scp<<filePath;

    OpenOrReopenMatEngine();

    CHECK_MATLAB_ENGINE_AND_DO(engSetVisible,0);

    pitz::daq::RootInitialize();

    ::QObject::connect(&m_settingsUpdateTimer,&QTimer::timeout,this,[this](){
        emit UpdateSettingsSignal(*m_pSettings);
    });

    //auto func = [=](const QString&)->void {};
    ////auto aSignal = &Application::MatlabOutputSignal;
    //void (Application::*aSignal)(const QString&) = &Application::MatlabOutputSignal;
    ////emit aSignal("Hi");
    //connect(this,"Hi",this,"Huy");
    //m_functionsMap.insert("exit",&Application::MatlabOutputSignal);
    //m_functionsMap.insert("exit",&Application::RunCommand2);

    m_functionsMap.insert("exit",CommandStruct("To exit App",[](Application*,const QString&,const QString&)->void{QCoreApplication::quit();}));
    m_functionsMap.insert("matlab",CommandStruct("To call MATLAB command",[](Application* a_this,const QString& a_inputArgumentsLine,const QString&){
        char vcOutBuffer[1024];
        ssize_t unReadFromError;

        vcOutBuffer[INDX_TO_DISPLAY]=0;

        CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engOutputBuffer,vcOutBuffer,1023);
        CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engEvalString,a_inputArgumentsLine.toStdString().c_str());
        if(vcOutBuffer[INDX_TO_DISPLAY]){
            vcOutBuffer[INDX_TO_DISPLAY]='\n';
            emit a_this->InsertOutputSignal(&vcOutBuffer[INDX_TO_DISPLAY]);
        }
        unReadFromError = a_this->ReadMatlabErrorPipe(vcOutBuffer,1023);

        if(unReadFromError>0){
            vcOutBuffer[unReadFromError]=0;
            emit a_this->InsertErrorSignal(vcOutBuffer);
        }

    }));
    m_functionsMap.insert("showmatlab",CommandStruct("Show embedded MATLAB (does not work on LINUX)",[](Application* a_this,const QString&,const QString&){
        // nReturn = engGetVisible(m_pEngine,&vis);
        CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engSetVisible,true);
    }));
    m_functionsMap.insert("hidematlab",CommandStruct("Hide embedded MATLAB (does not work on LINUX, it is always hidden)",[](Application* a_this,const QString&,const QString&){
        CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engSetVisible,false);
    }));
    m_functionsMap.insert("variables",CommandStruct("lists all variables",[](Application* a_this,const QString&,const QString&){
        QString numbOfVars = QString("\nNumberOfVariables=") + QString::number(a_this->m_variablesMap.size());
        auto keys = a_this->m_variablesMap.keys();
        emit a_this->InsertOutputSignal(numbOfVars);
        for( auto aKey : keys ){
            emit a_this->InsertOutputSignal(QString("\n'")+aKey+"'");
        }
    }));
    m_functionsMap.insert("getdatafl",CommandStruct("gets and uncompress root data from specified file",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){
        if((!a_inputArgumentsLine.size())||(!a_retArgumetName.size())){
            // make error report
            return;
        }

        mxArray* mxData = a_this->GetMultipleBranchesFromFileCls(a_inputArgumentsLine);
        if(mxData){
            if(a_this->m_variablesMap.contains(a_retArgumetName)){
                mxArray* mxOldData = a_this->m_variablesMap[a_retArgumetName];
                if(mxOldData){
                    mxDestroyArray(mxOldData);
                }
            }
            a_this->m_variablesMap.insert(a_retArgumetName,mxData);
        }
    }));
    m_functionsMap.insert("getdatatm",CommandStruct("gets data for multiple branches for given time interval",
                                                    [](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){
        if((!a_inputArgumentsLine.size())||(!a_retArgumetName.size())){
            // make error report
            return;
        }

        mxArray* mxData = a_this->GetMultipleBranchesForTimeInterval(a_inputArgumentsLine);
        if(mxData){
            if(a_this->m_variablesMap.contains(a_retArgumetName)){
                mxArray* mxOldData = a_this->m_variablesMap[a_retArgumetName];
                if(mxOldData){
                    mxDestroyArray(mxOldData);
                }
            }
            a_this->m_variablesMap.insert(a_retArgumetName,mxData);
        }
    }));
    m_functionsMap.insert("tomatlab",CommandStruct("put data from program memory to embedded MATLAB memory",
                                                   [](Application* a_this,const QString& a_inputArgumentsLine,const QString&){
        //qDebug()<< "varSize=" << a_this->m_variablesMap.size() << "; keys=" << a_this->m_variablesMap.keys();
        if(a_this->m_variablesMap.contains(a_inputArgumentsLine)){
            CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engPutVariable,a_inputArgumentsLine.toStdString().c_str(),a_this->m_variablesMap[a_inputArgumentsLine]);
        }
        else{
            // InsertErrorSignal
            QString errStr = QString("\nVariable with name \"") + a_inputArgumentsLine + QString("\" can not be found!");
            emit a_this->InsertErrorSignal(errStr);
        }
    }));
    m_functionsMap.insert("script",CommandStruct("Run sequence of commands provided in the script",
                                                 [](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){

        a_this->RunScript(a_inputArgumentsLine,a_retArgumetName);

    }));
    m_functionsMap.insert("open",CommandStruct("Open script file for modification",
                                               [](Application* a_this,const QString& a_inputArgumentsLine,const QString& /*a_retArgumetName*/){
        QString scriptPath;

        if(a_this->FindScriptFile(a_inputArgumentsLine,&scriptPath)){
            CodeEditor* pEditor = new CodeEditor(a_this->m_last,scriptPath);
            if(!a_this->m_first){
                a_this->m_first = pEditor;
            }
            a_this->m_last = pEditor;

            ::QObject::connect(pEditor,&QWidget::destroyed,a_this,[a_this]{
                CodeEditor* pEditorInner = STATIC_CAST(CodeEditor*,a_this->sender());
                qDebug()<<__FUNCTION__;
                if(pEditorInner==a_this->m_first){
                    a_this->m_first = NEWNULLPTR;
                }
                if(pEditorInner==a_this->m_last){
                    a_this->m_last = NEWNULLPTR;
                }
            });

            pEditor->show();
        }

    }));
    m_functionsMap.insert("addpath",CommandStruct("To add new known path",[](Application* a_this,const QString& a_inputArgumentsLine,const QString&){
        a_this->m_knownPaths.push_back(a_inputArgumentsLine);

    }));
    m_functionsMap.insert("paths",CommandStruct("Show all known paths",[](Application* a_this,const QString&,const QString&){
        const size_t unNumberOfPaths(a_this->m_knownPaths.size());
        size_t i;

        emit a_this->InsertOutputSignal(QString("\n") + QDir::currentPath() + " (current directory)");
        for(i=0;i<unNumberOfPaths;++i){
            emit a_this->InsertOutputSignal(QString("\n") + a_this->m_knownPaths[i]);
        }
    }));
    m_functionsMap.insert("pwd",CommandStruct("Show current directory",[](Application* a_this,const QString&,const QString&){

        emit a_this->InsertOutputSignal(QString("\n") + QDir::currentPath() );

    }));
    m_functionsMap.insert("cd",CommandStruct("Change current directory",[](Application* a_this,const QString& a_inputArgumentsLine,const QString&){

        if(!QDir::setCurrent(a_inputArgumentsLine)){
            emit a_this->InsertOutputSignal(QString("\nBad directory: ")+a_inputArgumentsLine)    ;
        }

    }));
    m_functionsMap.insert("system-out",CommandStruct("Starting system command. If return variable provided the stdout is saved there",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){

        // todo: DK
        a_this->m_isCommandRunning2 = 2;
        emit a_this->RunSystemOutSignal(a_inputArgumentsLine,a_retArgumetName);

    }));
    m_functionsMap.insert("system-err",CommandStruct("Starting system command. If return variable provided the stderr is saved there",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){

        // todo: DK
        a_this->m_isCommandRunning2 = 2;
        emit a_this->RunSystemErrSignal(a_inputArgumentsLine,a_retArgumetName);

    }));
    m_functionsMap.insert("system-both",CommandStruct("Starting system command. If return variable provided the stdout and stderr is saved there",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){

        // todo: DK
        a_this->m_isCommandRunning2 = 2;
        emit a_this->RunSystemBothSignal(a_inputArgumentsLine,a_retArgumetName);

    }));
    m_functionsMap.insert("scripts-list",CommandStruct("Show all scripts from all known paths",[](Application* a_this,const QString&,const QString&){
        const size_t unNumberOfPaths(a_this->m_knownPaths.size());
        size_t i;
        QFileInfoList fileInfList;
        QDir  knownDir;
        QString filePath;
        size_t unExtensionIndex;

        knownDir.cd( QDir::currentPath() );
        fileInfList = knownDir.entryInfoList();

        for(auto fileInfo : fileInfList){
            if(fileInfo.isFile()){
                filePath = fileInfo.filePath();

                for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
                    if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                        emit a_this->InsertOutputSignal(QString("\n")+filePath);
                    } // if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                } // for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
            } // if(fileInfo.isFile()){
        } // for(auto fileInfo : fileInfList){

        for(i=0;i<unNumberOfPaths;++i){
            if( knownDir.cd( a_this->m_knownPaths[i] ) ){
                fileInfList = knownDir.entryInfoList();

                for(auto fileInfo : fileInfList){
                    if(fileInfo.isFile()){
                        filePath = fileInfo.filePath();

                        for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
                            if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                                emit a_this->InsertOutputSignal(QString("\n")+filePath);
                            } // if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                        } // for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
                    } // if(fileInfo.isFile()){
                } // for(auto fileInfo : fileInfList){
            } // if( knownDir.cd( a_this->m_knownPaths[i] ) ){
        } // for(i=0;i<unNumberOfPaths;++i){
    }));
    m_functionsMap.insert("clear-prpt",CommandStruct("To clear the command prompt",[](Application* a_this,const QString& ,const QString&){
        emit a_this->ClearPromptSignal();

    }));
    m_functionsMap.insert("clear-vars",CommandStruct("To clear the variables",[](Application* a_this,const QString& ,const QString&){
        a_this->ClearAllVariables();

    }));
    m_functionsMap.insert("commands",CommandStruct("History for the commands",[](Application* a_this,const QString&,const QString&){
        emit a_this->PrintCommandsHistSignal();
    }));
    m_functionsMap.insert("extend",CommandStruct("Extends app by library using method 1",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){
        a_this->ExtendMethod1(a_inputArgumentsLine,a_retArgumetName);

    }));
    m_functionsMap.insert("clear-ext1",CommandStruct("Clears all extends, those are done using method 1",[](Application* a_this,const QString&,const QString&){
        a_this->ClearExtends1();

    }));
    m_functionsMap.insert("list-ext1",CommandStruct("Lists all ext1 and functions",[](Application* a_this,const QString&,const QString&){
        QString reportString = QString("\nNumberOfExtend1=")+QString::number( a_this->m_librariesExt1.size());

        emit a_this->InsertOutputSignal(reportString);
        for (auto const& lib : a_this->m_librariesExt1){
            reportString =
              QString("\n\tentryName=\"") + lib.first + "\", functionsCount=" +
              QString::number(lib.second->functions.size()) +  ", libFile=\"" + lib.second->libraryPath + "\"";
            emit a_this->InsertOutputSignal(reportString);
            for(auto func : lib.second->functions){
                reportString = QString("\n\t\tfunction: \"") + func->functionName + "\"  -> " + func->helpString;
                emit a_this->InsertOutputSignal(reportString);
                if(!func->isMapped){
                    reportString = QString(" WARNING: this function is not mapped");
                    emit a_this->InsertWarningSignal(reportString);
                }
            }  // for(auto func : lib.second->functions){
        } // for (auto const& lib : a_this->m_librariesExt1){

    }));
    m_functionsMap.insert("show-var",CommandStruct("Shows the variable content ifthe type is implemented",[](Application* a_this,const QString& a_inputArgumentsLine,const QString&){
        a_this->ShowVariableIfImplemented(a_inputArgumentsLine);

    }));
    m_functionsMap.insert("help",CommandStruct("Show this help",[](Application* a_this,const QString&,const QString&){
        QString helpLine;
        auto keys = a_this->m_functionsMap.keys();
        for( auto aKey : keys ){
            helpLine = QString("\n")+aKey + ":\t" + (a_this->m_functionsMap)[aKey].help ;
            //emit a_this->MatlabOutputSignal(QString("\n")+aKey);
            emit a_this->InsertOutputSignal(helpLine);
        }
    }));

    //m_calcThread.start();

    m_workerThread.start();
    m_objectInWorkerThread.moveToThread(&m_workerThread);

    ::QObject::connect(this,&Application::RunSystemOutSignal,&m_objectInWorkerThread,[this](const QString& a_systemLine, const QString& a_retArgumetName){
        KeepSystemOutputIntoVarSysThread(a_systemLine,a_retArgumetName,common::system::readCode::RCstdout);
    });

    ::QObject::connect(this,&Application::RunSystemErrSignal,&m_objectInWorkerThread,[this](const QString& a_systemLine, const QString& a_retArgumetName){
        KeepSystemOutputIntoVarSysThread(a_systemLine,a_retArgumetName,common::system::readCode::RCstderr);
    });

    ::QObject::connect(this,&Application::RunSystemBothSignal,&m_objectInWorkerThread,[this](const QString& a_systemLine, const QString& a_retArgumetName){
        KeepSystemOutputIntoVarSysThread(a_systemLine,a_retArgumetName,common::system::readCode::RCstdout|common::system::readCode::RCstderr);
    });
}


emulator::Application::~Application()
{
    CodeEditor* pEditorNext;

    m_workerThread.quit();
    m_workerThread.wait();

    //m_calcThread.quit();
    //m_calcThread.wait();

    while(m_first){
        pEditorNext = m_first->next();
        delete m_first;
        m_first = pEditorNext;
    }

    pitz::daq::RootCleanup();

    if(m_pEngine){
        engClose(m_pEngine);
    }

    if(m_vErrorPipes[0]){
        gclose(m_vErrorPipes[1]);
        gclose(m_vErrorPipes[0]);
    }

    delete m_pSettings;
    ClearExtends1();
    ClearAllVariables();

    qInstallMessageHandler(m_originalMessageeHandler);
}


void emulator::Application::ClearAllVariables()
{
    for(mxArray* var : m_variablesMap){
        mxDestroyArray(var);
    }
    m_variablesMap.clear();
}


void emulator::Application::ShowVariableIfImplemented(const QString& a_inputArgumentsLine)
{
    if(m_variablesMap.contains(a_inputArgumentsLine)){
        mxArray* pValue = m_variablesMap[a_inputArgumentsLine];

        if(pValue){
            mxClassID clsId = mxGetClassID(pValue);

            switch(clsId){
            case mxCHAR_CLASS:{
                size_t unBufSize = mxGetNumberOfElements(pValue) * mxGetElementSize(pValue);
                if(unBufSize){
                    char* pcBufferForData = static_cast<char*>(alloca(unBufSize + 4));
                    mxGetString(pValue,pcBufferForData,unBufSize+2);
                    emit InsertOutputSignal(QString("\n") + a_inputArgumentsLine + "=\"" + QString(pcBufferForData) + "\"");
                }
                else{
                    emit InsertWarningSignal(QString("\nEmpty string is under ")+a_inputArgumentsLine);
                }
            }break;
            default:
                emit InsertWarningSignal(QString("\nDisplaying class \"") + mxGetClassName(pValue) + QString("\" is not implemented yet"));
                break;
            }
        }  // if(pValue){
        else{
            emit InsertWarningSignal(QString("\nNull is under ")+a_inputArgumentsLine);
        }

    }
    else{
        emit InsertErrorSignal(QString("\nVariable wit the name \"") + a_inputArgumentsLine + "\" does not exist");
    }
}


void emulator::Application::ClearExtends1()
{
    m_ext1Functions.clear();

    for (auto const& lib : m_librariesExt1){
        delete lib.second;
    }

    m_librariesExt1.clear();
}


void emulator::Application::ExtendMethod1(const QString& a_inputArgumentsLine,const QString& /*a_retArgumetName*/)
{
    ::std::vector<QString> vectInputArgs;

    if(ParseInputArgumentsLine(a_inputArgumentsLine,&vectInputArgs)<2){
        emit InsertErrorSignal("\nWrong syntax, 2 inputs should be provided");
        return;
    }

    if(m_librariesExt1.count(vectInputArgs[0])){
        QString errorMessage = QString("\nEntry with the name \"") + vectInputArgs[0] + "\" already exist";
        emit InsertErrorSignal(errorMessage);
        return;
    }

    void* pLibraryHandle = ::common::system::LoadDynLib(vectInputArgs[1].toStdString().c_str());

    if(!pLibraryHandle){
        QString errorMessage = QString("\nLibrary with the name \"") + vectInputArgs[1] + "\" is not found";
        emit InsertErrorSignal(errorMessage);
        return;
    }

    //TypeMatlabEmulatorTable* pTable = static_cast<TypeMatlabEmulatorTable*>(::common::system::GetSymbolAddress(pLibraryHandle,MTLAB_EMUL_TABLE_NAME));
    SEmTableEntry* pTable = static_cast<SEmTableEntry*>(::common::system::GetSymbolAddress(pLibraryHandle,MTLAB_EMUL_TABLE_NAME));

    if(!pTable){
        QString errorMessage = QString("\nTable of symbols is not found from library \"") + vectInputArgs[1] + "\" ";
        ::common::system::UnloadDynLib(pLibraryHandle);
        emit InsertErrorSignal(errorMessage);
        return;
    }

    if(!pTable[0].function){
        QString errorMessage = QString("\no function is exported to the table of symbols in the library \"") + vectInputArgs[1] + "\" ";
        ::common::system::UnloadDynLib(pLibraryHandle);
        emit InsertErrorSignal(errorMessage);
        return;
    }

    Extend1LibraryStruct* pNewLibrary = new Extend1LibraryStruct(pLibraryHandle,pTable, ::std::move(vectInputArgs[1]) );
    ExtendedFunction* pFuncEntry;
    QString tmpFuncNameStr, tmpHelpStr, warningString;

    for(int i(0);pTable[i].function;++i/*,++pNewLibrary->totalNumberOfFunctions*/){
        tmpFuncNameStr = pTable[i].functionName ? pTable[i].functionName : (QString("UnknownFunc_")+QString::number(m_nNumberUnknownExtend1Functions++));
        tmpHelpStr = pTable[i].helpString ? pTable[i].helpString : (QString("Help for function \"")+tmpFuncNameStr + "\" is not available");
        pFuncEntry = new ExtendedFunction(pTable[i],::std::move(tmpFuncNameStr),::std::move(tmpHelpStr),pNewLibrary);
        pNewLibrary->functions.push_front(pFuncEntry);
        pFuncEntry->thisIter = pNewLibrary->functions.begin();
        //if(!LIKELY_VALUE2(m_ext1Functions.count(tmpFuncNameStr),0)){
        if(!m_ext1Functions.count(tmpFuncNameStr)){
            m_ext1Functions.insert( ::std::pair<QString,ExtendedFunction*>(pFuncEntry->functionName,pFuncEntry));
            pFuncEntry->isMapped = 1;
        }
        else{
            warningString = QString("\nFunction with name \"") + tmpFuncNameStr + "\" already exist, so function will not be added.\n";
            warningString += "Anyhow you are able to access this function by command below\n";
            warningString += "run-extend(" + vectInputArgs[0] + "," + QString::number(i)+",inputArgumentLine)";
            emit InsertWarningSignal(warningString);
            continue;
        }
    }

    m_librariesExt1.insert( ::std::pair<QString,Extend1LibraryStruct*>(vectInputArgs[0],pNewLibrary) );
}


void emulator::Application::KeepSystemOutputIntoVarSysThread(const QString& a_systemLine, const QString& a_retArgumetName, int a_returnsToMask)
{
    char vcOutBuffer[1024];
    //void* vBuffers[NUMBER_OF_EXE_READ_PIPES]={[0..3]=vcOutBuffer,vcOutBuffer,vcOutBuffer,vcOutBuffer};
    void* vBuffers[NUMBER_OF_EXE_READ_PIPES]=INIT_C_ARRAYRD(vcOutBuffer);
    size_t vBuffersSizes[NUMBER_OF_EXE_READ_PIPES]=INIT_C_ARRAYRD(1024);
    size_t unReadSize;
    bool bRunWhile=true;
    ::common::system::readCode::Type readReturn;
    ::common::system::TExecHandle   pPrcHandle;
    bool retArgumentIsTaken = (a_retArgumetName=="") ? false : true;
    int nReadReturnCode;
    QString returnArgValue;

    m_numberOfUsers = 0;
    pPrcHandle=m_pPrcHandle = ::common::system::RunExecutableNoWaitLine(a_systemLine.toStdString().c_str());

    if(!m_pPrcHandle){
        // here we use this, because no need to keep pointer of command prompt
        emit InsertErrorSignal(QString("Unable to execute line: ")+a_systemLine);
        return;
    }

    emit InsertOutputSignal("\n");

    m_isAllowedToUse = 1;

    while(bRunWhile){
        readReturn = ::common::system::TExecHandle_WatitForEndAndReadFromOutOrErr(m_pPrcHandle,vBuffers,vBuffersSizes,&unReadSize,-1);
        nReadReturnCode = static_cast<int>(readReturn);

        vcOutBuffer[unReadSize]=0;

        if( retArgumentIsTaken && (nReadReturnCode>0) && (nReadReturnCode&a_returnsToMask) ){
            returnArgValue += vcOutBuffer;
        }
        //else
        {
            switch(readReturn){
            case ::common::system::readCode::RCstdout:
                emit InsertOutputSignal(vcOutBuffer);
                break;
            case ::common::system::readCode::RCstderr:
                emit InsertErrorSignal(vcOutBuffer);
                break;
            //case ::common::system::readCode::RCexeFinished2:
                //bRunWhile = false;
                //break;
            case ::common::system::readCode::RCallPipesInerror:
                if( ::common::system::TExecHandle_IsExeFinished(pPrcHandle)){
                    bRunWhile = false;
                }
                else{
                    SleepEx(1,TRUE);
                }
                break;
            default:
                //bRunWhile = false;
                qDebug()<<"Default";
                break;
            }  // switch(readReturn){

        }  // else of if( retArgumentIsTaken && (nReadReturnCode>0) && (nReadReturnCode&a_returnsToMask) ){

    }  // while(bRunWhile){

    m_isAllowedToUse = 0;

    while(m_numberOfUsers){
        SleepEx(1,TRUE);
    }

    m_pPrcHandle = nullptr;
    m_isCommandRunning2 = 0;
    TExecHandle_WaitAndClearExecutable(pPrcHandle);
    emit AppendNewPromptSignal();

    if(retArgumentIsTaken){
        mxArray* pNewArray = mxCreateString(returnArgValue.toStdString().c_str());
        if(m_variablesMap.contains(a_retArgumetName)){
            mxArray* mxOldData = m_variablesMap[a_retArgumetName];
            if(mxOldData){
                mxDestroyArray(mxOldData);
            }
        }
        this->m_variablesMap.insert(a_retArgumetName,pNewArray);
    }
}


bool emulator::Application::IsUsedAsStdin()const
{
    return m_pPrcHandle ? true : false;
}


//bool emulator::Application::IsCommandRunning() const
//{
//    return m_isCommandRunning ? true : false;
//}


/*::common::system::TExecHandle*/bool emulator::Application::IncreaseUsage()
{
    ++m_numberOfUsers;

    if(m_isAllowedToUse && m_pPrcHandle){
        //++m_numberOfUsers;
        //return m_pPrcHandle;
        return true;
    }

    --m_numberOfUsers;
    //return nullptr;
    return false;
}


void emulator::Application::DecreaseUsageOfSysHandle( /*::common::system::TExecHandle a_handle*/ )
{
    if(m_numberOfUsers>0){
        --m_numberOfUsers;
    }
}


void emulator::Application::WriteToExeStdin(const QString& a_str)
{
    if(LIKELY_VALUE2(IncreaseUsage(),true)){
        ::std::string strToWrite = a_str.toStdString();
        common::system::TExecHandle_WriteToStdIn(m_pPrcHandle,strToWrite.c_str(),strToWrite.size());

        DecreaseUsageOfSysHandle();
    }
}



void emulator::Application::MainWindowClosedGui()
{
    CodeEditor* pEditorNext;

    while(m_first){
        pEditorNext = m_first->next();
        //delete m_first;
        pEditorNext->close();
        m_first = pEditorNext;
    }
}


bool emulator::Application::FindScriptFile(const QString& a_inputArgumentsLine,QString* a_pScriptPath)
{
#ifdef USE_QDIR_SEARCH_ENGINE

#else  // #ifdef USE_QDIR_SEARCH_ENGINE
    QString& scriptPath = *a_pScriptPath;
    size_t unExtensionIndex, unDirectoryIndex, unDirsCount;
    bool bExtensionAdded=false;

    //QString scriptName;
    //qDebug()<<a_inputArgumentsLine << a_retArgumetName;

    for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
        if(a_inputArgumentsLine.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
            bExtensionAdded = true;
            break;
        }
    }

    if(bExtensionAdded){
        if(QFile::exists(a_inputArgumentsLine)){
            scriptPath  = a_inputArgumentsLine;
            return true;
        }
    }
    else{
        for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
            scriptPath = a_inputArgumentsLine + s_emulExtensions[unExtensionIndex];
            if(QFile::exists(scriptPath)){
                return true;
            }
        }
    }

    unDirsCount = m_knownPaths.size();
    for(unDirectoryIndex=0;unDirectoryIndex<unDirsCount;++unDirectoryIndex){
        scriptPath = m_knownPaths[unDirectoryIndex] + "/" + a_inputArgumentsLine;
        if(bExtensionAdded){
            if(QFile::exists(scriptPath)){
                return true;
            }
        }
        else{
            for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
                scriptPath += s_emulExtensions[unExtensionIndex];
                if(QFile::exists(scriptPath)){
                    return true;
                }
            }
        }
    }

    return false;
#endif  // #ifdef USE_QDIR_SEARCH_ENGINE
}


void emulator::Application::RunScript(const QString& a_inputArgumentsLine,const QString& a_retArgumetName)
{
    m_isCommandRunning2 = 1;

    try {
        QString scriptPath;

        if(FindScriptFile(a_inputArgumentsLine,&scriptPath)){
            QFile scriptFile(scriptPath);

            if (scriptFile.open(QIODevice::ReadOnly)){
                QString aCommand;
                QString line;
                QTextStream in(&scriptFile);
                while (!in.atEnd()){
                   line = in.readLine();
                   aCommand = line.trimmed();
                   if((aCommand.size()>0)&&(aCommand.at(0)!='#')){
                       RunCommand(aCommand,true);
                       emit InsertOutputSignal("\n");
                   }
                }
                scriptFile.close();
            }

        }
        else{
            qDebug()<<a_inputArgumentsLine << a_retArgumetName;
        }
    }
    catch (const BadCommandException&) {
        //
    }
    catch (...) {
        //
    }

    m_isCommandRunning2 = 0;
}


void emulator::Application::OpenOrReopenMatEngine()
{
    int stderrCopy;
    if(!m_vErrorPipes[0]){
        if(pipe(m_vErrorPipes)){
            return;
        }
    }
    stderrCopy = gdup(STDERR_FILENO);
    gdup2(m_vErrorPipes[1],STDERR_FILENO);
    m_pEngine = engOpen(MATLAB_START_COMMAND);
    //m_pEngine = engOpenSingleUse(MATLAB_START_COMMAND,nullptr,nullptr);
    gdup2(stderrCopy,STDERR_FILENO);
    gclose(stderrCopy);
}


ssize_t emulator::Application::ReadMatlabErrorPipe(char* a_pBuffer, rdtype_t a_bufferSize)
{
    ssize_t nReturn;
    if(m_vErrorPipes[0]){
        gwrite(m_vErrorPipes[1]," ",1);
        nReturn = (gread(m_vErrorPipes[0],a_pBuffer,a_bufferSize)-1);
    }
    else{
        nReturn = 0;
    }

    return nReturn;
}


void emulator::Application::noMessageOutputStatic(QtMsgType a_type, const QMessageLogContext & a_context,const QString &a_message)
{
    ThisAppPtr->noMessageOutput(a_type,a_context,a_message);
}


void emulator::Application::noMessageOutput(QtMsgType a_type, const QMessageLogContext &
                                            #ifdef QT_DEBUG
                                            a_context
                                            #endif
                                            ,const QString &a_message)
{

#ifdef QT_DEBUG
    QString msg =
            QString("fl:") + QFileInfo(a_context.file).fileName() + QString(",ln:") + QString::number(a_context.line) + ": ";
#else
    QString msg;
#endif

    switch (a_type) {
    case QtDebugMsg:
        msg += ("debug   : " + a_message);
        break;
    case QtWarningMsg:
        msg += ("warning : " + a_message);
        break;
    case QtCriticalMsg:
        msg += ("critical: " + a_message);
        break;
    case QtFatalMsg:
        msg += ("fatal   : " + a_message);
        break;
    case QtInfoMsg:
        msg += ("info    : " + a_message);
        break;
    //default:
    //    msg = QString("%1 - def: %2").arg(nowstr, a_message);
    //    break;
    }

    emit NewLoggingReadySignal(a_type,msg);
    ::std::wcout << msg.toStdWString() << ::std::endl;
}

emulator::Application::operator ::QSettings& ()
{
    return *m_pSettings;
}


bool emulator::Application::TryToRunExt1Function(const QString& a_coreCommand, const QString& a_inputArgumentsLine,const QString& a_retArgumetName)
{
    if( m_ext1Functions.count(a_coreCommand)){
        ExtendedFunction* ext1Func = m_ext1Functions[a_coreCommand];
        const char* cpcStringReturned = (*ext1Func->tableEntryItem.function)(a_inputArgumentsLine.toStdString().c_str());

        if(cpcStringReturned && a_retArgumetName.size()){
            mxArray* pStringomExt = mxCreateString(cpcStringReturned);
            if(m_variablesMap.contains(a_retArgumetName)){
                mxArray* mxOldData = m_variablesMap[a_retArgumetName];
                if(mxOldData){
                    mxDestroyArray(mxOldData);
                }
            }
            m_variablesMap.insert(a_retArgumetName,pStringomExt);
        }

        return true;
    }
    return false;
}


void emulator::Application::RunCommand( QString& a_command, bool a_bThrowException )
{
    if(LIKELY_VALUE2(m_isCommandRunning2,0)<2){
        QString errorString;
        QString::const_iterator strEnd;
        QChar cLast ;
        QString retArgumetName;
        QString coreCommand;
        QString inputArgumentsLine;
        QString aCommandWholeTrimed = a_command.trimmed();
        int nIndexEq = aCommandWholeTrimed.indexOf(QChar('='),0);
        int nIndexBr1 = aCommandWholeTrimed.indexOf(QChar('('),0);
        int nIndexBr2;

        // todo: DK
        //++m_isCommandRunning;

        qDebug() << "commandToRun: " << aCommandWholeTrimed;


        if(  ((nIndexEq>0)&&(nIndexEq<nIndexBr1))||((nIndexEq>0)&&(nIndexBr1<0)) ){
            retArgumetName=aCommandWholeTrimed.left(nIndexEq);
            retArgumetName = retArgumetName.trimmed();
            coreCommand = aCommandWholeTrimed.mid(nIndexEq+1);
            coreCommand = coreCommand.trimmed();
            nIndexBr1 = coreCommand.indexOf(QChar('('),0); // todo: make it faster
        }
        else{
            coreCommand = aCommandWholeTrimed;
        }

        if((++nIndexBr1)>0){
            strEnd = coreCommand.end();
            cLast = *strEnd;
            while( (cLast != ')') && (cLast != ' ') && (cLast != '\t') && (cLast != '(') ){
                cLast = *(--strEnd);
            }

            if(cLast != ')'){
                errorString =  "\nWrong syntax";
                goto errorReturnPoint;
            }

            nIndexBr2 = static_cast<int>(strEnd-coreCommand.begin());
            inputArgumentsLine = coreCommand.mid(nIndexBr1,(nIndexBr2-nIndexBr1)).trimmed();
            coreCommand = coreCommand.left(--nIndexBr1).trimmed();
        }


        if(m_functionsMap.contains(coreCommand)){
            (*m_functionsMap[coreCommand].clbk)(this,inputArgumentsLine,retArgumetName);
            if(!LIKELY_VALUE2(m_isCommandRunning2,0)){
                emit AppendNewPromptSignal();
            }
            return;
        }
        else if( !TryToRunExt1Function(coreCommand,inputArgumentsLine,retArgumetName)){
            errorString =  QString("\nCommand \"") + coreCommand + "\" could not be found";
            goto errorReturnPoint;
        }

errorReturnPoint:
        // todo: DK
        //--m_isCommandRunning;
        emit InsertErrorSignal(errorString);
        emit AppendNewPromptSignal();
        if(a_bThrowException){
            throw BadCommandException("Unable to run");
        }
        return;
    }

}

static mxArray* DataToMatlab( const ::std::list< pitz::daq::BranchOutputForUserInfo* >& a_data );

mxArray*  emulator::Application::GetMultipleBranchesFromFileCls(const QString& a_argumentsLine)
{
    using namespace ::pitz::daq;
    ::std::list< BranchUserInputInfo > aInput;
    ::std::list< BranchOutputForUserInfo* > aOutput;
    int nIndex = a_argumentsLine.indexOf(QChar(','));
    QString fileName;
    QString branchName;
    QString remainingLine;

    if(nIndex<1){
        return nullptr;
    }

    fileName = a_argumentsLine.left(nIndex).trimmed();
    remainingLine = a_argumentsLine.mid(nIndex+1);

    while(1){
        nIndex = remainingLine.indexOf(QChar(','));
        if(nIndex<0){
            break;
        }
        branchName = a_argumentsLine.left(nIndex).trimmed();
        remainingLine = remainingLine.mid(nIndex+1);
        aInput.push_back(branchName.toStdString());
    }

    aInput.push_back(remainingLine.toStdString());

    if( ::pitz::daq::GetMultipleBranchesFromFile(fileName.toStdString().c_str(),aInput,&aOutput) ){
        return nullptr;
    }

    return DataToMatlab(aOutput);

}


mxArray*  emulator::Application::GetMultipleBranchesForTimeInterval(const QString& a_argumentsLine)
{
    using namespace ::pitz::daq;
    ::std::list< BranchUserInputInfo > aInput;
    ::std::list< BranchOutputForUserInfo* > aOutput;
    int nIndex = a_argumentsLine.indexOf(QChar(','));
    QString timeStartStr, timeEndStr;
    QString branchName;
    QString remainingLine;
    time_t timeStart, timeEnd;

    if(nIndex<1){
        return nullptr;
    }
    timeStartStr = a_argumentsLine.left(nIndex).trimmed();
    remainingLine = a_argumentsLine.mid(nIndex+1);

    nIndex = remainingLine.indexOf(QChar(','));
    if(nIndex<1){
        return nullptr;
    }
    timeEndStr = remainingLine.left(nIndex).trimmed();
    remainingLine = remainingLine.mid(nIndex+1);

    timeStart = timeStartStr.toLong();
    timeEnd = timeEndStr.toLong();

    while(1){
        nIndex = remainingLine.indexOf(QChar(','));
        if(nIndex<0){
            break;
        }
        branchName = a_argumentsLine.left(nIndex).trimmed();
        remainingLine = remainingLine.mid(nIndex+1);
        aInput.push_back(branchName.toStdString());
    }

    aInput.push_back(remainingLine.toStdString());

    ::pitz::daq::GetMultipleBranchesForTime(timeStart,timeEnd,aInput,&aOutput);

    return DataToMatlab(aOutput);

}

static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const pitz::daq::BranchOutputForUserInfo& a_info);

namespace glbDataFields{enum{Name,Data=3};}
namespace locDataFields{enum{Time,Event,Data};}
namespace baseInfoFields{enum{Type,itemsCount};}
namespace advInfoFields{enum{Name=0,numberOfEntriesInTheFile=3,firstTime,lastTime,firstEvent,lastEvent};}

static mxArray* DataToMatlab( const ::std::list< pitz::daq::BranchOutputForUserInfo* >& a_data )
{
    //Type::Type   dataType;int          itemsCount;  int          numberOfEntriesInTheFile;
    // struct EntryInfoAdv : data::EntryInfo{int firstTime,lastTime,firstEvent,lastEvent; ::std::string name; const int* ptr()const{return &firstTime;} int* ptr(){return &firstTime;}};
    static const char* svcpcFieldNamesGlb[] = {"name","type","itemsCount","data"};
    static const int scnNumberOfFieldsGlb = sizeof(svcpcFieldNamesGlb)/sizeof(const char*);
    static const char* svcpcFieldNames[] = {"time","event","data"};
    static const int scnNumberOfFields = sizeof(svcpcFieldNames)/sizeof(const char*);
    pitz::daq::data::memory::ForClient* pDataRaw;
    const size_t cunNumOfBranches(a_data.size());
    size_t nIndexBranch,nIndexEvent, nNumberOfEvents;
    size_t unItemSize;
    mxClassID clsId;
    mxArray *pNameGlb,*pDataGlb;
    mxArray *pTime, *pEvent,*pData;
    ::std::list< pitz::daq::BranchOutputForUserInfo* >::const_iterator pIter(a_data.begin()),pIterEnd(a_data.end());
    mxArray* pMatArray = mxCreateStructMatrix(cunNumOfBranches,1,scnNumberOfFieldsGlb,svcpcFieldNamesGlb);
    HANDLE_MEM_DEF(pMatArray,"No memory to create struct matrix");

    for( nIndexBranch=0;pIter != pIterEnd;++nIndexBranch,++pIter ){

        //nNumberOfEvents = a_data[nIndexBranch].entryData.size();
        nNumberOfEvents = (*pIter)->data.size();

        unItemSize=InfoToMatlabRaw(&clsId,pMatArray,nIndexBranch,1,*(*pIter));

        pNameGlb = mxCreateString((*pIter)->userClbk->branchName.c_str());HANDLE_MEM_DEF(pNameGlb," ");
        pDataGlb = mxCreateStructMatrix(nNumberOfEvents,1,scnNumberOfFields,svcpcFieldNames);HANDLE_MEM_DEF(pDataGlb," ");

        for(nIndexEvent=0; nIndexEvent<nNumberOfEvents;++nIndexEvent){
            pDataRaw = (*pIter)->data[nIndexEvent];
            pTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pTime," ");
            *(STATIC_CAST(int32_t*,mxGetData(pTime)))=(*pIter)->data[nIndexEvent]->time();
            pEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pEvent," ");*(STATIC_CAST(int32_t*,mxGetData(pEvent)))=pDataRaw->gen_event();
            pData = mxCreateNumericMatrix(1,STATIC_CAST(size_t,(*pIter)->info.itemsCountPerEntry),clsId,mxREAL);HANDLE_MEM_DEF(pData," ");
            memcpy( mxGetData(pData),(*pIter)->data[nIndexEvent]->dataPtr<void>(),unItemSize* STATIC_CAST(size_t,(*pIter)->info.itemsCountPerEntry));

            mxSetFieldByNumber(pDataGlb,nIndexEvent,STATIC_CAST(size_t,locDataFields::Time),pTime);
            mxSetFieldByNumber(pDataGlb,nIndexEvent,STATIC_CAST(size_t,locDataFields::Event),pEvent);
            mxSetFieldByNumber(pDataGlb,nIndexEvent,STATIC_CAST(size_t,locDataFields::Data),pData);

        }

        mxSetFieldByNumber(pMatArray,nIndexBranch,STATIC_CAST(size_t,glbDataFields::Name),pNameGlb);
        mxSetFieldByNumber(pMatArray,nIndexBranch,STATIC_CAST(size_t,glbDataFields::Data),pDataGlb);

    }

    return pMatArray;
}


static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const pitz::daq::BranchOutputForUserInfo& a_info)
{
    // time_field = mxGetFieldNumber(pDataGlb,svcpcFieldNames[0]);
    mxArray *pType, *pItemsCount;
    size_t unItemSize;
    mxClassID& clsId = *a_pClsIdOfData;

    switch(a_info.info.dataType){
    case pitz::daq::data::type::Int:
        pType = mxCreateString("Int_t");
        clsId = mxINT32_CLASS;
        unItemSize = 4;
        break;
    case pitz::daq::data::type::Float:
        pType = mxCreateString("Float_t");
        clsId = mxSINGLE_CLASS;
        unItemSize = 4;
        break;
    case pitz::daq::data::type::CharAscii:
        pType = mxCreateString("String");
        clsId = mxCHAR_CLASS;
        unItemSize = 1;
        break;
    case pitz::daq::data::type::IIII_old:
        pType = mxCreateString("IIII_old");
        clsId = mxSTRUCT_CLASS;
        unItemSize = 16;
        break;
    case pitz::daq::data::type::IFFF_old:
        pType = mxCreateString("IFFF_old");
        clsId = mxSTRUCT_CLASS;
        unItemSize = 16;
        break;
    default:
        pType = mxCreateString("Unknown");
        clsId = mxUNKNOWN_CLASS;
        unItemSize = 1;
        break;
    }
    HANDLE_MEM_DEF(pType,"No memory to create type string");

    pItemsCount = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);HANDLE_MEM_DEF(pItemsCount," ");*(STATIC_CAST(int32_t*,mxGetData( pItemsCount )))=a_info.info.itemsCountPerEntry;

    mxSetFieldByNumber(a_pMatlabArray,a_nIndex,a_unFieldOffset + STATIC_CAST(int,baseInfoFields::Type),pType);
    mxSetFieldByNumber(a_pMatlabArray,a_nIndex,a_unFieldOffset + STATIC_CAST(int,baseInfoFields::itemsCount),pItemsCount);

    return unItemSize;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/
emulator::WorkerThread::WorkerThread()
{
}


emulator::WorkerThread::~WorkerThread()
{
}


void emulator::WorkerThread::run()
{
#ifdef _WIN32
#else
    struct sigaction oldSigaction, newSigAction;

    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;
    newSigAction.sa_restorer = nullptr;
    newSigAction.sa_handler = [](int){};

    sigaction(SIGPIPE,&newSigAction,&oldSigaction);
#endif

    QThread::exec();

#ifdef _WIN32
#else
    sigaction(SIGPIPE,&oldSigaction,nullptr);
#endif
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/

emulator::BadCommandException::BadCommandException( const ::std::string& a_what)
    :
      m_what(a_what)
{
}


const char* emulator::BadCommandException::what()const noexcept
{
    return m_what.c_str();
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/

emulator::Extend1LibraryStruct::~Extend1LibraryStruct()
{
    ExtendedFunction* pFncToDelete;

    this->notDestructing = 0;
    while(this->functions.size()){
        pFncToDelete = *this->functions.begin();
        this->functions.pop_front();
        delete pFncToDelete;
    }

    if(this->libraryHandle){
        ::common::system::UnloadDynLib(this->libraryHandle);
    }
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/

emulator::ExtendedFunction::~ExtendedFunction()
{
    if(this->parentLibHandle && this->parentLibHandle->notDestructing){
        this->parentLibHandle->functions.erase(this->thisIter);
    }
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/

static size_t ParseInputArgumentsLine( const QString& a_inputArgumentsLine, ::std::vector<QString>* a_pOutArgs)
{
    if(!a_inputArgumentsLine.size()){
        return 0;
    }

    QString aArg;
    QString remainingArgLine(a_inputArgumentsLine);
    int nComaIndex = a_inputArgumentsLine.indexOf(',');

    for(;nComaIndex>0;nComaIndex = remainingArgLine.indexOf(',')){
        aArg = remainingArgLine.left(nComaIndex);
        aArg = aArg.trimmed();
        a_pOutArgs->push_back(aArg);
        remainingArgLine = remainingArgLine.mid(nComaIndex+1);
    }

    a_pOutArgs->push_back(remainingArgLine);

    return a_pOutArgs->size();
}
