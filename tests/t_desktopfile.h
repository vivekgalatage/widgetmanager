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

#ifndef T_WRT_DESKTOPFILE_H
#define T_WRT_DESKTOPFILE_H

#include <QtTest>
#include <QString>

class Q_DECL_EXPORT WrtDesktopFileTest: public QObject
{
    Q_OBJECT

public:
    WrtDesktopFileTest(QWidget *parent = NULL);
    ~WrtDesktopFileTest();

private Q_SLOTS:
    void initTestCase();
    void WidgetDesktopFileTest();
    void cleanupTestCase();


private:
    QString twidgetInstall(QString &aPkgPath);
    int twidgetUnInstall(QString &id) ;
    bool checkDesktopFile(QStringList fileWidgetNames);

private:
    QString m_widgetId;
    QString m_appPath;
};

#endif
