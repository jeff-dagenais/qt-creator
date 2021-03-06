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

#ifndef QMLJSSCOPEASTPATH_H
#define QMLJSSCOPEASTPATH_H

#include "qmljs_global.h"
#include "parser/qmljsastvisitor_p.h"
#include "qmljsdocument.h"
#include <utils/qtcoverride.h>

namespace QmlJS {

class QMLJS_EXPORT ScopeAstPath: protected AST::Visitor
{
public:
    ScopeAstPath(Document::Ptr doc);

    QList<AST::Node *> operator()(quint32 offset);

protected:
    void accept(AST::Node *node);

    using Visitor::visit;

    bool preVisit(AST::Node *node) QTC_OVERRIDE;
    bool visit(AST::UiPublicMember *node) QTC_OVERRIDE;
    bool visit(AST::UiScriptBinding *node) QTC_OVERRIDE;
    bool visit(AST::UiObjectDefinition *node) QTC_OVERRIDE;
    bool visit(AST::UiObjectBinding *node) QTC_OVERRIDE;
    bool visit(AST::FunctionDeclaration *node) QTC_OVERRIDE;
    bool visit(AST::FunctionExpression *node) QTC_OVERRIDE;

private:
    bool containsOffset(AST::SourceLocation start, AST::SourceLocation end);

    QList<AST::Node *> _result;
    Document::Ptr _doc;
    quint32 _offset;
};

} // namespace QmlJS

#endif // QMLJSSCOPEASTPATH_H
