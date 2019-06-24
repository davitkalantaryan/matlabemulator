//
// file:        matlab_emulator_application.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_application.hpp"
#include <QDebug>
#include <QRegExp>

using namespace matlab;

#define MATLAB_START_COMMAND    nullptr

#define CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,...)   \
    if(m_pEngine){ \
        int nReturn=_function(__VA_ARGS__); \
        if(nReturn){ \
            Engine* pTmpEngine = m_pEngine; \
            m_pEngine = nullptr; \
            engClose(pTmpEngine);\
            m_pEngine = engOpen(MATLAB_START_COMMAND); \
            if(m_pEngine){ \
                _function(__VA_ARGS__); \
            } \
        } \
    }

#define CHECK_MATLAB_ENGINE_AND_DO(_function,...)   CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,m_pEngine,__VA_ARGS__)
#define CHECK_MATLAB_ENGINE_AND_DO_NO_ARGS(_function)   CHECK_MATLAB_ENGINE_AND_DO_RAW(_function,m_pEngine)


emulator::Application::Application(int& a_argc, char** a_argv)
    :
      QApplication (a_argc,a_argv)
{
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

    m_pEngine = engOpen("matlab");
    //m_pEngine = engOpen(nullptr);

    //if(m_pEngine){
    //    engSetVisible(m_pEngine,0);
    //}
    CHECK_MATLAB_ENGINE_AND_DO(engSetVisible,0);
}


emulator::Application::~Application()
{
    m_calcThread.quit();
    m_calcThread.wait();

    if(m_pEngine){
        engClose(m_pEngine);
    }

    delete m_pSettings;
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
                int nIndex2 = static_cast<int>(strEnd-a_command.begin());
                QString matCommand = a_command.mid(nIndex1,(nIndex2-nIndex1));
                CHECK_MATLAB_ENGINE_AND_DO(engEvalString,matCommand.toStdString().c_str());
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
