//
// file:        main_matlabemulator.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_application.hpp"
#include "matlab_emulator_mainwindow.hpp"

int main(int a_argc, char* a_argv[])
{
    matlab::emulator::Application app(a_argc,a_argv);

    matlab::emulator::MainWindow    mainWindow;

    mainWindow.show();

    app.exec();

    return 0;
}
