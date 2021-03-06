/**************************************************************************
**
** Copyright (c) 2014 BogDan Vatra <bog_dan_ro@yahoo.com>
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

#ifndef ANDROIDDEPLOYSTEPWIDGET_H
#define ANDROIDDEPLOYSTEPWIDGET_H

#include <projectexplorer/buildstep.h>

QT_BEGIN_NAMESPACE
namespace Ui { class AndroidDeployStepWidget; }
QT_END_NAMESPACE

namespace Android {
namespace Internal {
class AndroidDeployStep;

class AndroidDeployStepWidget : public ProjectExplorer::BuildStepConfigWidget
{
    Q_OBJECT

public:
    AndroidDeployStepWidget(AndroidDeployStep *step);
    ~AndroidDeployStepWidget();

private slots:
    void setMinistro();
    void setDeployLocalQtLibs();
    void setBundleQtLibs();

    void setQASIPackagePath();
    void cleanLibsOnDevice();
    void resetDefaultDevices();

    void deployOptionsChanged();
private:
    virtual QString summaryText() const;
    virtual QString displayName() const;

    Ui::AndroidDeployStepWidget *ui;
    AndroidDeployStep *m_step;
};

} // namespace Internal
} // namespace Android

#endif // ANDROIDDEPLOYSTEPWIDGET_H
