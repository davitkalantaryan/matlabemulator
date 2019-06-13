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

namespace matlab { namespace emulator {

class MatlabThread : public QThread
{
public:
    MatlabThread();
    ~MatlabThread() override;
private:
    void run() override ;

private:
    Engine*                         m_pEngine;
    uint64_t                        m_isEngineVisible : 1;
    uint64_t                        m_bitwise64Reserved : 63;
};

class Application : public QApplication
{
public:
    Application(int& argc, char** argv);
    ~Application() override ;

    void RunCommand( QString&& a_command );

private:
    //
};

}} // namespace matlab { namespace  {

#define ThisAppPtr  static_cast< ::matlab::emulator::Application* >(qApp)


#endif   // #ifndef MATLAB_EMULATOR_APPLICATION_HPP
