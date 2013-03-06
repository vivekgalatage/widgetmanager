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

#include "wacSuperWidget.h"
#include "WgtWidget.h"

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
#include "WgzWidget.h"
#endif

#include "signatureparser.h"
#include "DigSigService.h"
#include "w3cxmlplugin.h"
#include "WidgetPListParser.h"
#include "wacsecuritymanager.h"
#include "wacwebappinfo.h"
#include "wacsettings.h"
#include "WidgetUtilsLogs.h"
#include "wacw3csettingskeys.h"
#include "proprietarysettingskeys.h"
#include "widgetinstaller.h"
#include "featuremapping.h"
#include "webapplocalizer.h"
#include "webappuniquenesshelper.h"
#include "desktoputils.h"

#include <QRegExp>
#include <QStringList>
#include <QDir>
#include <QTextDocument>
#include <QFile>
#include <QIODevice>
#include <QHash>
#include <QUuid>
#include <QDateTime>
#include <QDirIterator>


#ifdef Q_OS_SYMBIAN


#ifdef QTWRT_USE_USIF

#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/scr/appreginfo.h>
#include "SCRConstants.h"
_LIT(KUid, "Uid");
_LIT( KLauncherApp, "wgtwidgetlauncher.exe" );

using namespace Usif;


#else
#include <WidgetRegistryClient.h>
#include <WidgetRegistryConstants.h>
#endif

#include <apgcli.h>
#include <apacmdln.h>
#include <S32MEM.H>
#include <f32file.h>
#include "WidgetUnzipUtilityS60.h"
#define PROCESS_UID "processUid"
#endif

#include "wbenchmark.h"
#include "wacsecmgr.h"

const QString TrustedDomain("TrustedWidgets");
const QString OperatorDomain("Operator");

QHash<QString, QString> WgtWidget::s_widgetStartFileCache;

WgtWidget::WgtWidget(QString& rootDirectory)
 : m_disableUnsignWidgetSignCheck(false)
{
    initialize(rootDirectory);
    m_widgetType = WidgetTypeW3c;
    m_contentType = CONTENT_TYPE_WGT;
    m_validCertificateAKI = QString();
    m_appLocalizer = new WebAppLocalizer();
    m_uniquenessHelper = new WebAppUniquenessHelper(m_installDir);
}

WgtWidget::~WgtWidget()
{
    delete m_appLocalizer;
    delete m_uniquenessHelper;
    s_widgetStartFileCache.clear();
}

// function to get the size of the directory
//
// return:
//  unsigned long Total size of the directory
//
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

//
// function to parse widget manifest (like info.plist or config.xml)
// prereq:    should have set proper WidgetUnZipPath to find the widget contents
// parameters:
//    path    path to bundle
//    force   after first parse m_manifest holds manifest info.
//            if you want to force parsing each time, set this to true.
//            default = false
// return:
//    bool    true if parsing successful
//            false if failure
//
bool WgtWidget::parseManifest(const QString& path, const bool minimal)
{
    bool status = true;
    if (m_manifest) {
        // Manifest already parsed and not forcing a re-parse
        return status;
    }

    m_manifest = new W3cXmlPlugin;

    if (path.isEmpty()) {
        m_manifest->setdir(widgetUnZipPath());
    }
    else {
#ifdef Q_OS_SYMBIAN
        // Check if the passed path is a file.  If it is, process the contents of config.xml from
        // within the widget bundle
        if (QFileInfo(path).isFile()) {
            QString contents = WidgetUnzipUtilityS60::fileContents(path, "config.xml", 
                    Qt::CaseSensitive);
            status = m_manifest->processContents(contents);
            if (status) {
                if (!minimal)
                    widgetProperties();
                else
                    minimalWidgetProperties();
            }
            return status;
        }
#endif
        LOG("WgtWidget::parseManifest path=" << path);
        m_manifest->setdir(path);
    }

    status = m_manifest->process();
    if (status) {
        if (!minimal)
            widgetProperties();
        else
            minimalWidgetProperties();
    }

    return status;
}

//
// function to indicate if widget should be allowed to be installed on
// removable media
//
// prereq:   should parseManifest
//
// return:
//   bool    true if widget installation could be allowed on removable media
//           false if installation on removable media should be prevented
bool WgtWidget::allowRemovableInstallation()
{
    WidgetProperties* props = minimalWidgetProperties();

    if (props->isSharedLibrary()) {
        return false;
    } else {
        return true;
    }
}

// function to find widget start file
// parameters:
//    startFile    returns a string that specifies startFile of widget
//    path         path to widget files
// return:
//    bool         true if start file found
//                 false if start file is not found
//
bool WgtWidget::findStartFile(QString& startFile, const QString& path)
{
    // Most widgets (not all) call findStartFile numerous times. Check if we
    // already found the start file, this eliminates searching the
    // file system, which takes TOO MUCH TIME!
    bool savedStartFile = getStartFile(startFile, path);
    if (savedStartFile) {
        return true;
    }

    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo();
    if (!manifestinfo) {
        return false;
    }

    // If can't locate, try some default filenames.
    QString widgetPath(path);
    if (path.isEmpty()) {
        widgetPath = widgetRootPath();
    }

    QString contentSrc = manifestinfo->value(W3CSettingsKey::WIDGET_CONTENT, QString("src"));
    startFile = WebAppLocalizer::findStartFile(contentSrc, widgetPath);
    if (!startFile.isEmpty()) {
        // Start file found. Save it to cache.
        saveStartFile(startFile, widgetPath);
        return true;
    }

    return false;
}

//
// function to find widget icon files
// parameters:
//    icons    returns a stringlist that specifies icons of widget
//    path     path to widget files
// return:
//    bool     true if icon found
//             false if icon is not found
//
bool WgtWidget::findIcons(QStringList& icons, const QString& path)
{
    QString widgetPath(path);
    LOG("WgtWidget::findIcons ....");

    if (path.isEmpty()) {
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
        if (widgetRootPath().isEmpty())
        {
            widgetPath = widgetUnZipPath();
        }
        else
        {
            widgetPath=widgetRootPath();
        }
#else
        widgetPath=widgetRootPath();
#endif
    }
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    QString absPath = widgetUnZipPath();
    if (!widgetRootPath().isEmpty())
    {
        absPath = widgetRootPath();
    }
#else
    QString absPath = widgetRootPath();
#endif
    if (!absPath.endsWith(QDir::separator())) {
        absPath = absPath+QDir::separator();
    }

    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo();
    if (!manifestinfo) {
        return false ;
    }

    QStringList iconFiles;
    int count = manifestinfo->count(W3CSettingsKey::WIDGET_ICON);

    for (int iter = 1; iter <= count; ++iter) {
        QString icon = manifestinfo->value(W3CSettingsKey::WIDGET_ICON, iter, 
                QString("src"));
        if (!icon.isEmpty()) {
            //icons.append(icon);
            LOG("Found icon" << icon);

#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
              if ( icon.endsWith(".png",Qt::CaseInsensitive) 
                 || icon.endsWith(".jpg", Qt::CaseInsensitive)
                 || icon.endsWith(".gif", Qt::CaseInsensitive)
                 || icon.endsWith(".svg", Qt::CaseInsensitive)
                 || icon.endsWith(".ico", Qt::CaseInsensitive)
               ) {
               // Symbian and Maemo can only handle png
#endif
                iconFiles.append(icon);
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
            }
#endif
        }
    }

    iconFiles.append("icon.png");
    iconFiles.append("icon.gif");
    iconFiles.append("icon.ico");
    iconFiles.append("icon.svg");
    iconFiles.append("icon.jpeg");
    iconFiles.append("icon.jpg");
#if !defined(Q_OS_MAEMO5) && !defined(Q_OS_MAEMO6) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
    iconFiles.append("icon.gif");
    iconFiles.append("icon.ico");
    iconFiles.append("icon.svg");
    iconFiles.append("icon.jpeg");
    iconFiles.append("icon.png");
#endif

    icons.clear();

    foreach (const QString &fileName, iconFiles) {
        QString locFile = WebAppLocalizer::getLocalizedFile(fileName, 
                false, absPath);
        if (!locFile.isEmpty() && QFile::exists(widgetPath + QDir::separator() + 
                locFile)) {
            LOG("Localized icon: " << locFile);
            icons.append(QDir::toNativeSeparators(locFile));
        }
    }

    return icons.count();
}

//
// function to generate Manifest
// prereq:    should parseManifest before calling writeManifest()
//
// parameters:
//    path    path to write the manifest. Default is /tmp/config.xml
//
void WgtWidget:: writeManifest(const QString& path)
{
    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo() ;
    if (!manifestinfo)
        return ;

    manifestinfo->WriteManifestFile(path) ;
}

//
// function to get values from dictionary
// prereq:      should parseManifest before calling value()
//
// parameters:
//    key       key to query the dictionary
// return:
//    QString   value matching key from dictionary
//
QString WgtWidget::value(const QString& key, const QString & attribute)
{
    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo();
    if (!manifestinfo)   {
        return "";
    }

    // TODO : This is copy-paste code. Should be on constructor and only once.
    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();
    QString myLang = m_settings->valueAsString("UserAgentLanguage");
    if (myLang.isEmpty() || myLang.compare(" ") == 0) {
        QLocale language = QLocale::system();
        myLang = language.name().toLower();
        myLang.replace(QString("_"),QString("-"));
    }

    QString val;
    if (!key.compare(W3CSettingsKey::WIDGET_NAME))
        val = manifestinfo->value(key, attribute, W3CSettingsKey::WIDGET_LANGUAGE_KEY);
    else {
        val = manifestinfo->value(key, attribute, myLang);
        if (val.isEmpty())
            val = manifestinfo->value(key, attribute, "");
    }
    return val;
}

bool WgtWidget::contains( const QString & key, const QString & attribute ) {
    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo();
    if (!manifestinfo)   {
        return false;
    }

    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();
    QString myLang = m_settings->valueAsString("UserAgentLanguage");
    if (myLang.isEmpty() || myLang.compare(" ") == 0) {
        QLocale language = QLocale::system();
        myLang = language.name().toLower();
        myLang.replace(QString("_"),QString("-"));
    }

    if (attribute.isEmpty()) {
        bool hasElement = manifestinfo->getDictionary().contains(myLang +
                '/' + key);
        if (!hasElement) {
            hasElement = manifestinfo->getDictionary().contains(key);
        }
        return hasElement;
    } else {
        QVariant val = manifestinfo->value(key, attribute, myLang);
        if (!val.isNull()) {
            return true;
        } else {
            val = manifestinfo->value(myLang + '/' + key, attribute, myLang);
            return !val.isNull();
        }

    }
}

//
// function to get widgetProperties
// prereq: should parseManifest before calling ()
//
// params:
//   force: if true don't return cached m_widgetProperties, recompute
//   minimal: if true don't compute unique widget ID or icons list
//            (used prior to .wgt unbundling)
//
// return:
//    WidgetProperties    specific widget properties required for WebAppRegistry
//                        if you want more,just use value()
//
WidgetProperties* WgtWidget::widgetProperties(bool forceUpdate, bool minimal)
{
    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo();
    if (!manifestinfo) {
        // No manifestInfo, probably didn't call parseManifest() before this
        return 0;
    }

    if (m_widgetPropertiesCacheValid && (m_widgetProperties != 0) && !forceUpdate) {
        return m_widgetProperties;
    }

    LOG("Getting Widget Properties");

    WidgetProperties *props;

    if (!m_widgetProperties) {
        m_widgetProperties = new WidgetProperties;
    }
    props = m_widgetProperties;

    //props->setId(value(W3CSettingsKey::WIDGET_ID));
    QString widgetid = manifestinfo->value(W3CSettingsKey::WIDGET_ID );
    QString widgetType = manifestinfo->value(ProprietarySettingsKey::WIDGET_TYPE);

    /**
     *  1) Try to: set id for widget from config.xml id field
     *  2) Set QUid
     *
     *  This avoid widgets installation failure in following case:
     *  - Install a widget which does not have id in field config.xml (id will be "", this hashed to zero)
     *  - Install a second widget that does not have id field in config.xml (install of second widget failed)
     *
     */
    if (widgetid.isEmpty()) {
      widgetid = QUuid::createUuid().toString();
    }

    if (!minimal) {
        QString certId("");
        getCertificateAKI(certId);
        QString uniqueId("");
        // If widget is already installed, don't generate uniqueId
        if (WebAppUniquenessHelper::getUniqueNameFromPath(widgetRootPath(), 
                uniqueId)) {
            WebAppInfo appInfo;
            if (WebAppRegistry::instance()->isRegistered(uniqueId, appInfo)) {
                // grab it from registry
                uniqueId = appInfo.appId();
            }
            else
                 uniqueId.clear();
        }

        if (uniqueId.isEmpty() || uniqueId.isNull()) {
            // if it is not already installed, generate uniqueId
            uniqueId = m_uniquenessHelper->generateUniqueWebAppId(widgetid, certId, 
                    getTrustDomain());
        }
        props->setId(uniqueId);
    }

    // if this is a shared library and not a launchable widget set the type to
    // shared library to prevent this from being confused as a launchable widget
    props->setSharedLibrary(!manifestinfo->value(
            ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_FOLDER).isEmpty());
    props->setSharedLibraryWidget(manifestinfo->value(
            ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_WIDGET).startsWith('T',
                    Qt::CaseInsensitive));

    if (props->isSharedLibrary() && !props->isSharedLibraryWidget())
        props->setType(WIDGET_PACKAGE_FORMAT_SHARED_LIBRARY);
    else if (!widgetType.isEmpty() && !widgetType.compare(WIDGET_PACKAGE_FORMAT_JIL))
        props->setType(WIDGET_PACKAGE_FORMAT_JIL);
    else
        props->setType(WIDGET_PACKAGE_FORMAT_WGT);

    WidgetFeatures features;
    if (findFeatures(features)) {
        QList<QString> required_capabilities;
        QList<QString> optional_capabilities;
        if (m_mapping.getCapabilities(features,required_capabilities,optional_capabilities)) {
            if (required_capabilities.contains(CAPABILITY_OVI)) {
                props->setType(WIDGET_PACKAGE_FORMAT_OVIAPP);
            }
        }
    }

    if (props->isSharedLibrary()) {
        QString path(m_sharedLibPath);
        path.append(manifestinfo->value(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_FOLDER).toLower());
        props->setInstallPath(path);


        // Shared libraries have hideIcon = true
        props->setHideIcon(true);

    } else {
        QString id = props->id();
        props->setInstallPath(widgetInstallPath() + QDir::separator() + id + QDir::separator());
        props->setHiddenWidget(manifestinfo->value(
                ProprietarySettingsKey::WIDGET_HIDDEN).startsWith('T',Qt::CaseInsensitive));
    }

    props->setSource(widgetBundlePath());
    props->setInfoPList(getWidgetManifestInfo()->getDictionary());
    props->setSize(getSize());
    props->setResourcePath(SuperWidget::resourcePath(props->installPath()));
    props->setAllowBackgroundTimers(manifestinfo->value(
              ProprietarySettingsKey::WIDGET_BACKGROUND_TIMERS).startsWith("neversuspend",Qt::CaseInsensitive));

    props->setMinimizedSize(QSize( manifestinfo->value(
            ProprietarySettingsKey::WIDGET_VIEWMODE_MINIMIZED_SETTINGS,"width").toInt(),
                                   manifestinfo->value(
            ProprietarySettingsKey::WIDGET_VIEWMODE_MINIMIZED_SETTINGS,"height").toInt()) );
    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();
    QString myLang = m_settings->valueAsString("UserAgentLanguage");
    if (myLang.isEmpty() || myLang.compare(" ") == 0) {
        QLocale language = QLocale::system();
        myLang = language.name().toLower();
        myLang.replace(QString("_"),QString("-"));
    }

    QString myTitle = manifestinfo->value(W3CSettingsKey::WIDGET_NAME, QString(""), myLang);
    if (!myLang.isEmpty()) {
        if (!myTitle.isEmpty()) {
            props->setTitle(myTitle);
        }
    }

    if (myLang.isEmpty() || myTitle.isEmpty()) {
        QString wName = manifestinfo->value(W3CSettingsKey::WIDGET_NAME);
        if (wName.isEmpty()) {
            wName = "NoName";
        }
        props->setTitle(wName);
    }

    QString myTitleDir = manifestinfo->value(W3CSettingsKey::WIDGET_NAME, QString("its:dir"));
    if (!myTitle.isEmpty() && !myTitleDir.isEmpty() && isDirectionValid(myTitleDir)) {
        props->setTitleDir(myTitleDir);
    }

    if (!minimal) {
        QStringList icons;
        if (findIcons(icons)) {
            QString iconPath = props->installPath() + icons.at(0);
            if (!icons.at(0).startsWith(QDir::separator()) && 
                    !props->installPath().endsWith(QDir::separator())) {
                iconPath = props->installPath()+QDir::separator()+icons.at(0);
            }
            if (icons.at(0).startsWith(QDir::separator()) && 
                    props->installPath().endsWith(QDir::separator())) {
                iconPath = props->installPath();
                iconPath.chop(1);
                iconPath.append(icons.at(0));
            }
            props->setIconPath(iconPath);
        } else {
            LOG("No icons found, using the default one");
            props->setIconPath(":/resource/default_widget_icon.png");
        }
    } else {
        props->setIconPath(":/resource/default_widget_icon.png");
    }

    LOG("Icon path = " << props->iconPath());

    QString myDesc    = manifestinfo->value(W3CSettingsKey::WIDGET_DESCRIPTION, 
            QString(""), myLang);
    QString myDescDir = manifestinfo->value(W3CSettingsKey::WIDGET_DESCRIPTION, 
            QString("its:dir"));
    if (!myDesc.isEmpty() && !myDescDir.isEmpty() && isDirectionValid(myDescDir)) {
        props->setDescriptionDir(myDescDir);
    }

    QString myAuthor    = manifestinfo->value(W3CSettingsKey::WIDGET_AUTHOR, 
            QString(""), myLang);
    QString myAuthorDir = manifestinfo->value(W3CSettingsKey::WIDGET_AUTHOR, 
            QString("its:dir"));
    if (!myAuthor.isEmpty() && !myAuthorDir.isEmpty() && isDirectionValid(myAuthorDir)) {
        props->setAuthorDir(myAuthorDir);
    }

    QString myLic    = manifestinfo->value(W3CSettingsKey::WIDGET_LICENSE, 
            QString(""), myLang);
    QString myLicDir = manifestinfo->value(W3CSettingsKey::WIDGET_LICENSE, 
            QString("its:dir"));
    if (!myLic.isEmpty() && !myLicDir.isEmpty() && isDirectionValid(myLicDir)) {
        props->setLicenseDir(myLicDir);
    }

    if (!minimal)
        m_widgetPropertiesCacheValid = true;

    LOG("setting widget properties");

    return props;
}

//
// function to get subset of widgetProperties
// prereq: should parseManifest before calling ()
//
// return:
//    WidgetProperties    specific widget properties required for WebAppRegistry
//                        if you want more,just use value()
//
WidgetProperties* WgtWidget::minimalWidgetProperties(bool forceUpdate)
{
    return widgetProperties(forceUpdate, true);
}

//
// function to install widget
//
// parameter:
//    silent   true for silent install, false otherwise
//    QWidget* pointer to parent widget
// return:
//    bool     true if successful
//
WidgetInstallError WgtWidget::install(bool update) {
    return install(QString(), update);
}


WidgetInstallError WgtWidget::install(const QString& sigId, bool update)
{
    m_continueInstallation = false;

    // unzip widget bundle
    QString bundlePath = widgetBundlePath();
    
#if defined(Q_OS_SYMBIAN)
    // parse manifest, minimal
    if (!parseManifest(bundlePath, true))
    {
        emit installationError(WidgetParseManifestFailed);
        cleanup(WidgetParseManifestFailed);
        return WidgetParseManifestFailed;
    }

    unsigned long spaceRequired = uncompressedSize(bundlePath) * 2;
    m_size = spaceRequired;
    m_continueInstallation = false;
    emit queryInstallationDestination(spaceRequired, allowRemovableInstallation());
    if (!m_continueInstallation) {
        return WidgetUserConfirmFailed;
    }

    m_continueInstallation = false;
    emit queryConfirmation();
    if (!m_continueInstallation) {
        emit installationError(WidgetUserConfirmFailed);
        return WidgetUserConfirmFailed;
    }

    // force updating of widgetPropreties per selected installation destination
    minimalWidgetProperties(true);

    setWidgetUnZipPath(widgetInstallPath()+QDir::separator()+"widgetUnZipPath");
    
#endif

    // Maemo 5 unzips the widget under /home/user/.cache/<timestamp>/
    // most likely similar decision needs to be taken on Maemo 6 too
    // /home has roughly 2 gigabytes of free space by default thus we
    //

#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    // add some randomness in to unzip path so that we won't unzip multiple
    // widgets in to the same directory if multiple installations are happening
    // at the same time
    QString suffix("%1/unzip");
    qsrand(QTime::currentTime().msec());
    suffix = suffix.arg(qrand());
#if USE(AEGIS)
    // Maemo 6 uses an aegisfs mountpoint <widgetUnzipPath>/wrt to protect unzipped widget content
    setWidgetUnZipPath(widgetUnZipPath() +"wrt/"+ suffix);
#else
    setWidgetUnZipPath(widgetUnZipPath() + suffix);
#endif
#endif

#if !defined(Q_OS_MAEMO5) && !defined(Q_OS_MAEMO6) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
    setWidgetRootPath(widgetUnZipPath());
#endif

    bool removed = false;

    emit installProgress(10);
    if (QFileInfo(bundlePath).isFile()) {

        // make sure there's enough free disk space
        if (!checkDiskSpaceForInstallation(bundlePath)) {
            emit installationError(WidgetInsufficientDiskSpace);
            cleanup(WidgetInsufficientDiskSpace);
            return WidgetInsufficientDiskSpace;
        }

        // unzip widget bundle
        WBM_BEGIN("unzip", unzip);
        if (!unZipBundle(bundlePath)) {
            emit installationError(WidgetUnZipBundleFailed);
            cleanup(WidgetUnZipBundleFailed);
            return WidgetUnZipBundleFailed;
        }
        WBM_END(unzip);
    }
    else {

        // we have an unzipped widget folder
#ifdef Q_OS_SYMBIAN
#ifdef QTWRT_USE_USIF
        WebAppRegistry* war = WebAppRegistry::instance();
        WebAppInfo webInfo;
        removed = war->isRegistered(m_widgetProperties->id(),webInfo) && !webInfo.isPresent();

        // Media Monitor can uninstall/install widgets when a memory card is inserted or removed
        // installation should proceed when a widget is NOT present AND registered
        if ( !removed ) {
#endif
            // for security reasons, any process other than backup-restore should not be
            // allowed to install widget from an unzipped folder.  the only exception is
            // MediaMonitor handling the insertion or removal of a drive
            RProcess me;
            TUidType uidType(me.Type());
            TUint32 uid3 = uidType[2].iUid;
            LOG("UID3 of this process : " << uid3);
            if (uid3 != BACKUP_RESTORE_UID && uid3 != MEDIA_MANAGER_UID) {
                LOG("WgtWidget::install()-Abort.This is not Backup-Restore/MediaManager process");
                emit installationError(WidgetInstallPermissionFailed);
                return WidgetInstallPermissionFailed;
            }
        //Set Widget size
        unsigned long size = dirSize(bundlePath);
        setSize(size);
#ifdef QTWRT_USE_USIF
        } // if (!removed)
#endif
#endif

        setWidgetUnZipPath(bundlePath);
#if !defined(Q_OS_MAEMO5) && !defined(Q_OS_MAEMO6) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
    setWidgetRootPath(widgetUnZipPath());
#endif

    }
    // minimal parse manifest so we can access widgetProperties to set certificate AKIs later
    if (!parseManifest("", true)) {
        emit installationError(WidgetParseManifestFailed);
        cleanup(WidgetParseManifestFailed,removed);
        return WidgetParseManifestFailed;
    }
    emit installProgress(20);

    WidgetInstallError errorCode = WidgetInstallSuccess;

    QDir widgetDir(widgetUnZipPath());
    //widgetDir.setPath(widgetUnZipPath());
    QString widgetResourceDir = widgetDir.absolutePath();

    WBM_BEGIN("digital signature validation", digsig);
    WidgetValidationError validationError(ValidateOK);
    QString commaSeparatedAKIs;
    if (sigId.isNull() || sigId.isEmpty()) {
        // Widget which has digital signatures must have valid dig sigs.
        DigSigService* dsService = new DigSigService();
        validationError = dsService->validateWidget(widgetResourceDir);

        if (validationError != CertificateRevoked) {
            m_validCertificateAKI = dsService->getCertificateAKI();
            QStringList certificateAKIs = dsService->getCertificateAKIs();
            if (!certificateAKIs.isEmpty()) {
                commaSeparatedAKIs = certificateAKIs.join(",");
            }
        }
        delete dsService;
    } else {
        m_validCertificateAKI = sigId;
        commaSeparatedAKIs = sigId;
    }
    WBM_END(digsig);

    if (validationError == CertificateRevoked) {
        LOG("validation error: " << validationError);
        m_continueInstallation = false;
		emit installationError(WidgetSignatureValidationFailed);
		return WidgetSignatureValidationFailed;
    } 
    if( true ){
        // If m_disableUnsignWidgetSignCheck is true and if widget is an unsigned widget than installation should be continued.
        // Silent mode is signaled back from WidgetManager.cpp.
#ifdef BLOCK_UNSIGNED_WIDGETS
        emit installationError(WidgetSignatureValidationFailed);
#else
        // TODO: merge WidgetInstallError and WidgetValidationError
        if (validationError <= 7) {
            errorCode = (WidgetInstallError) validationError;
        } else if (validationError > 7 && validationError < 14) {
            errorCode = WidgetCertValidationFailed;
        } else if (validationError >= 14) {
            errorCode = WidgetSignatureValidationFailed;
        }
        emit aboutToInstallUntrustedWidget(errorCode);
#endif
        if (!m_continueInstallation && !m_disableUnsignWidgetSignCheck) {
            cleanup(WidgetSignatureValidationFailed,removed);
            emit installationError(WidgetUserConfirmFailed);
            return WidgetSignatureValidationFailed;
        }
    } else {
        errorCode = WidgetValidSignature;
    }

    WidgetProperties* props = minimalWidgetProperties();
    props->setCertificateAki(commaSeparatedAKIs);

    emit installProgress(30);
    
    WBM_BEGIN("reading config.xml", configxml);
    // parse manifest
    if (!parseManifest())
    {
        emit installationError(WidgetParseManifestFailed);
        cleanup(WidgetParseManifestFailed,removed);
        return WidgetParseManifestFailed;
    }
    WBM_END(configxml);

#ifdef QTWRT_USE_USIF
    // since parseManifest() occured prior to extraction from the widget bundle
    // widgetProperties() must be forced to update icon paths
    widgetProperties(true);
#endif

    emit installProgress(40);

    // allow only NOKIA signed widgets to installed as shared library
    bool isTrustedDomainWidget = (errorCode == WidgetValidSignature) &&
                               (!getTrustDomain().compare(TrustedDomain, 
                                       Qt::CaseInsensitive));

    props = widgetProperties();

    // shared libraries must be signed
    if (props->isSharedLibrary() && !isTrustedDomainWidget) {
        emit installationError(WidgetSharedLibraryNotSigned);
        cleanup( WidgetSharedLibraryNotSigned,removed);
        return WidgetSharedLibraryNotSigned;
    }

    QString updateWgtId = getWidgetManifestInfo()->value(W3CSettingsKey::WIDGET_ID, 
            QString("id"));
    
    if (!isTrustedDomainWidget) {
       // Not a Nokia signed widget
       if (updateWgtId.startsWith("com.nokia", Qt::CaseInsensitive) ||
           updateWgtId.startsWith("com.ovi", Qt::CaseInsensitive)) {
          // block this
          emit installationError(WidgetIdInvalid);
          cleanup(WidgetIdInvalid);
          return WidgetIdInvalid;
       }
    }

    if (update) {
        WebAppInfo info;

        if (WebAppRegistry::instance()->isRegistered(props->id(), info)) {
            LOG("updateWgtId" << updateWgtId << ':' << props->id());
            QString srcPath = widgetUnZipPath();
            QString dstPath = props->installPath();
            LOG("install path: " << dstPath);
            LOG("unzip path  : " << srcPath);

            // create a backup directory before this widget is updated
            QString updateBackupPath = widgetInstallPath() + QDir::separator() + 
                    "widgetUpdateBackupPath" + QDir::separator() + props->id();
            QDir updateBackupDir(updateBackupPath);
            if (!updateBackupDir.exists()) {
                if (!updateBackupDir.mkpath(updateBackupPath)) {
                    cleanup(WidgetSystemError);
                    emit installationError(WidgetSystemError);
                    return WidgetSystemError;
                }
            }

            // backup the widget to be updated
            SuperWidget::rmDir(updateBackupPath);
            if (!m_widgetInstaller->update(dstPath, updateBackupPath, props->id())) {
                SuperWidget::rmDir(updateBackupPath);
                cleanup(WidgetSystemError);
                emit installationError(WidgetSystemError);
                return WidgetSystemError;
            }

            // clean dest dir before update files
            SuperWidget::rmDir(dstPath);
            if (!m_widgetInstaller->update(srcPath, dstPath, props->id())) {
                // restore from updateBackup widget in case update failed
                SuperWidget::rmDir(dstPath);
                m_widgetInstaller->update(updateBackupPath, dstPath, props->id());
                SuperWidget::rmDir(updateBackupPath);
                cleanup(WidgetSystemError);
                emit installationError(WidgetSystemError);
                return WidgetSystemError;
            }

#if defined(Q_OS_SYMBIAN) && defined(QTWRT_USE_USIF)
            if (!WebAppRegistry::instance()->unregister(props->id(), true)) {
                cleanup(WidgetSystemError);
                emit installationError(WidgetSystemError);
                return WidgetSystemError;
            }

            TRAP_IGNORE(SuperWidget::NotifyAppArcOfUninstallL(info.uid()));

            WidgetInstallError err = setupSecuritySession();
            if (err != WidgetInstallSuccess)
            {
                emit installationError(err);
                cleanup(err);
                return err;
            }

            QString procUid = setProcessUidInProps(props);

            QString startFile;
            if (!findStartFile(startFile, dstPath)) {
                cleanup(WidgetStartFileNotFound);
                emit installationError(WidgetStartFileNotFound);
                return WidgetStartFileNotFound;
            }

            if (!registerWidget(startFile)) {
                cleanup(WidgetRegistrationFailed);
                emit installationError(WidgetRegistrationFailed);
                return WidgetRegistrationFailed;
            }

            if (WebAppRegistry::instance()->isRegistered(props->id(), info))
            {
                TRAP_IGNORE(SuperWidget::NotifyAppArcOfInstallL(info.uid()));
            }

            if (!commaSeparatedAKIs.isEmpty()) {
                WebAppRegistry::instance()->setCertificateAki(props->id(), commaSeparatedAKIs);
            }
#else
            WebAppRegistry::instance()->setWebAppVersion(props->id(),
                                                         props->plistValue(W3CSettingsKey::WIDGET_VERSION),
                                                         props->iconPath() );
#endif

            SuperWidget::rmDir(updateBackupPath);
            cleanup(WidgetInstallSuccess);
            emit installationSucceed();
            return WidgetInstallSuccess;
        }
        else {
            emit installationError(WidgetUpdateFailed);
            return WidgetUpdateFailed;
        }
    }

    emit installProgress(50);

    if (WebAppRegistry::instance()->isRegistered(props->id())) {
        m_continueInstallation = false;

        emit aboutToReplaceExistingWidget(props->title());
        if (!m_continueInstallation) {
            cleanup(WidgetUserConfirmFailed,removed );
            emit installationError(WidgetUserConfirmFailed);
            return WidgetUserConfirmFailed;
        }

#ifdef Q_OS_SYMBIAN
        emit installProgress(55);
        TRAP_IGNORE( restartNotificationL());
#endif
        emit installProgress(60);

        WidgetUninstallError err = SuperWidget::uninstall(props->id(), false);
        if (err == WidgetUninstallFailed)
        {
            emit installationError(WidgetReplaceFailed);
            cleanup(WidgetReplaceFailed,removed);
            return WidgetReplaceFailed;
        }
    }

    WidgetInstallError err = setupSecuritySession();
    if (err != WidgetInstallSuccess)
    {
        emit installationError(err);
        cleanup(err,removed);
        return err;
    }

    if (m_mapping.getWidgetType() == WidgetTypeJIL)
        props->setType(WIDGET_PACKAGE_FORMAT_JIL);
    emit installProgress(70);

    props->setSecureSessionPath(props->resourcePath() + SECSESSION_FILE);

#ifdef Q_OS_SYMBIAN
    QString procUid = setProcessUidInProps(props);

    // Skip update paths for trusted nokia embed widgets
    if (procUid.compare(DEFAULT_DIRECTORY, Qt::CaseInsensitive) != 0)
    {
        updatePaths(procUid);
    }
#endif

    // if the start file doesn't exist, treat as invalid widget
    QString startFile;
    if (!findStartFile(startFile, widgetUnZipPath())) {
        LOG("WgtWidget::install() failed: start file not found");
        emit installationError(WidgetStartFileNotFound);
        cleanup(WidgetStartFileNotFound,removed);
        return WidgetStartFileNotFound;
    }

    emit installProgress(80);

    QString srcPath = widgetUnZipPath();
    QString dstPath = props->installPath();
    if (dstPath.endsWith(QDir::separator()))
        dstPath.chop(1);
    if (srcPath.endsWith(QDir::separator()))
        srcPath.chop(1);
    LOG("install path: " << dstPath);
    LOG("unzip path  : " << srcPath);

    if (!m_widgetInstaller->install(srcPath,dstPath, props->id())) {
        LOG("WgtWidget::install(), Widget install failed");
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
        //Maemo specific installation error's are emitted from Private implementations
        emit installationError(WidgetPlatformSpecificInstallFailed);
#endif
	cleanup(WidgetPlatformSpecificInstallFailed);
        cleanup(WidgetPlatformSpecificInstallFailed,removed,dstPath);
        return WidgetPlatformSpecificInstallFailed;
    }

    emit installProgress(90);

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
    if (!registerWidget(startFile)) {
        LOG("WgtWidget::install(), Widget register failed");
        emit installationError(WidgetRegistrationFailed);
        cleanup(WidgetPlatformSpecificInstallFailed,removed);
        cleanup(WidgetPlatformSpecificInstallFailed,removed, dstPath);
        return WidgetRegistrationFailed;
    }
#endif

#if defined(Q_OS_SYMBIAN) && defined(QTWRT_USE_USIF)
    WebAppInfo info;
    if (WebAppRegistry::instance()->isRegistered(props->id(), info))
    {
        TRAP_IGNORE(this->NotifyAppArcOfInstallL(info.uid()));
    }
#endif
    //write widget certificateaki to webappregistry this is needed for runtime capability checks
    if (!commaSeparatedAKIs.isEmpty()) {
        WebAppRegistry::instance()->setCertificateAki(props->id(),commaSeparatedAKIs);
    }

    emit installProgress(100);

#if defined(QTWRT_USE_DESKTOPFILEWRITER)
    DesktopUtils::WriteDesktopFile(this, props->title(), dstPath, props->id(), m_manifest);
#endif

    emit installationSucceed();
    // dont cleanup src if it is the same as destination
    // they are the same if backup-restore
    if (srcPath != dstPath) {
        cleanup(WidgetInstallSuccess,removed);
    }

    return WidgetInstallSuccess;
}

//
// this function provides the widget launcher path
// parameters:
//    pkgPath  path to installed widget
//
// return
//    QString  absolute path to start file
//
QString WgtWidget::launcherPath(const QString &pkgPath)
{
    WidgetProperties *widget = SuperWidget::getProperties(pkgPath);
    if (widget)
    {
        QString startFile;
        if (findStartFile(startFile,widget->installPath())) {
            LOG("WgtWidget::launcherPath start file =" << startFile);
            return (widget->installPath()+QDir::separator()+startFile);
        }
    }

    return "";
}


//
// function to find widget features/capabilities
// prereq:        should parseManifest before calling findFeatures
//
// parameters:
//    features    returns WidgetFeatures
//    path        path to widget files
// return:
//    bool        true if features found
//                false if features not found
//
bool WgtWidget::findFeatures(WidgetFeatures& features, const QString& path)
{
    QString widgetPath(path);
    LOG("WgtWidget::findFeatures ....");
    if (path.isEmpty()) widgetPath=widgetRootPath();

    WidgetManifestInfo *manifestinfo = getWidgetManifestInfo();
    if (!manifestinfo) return false;

    int featureCount = manifestinfo->count(W3CSettingsKey::WIDGET_FEATURE);

    //Change for incorporating required attribute
    for (int iter = 0 ; iter < featureCount ; ++iter)   {
        QString featureName = manifestinfo->value(W3CSettingsKey::WIDGET_FEATURE , 
                iter + 1 , QString("name"));
        QString required_attr = manifestinfo->value(W3CSettingsKey::WIDGET_FEATURE , 
                iter + 1 , QString("required"));
        if (required_attr.isEmpty())
            required_attr = "true";
        if (!featureName.isEmpty()) {
            features.insert(featureName ,required_attr);
        }
    }
    return (features.count() > 0);
}

// returns the AKI of signer of the widget's certificate if the widget was signed and certificate was validated
bool WgtWidget::getCertificateAKI(QString& aki) {
    if (m_validCertificateAKI.isNull() || m_validCertificateAKI.isEmpty()) {
        QString uniqueID;
        if( !(WebAppUniquenessHelper::getUniqueNameFromPath(widgetRootPath(), uniqueID) ) )
            return false;
        
        WebAppInfo appInfo;
        if (WebAppRegistry::instance()->isRegistered(uniqueID, appInfo)) {
            QString commaSeparatedAKIs = appInfo.certificateAki();
            QStringList certificateAKIs = commaSeparatedAKIs.split(",");
            if (certificateAKIs.length() > 0)
                m_validCertificateAKI = certificateAKIs.at(0); // the first is the EE cert
            else
                m_validCertificateAKI = "";
        }
    }
    if (!m_validCertificateAKI.isNull() && !m_validCertificateAKI.isEmpty()) {
        aki = m_validCertificateAKI;
        return true;
    }
    return false;
}

// returns the trust domain associated with this widget
QString WgtWidget::getTrustDomain() {
    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();
    QString domain;
    
    // get policy files from WrtSettings
    QString trustPath = m_settings->valueAsString("SecurityTrustPolicyFile");

    // create a trust session
    WAC::TrustSession trustSession(trustPath);
    WidgetProperties* props = minimalWidgetProperties();

    // if we are installing, we put certificate AKI list in widget properties
    // if we are already installed (launch time) we get the AKI list from the registry
    QString uniqueID;
    QString commaSeparatedAKIs;
    if(WebAppUniquenessHelper::getUniqueNameFromPath(widgetRootPath(), uniqueID)) {
        WebAppInfo appInfo;
        if (WebAppRegistry::instance()->isRegistered(uniqueID, appInfo)) {
            commaSeparatedAKIs = appInfo.certificateAki();
        } 
    }
    else {
        if(props) {
            commaSeparatedAKIs = props->certificateAki();
            }
    }
    QStringList certificateAKIs = commaSeparatedAKIs.split(",");
    
    // End Entity AKI is first in the list, then any SubCAs
    for (int i = 0; i < certificateAKIs.length(); i++) {
        QString aki = certificateAKIs.at(i);
        domain = trustSession.domainFor("certificate", aki);
        // compare against UntrustedWidgets
        if (domain != trustSession.domainFor("certificate", "")) {
            break;
        }
    }
    return domain;
}

// function to check if feature is allowed
// parameters:
//    features    list of features from manifest
// return:
//    bool        true if successful
//
bool WgtWidget::isFeatureAllowed(WidgetFeatures& features, 
        WAC::SecSession* secureSession ,bool runtime)
{

    QList<QString> required_capabilities;
    QList<QString> optional_capabilities;
    if (!m_mapping.getCapabilities(features,required_capabilities,optional_capabilities))
        return false;

    // check if capability is allowed
    // According to current implementation of security manager, even if one
    // capability is not allowed isAllowed will return false
    // If trustPath or secPath are empty, isAllowed() will return true

    if (secureSession->isAllowed(required_capabilities)) {
        // done to add these caps to allowed capabilities in secsession
        //secureSession.isAllowed(optional_capabilities);
        QList<QString> caps;
        caps.append(required_capabilities);
        caps.append(optional_capabilities);
        QList<QString> services;
        m_mapping.getServiceNames(features,services);
        if (!services.isEmpty() && !runtime) {
            // only prompt if this is a JIL widget
            WidgetManifestInfo* manifestinfo = getWidgetManifestInfo();
            QString widgetType = manifestinfo->value(ProprietarySettingsKey::WIDGET_TYPE);
            if (!widgetType.isEmpty() && widgetType.compare(WIDGET_PACKAGE_FORMAT_JIL) == 0) {
                m_continueInstallation = false;
                emit aboutToInstallWidgetWithFeatures(services);
                if (!m_continueInstallation)
                    return false;
            }
        }
        return true;
    }
    return false;
}

#ifdef Q_OS_SYMBIAN
void WgtWidget::restartNotificationL()
{
    TInt64 isInMiniview;
    WidgetProperties* props = widgetProperties();
    TBuf<120> bundleId(props->id().utf16());
    TUid aUid;

#ifdef QTWRT_USE_USIF
    RSoftwareComponentRegistry scrClient;
    User::LeaveIfError( scrClient.Connect() );
    TComponentId compId = 0;
    TRAPD(err, compId = scrClient.GetComponentIdL(bundleId, Usif::KSoftwareTypeWidget));
    //TComponentId compId = scrClient.GetComponentIdForAppL(widgetUid);

    HBufC *isInMiniviewName = qt_QString2HBufCNewL(SCR_PROP_ISMINIVIEW);
    CleanupStack::PushL(isInMiniviewName);

    CPropertyEntry* propMiniview = scrClient.GetComponentPropertyL(compId, *isInMiniviewName);
    if (propMiniview) {
        CIntPropertyEntry* intPropertyMiniview = dynamic_cast<CIntPropertyEntry*>(propMiniview);
        isInMiniview = intPropertyMiniview->Int64Value();
    } else {
        isInMiniview = 0;
    }

    CPropertyEntry* propUid = scrClient.GetComponentPropertyL(compId, KUid);
    if (propUid) {
        CIntPropertyEntry* intProperty = dynamic_cast<CIntPropertyEntry*>(propUid);
        aUid.iUid = intProperty->Int64Value();
    } else {
        aUid.iUid = 0;
    }
    scrClient.Close();
    CleanupStack::PopAndDestroy(isInMiniviewName);
    if (aUid.iUid == 0)
        return;

#else
    RWidgetRegistryClientSession clientReg;
    User::LeaveIfError( clientReg.Connect() );
    aUid.iUid = clientReg.GetWidgetUidL(bundleId);
    isInMiniview = clientReg.IsWidgetInMiniView(aUid);

#endif
    if (isInMiniview) {
        RApaLsSession apparcSession;
        User::LeaveIfError( apparcSession.Connect() );
        //        TApaAppInfo info;
        //        User::LeaveIfError( apparcSession.GetAppInfo( info, aUid ) );
        //        HBufC* widgetName(info.iFullName.AllocLC());
        const TInt size( 3*sizeof( TUint32 ) );
        CApaCommandLine* cmd( CApaCommandLine::NewLC() );
        HBufC8* opaque( HBufC8::NewLC( size ) );
        RDesWriteStream stream;
        TPtr8 des( opaque->Des() );
        stream.Open( des );
        CleanupClosePushL( stream );
        // Generate the command.
        stream.WriteUint32L( aUid.iUid );
        stream.WriteUint32L( 0 );
        //        stream.WriteL( reinterpret_cast< const TUint8* >( widgetName->Ptr() ),
        //                       widgetName->Size() );
        // Number 8 is for EWidgetRestart WidgetOperation.
        stream.WriteInt32L( 8 );
        CleanupStack::PopAndDestroy( &stream );
        // Generate command.
        cmd->SetCommandL( EApaCommandBackgroundAndWithoutViews );
        cmd->SetOpaqueDataL( *opaque );
        CleanupStack::PopAndDestroy( opaque );
        _LIT( KLauncherApp, "wgtwidgetlauncher.exe" );
        cmd->SetExecutableNameL( KLauncherApp );
        User::LeaveIfError( apparcSession.StartApp( *cmd ) );
        CleanupStack::PopAndDestroy( cmd );
        apparcSession.Close();
        //        CleanupStack::PopAndDestroy( widgetName );
    }
}
#endif

void WgtWidget::disableBackupRestoreValidation(bool disableUnsignWidgetSignCheck)
{
    m_disableUnsignWidgetSignCheck = disableUnsignWidgetSignCheck;
}

#ifdef Q_OS_SYMBIAN
HBufC* WgtWidget::qt_QString2HBufCNewL(const QString& aString)
{
    HBufC *buffer;
#ifdef QT_NO_UNICODE
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString.toLocal8Bit().constData()));
    buffer = HBufC8::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#else
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(aString.utf16()));
    buffer = HBufC16::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#endif
    return buffer;
}
#endif

//sets securitySessionString to widget properties
void WgtWidget::setSecuritySessionString(WAC::SecSession* secureSession) {
    WidgetProperties* props = widgetProperties();
    if (props) {
        secureSession->setClientInfo(WAC::KWIDGETPATH, props->installPath());
        QByteArray secureSessionByteArr;
        secureSession->persist(secureSessionByteArr);
        QString secureSessionString(secureSessionByteArr);
        props->setSecureSessionString(secureSessionString);
    }
}


// Method used to get the capability lists based on feature name
//
// parameters:
//      features the feature list for which the device-caps are to be determined
//      required_capabilities the list of required capabilities
//      optional_capabilities the list of optional capabilities
//
// @return:
//      bool false if plugin load fails or feature is not found in the mapping file

bool WgtWidget::createSecuritySession(WAC::SecSession** secureSession,QString& trustDomain,
        const QString& sigId,bool runtime)
{
    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();

    // get policy files from WrtSettings
    QString trustPath =     m_settings->valueAsString("SecurityTrustPolicyFile");
    QString secPath =     m_settings->valueAsString("SecurityAccessPolicyFile");


    // create a trust session
    WAC::TrustSession trustSession(trustPath);

    //At runtime read the widget certificateAki from webappregistry
    if (runtime) {
        QString aki;
        getCertificateAKI(aki);
        m_validCertificateAKI = aki;
    }

    // get trust domain
    if (!sigId.isEmpty()) {
        trustDomain = trustSession.domainFor("certificate", sigId);
    } else {
        trustDomain = trustSession.domainFor("certificate",m_validCertificateAKI);
    }

    // if we end up with the default domain, make sure it's UntrustedWidgets
    // redundant - change default domain in browser_access_policy.xml
    if (trustDomain == trustSession.domainFor("certificate", "")) {
        trustDomain = "UntrustedWidgets";
    }

    // create secure session
    *secureSession = new WAC::SecSession(secPath,trustDomain,"");
    return !(*secureSession == NULL);
}

void WgtWidget::saveStartFile(QString& startFile, const QString& path)
{
    QString uniqueName;
    if (WebAppUniquenessHelper::getUniqueNameFromPath(path, uniqueName)) {
        if (!startFile.isEmpty() && !uniqueName.isEmpty() &&
            !s_widgetStartFileCache.contains(uniqueName)) {
            s_widgetStartFileCache.insert(uniqueName, startFile);
        }
    }
}

bool WgtWidget::getStartFile(QString& startFile, const QString& path)
{
    QString uniqueName;
    if (WebAppUniquenessHelper::getUniqueNameFromPath(path, uniqueName)) {
        if (s_widgetStartFileCache.contains(uniqueName)) {
            startFile = s_widgetStartFileCache.value(uniqueName);
            return true;
        }
    }
    return false;
}

bool  WgtWidget::isDirectionValid(const QString& AttributeValue)
{
    if (((AttributeValue.compare("ltr",Qt::CaseInsensitive)) == 0) ||
       ((AttributeValue.compare("rtl",Qt::CaseInsensitive)) == 0) ||
       ((AttributeValue.compare("lro",Qt::CaseInsensitive)) == 0) ||
       ((AttributeValue.compare("rlo",Qt::CaseInsensitive)) == 0))   {
        return true;
    }
    return false;
}

#ifdef Q_OS_SYMBIAN
void WgtWidget::updatePaths(const QString& domainNameUid)
{

    QString currUid = m_installDir;
    m_widgetInstallPath.replace(currUid, domainNameUid, Qt::CaseInsensitive);
    m_widgetRootPath.replace(currUid, domainNameUid, Qt::CaseInsensitive);

    WidgetProperties* props = widgetProperties();
    props->setInstallPath(props->installPath().replace(currUid, domainNameUid, 
            Qt::CaseInsensitive));
    QString iconpath = props->iconPath();
    props->setIconPath(iconpath.replace(currUid, domainNameUid, Qt::CaseInsensitive));
    props->setResourcePath(props->resourcePath().replace(currUid, domainNameUid, 
            Qt::CaseInsensitive));

}

QString WgtWidget::getProcUid(const QString& domain) const
{
    QSettings settings(INI_PATH,QSettings::IniFormat);
    QString uid = settings.value("DomainNames/" + domain).toString();
    if (uid.isEmpty())
        return settings.value("DomainNames/others").toString();
    return uid;
}

QString WgtWidget::setProcessUidInProps(WidgetProperties *props)
{
    QString domainName = getTrustDomain();
    QString procUid = getProcUid(domainName);

    const QString ProcessUid(PROCESS_UID);
    AttributeMap map = props->plist();
    map.insert(ProcessUid, procUid);
    props->setInfoPList(map);

    return procUid;
}
#endif

WidgetInstallError WgtWidget::setupSecuritySession()
{
    WidgetFeatures features;
    bool res = findFeatures(features);
    QString trustDn;
    WAC::SecSession* secureSession;

    QList<QString> required_capabilities;
    QList<QString> optional_capabilities;
    if (!m_mapping.getCapabilities(features,required_capabilities,optional_capabilities))
        return WidgetCapabilityNotAllowed;

	QString trustDomain = getTrustDomain();
	LOG( " setupSecuritySession(): the trust domain is: " << trustDomain);	

	WAC::SecMgr secMgr;
	if(	secMgr.createSecsession(&secureSession, trustDomain, required_capabilities, 
	        optional_capabilities, false)){
        setSecuritySessionString(secureSession);
        delete secureSession;
        return WidgetInstallSuccess;
    } else {
        return WidgetSystemError;
    }
}

/*!
 * tells wether the widget is from trusted domain
 * @return true is the widgets is from trusted domain, otherwise false
 */

#if ENABLE(AEGIS_LOCALSIG)
bool WgtWidget::isFromTrustedDomain()
{
    return  (!getTrustDomain().compare(TrustedDomain, Qt::CaseInsensitive)|| 
             !getTrustDomain().compare(OperatorDomain, Qt::CaseInsensitive));

}
#endif
