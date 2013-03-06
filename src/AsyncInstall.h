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


#ifndef _ASYNC_INSATLL_H_
#define _ASYNC_INSATLL_H_

#include <QtCore>

class WidgetManager;

class AsyncInstall : public QRunnable
{
public:
    AsyncInstall(const QString &pkgPath, bool silent, 
            bool update, WidgetManager *widgetmgr, const QString& rootDirectory);
    ~AsyncInstall();
    void run();

private:
    WidgetManager *m_widgetmanager;
    QString m_PkgPath;
    bool m_Silent;
    bool m_Update;
    const QString& m_RootDirectory;
};


#endif  // _ASYNC_INSATLL_H_

