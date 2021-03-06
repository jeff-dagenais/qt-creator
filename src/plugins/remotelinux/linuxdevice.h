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

#ifndef LINUXDEVICECONFIGURATION_H
#define LINUXDEVICECONFIGURATION_H

#include "remotelinux_export.h"

#include <projectexplorer/devicesupport/idevice.h>

#include <QCoreApplication>

namespace QSsh { class SshConnectionParameters; }
namespace Utils { class PortList; }

namespace RemoteLinux {
namespace Internal { class LinuxDevicePrivate; }

class REMOTELINUX_EXPORT LinuxDevice : public ProjectExplorer::IDevice
{
    Q_DECLARE_TR_FUNCTIONS(RemoteLinux::Internal::LinuxDevice)

public:
    typedef QSharedPointer<LinuxDevice> Ptr;
    typedef QSharedPointer<const LinuxDevice> ConstPtr;

    static Ptr create();
    static Ptr create(const QString &name, Core::Id type, MachineType machineType,
                      Origin origin = ManuallyAdded, Core::Id id = Core::Id());

    QString displayType() const;
    ProjectExplorer::IDeviceWidget *createWidget();
    QList<Core::Id> actionIds() const;
    QString displayNameForActionId(Core::Id actionId) const;
    void executeAction(Core::Id actionId, QWidget *parent);
    ProjectExplorer::IDevice::Ptr clone() const;

    bool canCreateProcess() const { return true; }
    ProjectExplorer::DeviceProcess *createProcess(QObject *parent) const;
    bool canAutoDetectPorts() const;
    ProjectExplorer::PortsGatheringMethod::Ptr portsGatheringMethod() const;
    bool canCreateProcessModel() const { return true; }
    ProjectExplorer::DeviceProcessList *createProcessListModel(QObject *parent) const;
    bool hasDeviceTester() const { return true; }
    ProjectExplorer::DeviceTester *createDeviceTester() const;
    ProjectExplorer::DeviceProcessSignalOperation::Ptr signalOperation() const;

protected:
    LinuxDevice() {}
    LinuxDevice(const QString &name, Core::Id type,
                             MachineType machineType, Origin origin, Core::Id id);
    LinuxDevice(const LinuxDevice &other);

private:
    LinuxDevice &operator=(const LinuxDevice &);
};

} // namespace RemoteLinux

#endif // LINUXDEVICECONFIGURATION_H
