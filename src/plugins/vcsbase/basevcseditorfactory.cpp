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

#include "basevcseditorfactory.h"
#include "vcsbaseeditor.h"

#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>

#include <diffeditor/diffeditorconstants.h>

#include <QCoreApplication>
#include <QStringList>

/*!
    \class VcsBase::BaseVCSEditorFactory

    \brief The BaseVCSEditorFactory class is the base class for editor
    factories creating instances of VcsBaseEditor subclasses.

    \sa VcsBase::VcsBaseEditorWidget
*/

namespace VcsBase {
namespace Internal {

class BaseVcsEditorFactoryPrivate
{
public:
    const VcsBaseEditorParameters *m_type;
    QObject *m_describeReceiver;
    const char *m_describeSlot;
};

} // namespace Internal

BaseVcsEditorFactory::BaseVcsEditorFactory(const VcsBaseEditorParameters *t,
                                           QObject *describeReceiver, const char *describeSlot)
  : d(new Internal::BaseVcsEditorFactoryPrivate)
{
    d->m_type = t;
    d->m_describeReceiver = describeReceiver;
    d->m_describeSlot = describeSlot;
    setId(t->id);
    setDisplayName(QCoreApplication::translate("VCS", t->displayName));
    if (QLatin1String(t->mimeType) != QLatin1String(DiffEditor::Constants::DIFF_EDITOR_MIMETYPE))
        addMimeType(t->mimeType);
    new TextEditor::TextEditorActionHandler(this, t->context);
}

BaseVcsEditorFactory::~BaseVcsEditorFactory()
{
    delete d;
}

Core::IEditor *BaseVcsEditorFactory::createEditor()
{
    VcsBaseEditorWidget *vcsEditor = createVcsBaseEditor(d->m_type);

    vcsEditor->init();
    if (d->m_describeReceiver)
        connect(vcsEditor, SIGNAL(describeRequested(QString,QString)), d->m_describeReceiver, d->m_describeSlot);

    if (!mimeTypes().isEmpty())
        vcsEditor->baseTextDocument()->setMimeType(mimeTypes().front());

    TextEditor::TextEditorSettings::initializeEditor(vcsEditor);
    return vcsEditor->editor();
}

} // namespace VcsBase
