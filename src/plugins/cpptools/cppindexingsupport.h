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

#ifndef CPPTOOLS_CPPINDEXINGSUPPORT_H
#define CPPTOOLS_CPPINDEXINGSUPPORT_H

#include "cpptools_global.h"

#include "cppmodelmanagerinterface.h"

#include <coreplugin/find/searchresultwindow.h>
#include <coreplugin/find/textfindconstants.h>

#include <QFuture>
#include <QStringList>

namespace CppTools {

class CPPTOOLS_EXPORT SymbolSearcher: public QObject
{
    Q_OBJECT

public:
    enum SymbolType {
        Classes      = 0x1,
        Functions    = 0x2,
        Enums        = 0x4,
        Declarations = 0x8
    };

    Q_DECLARE_FLAGS(SymbolTypes, SymbolType)

    enum SearchScope {
        SearchProjectsOnly,
        SearchGlobal
    };

    struct Parameters
    {
        QString text;
        Core::FindFlags flags;
        SymbolTypes types;
        SearchScope scope;
    };


public:
    SymbolSearcher(QObject *parent = 0);
    virtual ~SymbolSearcher() = 0;
    virtual void runSearch(QFutureInterface<Core::SearchResultItem> &future) = 0;
};


class CPPTOOLS_EXPORT CppIndexingSupport
{
public:
    virtual ~CppIndexingSupport() = 0;

    virtual QFuture<void> refreshSourceFiles(const QStringList &sourceFiles,
        CppModelManagerInterface::ProgressNotificationMode mode) = 0;
    virtual SymbolSearcher *createSymbolSearcher(SymbolSearcher::Parameters parameters,
                                                 QSet<QString> fileNames) = 0;
};

} // namespace CppTools

Q_DECLARE_METATYPE(CppTools::SymbolSearcher::SearchScope)
Q_DECLARE_METATYPE(CppTools::SymbolSearcher::Parameters)
Q_DECLARE_METATYPE(CppTools::SymbolSearcher::SymbolTypes)

#endif // CPPTOOLS_CPPINDEXINGSUPPORT_H
