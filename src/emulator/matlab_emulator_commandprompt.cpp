//
// file:        matlab_emulator_commandprompt.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_commandprompt.hpp"

using namespace matlab;

emulator::CommandPrompt::CommandPrompt()
{
    setText(">>");
    moveCursor(QTextCursor::End);
}


emulator::CommandPrompt::~CommandPrompt()
{
    //
}
