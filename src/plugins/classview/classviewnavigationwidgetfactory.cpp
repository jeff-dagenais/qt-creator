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

#include "classviewnavigationwidgetfactory.h"
#include "classviewnavigationwidget.h"
#include "classviewconstants.h"

#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/qtcassert.h>

#include <QKeySequence>
#include <QSettings>

namespace ClassView {
namespace Internal {

static NavigationWidgetFactory *factoryInstance;

///////////////////////////////// NavigationWidgetFactory //////////////////////////////////

/*!
    \class NavigationWidgetFactory
    \brief The NavigationWidgetFactory class implements a singleton instance of
    the INavigationWidgetFactory for the Class View.

    Supports the \c setState public slot for adding the widget factory to or
    removing it from \c ExtensionSystem::PluginManager.

    Also supports the \c widgetIsCreated and \c stateChanged signals.
*/

/*!
    \fn void NavigationWidgetFactory::widgetIsCreated()

    Informs that the widget factory created a widget.
*/

NavigationWidgetFactory::NavigationWidgetFactory()
{
    factoryInstance = this;
}

NavigationWidgetFactory::~NavigationWidgetFactory()
{
    factoryInstance = 0;
}

NavigationWidgetFactory *NavigationWidgetFactory::instance()
{
    return factoryInstance;
}

QString NavigationWidgetFactory::displayName() const
{
    return tr("Class View");
}

int NavigationWidgetFactory::priority() const
{
    return 500;
}

Core::Id NavigationWidgetFactory::id() const
{
    return "Class View";
}

QKeySequence NavigationWidgetFactory::activationSequence() const
{
    return QKeySequence();
}

Core::NavigationView NavigationWidgetFactory::createWidget()
{
    Core::NavigationView navigationView;
    NavigationWidget *widget = new NavigationWidget();
    navigationView.widget = widget;
    navigationView.dockToolBarWidgets = widget->createToolButtons();
    emit widgetIsCreated();
    return navigationView;
}


/*!
   Returns a settings prefix for \a position.
*/
static QString settingsPrefix(int position)
{
    return QString::fromLatin1("ClassView/Treewidget.%1/FlatMode").arg(position);
}

//! Flat mode settings

void NavigationWidgetFactory::saveSettings(int position, QWidget *widget)
{
    NavigationWidget *pw = qobject_cast<NavigationWidget *>(widget);
    QTC_ASSERT(pw, return);

    // .beginGroup is not used - to prevent simultaneous access
    QString group = settingsPrefix(position);

    // Save settings
    Core::ICore::settings()->setValue(group, pw->flatMode());
}

void NavigationWidgetFactory::restoreSettings(int position, QWidget *widget)
{
    NavigationWidget *pw = qobject_cast<NavigationWidget *>(widget);
    QTC_ASSERT(pw, return);

    // .beginGroup is not used - to prevent simultaneous access
    QString group = settingsPrefix(position);

    // Load settings
    pw->setFlatMode(Core::ICore::settings()->value(group, false).toBool());
}

} // namespace Internal
} // namespace ClassView
