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

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>
#include <QTextStream>
#include "wacSuperWidget.h"
#include "WidgetLinkResolver.h"
#include "wacWidgetProperties.h"
#include "wacAttributeMap.h"
#include "wacPropertyNames.h"
#include "wacwebappinfo.h"
#include "wacWebAppRegistry.h"
#include "wacWebAppRegistry_p.h"

#include "wacwidgetmanagerconstants.h"
#include "private/WidgetUtilsLogs.h"
#include "wacw3csettingskeys.h"
#include "proprietarysettingskeys.h"
#include "wacsettings.h"

#ifdef Q_OS_SYMBIAN
#include "WidgetRegistrationS60.h"

#ifdef QTWRT_USE_USIF
#include "WidgetRegistrationS60_SCR.h"
#include "SCRConstants.h"
#endif

#endif

#ifdef Q_OS_MAEMO6
#include <sys/stat.h>
#endif

using namespace WAC;

const QString KEY_VALUE_SEPERATOR = ",";
const QString KEY_VALUE_PAIR_SEPERATOR = ";";

// Private implementation prevents client ability to delete actual singleton object
class WebAppRegistrySingleton : public WebAppRegistry
{
public:
    WebAppRegistrySingleton() {}
    ~WebAppRegistrySingleton() {}
};
// Scoped pointer deletes singleton object in application exit, no memory leak
static QScopedPointer<WebAppRegistrySingleton> g_webappRegistry;

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
  Create an empty database if not exists already.
  This should be the first function called by client before using any API's in this class
*/
void create()
{
    LOG("WebAppRegistry::create");
    QString dataLocation = WidgetLinkResolver::dataStorageLocation();
    QString dbLocation =   QDir::toNativeSeparators(dataLocation

#ifdef Q_OS_MAEMO6
                         + "registry/webapp_registry.db");
    QDir dir = QFileInfo(dbLocation).absoluteDir();
    if (!dir.exists()) {
        qDebug() << "Create" << dir.absolutePath()<<dir.mkpath(dir.absolutePath());
    }
#else
                         + "/webapp_registry.db");
#endif
    LOG("dbLocation = " << dbLocation);

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

#ifdef Q_OS_MAEMO6
        // Change permission bits for WebAppRegistry.db, so that other eligible processes get read/write access to it.
        // (ex : webappregisterer need to be able to update registry)
        int chmodError = 
                chmod(dbLocation.toAscii(), S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP);
        if ( chmodError != 0) {
            LOG("chmod failed, error : " <<chmodError);
        }
#endif

    QStringList tables = db.tables();
    if (!tables.contains("dbversion")) {

        //create table as it does not exists
        db.exec("CREATE TABLE dbversion (version INTEGER)");
        QString insertStatement;
        QTextStream(&insertStatement) << "INSERT INTO dbversion (version) VALUES ("
                                      << CurrentDatabaseVersion
                                      << ')';
        db.exec(insertStatement);

        // NOTE for webapp_registry columns:  Per SQLite FAQ:  "SQLite does not enforce the length of a VARCHAR.
        //       You can declare a VARCHAR(10) and SQLite will be happy to let you put 500 characters in it. And
        //       it will keep all 500 characters intact - it never truncates."

        db.exec("CREATE TABLE webapp_registry("
                                   "id INTEGER PRIMARY KEY, "
                                   "appId VARCHAR(50) UNIQUE,"
                                   "appTitle VARCHAR(100) NOT NULL,"
                                   "appPath VARCHAR(100) NOT NULL,"
                                   "iconPath VARCHAR,"
                                   "secureSession VARCHAR,"
                                   "widgetType VARCHAR(20),"
                                   "uid INTEGER,"
                                   "startPath VARCHAR,"
                                   "dataPath VARCHAR(100),"
                                   "capabilityCheck BOOLEAN DEFAULT False,"
                                   "certificateAki VARCHAR(100),"
                                   "nativeId VARCHAR)");
                                   
        db.exec("CREATE TABLE webapp_attributes (id INTEGER, key VARCHAR, value VARCHAR, PRIMARY KEY (id, key))");
        LOG("db error: " << db.lastError().text());
    }
}

// Upgrade specifically from version 1 to 2
void upgradeDBv1Tov2()
{
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    if (db.isOpen()) {
        // Create the new version 2 table
        db.exec("DROP TABLE IF EXISTS webapp_registry_2");
        db.exec("CREATE TABLE webapp_registry_2("
                                   "id INTEGER PRIMARY KEY, "
                                   "appId VARCHAR(50) UNIQUE,"
                                   "appTitle VARCHAR(100) NOT NULL,"
                                   "appPath VARCHAR(100) NOT NULL,"
                                   "iconPath VARCHAR,"
                                   "secureSession VARCHAR,"
                                   "widgetType VARCHAR(20),"
                                   "uid INTEGER,"
                                   "startPath VARCHAR,"
                                   "dataPath VARCHAR(100),"
                                   "capabilityCheck BOOLEAN DEFAULT False,"
                                   "certificateAki VARCHAR(100),"
                                   "nativeId VARCHAR)");
        // loop through all of the records in the old webapp_registry
        // and create a new entry in the new table
        QSqlQuery query = db.exec("SELECT id, appId, appTitle, appPath, iconPath, secureSession, widgetType, uid, startPath, dataPath FROM webapp_registry");
        while (query.next()) {
              int id = query.value(0).toInt();
            QString appId = query.value(1).toString();
            QString appTitle = query.value(2).toString();
            QString appPath = query.value(3).toString();
            QString iconPath = query.value(4).toString();
            QString secureSession = query.value(5).toString();
            QString widgetType = query.value(6).toString();
            int uid = query.value(7).toInt();
            QString startPath = query.value(8).toString();
            QString dataPath = query.value(9).toString();

            // Insert row into webapp_registry_2
            QSqlQuery insertQ(db);
            insertQ.prepare("INSERT INTO webapp_registry_2 (id, appId, appTitle, appPath, iconPath, secureSession, widgetType, uid, startPath, dataPath) VALUES (:id, :appId, :appTitle, :appPath, :iconPath, :secureSession, :widgetType, :uid, :startPath, :dataPath)");
            insertQ.bindValue(":id",QVariant(id));
            insertQ.bindValue(":appId", QVariant(appId));
            insertQ.bindValue(":appTitle", QVariant(appTitle));
            insertQ.bindValue(":appPath", QVariant(appPath));
            insertQ.bindValue(":iconPath", QVariant(iconPath));
            insertQ.bindValue(":secureSession", QVariant(secureSession));
            insertQ.bindValue(":widgetType", QVariant(widgetType));
            insertQ.bindValue(":uid", QVariant(uid));
            insertQ.bindValue(":startPath", QVariant(startPath));
            insertQ.bindValue(":dataPath", QVariant(dataPath));
            insertQ.exec();
          }

        // delete the previous webapp_registry table
        db.exec("DROP TABLE webapp_registry");

        // rename the replacement webapp_registry_2 table to webapp_registry
        db.exec("ALTER TABLE webapp_registry_2 RENAME TO webapp_registry");

        // update the database version to version 2
        db.exec("UPDATE dbversion SET version=2");
    }
}


/*!
  upgradeDB is called by checkDB when the version of the database is lower than the expected
  (current) version.  This function takes necessary steps to upgrade the database to the current
  version, maintaining data.

  currently a noop, as there's only ever been this version of the db
 */
int upgradeDB(int fromVersion)
{
    // Handle upgrades to the database version by version
    while (fromVersion < CurrentDatabaseVersion) {
        switch (fromVersion) {
        // upgrade from version 1 to 2
        case 1:
            upgradeDBv1Tov2();
            fromVersion++;
            break;
        default:
            // This should never happen, it means there's no upgrade path
            // to the current database version
            return fromVersion;
        }
    }
    return fromVersion;
 }
/*!
  Checks the database for existence and version.  If it doesn't exist, calls create() to create it.
  If the version is prior to the expected current version, calls upgrade() to update the database.
 */
void checkDB()
{
    int dbversion = 0;
    
    // Check for the DBs existence and establish connection
    create();
   
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");    
    if (db.isOpen()) {
        // ... if not, it's a version 0 database and we need to start upgrading
        QSqlQuery query = db.exec("SELECT version FROM dbversion");
        if (query.next()) {
            dbversion = query.value(0).toInt();
        } else {
            dbversion = 0;                  
        }
    } 
 
    if (dbversion < CurrentDatabaseVersion) {
        dbversion = upgradeDB(dbversion);
    }
}

/*!
 * \class WebAppRegistryPrivate
 *
 */
 
WebAppRegistryPrivate::WebAppRegistryPrivate()
{
}

WebAppRegistryPrivate::~WebAppRegistryPrivate()
{
}

bool WebAppRegistryPrivate::updateAttribute(const QString& appID, const QString& attribute, 
const QVariant& value)
{
    bool ret(false);
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    if (db.isOpen()) {
        QSqlQuery query(db);
      
        query.prepare("UPDATE webapp_registry SET " + attribute + " = :value WHERE appId = :appId");
        query.bindValue(":value", value);
        query.bindValue(":appId", QVariant(appID));
        query.exec();
        LOG("WebAppRegistry::setWebAppAttribute - " << query.lastQuery());
        LOG(query.lastError().text());
        ret = true;
    }
    return ret;
}

bool WebAppRegistryPrivate::retrieveRegistrationData(QList<WebAppInfo>& list, 
        const QList<int>& idList)
{
     QSqlDatabase db = QSqlDatabase::database("webAppConnection");

     if (db.isOpen()) {
        
        for (int i=0; i<idList.count(); i++) {

           WebAppInfo appInfo;
           QSqlQuery query(db);

           query.prepare("SELECT id, appId, appTitle,appPath,iconPath,"
                         "secureSession,widgetType,uid,startPath,dataPath,"
                         "capabilityCheck,certificateAki,nativeId "
                         "FROM webapp_registry "
                         "WHERE id=:id");
  
           query.bindValue(":id", QVariant(idList.value(i)));
           query.exec();

           if(query.next()) {
               appInfo.m_id = query.value(0).toInt();
               appInfo.m_appId = query.value(1).toString();
               appInfo.m_appTitle = query.value(2).toString();
               appInfo.m_appPath = query.value(3).toString();
               appInfo.m_iconPath = query.value(4).toString();
               appInfo.m_widgetType = query.value(6).toString();
               appInfo.m_uid = query.value(7).toInt();
               appInfo.m_startPath = query.value(8).toString();
               appInfo.m_dataPath = query.value(9).toString();
               appInfo.m_capabilityCheck = query.value(10).toBool();
               appInfo.m_certificateAki = query.value(11).toString();
               appInfo.m_nativeId = query.value(12).toString();
           }

           QSqlQuery attrQuery(db);
           attrQuery.prepare("SELECT key,value FROM webapp_attributes WHERE id=:id");       
           attrQuery.bindValue(":id", QVariant(idList.value(i)));
           attrQuery.exec();

           AttributeMap attributeList;
           while (attrQuery.next()) {
               attributeList[attrQuery.value(0).toString()] = attrQuery.value(1).toString();                       
           }              

          appInfo.m_data = attributeList;
          list.append(appInfo);
          }
    return true;
    }

    LOG("WebAppRegistryPrivate::retrieveRegistrationData db.isOpen fail");
    return false;
}

QList<WebAppInfo>* WebAppRegistryPrivate::registeredHavingAttribute(const QString& key, const QString& value, const bool& match)
{
    LOG("WebAppRegistryPrivate::registeredHavingAttribute: " << key);
    QList<int> idList;

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {

        QSqlQuery query(db);

        if (!key.isEmpty()) { // A Key,Value pair is passed.
        
            if (match) {
                query.prepare("SELECT id FROM webapp_attributes WHERE (key=:key AND value=:value) GROUP BY id");
            }
            else {
                query.prepare("SELECT id FROM webapp_attributes GROUP BY id EXCEPT "
                          "SELECT id from webapp_attributes WHERE (key=:key AND value=:value)");
            }
            
            query.bindValue(":key", QVariant(key));
            query.bindValue(":value", QVariant(value));
            query.exec();
        }
        else { // No key, value pair is passed, return a list of webappinfos for all registered widgets
            query.prepare("SELECT id FROM webapp_registry");
            query.exec();
        }
        
        LOG( "SELECT returns: " << query.lastError().type() << query.lastError().text());
        
        // Build the idList
        while (query.next()) {                
                idList.append(query.value(0).toInt());
            }
                       
        QList<WebAppInfo> *list = new QList<WebAppInfo>();

        retrieveRegistrationData(*list, idList);
        LOG("Total: " << list->count());
        return list;
    }

    LOG("WebAppRegistryPrivate::registeredHavingAttribute db.isOpen fail");
    return NULL;
}

/*!
 * \class WebAppRegistryCache
 *
 * \brief WebAppRegistryCache class caches reads from the WebAppRegistry.
 */
 
WebAppRegistryCache::WebAppRegistryCache()
{
    lastWriteCounter = m_sharedWriteCounter.get(); // since the cache is empty, it's in synch w/ the database
}
 
WebAppRegistryCache::~WebAppRegistryCache()
{
}

void WebAppRegistryCache::notifyDBWrite() {
    m_sharedWriteCounter.increment(); 
}

bool WebAppRegistryCache::appInfoExists(const QString &appID) 
{
    if (lastWriteCounter!=m_sharedWriteCounter.get()) {
        clear();
        return false;
    }
    if (!m_appIdtoWebAppInfo.isEmpty()) {
        if (m_appIdtoWebAppInfo.contains(appID)) {
            return true;
        }
    }
    return false;
}

bool WebAppRegistryCache::getAppInfo(const QString &appID, WebAppInfo &webAppInfo) 
{
    if (!appInfoExists(appID)) {
        return false;
    }    
    webAppInfo = m_appIdtoWebAppInfo[appID]; 
    return true;
}

void WebAppRegistryCache::saveAppInfo(const QString &appID, const WebAppInfo &info)
{
    m_appIdtoWebAppInfo[appID] = info;  
}

void WebAppRegistryCache::clear()
{
    m_appIdtoWebAppInfo.clear();
    // since the cache is empty, we can consider ourselves up-to-date
    lastWriteCounter=m_sharedWriteCounter.get();
}



/*!
 * \class WebAppRegistry
 *
 * \brief WebAppRegistry class maintains the secured database for registered web applications.
 *  This is a single-ton class and WebAppRegistry::create() has to be called before
 *  using any API's in this class.
 *
 * This class is the public API for Web Application Framework. (TBD)
 */

/*!
 Static function returns singleton object of WebAppRegistry

 \return singleton WebAppRegistry pointer object
*/
WebAppRegistry* WebAppRegistry::instance()
{
    if (!g_webappRegistry) {
        g_webappRegistry.reset( new WebAppRegistrySingleton() );
    }
    return (WebAppRegistry*)g_webappRegistry.data();
}

WebAppRegistry::WebAppRegistry()
    : d(new WebAppRegistryPrivate())
{
#ifdef Q_OS_SYMBIAN
    d->m_s60Instance = new WidgetRegistrationS60();
#ifdef QTWRT_USE_USIF
    d->m_s60SCRInstance = new WidgetRegistrationS60_SCR();
#endif // QTWRT_USE_USIF
#endif // Q_OS_SYMBIAN

    // Ensure that the DB exists and is up-to-date
    checkDB();
    // create cache
    m_cache = new WebAppRegistryCache;
}

WebAppRegistry::~WebAppRegistry()
{

#ifdef Q_OS_SYMBIAN
    delete d->m_s60Instance;
#ifdef QTWRT_USE_USIF
    delete d->m_s60SCRInstance;
#endif // QTWRT_USE_USIF
#endif // Q_OS_SYMBIAN

    delete d;
    delete m_cache;
}

bool deleteAppRow(const QString& appID)
{
    bool ret(false);

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        // Get the id of the row with the passed appId
        QSqlQuery query(db);
        query.prepare("SELECT id FROM webapp_registry WHERE appId = :appId");
        query.bindValue(":appId", QVariant(appID));
        query.exec();

        if (query.next()) {
            int id = query.value(0).toInt();

            // delete the row from webapp_registry
            QSqlQuery delQuery(db);
            delQuery.prepare("DELETE FROM webapp_registry WHERE id = :id");
            delQuery.bindValue(0, QVariant(id));
            delQuery.exec();

            // delete the associated attributes in webapp_attributes
            QSqlQuery delAttributes(db);
            delAttributes.prepare("DELETE FROM webapp_attributes WHERE id = :id");
            delAttributes.bindValue(0, QVariant(id));
            delAttributes.exec();
        }
        
        LOG(query.lastError().text());
        ret = true;
    }
    return ret;
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
bool WebAppRegistry::registerApp(const QString& appId,
                                       const QString& appTitle,
                                       const QString& appPath,
                                       const QString& iconPath,
                                       const AttributeMap& attributes,
                                       const QString& widgetType,
                                       unsigned long size,
                                       const QString& startPath,
                                       bool hideIcon,
                                       const QSet<QString>& /*languages*/,
                                       const QString& /*version*/,
                                       const QString& /*author*/)
{
    LOG ("WebAppRegistry::registerApp -" << appId << appTitle << appPath);
    if (appId.isEmpty() || appTitle.isEmpty() || appPath.isEmpty())
        return false;

    bool ret(false);

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

#ifdef Q_OS_SYMBIAN
    QString dataPath =   appPath.section(QDir::separator(),0,2)
                       + QDir::separator()
                       + QString(DATA_FOLDER)
                       + QDir::separator()
                       + appPath.section(QDir::separator(),4);
    if (!dataPath.endsWith(QDir::separator())) {
        dataPath.append(QDir::separator());
    }
    // Ensure that dataPath is always on C drive
    if (dataPath[1] == ':') {
        dataPath.replace(0, 1, 'C');
    }
#else
    QString dataPath = DATA_PATH;
#endif

    if (db.isOpen()) {
        QSqlQuery query(db);
                     
        query.prepare("INSERT INTO webapp_registry("
                      "id, appId, appTitle, appPath, iconPath, secureSession,"
                      "widgetType, startPath, dataPath) "
                      "VALUES (NULL, :appId, :appTitle, :appPath, :iconPath, :secureSession,"
                      ":widgetType, :startPath, :dataPath)");
                             
        query.bindValue(":appId", QVariant(appId));
        query.bindValue(":appTitle", QVariant(appTitle));

        QString appPathNative = QDir::toNativeSeparators(appPath);
        // FIXME enforce canonicalization in caller
        // must end in QDir::separator()
        if (QDir::separator() != appPathNative.at(appPathNative.count()-1))
        {
            appPathNative.append(QDir::separator());
        }

        query.bindValue(":appPath", QVariant(appPathNative));
        query.bindValue(":iconPath", QVariant(iconPath));
        query.bindValue(":widgetType", QVariant(widgetType));
        query.bindValue(":startPath", QVariant(startPath));
        query.bindValue(":dataPath", QVariant(dataPath));

       // Insert attributes into the atributes table if the registry insert was a success
       if (query.exec()) {
            int id = query.lastInsertId().toInt();

            AttributeMap::const_iterator i;
            QString key;
            QString value;
            QSqlQuery insertAttribute(db);
            for (i=attributes.begin(); i != attributes.end(); i++) {
                key = i.key();
                value = i.value().toString();

                insertAttribute.prepare("INSERT INTO webapp_attributes (id, key, value) VALUES (:id, :key, :value)");
                insertAttribute.bindValue(":id", QVariant(id));
                insertAttribute.bindValue(":key", QVariant(key));
                insertAttribute.bindValue(":value", QVariant(value));
                insertAttribute.exec();
            }
            ret = true;
          }
    }
    
    m_cache->notifyDBWrite();

#ifdef Q_OS_SYMBIAN
    if (ret == true){
        int widgetUid = 0;
        QString convertedIconPath = iconPath;

        LOG("WebAppRegistry::registerApp() - Registering data in S60 Registry");

        // d->m_s60Instance returns widgetUid which gets passed
        // to d->m_S60SCRInstance through its registerApp()


        ret = d->m_s60Instance->registerApp(appId,
                                             appTitle,
                                             appPath,
                                             dataPath,
                                             iconPath,
                                             attributes,
                                             widgetType,
                                             size,
                                             startPath,
                                             widgetUid,
                                             convertedIconPath,
                                             hideIcon);

#ifdef QTWRT_USE_USIF
        if (ret) {
            ret = d->m_s60SCRInstance->registerApp(appId,
                                                    appTitle,
                                                    appPath,
                                                    dataPath,
                                                    startPath,
                                                    convertedIconPath,
                                                    size,
                                                    widgetUid,
                                                    languages,
                                                    version,
                                                    author,
                                                    widgetType,
                                                    attributes,
                                                    hideIcon);

        }

        if (!ret) {
            d->m_s60Instance->unregister(appId, false);
        }

#endif // QTWRT_USE_USIF

                if (!ret) {
                    // S60 registration failed, remove SQL DB entry for registration
                    deleteAppRow(appId);
                }
    }
#endif // Q_OS_SYMBIAN
    return ret;
}

/*!
 unregister webapp

 \a appID web app ID to unregister
 \return true if unregister is successful.
*/
bool WebAppRegistry::unregister (const QString& appID, bool update)
{
    LOG ("WebAppRegistry::unregister -" << appID);

    bool ret;
#ifdef Q_OS_SYMBIAN
    LOG("WebAppRegistry::unregister() - UnRegistering data in S60 Registry");
    // FIXME revisit this code when CloseWidgetL doesn't leave when it's a shared lib.
    WebAppInfo appInfo;
    (void) WebAppRegistry::instance()->isRegistered(appID, appInfo);
    d->m_s60Instance->setSharedLib(WebAppInfo::WidgetSharedLibraryWidget == appInfo.widgetType());
    ret = d->m_s60Instance->unregister(appID, update);
    // Return the flag to the default value.
    d->m_s60Instance->setSharedLib(false);
#ifdef QTWRT_USE_USIF
    if (ret==true)
        ret = d->m_s60SCRInstance->unregister(appID);
#endif

    if (ret==true)
        ret = deleteAppRow(appID);
#else
    ret = deleteAppRow(appID);

#endif // Q_OS_SYMBIAN

    m_cache->notifyDBWrite();
    return ret;
}

/*!
 \a appID id to check if already registered
 \return true if webapp is registered
*/
bool WebAppRegistry::isRegistered (const QString& appID) const
{
    LOG ("WebAppRegistry::isRegistered -" << appID);
    
    // even tho we don't want it here, read the WebAppInfo data into the cache
    WebAppInfo info;
    if (instance()->isRegistered(appID, info)) {
        return true;
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
bool WebAppRegistry::isRegistered(const QString& appID, WebAppInfo& info)
{
    LOG ("WebAppRegistry::registered : " << appID);

    if (m_cache->appInfoExists(appID)) {
        return m_cache->getAppInfo(appID, info);       
    }

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT id,appId,appTitle,appPath,iconPath,secureSession,"
                      "widgetType,uid,startPath,dataPath,capabilityCheck,certificateAki, nativeId "
                      "FROM webapp_registry WHERE appId = :appId");
        query.bindValue(":appId", QVariant(appID));
        query.exec();

        LOG(query.lastError().text());
        if (query.next()) {
            int id = query.value(0).toInt();
            info.m_id = id;
            info.m_appId = query.value(1).toString();
            info.m_appTitle = query.value(2).toString();
            info.m_appPath = query.value(3).toString();
            info.m_iconPath= query.value(4).toString();
            info.m_widgetType = query.value(6).toString();
            info.m_uid = query.value(7).toInt();
            info.m_startPath = query.value(8).toString();
            info.m_dataPath = query.value(9).toString();
            info.m_capabilityCheck = query.value(10).toBool();
            info.m_certificateAki = query.value(11).toString();
            info.m_nativeId = query.value(12).toString();
            
            AttributeMap attributeList;
            QString key, value;
            QSqlQuery attributeQuery(db);
            attributeQuery.prepare("SELECT key,value FROM webapp_attributes WHERE id=:id");
            attributeQuery.bindValue(":id", QVariant(id));
            attributeQuery.exec();

            while (attributeQuery.next()) {
                attributeList[attributeQuery.value(0).toString()] = 
                        attributeQuery.value(1).toString();
            }

            info.m_data = attributeList;
            m_cache->saveAppInfo(appID, info);
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
QList<WebAppInfo>* WebAppRegistry::registered()
{
    return d->registeredHavingAttribute("", "", true);
}

/*!
 \return list of all already registered web apps, note that reciever responsible to free the list
*/
QList<WebAppInfo>* WebAppRegistry::present(bool isPresent)
{
    return d->registeredHavingAttribute(WEBAPPREGISTRY_ATTRIBUTE_NOTPRESENT, 
            "yes", !isPresent);
}

/*
 * List of widgets present on drive
 * @param driveLetter drive letter to check
 * @return list of widgets as WebAppInfo
 */
QList<WebAppInfo>* WebAppRegistry::presentOnDrive(const QString& driveLetter)
{
    QList<WebAppInfo>* allPresent = present(true);
    QList<WebAppInfo>* results = new QList<WebAppInfo>;
    QString driveStart(driveLetter);
#ifdef Q_OS_SYMBIAN
    driveStart.append(":");
#endif

    for (int i = 0 ; i < allPresent->count() ; ++i) {
        if ((*allPresent)[i].appPath().startsWith(driveStart, Qt::CaseInsensitive))
            results->append((*allPresent)[i]);
    }

    delete allPresent;

    return results;
}

/*
 * List of widgets not present on drive
 * @param driveLetter drive letter to check
 * @return list of widgets as WebAppInfo
 */

QList<WebAppInfo>* WebAppRegistry::notPresentOnDrive(const QString& driveLetter)
{
    QList<WebAppInfo>* allPresent = present(false);
    QList<WebAppInfo>* results = new QList<WebAppInfo>;
    QString driveStart(driveLetter);
#ifdef Q_OS_SYMBIAN
    driveStart.append(":");
#endif

    for (int i = 0 ; i < allPresent->count() ; ++i) {
        if ((*allPresent)[i].appPath().startsWith(driveStart, Qt::CaseInsensitive))
            results->append((*allPresent)[i]);
    }

    delete allPresent;

    return results;
}

bool WebAppRegistry::setIsPresent(const QString& appID, bool value) {
    bool ret(false);

    QString attribute(WEBAPPREGISTRY_ATTRIBUTE_NOTPRESENT);
    QString attributeValue(value ? "no" : "yes");

    if (isRegistered(appID)) {
        ret = setWebAppAttribute(appID, attribute, attributeValue);

#ifdef Q_OS_SYMBIAN
#ifdef QTWRT_USE_USIF
        TRAP_IGNORE(d->m_s60SCRInstance->SetIsPresentL(appID, value));
#endif
#endif
    }
    return ret;
}

#ifdef QTWRT_USE_USIF
void WebAppRegistry::setIsFullView(const QString& appID, bool value)
#else //!QTWRT_USE_USIF
void WebAppRegistry::setIsFullView(const QString& /*appID*/, bool /*value*/)
#endif
{
#ifdef Q_OS_SYMBIAN
#ifdef QTWRT_USE_USIF
        TRAP_IGNORE(d->m_s60SCRInstance->SetPropertyL(appID, SCR_PROP_ISFULLVIEW, value));
#endif
#endif
}

#ifdef QTWRT_USE_USIF
void WebAppRegistry::setIsActive(const QString& appID, bool value)
#else //!QTWRT_USE_USIF
void WebAppRegistry::setIsActive(const QString& /*appID*/, bool /*value*/)
#endif
{
#ifdef Q_OS_SYMBIAN
#ifdef QTWRT_USE_USIF
        TRAP_IGNORE(d->m_s60SCRInstance->SetPropertyL(appID, SCR_PROP_ISACTIVE, value));
#endif
#endif
}
#ifdef QTWRT_USE_USIF
void WebAppRegistry::setIsMiniView(const QString& appID, bool value)
#else
void WebAppRegistry::setIsMiniView(const QString& /*appID*/, bool /*value*/)
#endif
{
#ifdef Q_OS_SYMBIAN
#ifdef QTWRT_USE_USIF
        TRAP_IGNORE(d->m_s60SCRInstance->SetPropertyL(appID, SCR_PROP_ISMINIVIEW, value));
#endif
#endif
}

/*!
 sets the web application attribute.

 \a appID a unique ID for the webApp
 \a attribute is attribute key
 \a value is value to be set.
 \return true if successful
*/
bool WebAppRegistry::setWebAppAttribute(const QString& appID,
                                        const QString& attribute,
                                        const QVariant& value)
{
    LOG("WebAppRegistry::setWebAppAttribute -" << appID << attribute << value);

    bool ret(false);

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    WebAppInfo info;
    if (isRegistered(appID, info)) {
        QVariant presentValue = info.value(attribute);
        if (db.isOpen()) {
            if ((!value.isNull()) && (presentValue != value)) {
               QSqlQuery updateAttribute(db);
               updateAttribute.prepare("INSERT OR REPLACE INTO webapp_attributes (id, key, value) VALUES (:id, :key, :value)");
               updateAttribute.bindValue(":id", QVariant(info.m_id));
               updateAttribute.bindValue(":key", QVariant(attribute));
               updateAttribute.bindValue(":value", value);
               updateAttribute.exec();

               LOG("WebAppRegistry::setWebAppAttribute - " << updateAttribute.lastQuery());
               LOG(updateAttribute.lastError().text());
               ret = true;
            }
            ret = true;
        }
    }
    m_cache->notifyDBWrite();
    return ret;
}

QVariant WebAppRegistry::getAttribute(const QString& appID, const QString& attribute, 
const QVariant& defaultValue)
{
    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT id FROM webapp_registry WHERE appId = :appId");
        query.bindValue(":appId", QVariant(appID));
        query.exec();

        if (query.next()) {
            int id = query.value(0).toInt();

            QSqlQuery attQuery(db);
            attQuery.prepare("SELECT value FROM webapp_attributes WHERE (id=:id and key=:key)");
            attQuery.bindValue(":id", QVariant(id));
            attQuery.bindValue(":key", QVariant(attribute));
            attQuery.exec();

            if (attQuery.next()) {
                return attQuery.value(0);
            } else {
                return defaultValue;
            }
        }
    }
    return defaultValue;
}

/*!
 sets the web application version.
  \a appID a unique ID for the webApp
  \a value is value to be set.

  The newIconPath parameter is ignored in this implementation.

 \return true if successful
*/
bool WebAppRegistry::setWebAppVersion(const QString& appID,
                                            const QVariant& value,
                                            const QString& newIconPath)
{
    bool ret = setWebAppAttribute(appID, W3CSettingsKey::WIDGET_VERSION , value);

#ifdef Q_OS_SYMBIAN
    if ( ret ) {
        ret = d->m_s60Instance->setWebAppVersion(appID, value, newIconPath);
    }
#endif
    return ret;
}

QString WebAppRegistry::nativeIdToAppId(const QString& nativeID)
{
    LOG("WebAppRegistry::nativeIdToAppId() - nativeIdToAppId using WidgetRegistrationS60 API");

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        QSqlQuery query(db);

#ifdef Q_OS_SYMBIAN
        
        query.prepare("SELECT appId FROM webapp_registry WHERE uid = :uid");
        bool ok;
        int uid = nativeID.toInt(&ok, 10);

        if (!ok)
          return QString();

        query.bindValue(0, QVariant (nativeID.toInt(&ok, 10)));
#else
        query.prepare("SELECT appId FROM webapp_registry WHERE nativeId = :nativeId");
        query.bindValue(0, QVariant (nativeID));
#endif

        query.exec();
        LOG(query.lastError().text());

        if (query.next()) {
            return query.value(0).toString();
        }

        return QString();
    }

    return QString();
}

int WebAppRegistry::appIdTonativeId(const QString& appID)
{
    int rtn(0);
#ifdef Q_OS_SYMBIAN
    LOG("WebAppRegistry::nativeIdToAppId() - nativeIdToAppId using WidgetRegistrationS60 API");

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT uid FROM webapp_registry WHERE appId = :appid");
        query.bindValue(0, QVariant(appID));

        query.exec();
        LOG(query.lastError().text());

        bool ok(false);
        if (query.next()) {
            rtn =  query.value(0).toInt(&ok);
        }
        if (!ok) {
            rtn = 0;
        }
    }
#endif
    return rtn;
}

bool WebAppRegistry::setNativeId(const QString& appID,
                                 const QString& nativeID)
{
    LOG ("WebAppRegistry::setNativeId -" << appID);

    bool ret(false);
    QString attribute("nativeId");
    //    WebAppInfo info;
    if (isRegistered(appID)) {
        ret = d->updateAttribute(appID, attribute, QVariant(nativeID));
    }
    m_cache->notifyDBWrite();
    return ret;
}

bool WebAppRegistry::setUid(const QString& appID, int uid)
{
    LOG ("WebAppRegistry::setUid -" << appID);

    bool ret(false);

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    WebAppInfo info;
    if (isRegistered(appID, info)) {
        int presentValue = info.m_uid;

        if (db.isOpen()) {
            if (presentValue != uid) {
                QSqlQuery query(db);
                query.prepare("UPDATE webapp_registry SET uid = :uid WHERE appId = :appId");
                query.bindValue(":uid", uid);
                query.bindValue(":appId", QVariant(appID));
                query.exec();

                LOG("WebAppRegistry::setWebAppAttribute - "<<query.lastQuery());
                LOG(query.lastError().text());
                ret = true;
            }
            ret = true;
        }
    }
    m_cache->notifyDBWrite();
    return ret;
}

bool WebAppRegistry::setCapabilityCheck(const QString& appID, bool value)
{
    LOG ("WebAppRegistry::setcapabilityCheck -" << value);
    bool ret(false);
    QString attribute = "capabilityCheck";
    WebAppInfo info;
    if (isRegistered(appID, info)) {
        ret = d->updateAttribute(appID,attribute,value);
    }
    m_cache->notifyDBWrite();
    return ret;
}

bool WebAppRegistry::setCertificateAki(const QString& appID, const QString& value)
{
    LOG ("WebAppRegistry::setCertificateAki -" << value);

    bool ret(false);
    QString attribute = "certificateAki";
    WebAppInfo info;
    if (isRegistered(appID, info)) {
        ret = d->updateAttribute(appID,attribute,value);
    }
    m_cache->notifyDBWrite();

#ifdef QTWRT_USE_USIF
    TRAP_IGNORE(d->m_s60SCRInstance->SetIsComponentOriginVerifiedL(appID, ret));
#endif
    
    return ret;
}

/*!
  nextAvailableUid() returns the lowest non-used UID in the available ranges.
  This is a UID needed by Symbian only, but directly access the DB for used UIDs.
  only called from WidgetRegistrationS60::registerApp

  The WRT Widget Registry (platform) reserves a block of 999 UIDs to be allocated
  to widgets.  This range is split in two, with the first half being reserved for
  widgets installed on drive C, and the rest for those installed on external drives.

  We have taken the latter half of each of these ranges to be used for cWRT widgets,
  so we allocate in the ranges of KWidgetUidCWRTInternalMemoryStart through
  KWidgetUidCWRTInternalMemoryStop (inclusive) and KWidgetUidCWRTExternalMemoryStart
  through KWidgetUidCWRTExternalMemoryStop (inclusive)

  If no UID could be found or platform is not Symbian, 0 is returned
 */
int WebAppRegistry::nextAvailableUid()
{
#ifdef Q_OS_SYMBIAN
    const int KWidgetUidLowerBound = 0x200400E9;
    const int KWidgetUidUpperBound = 0x200404D1;

    // The current allocation scheme splits the range into two so that
    // in-device memory uses one range and removable memory cards use a
    // separate range.  Eventually, removable memory is probably going to
    // have to use a reallocation scheme on insertion.
    const int KWidgetUidInternalMemoryStart = KWidgetUidLowerBound;
    const int KWidgetUidExternalMemoryStart = (KWidgetUidLowerBound + KWidgetUidUpperBound + 1) 
                                                                    / 2; // half way
    const int KWidgetUidExternalMemoryStop = KWidgetUidUpperBound;

    // Additions for separation of CWRT Widget UID space from WRT Widget UID space
    //  WAC Internal widgets - 500
   // const int KWidgetUidWRTInternalMemoryStop = (KWidgetUidInternalMemoryStart + KWidgetUidExternalMemoryStart + 1) / 2;
  //  const int KWidgetUidCWRTInternalMemoryStart = KWidgetUidWRTInternalMemoryStop;
    const int KWidgetUidCWRTInternalMemoryStart = KWidgetUidInternalMemoryStart;
    const int KWidgetUidCWRTInternalMemoryStop = KWidgetUidExternalMemoryStart;

    //  WAC external widgets - 500
  //  const int KWidgetUidWRTExternalMemoryStop = (KWidgetUidExternalMemoryStart + KWidgetUidExternalMemoryStop + 1) / 2;
  //  const int KWidgetUidCWRTExternalMemoryStart = KWidgetUidWRTExternalMemoryStop;
    const int KWidgetUidCWRTExternalMemoryStart = KWidgetUidExternalMemoryStart+1;
  //  const int KWidgetUidCWRTExternalMemoryStop = KWidgetUidUpperBound + 1;
    const int KWidgetUidCWRTExternalMemoryStop = KWidgetUidExternalMemoryStop;

    QSqlDatabase db = QSqlDatabase::database("webAppConnection");

    if (db.isOpen()) {
        int nextPossibleUid = KWidgetUidCWRTInternalMemoryStart;

        QSqlQuery query(db);
        query.prepare("SELECT DISTINCT uid FROM webapp_registry WHERE uid NOTNULL ORDER BY uid ASC");
        query.exec();
        LOG(query.lastError().text());
        while (query.next()) {
            if (query.value(0).toInt() == nextPossibleUid)
                nextPossibleUid++;

            if (nextPossibleUid >= KWidgetUidCWRTExternalMemoryStop)
                return 0;

            if (nextPossibleUid == KWidgetUidCWRTInternalMemoryStop)
                nextPossibleUid = KWidgetUidCWRTExternalMemoryStart;
         }

         return nextPossibleUid;
    }
#endif

    return 0;
}

/*
 * given widget Id, return true if match found
 * @param widgetId from config.xml
 * @return true if widgetId is already registered
 */

bool WebAppRegistry::isWidgetIdRegistered(const QString& widgetId)
{
  QStringList uid = widgetIdToUniqueIdList(widgetId);

  return !uid.isEmpty();
}

/*
 * given widget Id, list all uniqueId's
 * if there is good conflict resolution policy during
 * installation, this should return only one item
 * @param widgetId from config.xml
 * @return list of matching generated uniqueId's
 */

QStringList WebAppRegistry::widgetIdToUniqueIdList(const QString& widgetId)
{
  QList<WebAppInfo>* winfo = 
          d->registeredHavingAttribute( W3CSettingsKey::WIDGET_ID, widgetId, true);

  QStringList uid;

  WebAppInfo webInfo;

  foreach(webInfo, *winfo) {
    uid.append(webInfo.appId());
  }

  delete winfo;

  return uid;
}

/*
 * given widget Id, return first uniqueId
 * @param widgetId from config.xml
 * @return first item from list of matching generated uniqueId's
 * usage: WebAppRegistry::instance()->widgetIdToUniqueId(props->plistValue(W3CSettingsKey::WIDGET_ID).toString()); 
 */

QString WebAppRegistry::widgetIdToUniqueId(const QString& widgetId)
{
  QStringList uidList = widgetIdToUniqueIdList(widgetId);

  return uidList.isEmpty() ? QString("") : uidList.first();
}

/*
 * given unique Id, return  widgetId
 * @param uniqueId that is generated
 * @return widgetId corresponding to uniqueId
 * usage:  WebAppRegistry::instance()->uniqueIdToWidgetId(props->id());
 */

QString WebAppRegistry::uniqueIdToWidgetId(const QString& uniqueId)
{
  WebAppInfo webInfo;

  QString wid;
  if( isRegistered( uniqueId, webInfo ) ) {
    wid = webInfo.value(W3CSettingsKey::WIDGET_ID).toString();
  }

  return wid;
}



