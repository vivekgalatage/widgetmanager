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
#include "WgzWidget.h"
#include "WidgetPListParser.h"
#include "WgtWidget.h"
#include "w3cxmlplugin.h"
#include "WidgetUtilsLogs.h"
#include "widgetinstaller.h"

#include <QTextDocument>
#include <QFile>
#include <QIODevice>
#include <QDesktopServices>
#include <QDir>
#include <QHash>


WgzWidget::WgzWidget(QString& rootDirectory)
{
    initialize(rootDirectory);
    m_widgetType = WidgetTypeWgz;
    m_contentType = CONTENT_TYPE_WGZ;
}

// function to set widget root path
// parameters:
//    path     path to widget files
//
// For wgz widgets are packaged in a folder.
// widgetUnZipPath is supposed to contain only one widget
// Pick that widget directory and set it as root
//
bool WgzWidget::setWidgetRootPath(const QString& path)
{
    if (!path.isEmpty())
    {
        SuperWidget::setWidgetRootPath(path);
        return true;
    }

    QDir rootDir(widgetUnZipPath());
    QFileInfoList tempExtract = rootDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach (const QFileInfo &isWidgetDir, tempExtract )
    {
        QDir widgetDir(isWidgetDir.absoluteFilePath());
        QString fileName("info.plist");

        foreach (const QFileInfo &file, widgetDir.entryInfoList(QDir::Files))
        {
            if ( !fileName.compare (file.fileName(), Qt::CaseInsensitive) )
            {
                LOG("Root " << isWidgetDir.absoluteFilePath());
                SuperWidget::setWidgetRootPath(isWidgetDir.absoluteFilePath());
                return true;
            }
        }
    }

    return false;
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
bool WgzWidget::parseManifest(const QString& path, const bool force)
{
    bool status = true;
    if (!force && m_manifest) return status;

    m_manifest = new WidgetInfoPList();
    LOG("WgzWidget::parseManifest:  path=" << path);
    if (path.isEmpty())m_manifest->setdir(widgetUnZipPath());
    else m_manifest->setdir(path);
    status = m_manifest->process();
    if (status) widgetProperties();
    return status;

}


//
// function to find widget start file
// prereq:    should parseManifest before calling this
// parameters:
//    startFile    returns a string that specifies startFile of widget
//    path         path to widget files
// return:
//    bool         true if start file found
//                 false if start file is not found
//
bool WgzWidget::findStartFile(QString& startFile, const QString& path)
{
    QString widgetPath(path);
    if (path.isEmpty()) {
        widgetPath = widgetRootPath();
    }
    LOG("WgzWidget::findStartFile widgetPath=" << widgetPath);
    AttributeMap* map = new AttributeMap();
    if (!getWidgetManifestInfo()) {
        delete map;
        return false;
    }

    *map = getWidgetManifestInfo()->getDictionary();
    QString key = "MainHTML";
    AttributeMap::iterator iter = map->find(key);
    if (iter != map->end()) {
        startFile = map->value(key).toString();
        LOG("WgzWidget::findStartFile startfile =" << startFile);
        delete map;
        return true;
    }

    // didn't find start file
    // look for default start files
    QDir dir(widgetPath);
    QStringList files = dir.entryList( QDir::Files | QDir::NoDotAndDotDot );
    QRegExp defaultFiles("index.htm|index.html");

    for (int i=0;i<files.count();i++)
    {
        if (defaultFiles.exactMatch(files.at(i)))
        {
            startFile = files.at(i);
            LOG("Startfile = " << startFile);
            delete map;
            return true;
        }
    }
    delete map;
    return false;
}

//
// function to find widget icon files
// parameters:
//    icons    returns a stringlist that specifies icons of widget
//    path     path to widget files
// return:
//    bool    true if icon found
//            false if icon is not found
//
bool WgzWidget::findIcons(QStringList& icons, const QString& path)
{
    QString widgetPath(path);
    LOG("WgtWidget::findIcons ....");
    if (path.isEmpty()) widgetPath=widgetRootPath();


    LOG("Finding default icons in " << widgetPath);
     // icons not specified in info.plist
     // look for default icons
    QDir dir(widgetPath);

    QRegExp defaultFiles("icon.png");
    defaultFiles.setCaseSensitivity(Qt::CaseInsensitive);


    foreach (const QFileInfo &file, dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
    {
        if (defaultFiles.exactMatch(file.fileName()))
        {
            icons.append(file.fileName());
            //LOG("iconfile=" << file.absoluteFilePath());
            LOG("iconfile=" << file.fileName());
        }
    }

    return (icons.count() > 0);
}

//
// function to generate Manifest
// prereq: should parseManifest before calling writeManifest()
//
// parameters:
//    path    path to write the manifest. Default is /tmp/info.plist
//
void WgzWidget::writeManifest(const QString& path)
{
    if (!getWidgetManifestInfo()) return;

    AttributeMap oldMap = getWidgetManifestInfo()->getDictionary();

    QList<QString> keyList = oldMap.keys();
    LOG("\n\nkeylist count = " << keyList.count());

    QString newList("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n <!DOCTYPE plist PUBLIC \"-//Nokia//DTD PLIST 1.0//EN\" \"http://www.nokia.com/NOKIA_COM_1/DTDs/plist-1.0.dtd\"> \n<plist version=\"1.0\"> \n<dict>\n");


    for (int i=0;i<keyList.count();i++)
    {
        if (!getWidgetMap()->value(keyList.at(i)).isEmpty())
        {
            LOG("this is: " << keyList.at(i));
            newList.append("<key>"+keyList.at(i)+"</key>\n<string>"+oldMap.value(keyList.at(i)).toString()+"</string>\n");
            LOG("toWgz: key=" << keyList.at(i) << " value = " <<oldMap.value(keyList.at(i)));

        }

    }
    newList.append("</dict>\n</plist>");

    LOG(path << "*********\n\n" << newList);

    QString fpath(path);
    if (path.isEmpty()){
        fpath=QDesktopServices::storageLocation(QDesktopServices::TempLocation)+QDir::separator()+"info.plist";
    }

    QFile file(fpath);
    file.open(QIODevice::WriteOnly);
    file.write(newList.toUtf8().data(),newList.length());
    file.close();
}

//
// function to indicate if widget should be allowed to be installed on
// removable media
//
// return: false always. WGZ widgets don't allow installation on removable media
//   bool
//
bool WgzWidget::allowRemovableInstallation()
{
    return false;
}

//
// function to get values from dictionary
// prereq: should parseManifest before calling value()
//
// parameters:
//    key    key to query the dictionary
// return:
//    QString   value matching key from dictionary
//
QString WgzWidget::value(const QString& key, const QString & attribute)
{
    Q_UNUSED(attribute)
    if (!getWidgetManifestInfo())
    {
        return "";
    }

    return getWidgetManifestInfo()->getDictionary().value(key).toString();
}

bool WgzWidget::contains( const QString & key, const QString & attribute ) {
    Q_UNUSED(attribute)

    if (!m_manifest)
    {
        return false;
    }

    return m_manifest->getDictionary().contains(key);
}


//
// function to get widgetProperties
// prereq: should parseManifest before calling this
//
// return:
//    WidgetProperties    specific widget properties required for WebAppRegistry
//                        if you want more,just use value()
//
WidgetProperties* WgzWidget::widgetProperties(bool forceUpdate, bool minimal)
{
    Q_UNUSED(minimal);

    if (!getWidgetManifestInfo())
    {
        return 0;
    }
    if ((m_widgetProperties!=0) && !forceUpdate)
        return m_widgetProperties;

    WidgetProperties *props;
    if (!m_widgetProperties) {
        m_widgetProperties = new WidgetProperties;
    }
    props = m_widgetProperties;
    AttributeMap dict = getWidgetManifestInfo()->getDictionary();
    getWidgetManifestInfo()->setDictionary(&dict);


    props->setId(value(Identifier));
    props->setTitle(value(DisplayName));
    QString id( QString::number(qHash(props->id())) );
    props->setInstallPath(widgetInstallPath()+QDir::separator()+id);
    props->setSource(widgetBundlePath());
    props->setInfoPList(getWidgetManifestInfo()->getDictionary());
    props->setSize(getSize());
    props->setType(WIDGET_PACKAGE_FORMAT_WGZ);

    QStringList icons;
    if (findIcons(icons))
    {
        props->setIconPath(icons.at(0));
    }
    else
    {
        props->setIconPath(":/resource/default_widget_icon.png");
    }

    LOG("WgzWidget::widgetProperties setting widget properties");

    return props;
}

//
// function to install widget
//
// parameter:
//    silent   true for silent install, false otherwise
//    QWidget* pointer to parent widget
// return:
//    bool    true if successful
//
WidgetInstallError WgzWidget::install(const bool /*update*/)
{
    if (!checkDiskSpaceForInstallation(widgetBundlePath()))
    {
        emit installationError(WidgetInsufficientDiskSpace);
        cleanup(WidgetInsufficientDiskSpace);
        return WidgetInsufficientDiskSpace;
    }

    // unzip widget bundle
    if (!unZipBundle(widgetBundlePath()))
    {
        emit installationError(WidgetUnZipBundleFailed);
        cleanup(WidgetUnZipBundleFailed);
        return WidgetUnZipBundleFailed;
    }

    if (!setWidgetRootPath())
    {
        emit installationError(WidgetCorrupted);
        cleanup(WidgetCorrupted);
        return WidgetCorrupted;
    }


    // parse manifest
    if (!parseManifest())
    {
        emit installationError(WidgetParseManifestFailed);
        cleanup(WidgetParseManifestFailed);
        return WidgetParseManifestFailed;
    }

    // if the start file doesn't exist, treat as invalid widget
    QString startFile;
    if (!findStartFile(startFile, widgetUnZipPath())) {
        LOG("WgtWidget::install() failed: start file not found");
        emit installationError(WidgetStartFileNotFound);
        cleanup(WidgetStartFileNotFound);
        return WidgetStartFileNotFound;
    }

    WidgetProperties* props = widgetProperties();

    if (WebAppRegistry::instance()->isRegistered(props->id())) {
        m_continueInstallation = false;
        emit aboutToReplaceExistingWidget(props->title());
        if (!m_continueInstallation) {
            cleanup(WidgetReplaceFailed);
            return WidgetReplaceFailed;
        }
        if (SuperWidget::uninstall(props->id(), false) == WidgetUninstallFailed)
        {
            emit installationError(WidgetReplaceFailed);
            cleanup(WidgetReplaceFailed);
            return WidgetReplaceFailed;
        }
    }

    if (!registerWidget(startFile)) {
        emit installationError(WidgetRegistrationFailed);
        cleanup(WidgetRegistrationFailed);
        return WidgetRegistrationFailed;
    }


    QString srcPath = widgetRootPath();
    QString dstPath = props->installPath();
    LOG("install path: " << dstPath);
    LOG("unzip path  : " << srcPath);

    if (!m_widgetInstaller->install(srcPath,dstPath, props->id())) {
        emit installationError(WidgetPlatformSpecificInstallFailed);
        cleanup(WidgetPlatformSpecificInstallFailed);
        return WidgetPlatformSpecificInstallFailed;
    }


    emit installationSucceed();
    cleanup(WidgetInstallSuccess);
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
QString WgzWidget::launcherPath(const QString &pkgPath)
{
    WidgetProperties *widget = SuperWidget::getProperties(pkgPath);
    if (widget) {
        LOG("plist value = " << widget->plistValue(MainHTML).toString());
        QString startFile;
        if (findStartFile(startFile, widget->installPath()))
        {
            LOG("WgzWidget::launcherPath start file =" << startFile);
            return (widget->installPath() + QDir::separator() + startFile);
        }
    }

    return "";
}


//
// function to find widget features/capabilities
// prereq: should parseManifest before calling findFeatures
//
// parameters:
//    features    returns WidgetFeatures
//    path        path to widget files
// return:
//    bool    true if features found
//            false if features not found
//
bool WgzWidget::findFeatures(WidgetFeatures& features, const QString& path)
{
    QString widgetPath(path);
    LOG("WgtWidget::findFeatures ....");
    if (path.isEmpty()) widgetPath=widgetRootPath();

    QString val = value(AllowNetworkAccess);
    if (val.isEmpty()) return false;
    features.insert(AllowNetworkAccess,val);
    return true;
}
