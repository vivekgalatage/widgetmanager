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

#ifndef __WEBAPPINFO_H__
#define __WEBAPPINFO_H__

#include <QList>
#include <QString>
#include <QUrl>
#include <QVariant>

#include "wacAttributeMap.h"
#include "wacWidgetUtils.h"

#define WEBAPPREGISTRY_ATTRIBUTE_NOTPRESENT "!!!NOTPRESENT"

class W3CElement;

class WIDGETUTILS_EXPORT WebAppInfo
{
public:
    enum WidgetType {
        WidgetUnknown,
        WidgetNokia,
        WidgetW3C,
        WidgetSharedLibraryWidget,
        WidgetJIL
    };

    QString appId() const;
    QString appTitle() const;
    QString name() const;
    QString appPath() const;
    QString iconPath() const;
    QString type() const;
    QString startPath() const;
    QString dataPath() const;
    bool capabilityCheck() const;
    QString certificateAki() const;
    int widgetType() const;
    QString nativeId() const;
    QUrl url() const;
    const AttributeMap& attributes() const;
    QList<W3CElement*> getElement(const QString &aKey, const W3CElement *parent = 0, 
	const QString &aLang = QString("")) const;
    QVariant value(const QString& key) const;
    bool operator==(const WebAppInfo &other) const;
    bool isPresent() const;
    int uid() const;

private:
    QString m_appId;
    QString m_appTitle;
    mutable QString m_name;
    QString m_appPath;
    QString m_iconPath;
    AttributeMap m_data;
    QString m_widgetType;
    QString m_startPath;
    QString m_dataPath;
    QString m_certificateAki;
    bool m_capabilityCheck;
    int m_uid;
    QString m_nativeId;
    int m_id;  // database ID; do not use

    // FIXME : We should fix WebAppRegistry interface to return an instance WebAppInfo.
    // Currently we're using isRegistered to get WebAppInfo out of WebAppRegistry.
    //explicit WebAppInfo();
    //Q_DISABLE_COPY(WebAppInfo)

    friend class WebAppRegistryPrivate;
    friend class WebAppRegistry;
    friend class WidgetRegistrationS60;
    friend class WidgetRegistrationQt;
    friend class WrtWidgetManagerTest;
};


#endif // __WEBAPPINFO_H__
