//
// file:        matlab_emulator_application.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_application.hpp"
#include <QDebug>

using namespace matlab;

emulator::Application::Application(int& a_argc, char** a_argv)
    :
      QApplication (a_argc,a_argv)
{
    //
}


emulator::Application::~Application()
{
    //
}


void emulator::Application::RunCommand( QString&& a_command )
{
    qDebug() << "commandToRun: " << a_command;

    if(a_command=="exit"){
        QCoreApplication::quit();
    }
}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////*/
emulator::MatlabThread::MatlabThread()
{
    m_pEngine = nullptr;
    m_isEngineVisible = 0;
    m_bitwise64Reserved = 0;
}


emulator::MatlabThread::~MatlabThread()
{
    if(m_pEngine){
        engClose(m_pEngine);
    }
}


void emulator::MatlabThread::run()
{
    m_pEngine = engOpen(nullptr);

    if(m_pEngine){
        engSetVisible(m_pEngine,0);
    }

    QThread::exec();
}
