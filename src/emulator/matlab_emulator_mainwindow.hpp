//
// file:        matlab_emulator_mainwindow.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_MAINWINDOW_HPP
#define MATLAB_EMULATOR_MAINWINDOW_HPP

#include <QMainWindow>
#include "matlab_emulator_centralwidget.hpp"

namespace matlab { namespace emulator {

class MainWindow : public QMainWindow
{
public:
    MainWindow();
    ~MainWindow() override ;

private:
    CentralWidget   m_centralWidget;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_MAINWINDOW_HPP
