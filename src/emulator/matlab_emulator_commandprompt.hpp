//
// file:        matlab_emulator_commandprompt.hpp
// created on:  2019 Jun 12
//
#ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
#define MATLAB_EMULATOR_COMMANDPROMPT_HPP

#include <QTextEdit>
#include <QTextLayout>
#include <list>
#include <QString>
#include <stdint.h>

#define MAX_COMMANDS_SIZE   128

#define BASE_CLASS  QTextEdit
#define RingListType    QString

#ifndef OVERRIDE
#ifdef CPP11_USED
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

namespace matlab { namespace emulator {

class RingList final : private ::std::list< RingListType >
{
public:
    void AddNewToTheFront( const RingListType& a_new );
    bool hasItem()const;
    ::std::list< RingListType >::const_iterator first()const;
    ::std::list< RingListType >::const_iterator lastPlus1()const;
};

class CommandPrompt : public BASE_CLASS
{
public:
    CommandPrompt();
    ~CommandPrompt() OVERRIDE ;

private:
    bool ShouldIgnoreKeyEvent(QKeyEvent* keyEvent)const;

private:
    void keyPressEvent(QKeyEvent* keyEvent) OVERRIDE;
    void keyReleaseEvent(QKeyEvent* keyEvent) OVERRIDE;

private:
    RingList                                m_commandsList;
    ::std::list< QString >::const_iterator  m_lastItem;
    uint64_t                                m_lastItemSet : 1;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
