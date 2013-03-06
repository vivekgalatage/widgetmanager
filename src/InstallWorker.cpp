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

#include "wacSuperWidget.h"
#include "InstallWorker.h"
#include "private/WidgetUtilsLogs.h"


InstallWorker::InstallWorker(QObject *parent)
    : QObject(parent)
{
    LOG("InstallWorker::InstallWorker()");
    qRegisterMetaType<WidgetInstallError>();
}

InstallWorker::~InstallWorker()
{
    LOG("InstallWorker::~InstallWorker()");
}

void InstallWorker::doInstall(WidgetManager* widgetmanager, const QString &pkgPath, 
        bool silent, bool update, const QString& rootDirectory)
{
    QString widgetId;
    LOG(">> InstallWorker::doInstall()");
    LOG("pkgpath " << pkgPath << "silent" << silent << "update" << update);

    enum WidgetInstallError result(WidgetInstallFailed);
    if(!widgetmanager) {
        LOG("FAIL: No widgetmanager");
        return;
    }

    LOG("widgetMgr.install()");
    result = widgetmanager->install(pkgPath, widgetId, silent,update, rootDirectory);
    emit installComplete(widgetId, result);

    LOG("InstallWorker::doInstall() widgetId " << widgetId << "result" << result);
    LOG("<< InstallWorker::doInstall()");
}
