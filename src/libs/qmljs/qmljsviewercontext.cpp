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

#include "qmljsviewercontext.h"

namespace QmlJS {
/*!
    \class QmlJS::ViewerContext
    \brief The ViewerContext class encapsulate selector and paths for a given viewer.

    Using a a different viewer context can emulate (the pure qml part) of a device.
    This allows checking how a given qml would be interpreted on another platform/viewer.

    Screen information will also most likely need to be added here.
*/
ViewerContext::ViewerContext()
    : language(Language::Qml), flags(AddAllPaths)
{ }

ViewerContext::ViewerContext(QStringList selectors, QStringList paths,
                                    QmlJS::Language::Enum language,
                                    QmlJS::ViewerContext::Flags flags)
    : selectors(selectors), paths(paths), language(language),
      flags(flags)
{ }


/*
 which languages might be imported in this context
 */
bool ViewerContext::languageIsCompatible(Language::Enum l) const
{
    if (l == Language::AnyLanguage && language != Language::NoLanguage)
        return true;
    switch (language) {
    case Language::JavaScript:
    case Language::Json:
    case Language::QmlProject:
    case Language::QmlQbs:
    case Language::QmlTypeInfo:
        return language == l;
    case Language::Qml:
        return l == Language::Qml || l == Language::QmlQtQuick1 || l == Language::QmlQtQuick2
                || l == Language::JavaScript;
    case Language::QmlQtQuick1:
        return l == Language::Qml || l == Language::QmlQtQuick1 || l == Language::JavaScript;
    case Language::QmlQtQuick2:
        return l == Language::Qml || l == Language::QmlQtQuick2 || l == Language::JavaScript;
    case Language::AnyLanguage:
        return true;
    case Language::NoLanguage:
        break;
    }
    return false;
}

void ViewerContext::maybeAddPath(const QString &path)
{
    if (!path.isEmpty() && !paths.contains(path))
        paths.append(path);
}

} // namespace QmlJS
