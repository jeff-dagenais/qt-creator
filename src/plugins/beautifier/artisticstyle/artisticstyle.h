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

#ifndef BEAUTIFIER_ARTISTICSTYLE_H
#define BEAUTIFIER_ARTISTICSTYLE_H

#include "../beautifierabstracttool.h"

#include <utils/qtcoverride.h>

QT_FORWARD_DECLARE_CLASS(QAction)

namespace Beautifier {
namespace Internal {

class BeautifierPlugin;

namespace ArtisticStyle {

class ArtisticStyleSettings;

class ArtisticStyle : public BeautifierAbstractTool
{
    Q_OBJECT

public:
    explicit ArtisticStyle(BeautifierPlugin *parent = 0);
    virtual ~ArtisticStyle();
    bool initialize() QTC_OVERRIDE;
    void updateActions(Core::IEditor *editor) QTC_OVERRIDE;
    QList<QObject *> autoReleaseObjects() QTC_OVERRIDE;

private slots:
    void formatFile();

private:
    BeautifierPlugin *m_beautifierPlugin;
    QAction *m_formatFile;
    ArtisticStyleSettings *m_settings;
};

} // namespace ArtisticStyle
} // namespace Internal
} // namespace Beautifier

#endif // BEAUTIFIER_ARTISTICSTYLE_H
