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

#ifndef ABSTRACTMOBILEAPP_H
#define ABSTRACTMOBILEAPP_H

#include "../qmakeprojectmanager_global.h"
#include <QFileInfo>
#include <QPair>

#ifndef CREATORLESSTEST
#include <coreplugin/basefilewizardfactory.h>
#endif // CREATORLESSTEST

QT_FORWARD_DECLARE_CLASS(QTextStream)

namespace QmakeProjectManager {

/// \internal
struct
#ifndef CREATORLESSTEST
    QMAKEPROJECTMANAGER_EXPORT
#endif // CREATORLESSTEST
    AbstractGeneratedFileInfo
{
    enum FileType {
        MainCppFile,
        AppProFile,
        DeploymentPriFile,
        ExtendedFile
    };

    AbstractGeneratedFileInfo();

    int fileType;
    QFileInfo fileInfo;
    int currentVersion; // Current version of the template file in Creator
    int version; // The version in the file header
    quint16 dataChecksum; // The calculated checksum
    quint16 statedChecksum; // The checksum in the file header
};

typedef QPair<QString, QString> DeploymentFolder; // QPair<.source, .target>

/// \internal
class
#ifndef CREATORLESSTEST
    QMAKEPROJECTMANAGER_EXPORT
#endif // CREATORLESSTEST
    AbstractMobileApp : public QObject
{
    Q_OBJECT

public:
    enum FileType {
        MainCpp,
        MainCppOrigin,
        AppPro,
        AppProOrigin,
        AppProPath,
        DesktopOrigin,
        DeploymentPri,
        DeploymentPriOrigin,
        ExtendedFile
    };

    virtual ~AbstractMobileApp();

    void setProjectName(const QString &name);
    QString projectName() const;
    void setProjectPath(const QString &path);
    QString path(int fileType) const;
    QString error() const;

#ifndef CREATORLESSTEST
    virtual Core::GeneratedFiles generateFiles(QString *errorMessage) const;
#else
    bool generateFiles(QString *errorMessage) const;
#endif // CREATORLESSTEST

    static int makeStubVersion(int minor);
    QList<AbstractGeneratedFileInfo> fileUpdates(const QString &mainProFile) const;
    bool updateFiles(const QList<AbstractGeneratedFileInfo> &list, QString &error) const;

    static const QString DeploymentPriFileName;
protected:
    AbstractMobileApp();
    virtual QByteArray generateProFile(QString *errorMessage) const;

    static QString templatesRoot();
    static void insertParameter(QString &line, const QString &parameter);

    QByteArray readBlob(const QString &filePath, QString *errorMsg) const;
    bool readTemplate(int fileType, QByteArray *data, QString *errorMessage) const;
    QByteArray generateFile(int fileType, QString *errorMessage) const;
    QString outputPathBase() const;

#ifndef CREATORLESSTEST
    static Core::GeneratedFile file(const QByteArray &data,
        const QString &targetFile);
#endif // CREATORLESSTEST

    static const QString CFileComment;
    static const QString ProFileComment;
    static const QString FileChecksum;
    static const QString FileStubVersion;
    static const int StubVersion;

    QString m_error;

private:
    QByteArray generateDesktopFile(QString *errorMessage, int fileType) const;
    QByteArray generateMainCpp(QString *errorMessage) const;

    virtual QByteArray generateFileExtended(int fileType,
        bool *versionAndCheckSum, QString *comment, QString *errorMessage) const = 0;
    virtual QString pathExtended(int fileType) const = 0;
    virtual QString originsRoot() const = 0;
    virtual QString mainWindowClassName() const = 0;
    virtual int stubVersionMinor() const = 0;
    virtual bool adaptCurrentMainCppTemplateLine(QString &line) const = 0;
    virtual void handleCurrentProFileTemplateLine(const QString &line,
        QTextStream &proFileTemplate, QTextStream &proFile,
        bool &commentOutNextLine) const = 0;
    virtual QList<AbstractGeneratedFileInfo> updateableFiles(const QString &mainProFile) const = 0;
    virtual QList<DeploymentFolder> deploymentFolders() const = 0;

    QString m_projectName;
    QFileInfo m_projectPath;
    QString m_pngIcon64;
    QString m_pngIcon80;
};

} // namespace QmakeProjectManager

#endif // ABSTRACTMOBILEAPP_H
