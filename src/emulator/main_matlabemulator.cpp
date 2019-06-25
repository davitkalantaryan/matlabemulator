//
// file:        main_matlabemulator.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_application.hpp"
#include "matlab_emulator_mainwindow.hpp"
#include <qlogging.h>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#define pipe(_pfds) _pipe((_pfds),256,O_BINARY)
#else
#include <unistd.h>
#endif

int main(int a_argc, char* a_argv[])
{
    int vnPipes[2];

    pipe(vnPipes);

    //freopen("stderr.txt","w",stderr);
    dup2(vnPipes[1],STDERR_FILENO);
    qRegisterMetaType<QtMsgType>( "QtMsgType" );

    matlab::emulator::Application app(vnPipes,a_argc,a_argv);

    matlab::emulator::MainWindow    mainWindow;

    mainWindow.show();

    app.exec();

    return 0;
}
