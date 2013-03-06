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

#ifndef __WACWEBAPPREGISTRYPRIVATE_H__
#define __WACWEBAPPREGISTRYPRIVATE_H__

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

#ifdef Q_OS_SYMBIAN
class WidgetRegistrationS60;
#ifdef QTWRT_USE_USIF
class WidgetRegistrationS60_SCR;
#endif // QTWRT_USE_USIF
#endif // Q_OS_SYMBIAN

class SharedWriteCounter {
public:

    SharedWriteCounter() {
        m_sharedMemory.setKey("WebAppRegistryWriteCount");
        if (!m_sharedMemory.attach(QSharedMemory::ReadWrite)) {
            m_sharedMemory.create(sizeof(int), QSharedMemory::ReadWrite);
            int sharedCounter=0;
            memcpy(m_sharedMemory.data(), &sharedCounter, sizeof(int));
        }   
    }
  
    void increment() {
        int sharedCounter = get();
        ++sharedCounter;
        m_sharedMemory.lock();
        memcpy(m_sharedMemory.data(), &sharedCounter, sizeof(int));
        m_sharedMemory.unlock();
    }
    int get() {
        m_sharedMemory.lock();
        int *sharedCounter = (int*) m_sharedMemory.data();
        m_sharedMemory.unlock();
        return *sharedCounter;
    } 
private:
    QSharedMemory m_sharedMemory;
};


class WebAppRegistryCache
{
public:
  
    WebAppRegistryCache();   

virtual ~WebAppRegistryCache();
    bool appInfoExists(const QString &appID);  
    bool getAppInfo(const QString &appID, WebAppInfo &webAppInfo);
  
    void saveAppInfo(const QString &appID, const WebAppInfo& info);
    void notifyDBWrite();
    void clear();
private:
    QMap<QString, WebAppInfo> m_appIdtoWebAppInfo;
    SharedWriteCounter m_sharedWriteCounter;
    int lastWriteCounter;
};


class WebAppRegistryPrivate
{
public:
    WebAppRegistryPrivate();
    ~WebAppRegistryPrivate();

    bool updateAttribute(const QString& appID, const QString& attribute, const QVariant& value);
    QList<WebAppInfo>* registeredHavingAttribute(const QString& key, const QString& value, const bool& match);
    bool retrieveRegistrationData(QList<WebAppInfo>& list, const QList<int>& idList);

#ifdef Q_OS_SYMBIAN
    
    WidgetRegistrationS60* m_s60Instance;

#ifdef QTWRT_USE_USIF
    WidgetRegistrationS60_SCR* m_s60SCRInstance;
#endif // QTWRT_USE_USIF

#endif // Q_OS_SYMBIAN

};

#endif // __WACWEBAPPREGISTRYPRIVATE_H__
