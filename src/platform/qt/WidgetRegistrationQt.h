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

#ifndef __WIDGETREGISTRATIONQT_H__
#define __WIDGETREGISTRATIONQT_H__

#include "wacwebappinfo.h"
#include "wacWebAppRegistry.h"
#ifdef Q_OS_SYMBIAN
#include "WidgetRegistrationS60.h"
#endif

class WidgetRegistrationQt : public WebAppRegistry
{
public:
    virtual bool registerApp(const QString& appId, const QString& appTitle,
                             const QString& appPath, const QString& iconPath,
                             const AttributeMap& attributes,
                             const QString& widgetType,
                             unsigned long size,
                             const QString& startPath);
    virtual bool unregister (const QString& appID, bool update = false);
    virtual bool isRegistered (const QString& appID) const;
    virtual bool isRegistered(const QString& appID, WebAppInfo& info);
    virtual QList<WebAppInfo>* registered();
    virtual bool setWebAppAttribute(const QString& appID,
                                    const QString& attribute,
                                    const QVariant& value);
    virtual bool setWebAppVersion(const QString& appID,
                                  const QVariant& value,
                                  const QString& newIconPath);
    virtual QString nativeIdToAppId(const QString& appID);

    virtual ~WidgetRegistrationQt();
private:
#ifdef Q_OS_SYMBIAN
    WidgetRegistrationS60 m_s60Instance;
#endif
};

#endif
