//
// file:        matlab_emulator_application.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_APPLICATION_HPP
#define MATLAB_EMULATOR_APPLICATION_HPP

#include <QApplication>

namespace matlab { namespace emulator {

class Application : public QApplication
{
public:
    Application(int& argc, char** argv);
    ~Application() override ;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_APPLICATION_HPP
