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


bool emulator::CommandPrompt::ShouldIgnoreKeyEvent(QKeyEvent* a_keyEvent)const
{
    Qt::Key aKey = static_cast<Qt::Key>(a_keyEvent->key());

    switch(aKey){
    case Qt::Key_Enter:case Qt::Key_Return:
        return true;
    case Qt::Key_Backspace:{
        QTextCursor aCursor = textCursor();
        int nCursorPossition = aCursor.position();

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
        int nCursorPossition = aCursor.position();

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

    return false;
}


void emulator::CommandPrompt::keyPressEvent(QKeyEvent* a_keyEvent)
{
    if(!ShouldIgnoreKeyEvent(a_keyEvent)){
        BASE_CLASS::keyPressEvent(a_keyEvent);
    }
}


void emulator::CommandPrompt::keyReleaseEvent(QKeyEvent* a_keyEvent)
{
    if(ShouldIgnoreKeyEvent(a_keyEvent)){
        Qt::Key aKey = static_cast<Qt::Key>(a_keyEvent->key());
        switch(aKey){
        case Qt::Key_Enter:case Qt::Key_Return:{
            QTextCursor aCursor = textCursor();
            QString lineText = aCursor.block().text();
            if(lineText.size()>2){ // todo: make this better, check the case of multiline command
                QString strCommand = lineText.mid(2);
                ThisAppPtr->RunCommand( ::std::move(strCommand) );
            }
            append(">>");
        }break;
        default:
            break;
        } // switch(aKey){

    }  // if(!ShouldIgnoreKeyEvent(a_keyEvent)){
    else{
        BASE_CLASS::keyReleaseEvent(a_keyEvent);
    }
}
