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

#include "webappuniquenesshelper.h"
#include "wacwidgetmanagerconstants.h"
#include "WidgetUtilsLogs.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>


WebAppUniquenessHelper::WebAppUniquenessHelper(const QString& installationDirectory)
    : m_installDir(installationDirectory)
{
}

bool WebAppUniquenessHelper::getUniqueNameFromPath(const QString& pkgPath, 
QString &uniqueName)
{
    // Convert the separators, remove trailing separator, and assign pkgpath
    // to local variable, since we will modify it
    if (!pkgPath.contains(WIDGET_FOLDER,Qt::CaseSensitive))
      return false;

    QString path = QDir::convertSeparators(pkgPath);
    if (path.endsWith(QDir::separator())) {
        path = path.remove(path.length()-1, path.length()-1);
    }

    int i = path.length() - path.lastIndexOf(QDir::separator()) - 1;
    if (i > 0) {
        uniqueName = path.right(i);
        if (uniqueName.compare(WIDGET_FOLDER,Qt::CaseSensitive)!=0)
            return true;
    }

    return false;
}

QString WebAppUniquenessHelper::generateUniqueWebAppId(const QString& widgetId, const QString& certId, 
const QString& domainName)
{
    QString cat = widgetId + ':' + certId + ':' + domainName;

    QByteArray uid;
    uid = cat.toUtf8();
    uid = QCryptographicHash::hash(uid,QCryptographicHash::Sha1);
    LOG("Unique widget ID for "<< widgetId << " widget is " << uid.toHex());
    return uid.toHex();
}
