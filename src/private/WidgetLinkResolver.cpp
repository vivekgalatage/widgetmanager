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

#include <QDir>
#include <QDesktopServices>

#include "WidgetLinkResolver.h"
#include "wacwidgetmanagerconstants.h"
#include "WidgetUtilsLogs.h"


Q_DECL_EXPORT const QString WidgetLinkResolver::resolveLink (const QString& dirPath, const QString& fileName)
{
    QDir widgetDir(dirPath);

    foreach (const QFileInfo &file, widgetDir.entryInfoList(QDir::Files))
    {
        if ( !fileName.compare (file.fileName(), Qt::CaseInsensitive) ) {
            LOG("WidgetLinkResolver::resolveLink : final link: " << file.absoluteFilePath());
            return file.absoluteFilePath();
        }
    }

    return "";
}

Q_DECL_EXPORT const QString WidgetLinkResolver::installedLocation()
{
    QString dirPath = QDir::toNativeSeparators(dataStorageLocation() + "/.webApps/");
#ifdef Q_OS_SYMBIAN
    if (dirPath.startsWith("Z"))
        dirPath.replace(0,1,"C");
#endif
    if (!QFileInfo(dirPath).exists()) {
        QDir dir (dataStorageLocation());
        dir.mkpath(dirPath);
    }

    return dirPath;
}

Q_DECL_EXPORT const QString WidgetLinkResolver::preInstalledLocation()
{
    return "../../../ui/simulator/pre-installers/";
}

Q_DECL_EXPORT const QString WidgetLinkResolver::dataStorageLocation()
{
    QString path;
    path = QDir::toNativeSeparators(REGISTRY_DB_PATH);
    if (!QFileInfo(path).exists()) {
        QDir dir(path);
        dir.mkpath(path);
    }

    return path;
}


