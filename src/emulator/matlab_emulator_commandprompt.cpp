//
// file:        matlab_emulator_commandprompt.cpp
// created on:  2019 Jun 12
//

#include "matlab_emulator_commandprompt.hpp"
#include <QKeyEvent>
#include <QTextBlock>
#include <QDebug>
#include <iostream>
#include "matlab_emulator_application.hpp"
#include <utility>
#include <common/matlabemulator_compiler_internal.h>
#include <QDebug>

using namespace matlab;

#ifdef CPP11_USED
#define STD_MOVE(_data) ::std::move(_data)
#else
#define STD_MOVE(_data)     (_data)
#endif

#define COMMANDS    "commands"

emulator::CommandPrompt::CommandPrompt()
{
    m_lastlyAsStdin = 0;
    m_lastItemSet = 0;
    QSettings& aSettings = static_cast<QSettings&>(*ThisAppPtr);

    //if(aSettings.contains(COMMANDS)){
    //    QList<QVariant> aComandsList = aSettings.value(COMMANDS).toList();
    //}
    m_commandsList.SetList(aSettings.value(COMMANDS,QList<QVariant>()).toList());

    ::QObject::connect(ThisAppPtr,&Application::InsertOutputSignal,this,[this](const QString& a_matlabOut){
        //append(a_matlabOut);// this inserts into new line
        insertPlainText(a_matlabOut); // this inserts into current possition
        moveCursor(QTextCursor::End);
    });

    ::QObject::connect(ThisAppPtr,&Application::InsertErrorSignal,this,[this](const QString& a_matlabOut){
        setTextColor(QColor(255,0,0));
        //append(a_matlabOut);// this inserts into new line
        insertPlainText(a_matlabOut); // this inserts into current possition
        moveCursor(QTextCursor::End);
        setTextColor(QColor(0,0,0));
    });

    ::QObject::connect(ThisAppPtr,&Application::InsertWarningSignal,this,[this](const QString& a_matlabOut){
        setTextColor(QColor(200,200,0));
        //append(a_matlabOut);// this inserts into new line
        insertPlainText(a_matlabOut); // this inserts into current possition
        moveCursor(QTextCursor::End);
        setTextColor(QColor(0,0,0));
    });

    ::QObject::connect(ThisAppPtr,&Application::InsertColoredTextSignal,this,[this](int a_rd, int a_gr, int a_bl, const QString& a_matlabOut){
        setTextColor(QColor(a_rd,a_gr,a_bl));
        //append(a_matlabOut);// this inserts into new line
        insertPlainText(a_matlabOut); // this inserts into current possition
        moveCursor(QTextCursor::End);
        setTextColor(QColor(0,0,0));
    });

    ::QObject::connect(ThisAppPtr,&Application::AppendNewPromptSignal,this,[this](){
        append(">>");
        moveCursor(QTextCursor::End);
    });

    ::QObject::connect(ThisAppPtr,&Application::PrintCommandsHistSignal,this,[this](){
        QVariant varntCmd;
        BASE_LIST< QVariant >::const_iterator cmdIter = m_commandsList.first(), cmdIterEnd = m_commandsList.lastPlus1();
        //auto keys = a_this->m_functionsMap.keys();
        for( ;cmdIter!=cmdIterEnd;++cmdIter ){
            append((*cmdIter).toString());
        }

        //append(">>");
        //moveCursor(QTextCursor::End);
    });

    ::QObject::connect(ThisAppPtr,&Application::UpdateSettingsSignal,this,[this](QSettings& a_appSettings){
        a_appSettings.setValue(COMMANDS,m_commandsList.list());
    });

    ::QObject::connect(ThisAppPtr,&Application::ClearPromptSignal,this,[this](){
        clear();
        //setPlainText(">>");
        //moveCursor(QTextCursor::End);
        //setTextColor(QColor(0,0,0));
    });

    setTextColor(QColor(0,0,0));
    //setText(">>");
    setPlainText(">>");
    moveCursor(QTextCursor::End);
}


emulator::CommandPrompt::~CommandPrompt()
{
    QSettings& aSettings = static_cast<QSettings&>(*ThisAppPtr);
    aSettings.setValue(COMMANDS,m_commandsList.list());
}


bool emulator::CommandPrompt::ShouldIgnoreKeyEvent(bool a_isKeyPress, QKeyEvent* a_keyEvent)
{
    Qt::Key aKey = static_cast<Qt::Key>(a_keyEvent->key());
    //*a_pbIsSystemRunning =

    if(LIKELY_VALUE2(ThisAppPtr->IsUsedAsStdin(),false)){
        if(!m_lastlyAsStdin){
            QTextCursor aCursor = textCursor();
            //m_nStdinCursorPossitionAll = aCursor.position();
            m_nStdinCursorPossitionAll = aCursor.position();
            m_lastlyAsStdin = 1;
        }

        if(a_isKeyPress){
            switch(aKey){
            case Qt::Key_Enter:case Qt::Key_Return:{
                QTextCursor aCursor = textCursor();
                QTextBlock textBlock = aCursor.block();
                int nCursorPossitionAll = aCursor.position();
                //int nCursorPossitionAll = textBlock.position();
                QString lineText = textBlock.text();
                QString strCommand = lineText.mid(lineText.length() - nCursorPossitionAll+m_nStdinCursorPossitionAll);
                strCommand.push_back('\n');
                ThisAppPtr->WriteToExeStdin(strCommand);
                m_nStdinCursorPossitionAll = 0;
            }break;
            default:
                break;
            }
        }  // if(a_isKeyRelease){

        return false;
    }
    else{

        switch(aKey){
        case Qt::Key_Enter:case Qt::Key_Return:
        case Qt::Key_Up: case Qt::Key_Down:
            return true;
        case Qt::Key_Backspace:{
            QTextCursor aCursor = textCursor();
            int nCursorPossitionAll = aCursor.position();
            int nTextBlockFirstPossition = aCursor.block().position();
            int nCursorPossition = nCursorPossitionAll - nTextBlockFirstPossition;

            switch(nCursorPossition){
            case 1: case 2:{
                QString lineText = aCursor.block().text();
                if(lineText.at(nCursorPossition-1)==QChar('>')){
                    return true;
                }
            }break;
            default:
                break;
            }  // switch(nCursorPossition){
        }break;
        case Qt::Key_Delete:{
            QTextCursor aCursor = textCursor();
            int nCursorPossitionAll = aCursor.position();
            int nTextBlockFirstPossition = aCursor.block().position();
            int nCursorPossition = nCursorPossitionAll - nTextBlockFirstPossition;

            switch(nCursorPossition){
            case 0: case 1:{
                QString lineText = aCursor.block().text();
                if(lineText.at(nCursorPossition-1)==QChar('>')){
                    return true;
                }
            }break;
            default:
                break;
            }  // switch(nCursorPossition){
        }break;
        default:
            break;
        }  // switch(aKey){
    }

    return false;
}


void emulator::CommandPrompt::keyPressEvent(QKeyEvent* a_keyEvent)
{
    if(!ShouldIgnoreKeyEvent(true,a_keyEvent)){
        BASE_CLASS::keyPressEvent(a_keyEvent);
    }
}


void emulator::CommandPrompt::keyReleaseEvent(QKeyEvent* a_keyEvent)
{
    if(ShouldIgnoreKeyEvent(false,a_keyEvent)){
        Qt::Key aKey = static_cast<Qt::Key>(a_keyEvent->key());
        switch(aKey){
        case Qt::Key_Enter:case Qt::Key_Return:{
            QTextCursor aCursor = textCursor();
            QString lineText = aCursor.block().text();
            if(lineText.size()>2){ // todo: make this better, check the case of multiline command
                QString strCommand = lineText.mid(2);
                //append("");
                moveCursor(QTextCursor::End);
                ThisAppPtr->RunCommand( strCommand, false );
                m_commandsList.AddNewToTheFront(strCommand);
                m_lastItemSet=0;
            }
        }break;
        case Qt::Key_Up:{

            QTextCursor aCursor = textCursor();
            QString lineText = aCursor.block().text();
            // if(lineText.size()==2) // todo: make sme cases when this is not done
            {
                if(m_commandsList.hasItem()){
                    if(!m_lastItemSet){
                        m_lastItem = m_commandsList.first();
                        m_lastItemSet = 1;
                    }
                    else{
                        // todo: remove last line (https://stackoverflow.com/questions/15326569/removing-last-line-from-qtextedit)
                        aCursor.movePosition(QTextCursor::End);
                        aCursor.select(QTextCursor::LineUnderCursor);
                        aCursor.removeSelectedText();
                        insertPlainText(">>");
                        //append(">>");
                        if((++m_lastItem)==m_commandsList.lastPlus1()){
                            m_lastItem = m_commandsList.first();
                        }

                    }

                    insertPlainText((*m_lastItem).toString());
                    moveCursor(QTextCursor::End);

                } // if(m_commandsList.hasItem()){
            }

        }break;
        case Qt::Key_Down:{

            QTextCursor aCursor = textCursor();
            QString lineText = aCursor.block().text();
            // if(lineText.size()==2) // todo: make sme cases when this is not done
            {
                if(m_commandsList.hasItem()){
                    if(!m_lastItemSet){
                        m_lastItem = --m_commandsList.lastPlus1();
                        m_lastItemSet = 1;
                    }
                    else{

                        aCursor.movePosition(QTextCursor::End);
                        aCursor.select(QTextCursor::LineUnderCursor);
                        aCursor.removeSelectedText();
                        insertPlainText(">>");
                        // append(">>");

                        if((m_lastItem--)==m_commandsList.first()){
                            m_lastItem = m_commandsList.lastPlus1();
                            --m_lastItem;
                        }
                    }

                    insertPlainText((*m_lastItem).toString());
                    moveCursor(QTextCursor::End);

                }  // if(m_commandsList.hasItem()){
            }

        }break;
        default:
            break;
        } // switch(aKey){

    }  // if(!ShouldIgnoreKeyEvent(a_keyEvent)){
    else{
        BASE_CLASS::keyReleaseEvent(a_keyEvent);
    }
}



/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

void emulator::RingList::AddNewToTheFront( const RingListType& a_new )
{
    size_t unSize = static_cast<size_t>(m_container.size());

    if(unSize && (a_new == m_container.front())){
        return;
    }

    if(unSize>=MAX_COMMANDS_SIZE){
        m_container.pop_back();
    }

    m_container.push_front(a_new);
}


const BASE_LIST< RingListType >& emulator::RingList::list()const
{
    return m_container;
}


void emulator::RingList::SetList(const BASE_LIST< RingListType >& a_list)
{
    m_container = a_list;
}


bool emulator::RingList::hasItem()const
{
    return m_container.size() ? true : false;
}


BASE_LIST< RingListType >::const_iterator emulator::RingList::first()const
{
    return m_container.begin();
}


BASE_LIST< RingListType >::const_iterator emulator::RingList::lastPlus1()const
{
    return m_container.end();
}
