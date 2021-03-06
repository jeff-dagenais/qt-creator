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

#include "componentcompletedcommand.h"

#include <QDataStream>
#include <QDebug>

namespace QmlDesigner {

ComponentCompletedCommand::ComponentCompletedCommand()
{
}

ComponentCompletedCommand::ComponentCompletedCommand(const QVector<qint32> &container)
    : m_instanceVector(container)
{
}

QVector<qint32> ComponentCompletedCommand::instances() const
{
    return m_instanceVector;
}

void ComponentCompletedCommand::sort()
{
    qSort(m_instanceVector);
}

QDataStream &operator<<(QDataStream &out, const ComponentCompletedCommand &command)
{
    out << command.instances();

    return out;
}

QDataStream &operator>>(QDataStream &in, ComponentCompletedCommand &command)
{
    in >> command.m_instanceVector;

    return in;
}

bool operator ==(const ComponentCompletedCommand &first, const ComponentCompletedCommand &second)
{
    return first.m_instanceVector == second.m_instanceVector;
}

QDebug operator <<(QDebug debug, const ComponentCompletedCommand &command)
{
    return debug.nospace() << "ComponentCompletedCommand(" << command.instances() << ")";

}

} // namespace QmlDesigner

