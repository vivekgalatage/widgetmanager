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
#include "AsyncInstall.h"
#include "InstallWorker.h"
#include "wacWidgetManager.h"
#include "private/WidgetUtilsLogs.h"
#ifdef Q_OS_MAEMO6
#include <QCoreApplication>
#endif

AsyncInstall::AsyncInstall(const QString &pkgPath, bool silent,
        bool update, WidgetManager *widgetmgr, const QString& rootDirectory)
    : m_widgetmanager(widgetmgr), m_PkgPath(pkgPath), m_Silent(silent), 
      m_Update(update), m_RootDirectory(rootDirectory)
{
    LOG("AsyncInstall::AsyncInstall()");
}

AsyncInstall::~AsyncInstall()
{
    LOG("AsyncInstall::~AsyncInstall()");

}

void AsyncInstall::run()
{
    LOG("AsyncInstall::run() >>");

    InstallWorker *iw = new InstallWorker();

    QObject::connect( iw,
                      SIGNAL(installComplete(QString, WidgetInstallError)),
                      m_widgetmanager,
                      SIGNAL(installComplete(QString, WidgetInstallError)), Qt::QueuedConnection );

    iw->doInstall( m_widgetmanager,  m_PkgPath, m_Silent, m_Update, m_RootDirectory );
    delete iw;

    LOG("AsyncInstall::run() <<");
}
