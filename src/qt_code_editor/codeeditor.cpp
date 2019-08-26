/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

///#include <QtWidgets>
#include <QKeyEvent>
#include <QDebug>
#include <QPainter>
#include <QTextBlock>

#include "codeeditor.hpp"

//![constructor]

CodeEditor::CodeEditor(CodeEditor *a_prev, const QString& a_filePath)
    :
      m_prev(a_prev), m_next(NEWNULLPTR),
      m_scriptFile(a_filePath),
      m_scriptPath(a_filePath),
      m_lineNumberArea(this)
{
    m_isControlPressed = m_isUnsavedChanges = 0;
    if(a_prev){
        a_prev->m_next = this;
    }

    setWindowTitle(a_filePath);

    if(m_scriptFile.open(QIODevice::ReadWrite)){
        QByteArray aFileContent = m_scriptFile.readAll();
        setPlainText(aFileContent);
    }

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this,&QPlainTextEdit::textChanged,this,[this](){
        QString windowTitle = m_scriptPath + "*";
        setWindowTitle(windowTitle);
        m_isUnsavedChanges = 1;
    });

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    setAttribute(Qt::WA_DeleteOnClose);

}

CodeEditor::~CodeEditor()
{
    qDebug()<<__FUNCTION__;
    if(m_prev){
        m_prev->m_next = m_next;
    }

    if(m_next){
        m_next->m_prev = m_prev;
    }

    m_scriptFile.close();
}

//![constructor]

//![extraAreaWidth]

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

//![extraAreaWidth]

//![slotUpdateExtraAreaWidth]

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea.scroll(0, dy);
    else
        m_lineNumberArea.update(0, rect.y(), m_lineNumberArea.width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

//![slotUpdateRequest]

//![resizeEvent]

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea.setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

//![resizeEvent]
//!


void CodeEditor::SaveCurrentContentGui()
{
    if(m_isUnsavedChanges){
        m_scriptFile.resize(0);
        QTextStream aTextStr(&m_scriptFile);
        aTextStr << toPlainText();
        m_scriptFile.flush();
        m_isUnsavedChanges = 0;
        setWindowTitle(m_scriptPath);
    }
}


void CodeEditor::keyPressEvent(QKeyEvent* a_event)
{
    if(a_event->key() == Qt::Key_Control){
        m_isControlPressed = 1;
    }
    QPlainTextEdit::keyPressEvent(a_event);
}

void CodeEditor::keyReleaseEvent(QKeyEvent* a_event)
{
    switch(a_event->key()){
    case Qt::Key_Control:
        m_isControlPressed = 0;
        break;
    case Qt::Key_S:
        if(m_isControlPressed){
            SaveCurrentContentGui();
        }
        break;
    default:
        break;
    }
    QPlainTextEdit::keyReleaseEvent(a_event);
}

//![cursorPositionChanged]

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

//![cursorPositionChanged]

//![extraAreaPaintEvent_0]

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(&m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

//![extraAreaPaintEvent_0]

//![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = STATIC_CAST(int, blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + STATIC_CAST(int,blockBoundingRect(block).height());
//![extraAreaPaintEvent_1]

//![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea.width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + STATIC_CAST(int,blockBoundingRect(block).height());
        ++blockNumber;
    }
}
//![extraAreaPaintEvent_2]
//!
//!

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////*/

LineNumberArea::LineNumberArea(CodeEditor *a_editor)
    :
      QWidget(a_editor)
{
    m_codeEditor = a_editor;
}


QSize LineNumberArea::sizeHint() const
{
    return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_codeEditor->lineNumberAreaPaintEvent(event);
}

