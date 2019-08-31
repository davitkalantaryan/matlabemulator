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

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#define pipe(_pfds) _pipe((_pfds),256,O_BINARY)
#define gclose _close
#define gread  _read
#define gwrite _write
// todo: DK, modify these macroses with propers
#define STDERR_FILENO   2
#else
#include <unistd.h>
#define gclose close
#define gread  _read
#define gwrite _write
#endif

#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_memory,...)
#endif


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

    m_calcThread.start();

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
            emit a_this->MatlabOutputSignal(&vcOutBuffer[INDX_TO_DISPLAY]);
        }
        unReadFromError = a_this->ReadMatlabErrorPipe(vcOutBuffer,1023);

        if(unReadFromError>0){
            vcOutBuffer[unReadFromError]=0;
            emit a_this->MatlabErrorOutputSignal(vcOutBuffer);
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
        auto keys = a_this->m_variablesMap.keys();
        for( auto aKey : keys ){
            emit a_this->MatlabOutputSignal(QString("\n")+aKey);
        }
    }));
    m_functionsMap.insert("getdatafl",CommandStruct("gets and uncompress root data from specified file",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){
        if((!a_inputArgumentsLine.size())||(!a_retArgumetName.size())){
            // make error report
            return;
        }

        mxArray* mxData = a_this->GetMultipleBranchesFromFileCls(a_inputArgumentsLine);
        if(mxData){
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
            a_this->m_variablesMap.insert(a_retArgumetName,mxData);
        }
    }));
    m_functionsMap.insert("tomatlab",CommandStruct("put data from program memory to embedded MATLAB memory",
                                                   [](Application* a_this,const QString& a_inputArgumentsLine,const QString&){
        if(a_this->m_variablesMap.contains(a_inputArgumentsLine)){
            CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engPutVariable,a_inputArgumentsLine.toStdString().c_str(),a_this->m_variablesMap[a_inputArgumentsLine]);
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

        emit a_this->MatlabOutputSignal(QString("\n") + QDir::currentPath() + " (current directory)");
        for(i=0;i<unNumberOfPaths;++i){
            emit a_this->MatlabOutputSignal(QString("\n") + a_this->m_knownPaths[i]);
        }
    }));
    m_functionsMap.insert("pwd",CommandStruct("Show current directory",[](Application* a_this,const QString&,const QString&){

        emit a_this->MatlabOutputSignal(QString("\n") + QDir::currentPath() );

    }));
    m_functionsMap.insert("cd",CommandStruct("Change current directory",[](Application* a_this,const QString& a_inputArgumentsLine,const QString&){

        if(!QDir::setCurrent(a_inputArgumentsLine)){
            emit a_this->MatlabErrorOutputSignal(QString("\nBad directory: ")+a_inputArgumentsLine)    ;
        }

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
                        emit a_this->MatlabOutputSignal(QString("\n")+filePath);
                    } // if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                } // for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
            } // if(fileInfo.isFile()){
        } // for(auto fileInfo : fileInfList){

        for(i=0;i<unNumberOfPaths;++i){
            if( knownDir.cd( a_this->m_knownPaths[i] ) ){
                fileInfList = knownDir.entryInfoList();

                for(auto fileInfo : fileInfList){
                    if(fileInfo.isFile()){
                        filePath = fileInfo.path();

                        for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
                            if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                                emit a_this->MatlabOutputSignal(filePath);
                            } // if(filePath.endsWith(s_emulExtensions[unExtensionIndex],Qt::CaseInsensitive)){
                        } // for(unExtensionIndex=0;unExtensionIndex<s_nNumberOfExtensions;++unExtensionIndex){
                    } // if(fileInfo.isFile()){
                } // for(auto fileInfo : fileInfList){
            } // if( knownDir.cd( a_this->m_knownPaths[i] ) ){
        } // for(i=0;i<unNumberOfPaths;++i){
    }));
    m_functionsMap.insert("help",CommandStruct("Show this help",[](Application* a_this,const QString&,const QString&){
        QString helpLine;
        auto keys = a_this->m_functionsMap.keys();
        for( auto aKey : keys ){
            helpLine = QString("\n")+aKey + ":\t" + (a_this->m_functionsMap)[aKey].help ;
            //emit a_this->MatlabOutputSignal(QString("\n")+aKey);
            emit a_this->MatlabOutputSignal(helpLine);
        }
    }));
}


emulator::Application::~Application()
{
    CodeEditor* pEditorNext;

    while(m_first){
        pEditorNext = m_first->next();
        delete m_first;
        m_first = pEditorNext;
    }

    m_calcThread.quit();
    m_calcThread.wait();

    pitz::daq::RootCleanup();

    if(m_pEngine){
        engClose(m_pEngine);
    }

    if(m_vErrorPipes[0]){
        gclose(m_vErrorPipes[1]);
        gclose(m_vErrorPipes[0]);
    }

    delete m_pSettings;

    qInstallMessageHandler(m_originalMessageeHandler);
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
    QString scriptPath;

    if(FindScriptFile(a_inputArgumentsLine,&scriptPath)){
        size_t firstNonEmpty;
        const wchar_t* cpcNonEmptyLine;
        QString aCommand;
        QFile scriptFile(scriptPath);

        if (scriptFile.open(QIODevice::ReadOnly)){
            const wchar_t* pwcLine;
            QString line;
            QTextStream in(&scriptFile);
            while (!in.atEnd()){
               line = in.readLine();
               pwcLine = line.toStdWString().c_str();
               firstNonEmpty = wcsspn(pwcLine,L" \t");
               cpcNonEmptyLine = pwcLine + firstNonEmpty;
               aCommand = QString::fromWCharArray(cpcNonEmptyLine);
               RunCommand(aCommand);
            }
            scriptFile.close();
        }

    }
    else{
        qDebug()<<a_inputArgumentsLine << a_retArgumetName;
    }
}


void emulator::Application::OpenOrReopenMatEngine()
{
    int stderrCopy;
    if(!m_vErrorPipes[0]){
        if(pipe(m_vErrorPipes)){
            return;
        }
    }
    stderrCopy = dup(STDERR_FILENO);
    dup2(m_vErrorPipes[1],STDERR_FILENO);
    m_pEngine = engOpen(MATLAB_START_COMMAND);
    //m_pEngine = engOpenSingleUse(MATLAB_START_COMMAND,nullptr,nullptr);
    dup2(stderrCopy,STDERR_FILENO);
    close(stderrCopy);
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



bool emulator::Application::RunCommand( QString& a_command )
{
    QString::const_iterator strEnd;
    QChar cLast ;
    QString retArgumetName;
    QString coreCommand;
    QString inputArgumentsLine;
    QString aCommandWholeTrimed = a_command.trimmed();
    int nIndexEq = aCommandWholeTrimed.indexOf(QChar('='),0);
    int nIndexBr1 = aCommandWholeTrimed.indexOf(QChar('('),0);
    int nIndexBr2;

    qDebug() << "commandToRun: " << aCommandWholeTrimed;


    if(  ((nIndexEq>0)&&(nIndexEq<nIndexBr1))||((nIndexEq>0)&&(nIndexBr1<0)) ){
        retArgumetName=aCommandWholeTrimed.left(nIndexEq);
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
            return false;
        }

        nIndexBr2 = static_cast<int>(strEnd-coreCommand.begin());
        inputArgumentsLine = coreCommand.mid(nIndexBr1,(nIndexBr2-nIndexBr1)).trimmed();
        coreCommand = coreCommand.left(--nIndexBr1).trimmed();
    }


    if(m_functionsMap.contains(coreCommand)){
        (*m_functionsMap[coreCommand].clbk)(this,inputArgumentsLine,retArgumetName);
        return true;
    }

    return false;
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
emulator::CalcThread::CalcThread()
{
}


emulator::CalcThread::~CalcThread()
{
}


void emulator::CalcThread::run()
{
    QThread::exec();
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/
