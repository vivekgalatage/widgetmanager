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

#ifndef __WIDGETREGISTRATIONS60_SCR_H__
#define __WIDGETREGISTRATIONS60_SCR_H__

#include <QList>
#include <QString>
#include "wacAttributeMap.h"
#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/scr/appreginfo.h>

using namespace Usif;

class WidgetRegistrationS60_SCR
{
public:
    WidgetRegistrationS60_SCR();

    bool registerApp(const QString& appId,
                     const QString& appTitle,
                     const QString& appPath,
                     const QString& dataPath,
                     const QString& startPath,
                     const QString& iconPath,
                     int size,
                     int widgetUid,
                     const QSet<QString>& languages,
                     const QString& version,
                     const QString& author,
                     const QString& widgetType,
                     const AttributeMap& attributes,
                     bool hideIcon);
    bool unregister(const QString& appID);
    void SetIsPresentL(const QString& appId, bool isPresent);
    void SetPropertyL(const QString& appId, const QString& attr, bool value);
    void SetIsComponentOriginVerifiedL(const QString& appId, bool aIsOriginVerified);

private:

    bool registerAppL(const QString& appId,
                      const QString& appTitle,
                      const QString& appPath,
                      const QString& dataPath,
                      const QString& startPath,
                      const QString& iconPath,
                      int size,
                      int widgetUid,
                      const QSet<QString>& languages,
                      const QString& version,
                      const QString& author,
                      const QString& widgetType,
                      const AttributeMap& attributes,
                      bool hideIcon);

    bool unregisterL(const QString& appID);
    bool RegisterApplicationL(
        const TComponentId& aComponentId,
        const QString& aAppId,
        int aAppUid,
        const QString& iconPath,
        const QString& widgetType,
        const QSet<QString>& languages,
        bool hideIcon);

    TComponentId RegisterComponentL(
        const QString &aVendor,
        const QString &aVersion,
        const QString &Name,
        const QString &aGlobalId,
        const TInt64& aComponentSize,
        const TBool& aIsDrmProtected);

    void SetComponentPropertyL(const TComponentId& aComponentId, const TDesC& aName, const TInt64& value);
    void SetComponentPropertyL(const TComponentId& aComponentId, const TDesC& aName, const TDesC& value);

    void SCROpenL(int retries, unsigned long retryInterval);
    void SCRCloseAndCommitL();
    QMap<QString, TLanguage> languageMap;
    RSoftwareComponentRegistry m_SCRClient;
};
#endif
