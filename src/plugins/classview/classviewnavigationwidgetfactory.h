/**************************************************************************
**
** Copyright (c) 2014 Denis Mingulov
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

#ifndef CLASSVIEWNAVIGATIONWIDGETFACTORY_H
#define CLASSVIEWNAVIGATIONWIDGETFACTORY_H

#include <coreplugin/inavigationwidgetfactory.h>

namespace ClassView {
namespace Internal {

class NavigationWidgetFactory : public Core::INavigationWidgetFactory
{
    Q_OBJECT

public:
    //! Constructor
    NavigationWidgetFactory();

    //! Destructor
    ~NavigationWidgetFactory();

    //! Access to static instance
    static NavigationWidgetFactory *instance();

    // Core::INavigationWidgetFactory
    //! \implements Core::INavigationWidgetFactory::displayName
    QString displayName() const;

    //! \implements Core::INavigationWidgetFactory::priority
    int priority() const;

    //! \implements Core::INavigationWidgetFactory::id
    Core::Id id() const;

    //! \implements Core::INavigationWidgetFactory::activationSequence
    QKeySequence activationSequence() const;

    //! \implements Core::INavigationWidgetFactory::createWidget
    Core::NavigationView createWidget();

    //! \implements Core::INavigationWidgetFactory::saveSettings
    void saveSettings(int position, QWidget *widget);

    //! \implements Core::INavigationWidgetFactory::restoreSettings
    void restoreSettings(int position, QWidget *widget);

signals:
    void widgetIsCreated();
};

} // namespace Internal
} // namespace ClassView

#endif // CLASSVIEWNAVIGATIONWIDGETFACTORY_H
