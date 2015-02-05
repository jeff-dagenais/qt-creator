/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://www.qt.io/licensing.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "SimpleLexer.h"

#include <cplusplus/ObjectiveCTypeQualifiers.h>
#include <cplusplus/Lexer.h>
#include <cplusplus/Token.h>

#include <QDebug>

using namespace CPlusPlus;

SimpleLexer::SimpleLexer()
    : _lastState(0),
      _skipComments(false),
      _endedJoined(false)
{}

SimpleLexer::~SimpleLexer()
{ }

bool SimpleLexer::skipComments() const
{
    return _skipComments;
}

void SimpleLexer::setSkipComments(bool skipComments)
{
    _skipComments = skipComments;
}

bool SimpleLexer::endedJoined() const
{
    return _endedJoined;
}

QList<Token> SimpleLexer::operator()(const QString &text, int state)
{
    QList<Token> tokens;

    const QByteArray bytes = text.toUtf8();
    const char *firstChar = bytes.constData();
    const char *lastChar = firstChar + bytes.size();

    Lexer lex(firstChar, lastChar);
    lex.setLanguageFeatures(_languageFeatures);
    lex.setStartWithNewline(true);

    if (! _skipComments)
        lex.setScanCommentTokens(true);

    if (state != -1)
        lex.setState(state & 0xff);

    bool inPreproc = false;

    for (;;) {
        Token tk;
        lex(&tk);
        if (tk.is(T_EOF_SYMBOL)) {
            _endedJoined = tk.joined();
            break;
        }

        QStringRef spell = text.midRef(tk.bytesBegin(), tk.bytes());
        lex.setScanAngleStringLiteralTokens(false);

        if (tk.newline() && tk.is(T_POUND))
            inPreproc = true;
        else if (inPreproc && tokens.size() == 1 && tk.is(T_IDENTIFIER) &&
                 spell == QLatin1String("include"))
            lex.setScanAngleStringLiteralTokens(true);
        else if (inPreproc && tokens.size() == 1 && tk.is(T_IDENTIFIER) &&
                 spell == QLatin1String("include_next"))
            lex.setScanAngleStringLiteralTokens(true);
        else if (_languageFeatures.objCEnabled
                 && inPreproc && tokens.size() == 1 && tk.is(T_IDENTIFIER) &&
                 spell == QLatin1String("import"))
            lex.setScanAngleStringLiteralTokens(true);

        tokens.append(tk);
    }

    _lastState = lex.state();
    return tokens;
}

int SimpleLexer::tokenAt(const QList<Token> &tokens, unsigned utf16charsOffset)
{
    for (int index = tokens.size() - 1; index >= 0; --index) {
        const Token &tk = tokens.at(index);
        if (tk.utf16charsBegin() <= utf16charsOffset && tk.utf16charsEnd() >= utf16charsOffset)
            return index;
    }

    return -1;
}

Token SimpleLexer::tokenAt(const QString &text,
                           unsigned utf16charsOffset,
                           int state,
                           bool qtMocRunEnabled)
{
    // FIXME: Check default values.
    LanguageFeatures features;
    features.qtMocRunEnabled = !cOnly && qtMocRunEnabled;
    features.qtEnabled = !cOnly && qtMocRunEnabled;
    features.qtKeywordsEnabled = !cOnly && qtMocRunEnabled;
    features.objCEnabled = !cOnly && qtMocRunEnabled;
    features.cxx11Enabled = !cOnly && qtMocRunEnabled;
    SimpleLexer tokenize;
    tokenize.setLanguageFeatures(features);
    const QList<Token> tokens = tokenize(text, state);
    const int tokenIdx = tokenAt(tokens, utf16charsOffset);
    return (tokenIdx == -1) ? Token() : tokens.at(tokenIdx);
}

int SimpleLexer::tokenBefore(const QList<Token> &tokens, unsigned utf16charsOffset)
{
    for (int index = tokens.size() - 1; index >= 0; --index) {
        const Token &tk = tokens.at(index);
        if (tk.utf16charsBegin() <= utf16charsOffset)
            return index;
    }

    return -1;
}
