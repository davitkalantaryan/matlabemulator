//
// file:        matlab_emulator_centralwidget.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_CENTRALWIDGET_HPP
#define MATLAB_EMULATOR_CENTRALWIDGET_HPP

#include <QWidget>
#include <QVBoxLayout>
#include "matlab_emulator_commandprompt.hpp"

namespace matlab { namespace emulator {

class CentralWidget : public QWidget
{
public:
    CentralWidget();
    ~CentralWidget() override ;

private:
    QVBoxLayout         m_mainLayout;
    CommandPrompt       m_commandPromptEdit;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_CENTRALWIDGET_HPP
