//
// file:        matlab_emulator_commandprompt.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
#define MATLAB_EMULATOR_COMMANDPROMPT_HPP

#include <QTextEdit>

namespace matlab { namespace emulator {

class CommandPrompt : public QTextEdit
{
public:
    CommandPrompt();
    ~CommandPrompt() override ;

private:
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
