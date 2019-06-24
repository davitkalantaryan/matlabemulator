//
// file:        matlab_emulator_application.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_APPLICATION_HPP
#define MATLAB_EMULATOR_APPLICATION_HPP

#include <QApplication>
#include <QString>
#include <engine.h>
#include <QThread>
#include <stdint.h>
#include <QSettings>

#ifndef OVERRIDE
#ifdef CPP11_USED
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

#ifndef NEWNULLPTR
#ifdef CPP11_USED
#define NEWNULLPTR nullptr
#else
#define NEWNULLPTR NULL
#endif
#endif

namespace matlab { namespace emulator {


class CalcThread : public QThread
{
public:
    CalcThread();
    ~CalcThread() OVERRIDE;
private:
    void run() OVERRIDE ;

private:
};

class Application : public QApplication
{
public:
    Application(int& argc, char** argv);
    ~Application() OVERRIDE ;

    bool RunCommand( QString& a_command );

private:
    CalcThread              m_calcThread;

    QSettings*              m_pSettings;
    Engine*                 m_pEngine;
    uint64_t                m_isEngineVisible : 1;
    uint64_t                m_bitwise64Reserved : 63;
};

}} // namespace matlab { namespace  {

#define ThisAppPtr  static_cast< ::matlab::emulator::Application* >(qApp)


#endif   // #ifndef MATLAB_EMULATOR_APPLICATION_HPP
