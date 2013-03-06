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

#ifndef __WACWEBAPPREGISTRY_H__
#define __WACWEBAPPREGISTRY_H__

#include <QString>
#include <QVariant>
#include <QSet>
#include <QSharedMemory>

// FIXME : WebAppInfo shouldn't be included over here. Forward declaration should be enough
// Laborious to find all the places that utilized WebAppInfo.
#include "wacwebappinfo.h"
#include "wacWidgetUtils.h"
#include "wacAttributeMap.h"
#include "wacPropertyNames.h"
#include "W3CXmlParser/wacw3celement.h"

class WebAppRegistryPrivate;
class WebAppRegistryCache;

enum DatabaseVersionInfo {
    CurrentDatabaseVersion = 2
};

class WebAppRegistryInterface
{
    virtual bool registerApp(const QString& appId,
                             const QString& appTitle,
                             const QString& appPath,
                             const QString& iconPath,
                             const AttributeMap& attributes,
                             const QString& type,
                             unsigned long size,
                             const QString& startPath,
                             bool hideIcon,
                             const QSet<QString>& languages = QSet<QString>(),
                             const QString& version = QString(),
                             const QString& author = QString()) = 0;
    virtual bool unregister (const QString& appID, bool update = false) = 0;
    virtual bool isRegistered (const QString& appID) const = 0;
    virtual bool isRegistered(const QString& appID, WebAppInfo& info) = 0;
    virtual QList<WebAppInfo>* registered() = 0;
    virtual QList<WebAppInfo>* present(bool isPresent) = 0;
    virtual bool setWebAppAttribute(const QString& appID,
                                    const QString& attribute,
                                    const QVariant& value) = 0;
    virtual QVariant getAttribute(const QString& appID,
                                        const QString& attribute,
                                        const QVariant& defaultValue = QVariant() ) = 0;

    virtual bool setWebAppVersion(const QString& appID,
                                  const QVariant& value,
                                  const QString& newIconPath) = 0;
    virtual bool setNativeId(const QString& appID,
                             const QString& nativeID) = 0;
    virtual QString nativeIdToAppId(const QString& nativeID) = 0;
    virtual int appIdTonativeId(const QString& appID) = 0;

    virtual int nextAvailableUid() = 0;

    virtual bool setUid(const QString& appID,
                        int uid) = 0;
    virtual bool setCapabilityCheck(const QString& appID, bool value) = 0;
    virtual bool setCertificateAki(const QString& appID, const QString& value) = 0;
    virtual bool setIsPresent(const QString& appID, bool value) = 0;
    virtual void setIsFullView(const QString& appID, bool value) = 0;
    virtual void setIsActive(const QString& appID, bool value) = 0;
    virtual void setIsMiniView(const QString& appID, bool value) = 0;
    virtual QList<WebAppInfo>* presentOnDrive(const QString& driveLetter) = 0;
    virtual bool isWidgetIdRegistered(const QString& widgetId) = 0;
    virtual QStringList widgetIdToUniqueIdList(const QString& widgetId) = 0;
    virtual QString widgetIdToUniqueId(const QString& widgetId) = 0;
    virtual QString uniqueIdToWidgetId(const QString& uniqueId) = 0;
};

class WIDGETUTILS_EXPORT WebAppRegistry : public WebAppRegistryInterface
{
public:
    // singleton
    static WebAppRegistry* instance();
    // FIXME singleton cleanup

    virtual bool registerApp(const QString& appId, const QString& appTitle,
                             const QString& appPath, const QString& iconPath,
                             const AttributeMap& attributes,
                             const QString& type,
                             unsigned long size,
                             const QString& startPath,
                             bool hideIcon=false,
                             const QSet<QString>& languages = QSet<QString>(),
                             const QString& version = QString(),
                             const QString& author = QString());
    virtual bool unregister(const QString& appID, bool update = false);
    virtual bool isRegistered(const QString& appID) const;
    virtual bool isRegistered(const QString& appID, WebAppInfo& info);
    virtual QList<WebAppInfo>* registered();
    virtual QList<WebAppInfo>* present(bool isPresent);
    virtual bool setWebAppAttribute(const QString& appID,
                                    const QString& attribute,
                                    const QVariant& value);
    virtual QVariant getAttribute(const QString& appID,
                                        const QString& attribute,
                                        const QVariant& defaultValue = QVariant() );
    virtual bool setWebAppVersion(const QString& appID,
                                  const QVariant& value,
                                  const QString& newIconPath);
    virtual bool setNativeId(const QString& appID,
                             const QString& nativeID);
    virtual QString nativeIdToAppId(const QString& nativeID);
    virtual int appIdTonativeId(const QString& appID);

    virtual int nextAvailableUid();

    virtual bool setUid(const QString& appID,
                              int uid);
    virtual bool setCapabilityCheck(const QString& appID, bool value);
    virtual bool setCertificateAki(const QString& appID, const QString& value);
    virtual bool setIsPresent(const QString& appID, bool value);
    virtual void setIsFullView(const QString& appID, bool value);
    virtual void setIsActive(const QString& appID, bool value);
    virtual void setIsMiniView(const QString& appID, bool value);

    virtual QList<WebAppInfo>* presentOnDrive(const QString& driveLetter);
    virtual QList<WebAppInfo>* notPresentOnDrive(const QString& driveLetter);
    virtual bool isWidgetIdRegistered(const QString& widgetId);
    virtual QStringList widgetIdToUniqueIdList(const QString& widgetId);
    virtual QString widgetIdToUniqueId(const QString& widgetId);
    virtual QString uniqueIdToWidgetId(const QString& uniqueId);

protected:
    WebAppRegistry();
    virtual ~WebAppRegistry();

private:
    WebAppRegistryPrivate* d;
    WebAppRegistryCache *m_cache;
};

#endif
