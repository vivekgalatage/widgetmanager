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

#ifndef __WIDGETREGISTRATIONS60_H__
#define __WIDGETREGISTRATIONS60_H__

#include <QObject>
#include <QList>
#include <QString>
#include "wacWebAppRegistry.h"

class WidgetRegistrationS60
{
public:  // Constructors and destructor
    WidgetRegistrationS60();

public:
    bool registerApp(const QString& appId, const QString& appTitle,
                     const QString& appPath, const QString& dataPath,
                     const QString& iconPath,
                     const AttributeMap& attributes,
                     const QString& type,
                     unsigned long size,
                     const QString& startPath,
                     int& widgetUid,
                     QString& convertedIconPath,
                     bool hideIcon=false);
    bool unregister (const QString& appID, bool update = false);
    bool setWebAppAttribute(const QString& appID,
                            const QString& attribute,
                            const QVariant& value);
    bool setWebAppVersion(const QString& appID,
                          const QVariant& value,
                          const QString& newIconPath);
    /**
    * Set if a type of the app is shared lib for unregistration
    * @param sharedLib - true if app to be unregister is a shared lib
    * @return void
    */
    void setSharedLib(bool sharedLib);

    //  Wrapped Leaving Functions
private:
    bool registerAppL(const QString& appId, const QString& appTitle,
                      const QString& appPath, const QString& dataPath,
                      const QString& iconPath,
                      const AttributeMap& attributes,
                      const QString& type,
                      unsigned long size,
                      const QString& startPath,
                      int& widgetUid,
                      QString& convertedIconPath,
                      bool hideIcon=false);
    bool unregisterL(const QString& appID, bool update = false);
    bool setWebAppAttributeL(const QString& appID,
                             const QString& attribute,
                             const QVariant& value);
    bool setWebAppVersionL(const QString& appID,
                           const QVariant& value,
                           const QString& newIconPath);
    //members
private:
    bool iSharedLib;       //type of the app

};

#endif
