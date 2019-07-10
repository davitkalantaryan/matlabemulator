//
// file:        matlab_emulator_application.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_application_cpp.hpp"
#include <QRegExp>
#include <iostream>
#include <QFileInfo>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#define pipe(_pfds) _pipe((_pfds),256,O_BINARY)
#else
#include <unistd.h>
#endif
#include <daq_root_reader.hpp>

#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_memory,...)
#endif

using namespace matlab;


#if 0
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

#endif

#define INDX_TO_DISPLAY 2

void noMessageOutputStatic(QtMsgType a_type, const QMessageLogContext & a_ctx,const QString &a_message);

emulator::Application::Application(int& a_argc, char** a_argv)
    :
      QApplication (a_argc,a_argv)
{

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

    //CHECK_MATLAB_ENGINE_AND_DO(engSetVisible,0);

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

    m_functionsMap.insert("exit",[](Application*,const QString&,const QString&)->void{QCoreApplication::quit();});
    m_functionsMap.insert("matlab",[](Application* a_this,const QString& a_inputArgumentsLine,const QString&){
        char vcOutBuffer[1024];
        ssize_t unReadFromError;

        vcOutBuffer[INDX_TO_DISPLAY]=0;

        if(!a_this->m_pEngineCpp.get()){return;}

        //CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engOutputBuffer,vcOutBuffer,1023);
        //CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engEvalString,a_inputArgumentsLine.toStdString().c_str());
        if(vcOutBuffer[INDX_TO_DISPLAY]){
            vcOutBuffer[INDX_TO_DISPLAY]='\n';
            emit a_this->MatlabOutputSignal(&vcOutBuffer[INDX_TO_DISPLAY]);
        }
        unReadFromError = a_this->ReadMatlabErrorPipe(vcOutBuffer,1023);

        if(unReadFromError>0){
            vcOutBuffer[unReadFromError]=0;
            emit a_this->MatlabErrorOutputSignal(vcOutBuffer);
        }

    });
    m_functionsMap.insert("showmatlab",[](Application* ,const QString&,const QString&){
        // nReturn = engGetVisible(m_pEngine,&vis);
        //CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engSetVisible,true);
    });
    m_functionsMap.insert("hidematlab",[](Application* ,const QString&,const QString&){
        //CHECK_MATLAB_ENGINE_AND_DO_LAMBDA(engSetVisible,false);
    });
    m_functionsMap.insert("list",[](Application* a_this,const QString&,const QString&){
        auto keys = a_this->m_variablesMap.keys();
        for( auto aKey : keys ){
            emit a_this->MatlabOutputSignal(QString("\n")+aKey);
        }
    });
    m_functionsMap.insert("getdata1",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){
        if((!a_inputArgumentsLine.size())||(!a_retArgumetName.size())){
            // make error report
            return;
        }

        mxArray* mxData = a_this->GetMultipleBranchesFromFileCls(a_inputArgumentsLine);
        if(mxData){
            a_this->m_variablesMap.insert(a_retArgumetName,mxData);
        }
    });
    m_functionsMap.insert("getdata2",[](Application* a_this,const QString& a_inputArgumentsLine,const QString& a_retArgumetName){
        if((!a_inputArgumentsLine.size())||(!a_retArgumetName.size())){
            // make error report
            return;
        }

        mxArray* mxData = a_this->GetMultipleBranchesFromFileCls2(a_inputArgumentsLine);
        if(mxData){
            a_this->m_variablesMap.insert(a_retArgumetName,mxData);
        }
    });
    m_functionsMap.insert("help",[](Application* a_this,const QString&,const QString&){
        auto keys = a_this->m_functionsMap.keys();
        for( auto aKey : keys ){
            emit a_this->MatlabOutputSignal(QString("\n")+aKey);
        }
    });
}


emulator::Application::~Application()
{
    m_calcThread.quit();
    m_calcThread.wait();

    pitz::daq::RootCleanup();

    //if(m_pEngine){
    //    engClose(m_pEngine);
    //}

    if(m_vErrorPipes[0]){
        close(m_vErrorPipes[1]);
        close(m_vErrorPipes[0]);
    }

    delete m_pSettings;

    qInstallMessageHandler(m_originalMessageeHandler);
}


void emulator::Application::OpenOrReopenMatEngine()
{
    int stderrCopy;
    if(!m_vErrorPipes[0]){
        if(pipe(m_vErrorPipes)){
            return;
        }
    }
    std::vector<std::u16string> vectEngines = matlab::engine::findMATLAB();
    stderrCopy = dup(STDERR_FILENO);
    dup2(m_vErrorPipes[1],STDERR_FILENO);
    //m_pEngineCpp = matlab::engine::connectMATLAB(u"engine_for_daq_data_handling");
    m_pEngineCpp = matlab::engine::connectMATLAB(vectEngines.size() ? vectEngines[0] : u"engine_for_daq_data_handling");
    //m_pEngine = engOpenSingleUse(MATLAB_START_COMMAND,nullptr,nullptr);
    dup2(stderrCopy,STDERR_FILENO);
    close(stderrCopy);
}


ssize_t emulator::Application::ReadMatlabErrorPipe(char* a_pBuffer, size_t a_bufferSize)
{
    ssize_t nReturn;
    if(m_vErrorPipes[0]){
        write(m_vErrorPipes[1]," ",1);
        nReturn = (read(m_vErrorPipes[0],a_pBuffer,a_bufferSize)-1);
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
        (*m_functionsMap[coreCommand])(this,inputArgumentsLine,retArgumetName);
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


mxArray*  emulator::Application::GetMultipleBranchesFromFileCls2(const QString& a_argumentsLine)
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

//static size_t InfoToMatlabRaw(mxClassID* a_pClsIdOfData,mxArray* a_pMatlabArray, size_t a_nIndex, int a_unFieldOffset,const pitz::daq::BranchOutputForUserInfo& a_info);

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
#if 0
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
#endif
    return nullptr;
}


#if 0
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
#endif


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
