//
// file:        matlab_emulator_commandprompt.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
#define MATLAB_EMULATOR_COMMANDPROMPT_HPP

#include <QTextEdit>
#include <QTextLayout>

#define BASE_CLASS  QTextEdit

namespace matlab { namespace emulator {

class CommandPrompt : public BASE_CLASS
{
public:
    CommandPrompt();
    ~CommandPrompt() override ;

private:
    bool ShouldIgnoreKeyEvent(QKeyEvent* keyEvent)const;

private:
    void keyPressEvent(QKeyEvent* keyEvent) override;
    void keyReleaseEvent(QKeyEvent* keyEvent) override;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
