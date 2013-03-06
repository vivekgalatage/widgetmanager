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

#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QImage>
#include <QImageWriter>
#include <e32property.h>

#ifndef QTWRT_USE_USIF
#include <WidgetRegistryClient.h>
#endif

#include "wacwebappinfo.h"
#include "wacwidgetmanagerconstants.h"
#include "WidgetRegistrationS60.h"
#include "WidgetRegistrationS60Apparc.h"
#include "IconConverter.h"
#include "WidgetUtilsLogs.h"
#include "wacw3csettingskeys.h"

#include <QDateTime>
#include <QEventLoop>

#define QT_DEFAULT_ICON ":/resource/default_widget_icon.png"

//static const TInt KWidgetAppUid = 0x2003DE07;     // temporary definition, will be removed w/ multiprocess branch merge

static bool processOtherFormats( QString& newIconPath, const QString& path, 
        IconConverterWrapper& converter, QEventLoop* loop )
{
    // first try to load the svg image into QImage
    const QString tmpFile(path + ".tmp"); 
    
    if (path.endsWith(".svg", Qt::CaseInsensitive )
    	  || path.endsWith(".jpg", Qt::CaseInsensitive )
    	  || path.endsWith(".gif" , Qt::CaseInsensitive )
    	  || path.endsWith(".ico", Qt::CaseInsensitive )
          || path.endsWith(".png", Qt::CaseInsensitive)) {
        QImage image;
        if (!image.load(path)) {
            LOG("processOtherFormats loading failed for: " + path);
            return false;
        }
    

        // then try to write the opened svg image as an png
        QImageWriter writer(tmpFile, "png");
        if (writer.write(image)) {
            bool ret = false;
#ifdef QTWRT_USE_USIF
            Q_UNUSED(converter);
            Q_UNUSED(loop);
            if (QFile::copy(tmpFile, newIconPath)) {
                ret = true;
            } else {
                ret = false;
            }
#else
            // then convert the png into S60 bitmap format
            if (converter.Init(tmpFile, newIconPath)) {
                loop->exec();
                ret = true;
            } else {
                ret = false;
            }
#endif
            QFile::remove(tmpFile);
            return ret;
        }
    }    

    LOG("processOtherFormats QImageWriter::write failed");
    return false;
}

static bool processIconL(TInt32 uid, QString& newIconPath, const QString& path, const QString& basePath)
{
    LOG("processIconL path" << path);
    LOG("processIconL basePath " << basePath);
    // this should never happen as there is usually at least
    // the default icon path here
    if (path.isEmpty())
        return false;

    // 3 cases here for widget icon:
    // 1: default icon (i.e. QT_DEFAULT_ICON)
    // 2: an image file
    // 3: svg(z) image file

    // generate icon path
    newIconPath = basePath;
    if (!basePath.endsWith(QDir::separator()))
      newIconPath.append(QDir::separator());

#ifdef QTWRT_USE_USIF
    newIconPath.append('[' + QString::number( uid, 16 ) + "].png");
#else
    newIconPath.append('[' + QString::number( uid, 16 ) + "].mbm");
#endif
    LOG("processIconL newIconPath" << newIconPath << QFile::exists(newIconPath));
    QDir dir = QDir::root();
    dir.mkpath(basePath);

    bool ret = false;

    QString srcPath = path;
    if (path == QT_DEFAULT_ICON) {
        srcPath = newIconPath + ".png";
        if (QFile::exists(srcPath))
            QFile::remove(srcPath);

        if (!QFile::copy(path, srcPath))
            return false;

        // when default icon file is read only, so changing the attribute of copied icon file
        // so that, it can be writable otherwise it may cause problems while deleting/cleanup
        if (!QFile::setPermissions(srcPath, QFile::WriteOwner))
            return false;
    }

    RProcess myUID;
    TUidType uidType(myUID.Type());
    TUint32 processuid = uidType[2].iUid;
    if ((processuid == BACKUP_RESTORE_UID || processuid ==  MEDIA_MANAGER_UID) &&
        QFile::exists(newIconPath)) {
        // skip the convertion since the file is good
        LOG("processIconL skipping conversion");
        ret = true;
    } else {

        // try to convert the image into symbian specific format
        QEventLoop* loop = new QEventLoop();
        IconConverterWrapper* iconConverter = new IconConverterWrapper(loop);

#ifdef QTWRT_USE_USIF
        // if the destination icon already exists, remove it
        if (QFile::exists(newIconPath))
            QFile::remove(newIconPath);

        if (srcPath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive) && 
            QFile::copy(srcPath, newIconPath))
            ret = true;
#else
        if (iconConverter && iconConverter->Init(srcPath, newIconPath)) {
            loop->exec();
            ret = true;
        }
#endif

        if (!ret) {
            // if convert fails, check if the file was .svg
            ret = processOtherFormats( newIconPath, path, *iconConverter, loop );
        }

        delete iconConverter;
        delete loop;
    }
    return ret;
}

static void NotifyW3cWidgetAltered()
{
    const TUid KMyPropertyCat = { 0x10282E5A };
    enum TMyPropertyKeys { EMyPropertyAltered = 210 };
    TInt altered( 1 );
    RProperty::Set( KMyPropertyCat, EMyPropertyAltered , altered );
}

WidgetRegistrationS60::WidgetRegistrationS60():iSharedLib(false)
{
}

/**
* Set if a type of the app is shared lib for unregistration
* @param sharedLib - true if app to be unregister is a shared lib
* @return void
*/
void WidgetRegistrationS60::setSharedLib(bool sharedLib)
{
   iSharedLib=sharedLib;
}

bool WidgetRegistrationS60::registerApp(const QString& appId,
                                        const QString& appTitle,
                                        const QString& appPath,
                                        const QString& dataPath,
                                        const QString& iconPath,
                                        const AttributeMap& attr,
                                        const QString& type,
                                        unsigned long size,
                                        const QString& startPath,
                                        int& widgetUid,
                                        QString& convertedIconPath,
                                        bool hideIcon)
{
    bool ret(false);

    TRAPD(err, ret = registerAppL(appId, appTitle, appPath, dataPath, iconPath, attr,
                                  type, size, startPath, widgetUid, convertedIconPath, hideIcon));

    if (err)
      LOG("WidgetRegistrationS60::registerApp() - registerAppL failed, error : " << err);

    return ret;
}

#ifdef QTWRT_USE_USIF
bool WidgetRegistrationS60::registerAppL(const QString& appId,
                                         const QString& appTitle,
                                         const QString& appPath,
                                         const QString& dataPath,
                                         const QString& iconPath,
                                         const AttributeMap& attr,
                                         const QString& type,
                                         unsigned long size,
                                         const QString& startPath,
                                         int& widgetUid,
                                         QString& convertedIconPath, 
                                         bool hideIcon)
{
      (void)attr;
      (void)type;
      (void)size;
      (void)startPath;
      (void)hideIcon;
    LOG("WidgetRegistrationS60::registerAppL()" << " appId : " << appId << " appTitle : " << 
            appTitle << " appPath : " << appPath << " iconPath : " << iconPath << " startPath : " << 
            startPath << " hideIcon : " << hideIcon);

    if (appId.isEmpty() || appTitle.isEmpty() || appPath.isEmpty())
        return false;

    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs); // pushed 1
    User::LeaveIfError(rfs.ShareProtected());

    SwiUI::CWidgetRegistrationS60Apparc* appArc = SwiUI::CWidgetRegistrationS60Apparc::NewL(rfs);
    CleanupStack::PushL(appArc); // pushed 2

    // Generate our UID based on UIDs in the Qt WidgetRegistry
    int iUid = WebAppRegistry::instance()->nextAvailableUid();
    widgetUid = iUid;

    if (iUid == 0) {
        LOG("WidgetRegistrationS60::registerAppL() - netAvailableUid() failed");
        CleanupStack::PopAndDestroy(2);
        return false;
    }

    // convert icon to required format and sizes
    QString newIconPath = "";
    if (!processIconL(iUid, newIconPath, iconPath, QDir::toNativeSeparators(dataPath))) {
        LOG("WidgetRegistrationS60::registerAppL() - processIconL() failed");
        CleanupStack::PopAndDestroy(2);
        return false;
    }

    // FIXME this translation doesn't cover all cases, if generalized
    // must cover S60 WRT names and W3C names
    QString appPathNative = QDir::toNativeSeparators(appPath);
    // FIXME enforce canonicalization in caller
    // must end in QDir::separator()
    if (QDir::separator() != appPathNative.at(appPathNative.count()-1)) {
        appPathNative.append(QDir::separator());
    }

    convertedIconPath = newIconPath;

    // Record the used UID in the qt registry
    WebAppRegistry::instance()->setUid(appId, iUid);

    CleanupStack::PopAndDestroy(2); // appArc, rfs

    //Notify widget altered
    NotifyW3cWidgetAltered();

    return true;
}
#else
static void PointerArrayCleanup( TAny* aArray )
{
    static_cast<RPointerArray<CWidgetPropertyValue>*>( aArray )->ResetAndDestroy();
}
bool WidgetRegistrationS60::registerAppL(const QString& appId,
                                         const QString& appTitle,
                                         const QString& appPath,
                                         const QString& dataPath,
                                         const QString& iconPath,
                                         const AttributeMap& attr,
                                         const QString& type,
                                         unsigned long size,
                                         const QString& startPath,
                                         int& widgetUid,
                                         QString& convertedIconPath,
                                         bool hideIcon)
{
    LOG("WidgetRegistrationS60::registerAppL()" << " appId : " << appId << " appTitle : " << 
            appTitle << " appPath : " << appPath << " iconPath : " << iconPath << " startPath : " << 
            startPath);

    if (appId.isEmpty() || appTitle.isEmpty() || appPath.isEmpty())
        return false;

    // S60 requires widgetProps as CWidgetPropertyValue
    RPointerArray<CWidgetPropertyValue> propertyValues(EWidgetPropertyIdCount);
    CleanupStack::PushL(TCleanupItem(PointerArrayCleanup, &propertyValues)); // pushed 1

    RWidgetRegistryClientSession registryClient;
    User::LeaveIfError(registryClient.Connect());
    CleanupClosePushL( registryClient ); // pushed 2

    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs); // pushed 3
    User::LeaveIfError(rfs.ShareProtected());

    // empty values
    for (TInt i = 0; i < EWidgetPropertyIdCount; ++i) {
        CWidgetPropertyValue* value = CWidgetPropertyValue::NewL();
        CleanupStack::PushL(value); // pushed 4
        propertyValues.AppendL(value);
        CleanupStack::Pop(value); // pushed 3
    }

    *(propertyValues[EWidgetPropertyListVersion]) = KWidgetPropertyListVersion71CWRT;
    *(propertyValues[EFileSize]) = size;

    SwiUI::CWidgetRegistrationS60Apparc* appArc = SwiUI::CWidgetRegistrationS60Apparc::NewL(rfs);
    CleanupStack::PushL(appArc); // pushed 4

    // get drive letter from appPath
    TUint driveLetter = appPath[0].unicode();

    // Generate our UID based on UIDs in the Qt WidgetRegistry
    int iUid = WebAppRegistry::instance()->nextAvailableUid();
    widgetUid = iUid;

    if (iUid == 0) {
        LOG("WidgetRegistrationS60::registerAppL() - registryClient.GetAvailableUidL() failed");
        CleanupStack::PopAndDestroy( 4, &propertyValues );
        return false;
    }

    // convert icon to required format and sizes
    QString newIconPath = "";
    if (!processIconL(iUid, newIconPath, iconPath, QDir::toNativeSeparators(dataPath))) {
        LOG("WidgetRegistrationS60::registerAppL() - processIconL() failed");
        CleanupStack::PopAndDestroy( 4, &propertyValues );
        return false;
        }

    // FIXME this translation doesn't cover all cases, if generalized
    // must cover S60 WRT names and W3C names
    QString appPathNative = QDir::toNativeSeparators(appPath);
    // FIXME enforce canonicalization in caller
    // must end in QDir::separator()
    if (QDir::separator() != appPathNative.at(appPathNative.count()-1)) {
        appPathNative.append(QDir::separator());
    }
    TPtrC16 basePathSymbian(reinterpret_cast<const TUint16*>
                            (appPathNative.constData()));
    *(propertyValues[EBasePath]) = basePathSymbian;

    QString driveName = appPathNative.left(2);
    TPtrC16 driveNameSymbian(reinterpret_cast<const TUint16*>
                             (driveName.constData()));
    *(propertyValues[EDriveName]) = driveNameSymbian;

    TPtrC16 mainHtmlSymbian(reinterpret_cast<const TUint16*>
                            (startPath.constData()));
    *(propertyValues[EMainHTML]) = mainHtmlSymbian;

    TPtrC16 identifierSymbian(reinterpret_cast<const TUint16*>
                              (appId.constData()));
    *(propertyValues[EBundleIdentifier]) = identifierSymbian;

    if (attr.contains(W3CSettingsKey::WIDGET_VERSION)) {
        QString ver = attr.value(W3CSettingsKey::WIDGET_VERSION).toString();
        if(!(ver.isEmpty())) {
            TPtrC16 version(reinterpret_cast<const TUint16*>
                            (attr.value(W3CSettingsKey::WIDGET_VERSION).toString().constData()));
            *(propertyValues[EBundleVersion]) = version;
        }
    }

    if (appTitle.isEmpty()) {
        // FIXME this probably should cause registration failure
        *(propertyValues[EBundleDisplayName]) = identifierSymbian;
    } else {
        TPtrC16 titleSymbian(reinterpret_cast<const TUint16*>
                             (appTitle.constData()));
        *(propertyValues[EBundleDisplayName]) = titleSymbian;
    }

    // TODO: We decided to drop BundleName and just use
    // DisplayName but the registry code has errors in it and uses
    // BundleName when it should use DisplayName so as a workaround,
    // set BundleName to DisplayName.  Should eventually remove
    // BundleName from set of registry values.
    const TDesC& name = *(propertyValues[EBundleDisplayName]);
    *(propertyValues[EBundleName]) = name;

    convertedIconPath = newIconPath;

    if (!newIconPath.isEmpty()) {
        // FIXME enforce canonicalization in caller
        // strangely icon path doesn't include icon file name
        int li = newIconPath.lastIndexOf(QDir::separator());
        if (li > 0) {
            newIconPath = newIconPath.left(li+1);
            TPtrC16 iconPathSymbian(reinterpret_cast<const TUint16*>
                                    (newIconPath.constData()));
            *(propertyValues[EIconPath]) = iconPathSymbian;
        }
    }

    *(propertyValues[EUid]) = iUid;

    *(propertyValues[EMiniViewEnable]) = 0;
    if (attr.contains(W3CSettingsKey::WIDGET_VIEWMODES)) {
        QStringList viewModeList = attr.value(W3CSettingsKey::WIDGET_VIEWMODES).toString().split(" ");
        foreach (const QString &str, viewModeList) {
            if (str.contains("minimized", Qt::CaseInsensitive)) {
                *(propertyValues[EMiniViewEnable]) = 1;
                break;
            }
        }
    }

    // FIXME
    *(propertyValues[EAllowNetworkAccess]) = 1;

    //For WIDGET_PACKAGE_FORMAT_WGZ) type = 1, JIL widgets have type same as WGT(2).
    if (type == WIDGET_PACKAGE_FORMAT_WGT || type == WIDGET_PACKAGE_FORMAT_JIL) {
          *(propertyValues[ENokiaWidget]) = 2;
    }
    else if (type == WIDGET_PACKAGE_FORMAT_SHARED_LIBRARY) {
          *(propertyValues[ENokiaWidget]) = 3;
    }

    // Set EProcessUid to constant process UID.  To be changed with
    // multi-process updates

    bool ok;
    *(propertyValues[EProcessUid]) = appPath.section('\\',2,2).toInt(&ok,16);

    QString mimeType(CONTENT_TYPE_WGT);
    TPtrC16 mimeTypeSymbian(reinterpret_cast<const TUint16*>
                            (mimeType.constData()));
    *(propertyValues[EMimeType]) = mimeTypeSymbian;

    registryClient.RegisterWidgetL( propertyValues );
    LOG("WidgetRegistrationS60::registerAppL() - registryClient.RegisterWidgetL() done");

    // Record the used UID in the qt registry
    WebAppRegistry::instance()->setUid(appId, iUid);

    // if widget is a shared library and not a launchable widget then make sure it is not registered with app arc
    if (type == WIDGET_PACKAGE_FORMAT_SHARED_LIBRARY) {
        LOG("WidgetRegistrationS60::registerAppL() - appArc->DeregisterWidgetL()");
        appArc->DeregisterWidgetL(TUid::Uid( *(propertyValues[EUid])));
    } else {
        LOG("WidgetRegistrationS60::registerAppL() - appArc->RegisterWidgetL()");
        appArc->RegisterWidgetL(*(propertyValues[EMainHTML]),
                                *(propertyValues[EBundleDisplayName]),
                                *(propertyValues[EIconPath]),
                                *(propertyValues[EDriveName]),
                                TUid::Uid( *(propertyValues[EUid])),
                                hideIcon);
    }

    CleanupStack::PopAndDestroy(4); // appArc, rfs, registryClient, propertyValues

    //Notify widget altered
    NotifyW3cWidgetAltered();

// FIXME
//        HandleLogsL(*(propertyValues[EBundleDisplayName]), TUid::Uid( *(propertyValues[EUid]) ), *(propertyValues[ENokiaWidget]), SwiUI::ELogTaskActionInstall);

    return true;
}
#endif

bool WidgetRegistrationS60::unregister(const QString& appID, bool update)
{
    bool ret(false);

    TRAPD(err, ret = unregisterL(appID, update));
    if (err != KErrNone)
      LOG("WidgetRegistrationS60::unregister() - unregisterL failed, error : " << err);

    return ret;
}

#ifdef QTWRT_USE_USIF
bool WidgetRegistrationS60::unregisterL(const QString& appID, bool update)
{
    LOG("WidgetRegistrationS60::unregisterL()");
    if (appID.isEmpty())
        return false;

    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs); // pushed 1

    User::LeaveIfError(rfs.ShareProtected());

    WebAppInfo info;
    if (!WebAppRegistry::instance()->isRegistered(appID, info)) {
        CleanupStack::PopAndDestroy(); // rfs
        return false;
    }

    TUid uid;
    uid.iUid = info.m_uid;

    // Don't close widget if it is widget partial update
    class SwiUI::CWidgetRegistrationS60Apparc* appArc = SwiUI::CWidgetRegistrationS60Apparc::NewL(rfs);
    CleanupStack::PushL(appArc); // pushed 2

    // Shared libs are not registered in s60 registry.
    if (!iSharedLib){
        if (!update) {
            LOG("WidgetRegistrationS60::unregisterL() UID:" << uid.iUid );
            appArc->closeWidgetL(uid);
        }

        // No longer in 10.1
        // appArc->DeregisterWidgetL(uid);
    }

    CleanupStack::PopAndDestroy(2); // appArc, rfs

    NotifyW3cWidgetAltered();

    return true;
}
#else
bool WidgetRegistrationS60::unregisterL(const QString& appID, bool update)
{
    LOG("WidgetRegistrationS60::unregisterL()");
    if (appID.isEmpty())
        return false;

    RWidgetRegistryClientSession registryClient;
    User::LeaveIfError(registryClient.Connect());
    CleanupClosePushL(registryClient); // pushed 1

    RFs rfs;
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs); // pushed 2

    User::LeaveIfError(rfs.ShareProtected());

    // FIXME this is incompelte, see S60 WRT version
    TPtrC16 identifierSymbian(reinterpret_cast<const TUint16*>
                              (appID.constData()));
    TUid uid;
    uid.iUid = registryClient.GetWidgetUidL(identifierSymbian);

    // Don't close widget if it is widget partial update
    class SwiUI::CWidgetRegistrationS60Apparc* appArc = SwiUI::CWidgetRegistrationS60Apparc::NewL(rfs);
    CleanupStack::PushL(appArc); // pushed 3

    // Shared libs are not registered in s60 registry.
    if (!iSharedLib){
        if (!update) {
            LOG("WidgetRegistrationS60::unregisterL() UID:" << uid.iUid);
            appArc->closeWidgetL(uid);
        }

        appArc->DeregisterWidgetL(uid);
    }

    registryClient.DeRegisterWidgetL(uid);

    CleanupStack::PopAndDestroy(3); // appArc, rfs, registryClient

    NotifyW3cWidgetAltered();

// FIXME
//     HandleLogsL(bundleName, aUid, aVendor, SwiUI::ELogTaskActionUninstall);

    return true;
}
#endif

static unsigned long dirSize(const QString& path)
{
    unsigned long size = 0;
    unsigned long sizeTotal = 0;
    QDir dir(path);
    dir.setFilter( QDir::Dirs | QDir::Files | QDir::NoSymLinks );
    if (dir.exists()) {
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            QString cat = fileInfo.fileName();
            if ( ( cat!="." ) && ( cat!=".." ) ) {
                if ( fileInfo.isDir() ) {
                    size = dirSize(path + '/' + cat);
                } else {
                    size = (unsigned long)fileInfo.size();
                }
                sizeTotal = sizeTotal + size;
            }
        }
    }
    return sizeTotal;
}

bool WidgetRegistrationS60::setWebAppAttribute(const QString& appID,
                                               const QString& attribute,
                                               const QVariant& value)
{
    bool ret(false);
    TRAPD(err, ret = setWebAppAttributeL(appID, attribute, value));
    LOG("WidgetRegistrationS60::setWebAppAttribute() - setWebAppAttributeL failed, error : " << err);

    return ret;
}

bool WidgetRegistrationS60::setWebAppAttributeL(const QString& appID,
                                                const QString& attribute,
                                                const QVariant& value)
{
    WebAppInfo appInfo;
    if (attribute != W3CSettingsKey::WIDGET_VERSION) {
        return false;
    }

    if (!WebAppRegistry::instance()->isRegistered(appID, appInfo)) {
       return false;
    }

    QString type = appInfo.m_widgetType;
    QString startPath = appInfo.m_startPath;
    QString convertedIconPath;

    if (!appInfo.m_data.contains(attribute)) {
       return false;
    }
    appInfo.m_data.insert(attribute, value);

    if (!unregister(appID)) {
       return false;
    }
    int widgetUid = 0;
    return registerAppL(appID,
                        appInfo.appTitle(),
                        appInfo.appPath(),
                        appInfo.dataPath(),
                        appInfo.iconPath(),
                        appInfo.attributes(),
                        type,
                        dirSize(appInfo.appPath()),
                        startPath,
                        widgetUid ,
                        convertedIconPath );
}

bool WidgetRegistrationS60::setWebAppVersion(const QString& appID,
                                             const QVariant& value,
                                             const QString& newIconPath)
{
    bool ret(false);
    TRAPD(err, ret = setWebAppVersionL(appID, value, newIconPath));
    LOG("WidgetRegistrationS60::setWebAppVersion() - setWebAppVersionL failed, error : " << err);

    return ret;
}

bool WidgetRegistrationS60::setWebAppVersionL(const QString& appID,
                                              const QVariant& value,
                                              const QString& newIconPath)
{
    const QString widgetVersion = W3CSettingsKey::WIDGET_VERSION;
    QString convertedIconPath;

    WebAppInfo appInfo;
    if (!WebAppRegistry::instance()->isRegistered(appID, appInfo)) {
       return false;
    }

    if (!appInfo.m_data.contains(widgetVersion)) {
       return false;
    }
    appInfo.m_data.insert(widgetVersion, value);

    if (!unregisterL(appID, true)) {
       return false;
    }

    QString appPath = appInfo.appPath();
    if ( appPath.endsWith(QDir::separator()) ) {
        appPath.chop(1);
    }

    QString type = appInfo.m_widgetType;
    QString startPath = appInfo.m_startPath;

    int widgetUid = 0;
    return registerAppL(appID,
                        appInfo.appTitle(),
                        appPath,
                        appInfo.dataPath(),
                        newIconPath,
                        appInfo.attributes(),
                        type,
                        dirSize(appInfo.appPath()),
                        startPath,
                        widgetUid ,
                        convertedIconPath );
}
