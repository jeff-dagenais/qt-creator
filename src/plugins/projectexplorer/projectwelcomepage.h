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

#ifndef PROJECTWELCOMEPAGE_H
#define PROJECTWELCOMEPAGE_H

#include <QAbstractListModel>

#include <utils/iwelcomepage.h>

QT_BEGIN_NAMESPACE
class QQmlEngine;
QT_END_NAMESPACE

namespace ProjectExplorer {

class ProjectExplorerPlugin;

namespace Internal {

class SessionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum { DefaultSessionRole = Qt::UserRole+1, LastSessionRole, ActiveSessionRole, ProjectsPathRole, ProjectsDisplayRole };

    explicit SessionModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    Q_SCRIPTABLE bool isDefaultVirgin() const;

public slots:
    void resetSessions();
    void cloneSession(const QString &session);
    void deleteSession(const QString &session);
    void renameSession(const QString &session);
};


class ProjectModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum { FilePathRole = Qt::UserRole+1, PrettyFilePathRole };

    ProjectModel(ProjectExplorerPlugin* plugin, QObject* parent = 0);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

public slots:
    void resetProjects();

private:
    ProjectExplorerPlugin *m_plugin;
};

class ProjectWelcomePage : public Utils::IWelcomePage
{
    Q_OBJECT
public:
    ProjectWelcomePage();

    void facilitateQml(QQmlEngine *engine);
    QUrl pageLocation() const;
    QWidget *page() { return 0; }
    QString title() const { return tr("Projects"); }
    int priority() const { return 20; }
    Id id() const;

    void reloadWelcomeScreenData();

public slots:
    void newProject();
    void openProject();

signals:
    void requestProject(const QString &project);
    void requestSession(const QString &session);
    void manageSessions();

private:
    SessionModel *m_sessionModel;
    ProjectModel *m_projectModel;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // PROJECTWELCOMEPAGE_H
