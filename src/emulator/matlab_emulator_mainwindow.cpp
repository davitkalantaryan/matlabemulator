//
// file:        matlab_emulator_mainwindow.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_mainwindow.hpp"
#include <QMenuBar>
#include <QMenu>
#include "matlab_emulator_application.hpp"

#define KEY_FOR_GEOMETRY    "geometry"

using namespace matlab;

emulator::MainWindow::MainWindow()
{
    QSettings& appSettings = static_cast<QSettings&>(ThisApp);
    QMenu* pMenu = menuBar()->addMenu("File");
    //pMenu->addAction("Exit",this,[](){
    //    QCoreApplication::quit();
    //});
    pMenu->addAction("Exit",this,SLOT(ExitActionSlot()));
    setCentralWidget(&m_centralWidget);

    if(appSettings.contains(KEY_FOR_GEOMETRY)){
        setGeometry(appSettings.value(KEY_FOR_GEOMETRY).toRect());
    }
}


emulator::MainWindow::~MainWindow()
{
    QRect geo = geometry();
    QSettings& appSettings = static_cast<QSettings&>(ThisApp);

    appSettings.setValue(KEY_FOR_GEOMETRY,geo);
}


void emulator::MainWindow::ExitActionSlot()
{
    QCoreApplication::quit();
}

void emulator::MainWindow::moveEvent(QMoveEvent * a_event)
{
    QRect geo = geometry();
    QSettings& appSettings = static_cast<QSettings&>(ThisApp);

    QMainWindow::moveEvent(a_event);
    appSettings.setValue(KEY_FOR_GEOMETRY,geo);
}


void emulator::MainWindow::resizeEvent(QResizeEvent *a_event)
{
    QRect geo = geometry();
    QSettings& appSettings = static_cast<QSettings&>(ThisApp);

    QMainWindow::resizeEvent(a_event);
    appSettings.setValue(KEY_FOR_GEOMETRY,geo);
}
