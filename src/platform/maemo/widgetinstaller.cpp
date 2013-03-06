/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This file is part of Qt Web Runtime.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "widgetinstaller.h"
#include "widgetinstaller_p.h"
#include "wacSuperWidget.h"
#include "storage.h"
#include "wacw3csettingskeys.h"
#include <QProcess>


WidgetInstaller::WidgetInstaller(SuperWidget* webWidget, WAC::Storage* storage)
    : d(new WidgetInstallerPrivate(webWidget))
{
    Q_UNUSED(storage)

#if defined(Q_OS_MAEMO6)
    QObject::connect( webWidget, SIGNAL(installationCancelled()), d, SLOT( installationCancelled()), Qt::DirectConnection);
#endif
}

WidgetInstaller::~WidgetInstaller()
{
    delete d;
}

bool WidgetInstaller::install(QDir source, QDir target, QString appId)
{
    bool bRet = d->install(source.absolutePath(), target.absolutePath(), appId) == WidgetInstallerPrivate::Success;
#ifdef Q_OS_MAEMO6
    source.cdUp();
    QProcess deldirs;
    deldirs.start("rm -rf " + source.path());
    deldirs.waitForFinished(5000);
#endif
    return bRet;
}

/*!
    \internal
    For Maemo, update a widget is the same as install a widget, as we rely on the packaging system.
 */
bool WidgetInstaller::update(QDir source, QDir target, QString appId)
{
    return install(source, target, appId);
}

/*!
    \internal
    Uninstall the widget with unique ID of \a appId. Currently it's not supported, as users are expected to uninstall the widgets from Application Manager.
 */
bool WidgetInstaller::uninstall(QString applicationPath, QString appId)
{
    Q_UNUSED(applicationPath)
    Q_UNUSED(appId)
    return true;
}

void WidgetInstaller::setInstallationAttributes(WAC::InstallationAttributes attributes)
{
    d->setInstallationAttributes(attributes);
}
