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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDir>
#include "WidgetRegistrationQt.h"
#include "WidgetLinkResolver.h"
#include "WidgetUtilsLogs.h"
#include "wacw3csettingskeys.h"

const QString KEY_VALUE_SEPERATOR = ",";
const QString KEY_VALUE_PAIR_SEPERATOR = ";";


void encodeDelimiters(QString& inputString)
{
    // encode all "%" chars with "%10"
    inputString.replace(QString("%"), QString("%10"));

    // encode all key value delimiter
    inputString.replace(KEY_VALUE_SEPERATOR, QString("%20"));

    // encode all key value pair delimiter
    inputString.replace(KEY_VALUE_PAIR_SEPERATOR, QString("%30"));
}

void decodeDelimiters(QString& inputString)
{
    // decode all "%20" to ":"
    inputString.replace(QString("%30"), KEY_VALUE_PAIR_SEPERATOR);

   // decode all "%30" to KEY_VALUE_PAIR_SEPERATOR
    inputString.replace(QString("%20"), KEY_VALUE_SEPERATOR);

   // decode all "%10" to "%"
    inputString.replace(QString("%10"), QString("%"));
}

/*!
  Create an empty database if not exists already. This should be the first function called by client before using any API's in this class
*/
void create()
{
    LOG("WebAppRegistry::create");
    QString dataLocation = WidgetLinkResolver::dataStorageLocation();
    QString dbLocation = QDir::toNativeSeparators(dataLocation
                                                  + "/webapp_registry.db");

    QSqlDatabase db = QSqlDatabase::database("webAppConnection", FALSE);
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE", "webAppConnection");
        db.setHostName("WebAppRegistry");
        db.setDatabaseName(dbLocation);
    }
    if (!db.isOpen() && !db.open()) {
        LOG(dbLocation);
        LOG("add db Error" << db.lastError().text());
        return;
    }
    QFileInfo dbFile(dbLocation);
    if (dbFile.exists() && dbFile.size() == 0) {
        QSqlQuery query (db);
        //create table as it does not exists
        query.exec ("CREATE TABLE webapp_registry ( id VARCHAR(50) PRIMARY KEY, appTitle VARCHAR(100) NOT NULL, appPath VARCHAR(100) NOT NULL, iconPath VARCHAR, attributeList VARCHAR, secureSession VARCHAR, widgetType VARCHAR(20) )");
        LOG("create table error: " << query.lastError().text());
        LOG("db error: " << db.lastError().text());
    }
}

/*!
 register application, takes all required inputs to register.

 \a appID a unique ID for the webApp,(mandatory)
 \a title a display name for webApp(mandatory)
 \a appPath path to the executable. URL if its a link(mandatory)
 \a iconPath iconPath for the webApp. If not specified default icon will be used(not mandatory)
  \a attributes is a map having <key, value> pairs for attributes(not mandatory)
 \return true if registration is successful
*/
bool WidgetRegistrationQt::registerApp(const QString& appId,
                                       const QString& appTitle,
                                       const QString& appPath,
                                       const QString& iconPath,
                                       const AttributeMap& attributes,
                                       const QString& widgetType,
                                       unsigned long size,
                                       const QString& startPath)
{
    LOG ("WebAppRegistry::registerApp -" << appId << appTitle << appPath);
    if (appId.isEmpty() || appTitle.isEmpty() || appPath.isEmpty())
        return false;

    bool ret(false);

    create();
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("INSERT INTO webapp_registry (id, appTitle, appPath, iconPath, attributeList, secureSession, widgetType) "
            "VALUES (:id, :appTitle, :appPath, :iconPath, :attributeList, :secureSession, :widgetType)");
        query.bindValue(":id", QVariant (appId));
        query.bindValue(":appTitle", QVariant (appTitle));
        query.bindValue(":appPath", QVariant (appPath));
        query.bindValue(":iconPath", QVariant (iconPath));

        QString attributesList;
        AttributeMap::const_iterator i;
        QString key;
        QString value;
        for (i=attributes.begin(); i != attributes.end(); i++) {
            key = i.key();
            value = i.value().toString();

            encodeDelimiters(key);
            encodeDelimiters(value);
            attributesList = attributesList + key +
                KEY_VALUE_SEPERATOR + value + KEY_VALUE_PAIR_SEPERATOR;
        }
        query.bindValue(":attributeList", QVariant (attributesList));

        query.bindValue(":widgetType", QVariant (widgetType));

        if (query.exec())
            ret = true;
    }

#ifdef Q_OS_SYMBIAN
    if (ret == true){
        LOG("WidgetRegistrationQt::registerApp() - Registering using WidgetRegistrationS60 API to store data in S60 Registry");

        ret = m_s60Instance.registerApp(appId,
                                     appTitle,
                                     appPath,
                                     iconPath,
                                     attributes,
                                     widgetType,
                                     size,
                                     startPath);
    }
#endif

    return ret;
}

/*!
 unregister webapp

 \a appID web app ID to unregister
 \return true if unregister is successful.
*/
bool WidgetRegistrationQt::unregister (const QString& appID, bool update)
{
    LOG ("WebAppRegistry::unregister -" << appID);

    bool ret(false);

    create();
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("DELETE FROM webapp_registry WHERE id = :id");
        query.bindValue(0, QVariant (appID));
        query.exec();
        LOG(query.lastError().text());
        ret = true;
    }

#ifdef Q_OS_SYMBIAN
    if (ret == true){
        LOG("WidgetRegistrationQt::unregister() - UnRegistering using WidgetRegistrationS60 API to update data in S60 Registry");

        ret = m_s60Instance.unregister(appID,
                                       update);
    }
#endif

    return ret;
}

/*!
 \a appID id to check if already registered
 \return true if webapp is registered
*/
bool WidgetRegistrationQt::isRegistered (const QString& appID) const
{
    LOG ("WebAppRegistry::isRegistered -" << appID);

    create();
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
         QSqlQuery query(db);
         query.prepare("SELECT id FROM webapp_registry WHERE id = :id");
         query.bindValue(0, QVariant (appID));
         query.exec();
         LOG(query.lastError().text());
         while (query.next()) {
             if (!query.value(0).toString().isEmpty()) {
                 LOG ("WebAppRegistry::isRegisters returns true");
                 return true;
             }
         }
    }

    LOG ("WebAppRegistry::isRegistered returns false");
    return false;
}

/*!
 This is an overloaded member function, provided for convenience

 \a appID web app ID to unregister
 \a WebAppInfo to fill related information for the webApp
 \return true if webApp is registered.
*/
bool WidgetRegistrationQt::isRegistered(const QString& appID, WebAppInfo& info)
{
    LOG ("WebAppRegistry::registered : " << appID);
    create();
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM webapp_registry WHERE id = :id");
        query.bindValue(":id", QVariant (appID));
        query.exec();
        LOG(query.lastError().text());
        if (query.next()) {
           info.m_appId = query.value(0).toString();
           info.m_appTitle = query.value(1).toString();
           info.m_appPath = query.value(2).toString();
           info.m_iconPath= query.value(3).toString();
           QString attributes = query.value(4).toString();
           AttributeMap attributeList;
           QString key, value;
           QStringList pairs = attributes.split(KEY_VALUE_PAIR_SEPERATOR,
                                                QString::SkipEmptyParts);
           for (int i=0; i<pairs.count(); i++) {
               QStringList pair = pairs[i].split(KEY_VALUE_SEPERATOR,
                                                 QString::SkipEmptyParts);
               if (pair.count() >1 )
               {
                   key = pair[0];
                   value = pair[1];
               }
               else if (pair.count() == 1 )
               {
                   key = pair[0];
                   value = "";
               }

               decodeDelimiters(key);
               decodeDelimiters(value);
               attributeList[key] = value;
           }

           info.m_data = attributeList;
           info.m_widgetType= query.value(6).toString();
           return true;
        }
        LOG ("Error: Notfound " << appID);
    }

    LOG ("WebAppRegistry::registered: error");
    return false;
}

/*!
 \return list of all already registered web apps, note that reciever responsible to free the list
*/
QList<WebAppInfo>* WidgetRegistrationQt::registered()
{
    LOG ("WebAppRegistry::registered");
    create();
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM webapp_registry");
        query.exec();
        LOG("[SELECT * FROM webapp_registry] returns: " << query.lastError().text());
        QList<WebAppInfo> *list = new QList<WebAppInfo>();

        while (query.next()) {
           WebAppInfo appInfo;
           appInfo.m_appId = query.value(0).toString();
           appInfo.m_appTitle = query.value(1).toString();
           appInfo.m_appPath = query.value(2).toString();
           appInfo.m_iconPath = query.value(3).toString();

           QString attributes = query.value(4).toString();
           AttributeMap attributeList;
           QString key, value;
           QStringList pairs = attributes.split(KEY_VALUE_PAIR_SEPERATOR, QString::SkipEmptyParts);
           for (int i=0; i<pairs.count(); i++) {
               QStringList pair = pairs[i].split(KEY_VALUE_SEPERATOR, QString::SkipEmptyParts);
               if (pair.count() >1 )
               {
                   key = pair[0];
                   value = pair[1];
               }
               else if (pair.count() == 1 )
               {
                   key = pair[0];
                   value = "";
               }
               decodeDelimiters(key);
               decodeDelimiters(value);
               attributeList[key] = value;
           }
           appInfo.m_data = attributeList;

           appInfo.m_widgetType = query.value(6).toString();
           list->append(appInfo);
        }
        LOG ("Total: " << list->count());
        return list;
    }
    LOG ("WebAppRegistry::registered: error");
    return NULL;
}

/*!
 sets the web application attribute.

 \a appID a unique ID for the webApp
 \a attribute is attribute key
 \a value is value to be set.
 \return true if successful
*/
bool WidgetRegistrationQt::setWebAppAttribute(const QString& appID,
                                              const QString& attribute,
                                              const QVariant& value)
{
    LOG ("WebAppRegistry::setWebAppAttribute -" << appID);

    bool ret(false);

    create();
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    WebAppInfo info;
    if (isRegistered(appID, info)) {
        QVariant presentValue = info.value(attribute);
        //Reopen connection because isRegistered() closes it
         create();
        QSqlDatabase db = QSqlDatabase::database("webAppConnection");
        if (db.isOpen()) {
            if ((!value.isNull()) && (presentValue != value)) {
                QList<QString> stringList = info.m_data.keys();
                bool keyFound = false;
                // case insensitive search for the key
                for (int i=0; i<stringList.count(); i++) {
                    if (0 == stringList[i].compare(attribute,
                                                   Qt::CaseInsensitive)) {
                        info.m_data[stringList[i]] = value;
                        keyFound = true;
                        break;
                    }
                }
                // add new attribute
                if (!keyFound)
                    info.m_data[attribute] = value;

                QString attributesList;
                AttributeMap::const_iterator i;
                QString key;
                QString val;
                for (i=info.m_data.begin(); i != info.m_data.end(); i++) {
                    key = i.key();
                    val = i.value().toString();

                    encodeDelimiters(key);
                    encodeDelimiters(val);
                    attributesList = attributesList + key
                        + KEY_VALUE_SEPERATOR + val + KEY_VALUE_PAIR_SEPERATOR;
                }
                QSqlQuery query(db);
                QString queryStr = "UPDATE webapp_registry SET attributeList = :value WHERE id = :id";
                query.prepare(queryStr);
                query.bindValue(":value", attributesList);
                query.bindValue(":id", QVariant (appID));
                query.exec();

                LOG("WebAppRegistry::setWebAppAttribute - "<<query.lastQuery());
                LOG(query.lastError().text());
                ret = true;
            }
            ret = true;
        }
    }
    return ret;
}

/*!
 sets the web application version.
  \a appID a unique ID for the webApp
  \a value is value to be set.

  The newIconPath parameter is ignored in this implementation.

 \return true if successful
*/
bool WidgetRegistrationQt::setWebAppVersion(const QString& appID,
                                            const QVariant& value,
                                            const QString& newIconPath)
{
    bool ret = setWebAppAttribute(appID, W3CSettingsKey::WIDGET_VERSION , value);

#ifdef Q_OS_SYMBIAN
    if ( ret ) {
        ret = m_s60Instance.setWebAppVersion(appID, value, newIconPath);
    }
#endif
    return ret;
}

QString WidgetRegistrationQt::nativeIdToAppId(const QString& appID)
{
#ifdef Q_OS_SYMBIAN
    LOG("WidgetRegistrationQt::nativeIdToAppId() - nativeIdToAppId using WidgetRegistrationS60 API");

    return m_s60Instance.nativeIdToAppId(appID);
#else
    return appID;
#endif
}

WidgetRegistrationQt::~WidgetRegistrationQt()
{
}
