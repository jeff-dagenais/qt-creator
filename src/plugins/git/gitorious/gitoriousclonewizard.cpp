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

#include "gitoriousclonewizard.h"
#include "gitorioushostwizardpage.h"
#include "gitoriousprojectwizardpage.h"
#include "gitoriousrepositorywizardpage.h"

#include "../clonewizardpage.h"
#include "../gitplugin.h"

#include <coreplugin/iversioncontrol.h>
#include <vcsbase/vcsbaseconstants.h>
#include <vcsbase/vcsconfigurationpage.h>
#include <utils/qtcassert.h>

#include <QUrl>

namespace Gitorious {
namespace Internal {

//  GitoriousCloneWizardPage: A git clone page taking its URL from the
//  projects page.

class GitoriousCloneWizardPage : public Git::CloneWizardPage {
public:
    explicit GitoriousCloneWizardPage(const GitoriousRepositoryWizardPage *rp, QWidget *parent = 0);
    void initializePage();

private:
    const GitoriousRepositoryWizardPage *m_repositoryPage;
};

GitoriousCloneWizardPage::GitoriousCloneWizardPage(const GitoriousRepositoryWizardPage *rp, QWidget *parent) :
    Git::CloneWizardPage(parent),
    m_repositoryPage(rp)
{
}

void GitoriousCloneWizardPage::initializePage()
{
    setRepository(m_repositoryPage->repositoryURL().toString());
}

// -------- GitoriousCloneWizard
GitoriousCloneWizardFactory::GitoriousCloneWizardFactory()
{
    setId(QLatin1String(VcsBase::Constants::VCS_ID_GIT));
    setIcon(QIcon(QLatin1String(":/git/images/gitorious.png")));
    setDescription(tr("Clones a Gitorious repository and tries to load the contained project."));
    setDisplayName(tr("Gitorious Repository Clone"));
}

VcsBase::BaseCheckoutWizard *GitoriousCloneWizardFactory::create(const Utils::FileName &path, QWidget *parent) const
{
    return new GitoriousCloneWizard(path, parent);
}

// --------------------------------------------------------------------
// GitoriousCloneWizard:
// --------------------------------------------------------------------

GitoriousCloneWizard::GitoriousCloneWizard(const Utils::FileName &path, QWidget *parent) :
    VcsBase::BaseCheckoutWizard(path, parent)
{
    setTitle(tr("Cloning"));
    setStartedStatus(tr("Cloning started..."));

    const Core::IVersionControl *vc = Git::Internal::GitPlugin::instance()->versionControl();
    if (!vc->isConfigured())
        addPage(new VcsBase::VcsConfigurationPage(vc));

    GitoriousHostWizardPage *hostPage = new GitoriousHostWizardPage;
    GitoriousProjectWizardPage *projectPage = new GitoriousProjectWizardPage(hostPage);
    GitoriousRepositoryWizardPage *repoPage = new GitoriousRepositoryWizardPage(projectPage);
    GitoriousCloneWizardPage *clonePage = new GitoriousCloneWizardPage(repoPage);
    clonePage->setPath(path.toString());

    addPage(hostPage);
    addPage(projectPage);
    addPage(repoPage);
    addPage(clonePage);
}

VcsBase::Command *GitoriousCloneWizard::createCommand(Utils::FileName *checkoutDir)
{
    const Git::CloneWizardPage *cwp = 0;
    foreach (int pageId, pageIds()) {
        if ((cwp = qobject_cast<const Git::CloneWizardPage *>(page(pageId))))
            break;
    }
    QTC_ASSERT(cwp, return 0);
    return cwp->createCheckoutJob(checkoutDir);
}

} // namespace Internal
} // namespace Gitorius
