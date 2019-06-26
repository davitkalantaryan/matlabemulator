//
// file:        matlab_emulator_mainwindow.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_MAINWINDOW_HPP
#define MATLAB_EMULATOR_MAINWINDOW_HPP

#include <QMainWindow>
#include "matlab_emulator_centralwidget.hpp"

#ifndef OVERRIDE
#ifdef CPP11_USED
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

namespace matlab { namespace emulator {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() OVERRIDE ;

private slots:
    void ExitActionSlot();

private:
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    CentralWidget   m_centralWidget;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_MAINWINDOW_HPP
