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

#include "formwizarddialog.h"
#include "formtemplatewizardpage.h"

#include <coreplugin/basefilewizardfactory.h>

#include <utils/filewizardpage.h>

#include <QDebug>
#include <QAbstractButton>

// Make sure there is a gap before the extension pages
enum { FormPageId, FilePageId, FirstExtensionPageId = 10 };

namespace Designer {
namespace Internal {

// ----------------- FormWizardDialog
FormWizardDialog::FormWizardDialog(const WizardPageList &extensionPages,
                                   QWidget *parent) :
    Core::BaseFileWizard(parent),
    m_formPage(new FormTemplateWizardPage)
{
    init(extensionPages);
}

void FormWizardDialog::init(const WizardPageList &extensionPages)
{
    setWindowTitle(tr("Qt Designer Form"));
    setPage(FormPageId, m_formPage);

    int id = FirstExtensionPageId;
    foreach (QWizardPage *p, extensionPages) {
       setPage(id, p);
       ++id;
    }
}

QString FormWizardDialog::templateContents() const
{
    // Template is expensive, cache
    if (m_templateContents.isEmpty())
        m_templateContents = m_formPage->templateContents();
    return m_templateContents;
}

// ----------------- FormFileWizardDialog
FormFileWizardDialog::FormFileWizardDialog(const WizardPageList &extensionPages,
                                           QWidget *parent)
  : FormWizardDialog(extensionPages, parent),
    m_filePage(new Utils::FileWizardPage)
{
    setPage(FilePageId, m_filePage);
    connect(m_filePage, SIGNAL(activated()),
            button(QWizard::FinishButton), SLOT(animateClick()));

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(slotCurrentIdChanged(int)));
}

QString FormFileWizardDialog::path() const
{
    return m_filePage->path();
}

void FormFileWizardDialog::setPath(const QString &path)
{
    m_filePage->setPath(path);
}

QString FormFileWizardDialog::fileName() const
{
    return m_filePage->fileName();
}

void FormFileWizardDialog::slotCurrentIdChanged(int id)
{
    if (id == FilePageId) {
        // Change from form to file: Store template and Suggest a name based on
        // the ui class
        QString formBaseClass;
        QString uiClassName;
        if (FormTemplateWizardPage::getUIXmlData(templateContents(), &formBaseClass, &uiClassName)) {
            QString fileName = FormTemplateWizardPage::stripNamespaces(uiClassName).toLower();
            fileName += QLatin1String(".ui");
            m_filePage->setFileName(fileName);
        }
    }
}

} // namespace Internal
} // namespace Designer
