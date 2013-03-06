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


#ifndef _INSTALL_WORKER_H_
#define _INSTALL_WORKER_H_

#include <QtCore>
#include "wacWidgetManager.h"

class InstallWorker : public QObject
{
    Q_OBJECT

public:
    explicit InstallWorker(QObject *parent=0);
    ~InstallWorker();
    void doInstall(WidgetManager* widgetmanager, const QString &pkgPath, 
            bool silent, bool update, const QString& rootDirectory);

signals:
    void installComplete(QString widgetId, WidgetInstallError result);

};




#endif  // _INSTALL_WORKER_H_

