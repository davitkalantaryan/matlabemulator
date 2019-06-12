//
// file:        matlab_emulator_centralwidget.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_centralwidget.hpp"

using namespace matlab;

emulator::CentralWidget::CentralWidget()
{
    m_mainLayout.addWidget(&m_commandPromptEdit);
    setLayout(&m_mainLayout);
}


emulator::CentralWidget::~CentralWidget()
{
    //
}
