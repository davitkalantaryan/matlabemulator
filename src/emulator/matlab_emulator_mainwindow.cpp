//
// file:        matlab_emulator_mainwindow.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_mainwindow.hpp"
#include <QMenuBar>
#include <QMenu>
#include "matlab_emulator_application.hpp"

using namespace matlab;

emulator::MainWindow::MainWindow()
{
    QMenu* pMenu = menuBar()->addMenu("File");
    pMenu->addAction("Exit",this,[](){
        QCoreApplication::quit();
    });
    setCentralWidget(&m_centralWidget);
}


emulator::MainWindow::~MainWindow()
{
    //
}
