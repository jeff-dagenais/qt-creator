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

#ifndef DIFFHIGHLIGHTER_H
#define DIFFHIGHLIGHTER_H

#include "vcsbase_global.h"

#include <texteditor/syntaxhighlighter.h>

QT_BEGIN_NAMESPACE
class QRegExp;
class QTextCharFormat;
QT_END_NAMESPACE

namespace Core { class ICore; }
namespace TextEditor { class FontSettingsPage; }

namespace VcsBase {

class DiffHighlighterPrivate;

class VCSBASE_EXPORT DiffHighlighter : public TextEditor::SyntaxHighlighter
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DiffHighlighter)
public:
    explicit DiffHighlighter(const QRegExp &filePattern);
    ~DiffHighlighter();

    void highlightBlock(const QString &text);

    virtual void setFontSettings(const TextEditor::FontSettings &fontSettings);

private:
    QScopedPointer<DiffHighlighterPrivate> d_ptr;
};

} // namespace VcsBase

#endif // DIFFHIGHLIGHTER_H
