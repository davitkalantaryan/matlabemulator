//
// file:        matlab_emulator_application.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_application.hpp"
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

using namespace matlab;

#define MATLAB_START_COMMAND    nullptr

#define CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,...)   \
    if(m_pEngine){ \
        int nReturn=_function(__VA_ARGS__); \
        if(nReturn){ \
            Engine* pTmpEngine = m_pEngine; \
            m_pEngine = nullptr; \
            engClose(pTmpEngine);\
            OpenOrReopenMatEngine(); \
            if(m_pEngine){ \
                _function(__VA_ARGS__); \
            } \
        } \
    }

#define CHECK_MATLAB_ENGINE_AND_DO(_function,...)   CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,m_pEngine,__VA_ARGS__)
#define CHECK_MATLAB_ENGINE_AND_DO_NO_ARGS(_function)   CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,m_pEngine)

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

    CHECK_MATLAB_ENGINE_AND_DO(engSetVisible,0);

    ::QObject::connect(&m_settingsUpdateTimer,&QTimer::timeout,this,[this](){
        emit UpdateSettingsSignal(*m_pSettings);
    });
}


emulator::Application::~Application()
{
    m_calcThread.quit();
    m_calcThread.wait();

    if(m_pEngine){
        engClose(m_pEngine);
    }

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
    stderrCopy = dup(STDERR_FILENO);
    dup2(m_vErrorPipes[1],STDERR_FILENO);
    m_pEngine = engOpen(MATLAB_START_COMMAND);
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
    bool bHandled = false;

    qDebug() << "commandToRun: " << a_command;

    if(a_command=="exit"){
        bHandled = true;
        QCoreApplication::quit();
    }
    else if(a_command.startsWith("matlab")){
        int nIndex1 = a_command.indexOf(QChar('('),6);
        if(++nIndex1>6){
            QString::const_iterator strEnd = a_command.end();
            QChar cLast = *strEnd;
            while( (cLast != ')') && (cLast != ' ') && (cLast != '\t') && (cLast != '(') ){
                cLast = *(--strEnd);
            }
            if(cLast == ')'){
#define INDX_TO_DISPLAY 2
                ssize_t unReadFromError;
                char vcOutBuffer[1024];
                int nIndex2 = static_cast<int>(strEnd-a_command.begin());
                QString matCommand = a_command.mid(nIndex1,(nIndex2-nIndex1));
                vcOutBuffer[INDX_TO_DISPLAY]=0;
                CHECK_MATLAB_ENGINE_AND_DO(engOutputBuffer,vcOutBuffer,1023);
                CHECK_MATLAB_ENGINE_AND_DO(engEvalString,matCommand.toStdString().c_str());
                if(vcOutBuffer[INDX_TO_DISPLAY]){
                    vcOutBuffer[INDX_TO_DISPLAY]='\n';
                    emit MatlabOutputSignal(&vcOutBuffer[INDX_TO_DISPLAY]);
                }
                unReadFromError = ReadMatlabErrorPipe(vcOutBuffer,1023);

                if(unReadFromError>0){
                    vcOutBuffer[unReadFromError]=0;
                    emit MatlabErrorOutputSignal(vcOutBuffer);
                }

                bHandled=true;
            }
        }
    }
    else if(a_command=="showmatlab"){
        bool vis;
        int nReturn = engGetVisible(m_pEngine,&vis);
        qDebug()<<vis<<nReturn;
        //CHECK_MATLAB_ENGINE_AND_DO(engSetVisible,1);
        nReturn=engSetVisible(m_pEngine,0);
        qDebug()<<vis<<nReturn;
        nReturn=engGetVisible(m_pEngine,&vis);
        qDebug()<<vis<<nReturn;
        bHandled=true;
    }
    else if(a_command=="hidematlab"){
        CHECK_MATLAB_ENGINE_AND_DO(engSetVisible,0);
        bHandled=true;
    }
    else{
        int nIndex1 = a_command.indexOf(QChar('='),0);
        if(nIndex1>0){
            //
        }
    }

    return bHandled;
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
