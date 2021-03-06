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

#include "glslfilewizard.h"
#include "glsleditorconstants.h"

#include <coreplugin/basefilewizard.h>

#include <utils/filewizardpage.h>
#include <utils/qtcassert.h>

#include <QFileInfo>
#include <QTextStream>
#include <QWizard>
#include <QPushButton>

using namespace GLSLEditor;

GLSLFileWizard::GLSLFileWizard(ShaderType shaderType)
    : m_shaderType(shaderType)
{
    setFlags(Core::IWizardFactory::PlatformIndependent);
}

Core::GeneratedFiles GLSLFileWizard::generateFiles(const QWizard *w,
                                                 QString * /*errorMessage*/) const
{
    const Core::BaseFileWizard *wizard = qobject_cast<const Core::BaseFileWizard *>(w);
    Utils::FileWizardPage *page = wizard->find<Utils::FileWizardPage>();
    QTC_ASSERT(page, return Core::GeneratedFiles());

    const QString path = page->path();
    const QString name = page->fileName();

    const QString fileName = Core::BaseFileWizardFactory::buildFileName(path, name, preferredSuffix(m_shaderType));

    Core::GeneratedFile file(fileName);
    file.setContents(fileContents(fileName, m_shaderType));
    file.setAttributes(Core::GeneratedFile::OpenEditorAttribute);
    return Core::GeneratedFiles() << file;
}

QString GLSLFileWizard::fileContents(const QString &, ShaderType shaderType) const
{
    QString contents;
    QTextStream str(&contents);

    switch (shaderType) {
    case GLSLFileWizard::VertexShaderES:
        str << QLatin1String("attribute highp vec4 qt_Vertex;\n")
            << QLatin1String("attribute highp vec4 qt_MultiTexCoord0;\n")
            << QLatin1String("uniform highp mat4 qt_ModelViewProjectionMatrix;\n")
            << QLatin1String("varying highp vec4 qt_TexCoord0;\n")
            << QLatin1String("\n")
            << QLatin1String("void main(void)\n")
            << QLatin1String("{\n")
            << QLatin1String("    gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;\n")
            << QLatin1String("    qt_TexCoord0 = qt_MultiTexCoord0;\n")
            << QLatin1String("}\n");
        break;
    case GLSLFileWizard::FragmentShaderES:
        str << QLatin1String("uniform sampler2D qt_Texture0;\n")
            << QLatin1String("varying highp vec4 qt_TexCoord0;\n")
            << QLatin1String("\n")
            << QLatin1String("void main(void)\n")
            << QLatin1String("{\n")
            << QLatin1String("    gl_FragColor = texture2D(qt_Texture0, qt_TexCoord0.st);\n")
            << QLatin1String("}\n");
        break;
    case GLSLFileWizard::VertexShaderDesktop:
        str << QLatin1String("attribute vec4 qt_Vertex;\n")
            << QLatin1String("attribute vec4 qt_MultiTexCoord0;\n")
            << QLatin1String("uniform mat4 qt_ModelViewProjectionMatrix;\n")
            << QLatin1String("varying vec4 qt_TexCoord0;\n")
            << QLatin1String("\n")
            << QLatin1String("void main(void)\n")
            << QLatin1String("{\n")
            << QLatin1String("    gl_Position = qt_ModelViewProjectionMatrix * qt_Vertex;\n")
            << QLatin1String("    qt_TexCoord0 = qt_MultiTexCoord0;\n")
            << QLatin1String("}\n");
        break;
    case GLSLFileWizard::FragmentShaderDesktop:
        str << QLatin1String("uniform sampler2D qt_Texture0;\n")
            << QLatin1String("varying vec4 qt_TexCoord0;\n")
            << QLatin1String("\n")
            << QLatin1String("void main(void)\n")
            << QLatin1String("{\n")
            << QLatin1String("    gl_FragColor = texture2D(qt_Texture0, qt_TexCoord0.st);\n")
            << QLatin1String("}\n");
        break;
    default: break;
    }

    return contents;
}

Core::BaseFileWizard *GLSLFileWizard::create(QWidget *parent, const Core::WizardDialogParameters &parameters) const
{
    Core::BaseFileWizard *wizard = new Core::BaseFileWizard(parent);
    wizard->setWindowTitle(tr("New %1").arg(displayName()));
    Utils::FileWizardPage *page = new Utils::FileWizardPage;
    page->setPath(parameters.defaultPath());
    wizard->addPage(page);

    foreach (QWizardPage *p, parameters.extensionPages())
        wizard->addPage(p);
    return wizard;
}

QString GLSLFileWizard::preferredSuffix(ShaderType shaderType) const
{
    switch (shaderType) {
    case GLSLFileWizard::VertexShaderES:
        return QLatin1String("vsh");
    case GLSLFileWizard::FragmentShaderES:
        return QLatin1String("fsh");
    case GLSLFileWizard::VertexShaderDesktop:
        return QLatin1String("vert");
    case GLSLFileWizard::FragmentShaderDesktop:
        return QLatin1String("frag");
    default:
        return QLatin1String("glsl");
    }
}
