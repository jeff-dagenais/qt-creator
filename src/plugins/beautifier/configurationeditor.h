/**************************************************************************
**
** Copyright (c) 2014 Lorenz Haas
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

#ifndef BEAUTIFIER_CONFIGURATIONEDITOR_H
#define BEAUTIFIER_CONFIGURATIONEDITOR_H

#include <utils/qtcoverride.h>

#include <QPlainTextEdit>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QCompleter;
class QRegExp;
class QStringListModel;
class QTextDocument;
QT_END_NAMESPACE

namespace Beautifier {
namespace Internal {

class AbstractSettings;

class ConfigurationSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit ConfigurationSyntaxHighlighter(QTextDocument *parent);
    void setKeywords(const QStringList &keywords);
    void setCommentExpression(const QRegExp &rx);

protected:
    void highlightBlock(const QString &text) QTC_OVERRIDE;

private:
    QRegExp m_expressionKeyword;
    QRegExp m_expressionComment;
    QTextCharFormat m_formatKeyword;
    QTextCharFormat m_formatComment;
};

class ConfigurationEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ConfigurationEditor(QWidget *parent = 0);
    void setSettings(AbstractSettings *settings);
    void setCommentExpression(const QRegExp &rx);

protected:
    bool eventFilter(QObject *object, QEvent *event) QTC_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) QTC_OVERRIDE;

private slots:
    void insertCompleterText(const QString &text);
    void updateDocumentation();

signals:
    void documentationChanged(const QString &word, const QString &documentation);

private:
    QTextCursor cursorForTextUnderCursor(QTextCursor tc = QTextCursor()) const;

    AbstractSettings *m_settings;
    QCompleter *m_completer;
    QStringListModel *m_model;
    ConfigurationSyntaxHighlighter *m_highlighter;
    QString m_lastDocumentation;
};

} // namespace Internal
} // namespace Beautifier

#endif // BEAUTIFIER_CONFIGURATIONEDITOR_H
