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
#include <QList>
#include <QVariant>

#define MAX_COMMANDS_SIZE   128

#define BASE_CLASS      QTextEdit
#define RingListType    QVariant
#define BASE_LIST       ::QList

#ifndef OVERRIDE
#ifdef CPP11_USED
#define OVERRIDE override
#else
#define OVERRIDE
#endif
#endif

namespace matlab { namespace emulator {

//class RingList final : private ::std::list< RingListType >
class RingList
{
public:
    void AddNewToTheFront( const RingListType& a_new );
    bool hasItem()const;
    BASE_LIST< RingListType >::const_iterator first()const;
    BASE_LIST< RingListType >::const_iterator lastPlus1()const;
    const BASE_LIST< RingListType >& list()const;
    void SetList(const BASE_LIST< RingListType >& aList);

private:
    BASE_LIST< RingListType >   m_container;
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
    BASE_LIST< QVariant >::const_iterator   m_lastItem;
    uint64_t                                m_lastItemSet : 1;
};

}} // namespace matlab { namespace  {


#endif   // #ifndef MATLAB_EMULATOR_COMMANDPROMPT_HPP
