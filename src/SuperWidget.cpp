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

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDesktopServices>
#include <QDirIterator>
#include <QCoreApplication>
#include <QHash>
#include <QTextStream>
#include <QRegExp>
#include <QCryptographicHash>
#include <QByteArray>

#include "proprietarysettingskeys.h"
#include "wacw3csettingskeys.h"
#include "wacwebappinfo.h"
#include "widgetinstaller.h"
#include "private/WidgetUtilsLogs.h"
#include "wacWidgetManager.h"

#ifdef Q_OS_SYMBIAN
#include "platform/symbian/WidgetUnzipUtilityS60.h"
#include <zipfile.h>
#include <apgcli.h>
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <f32fsys.h>
#include <coemain.h>
#include <sysutil.h>
#else
#include "unzip.h"
#include "zzip.h"
#endif

#ifdef Q_OS_LINUX
#include <sys/vfs.h> // for checking free disk space
#endif

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
#include "storage.h"
#endif

#ifdef Q_OS_MAEMO6
#include "platform/maemo/debianutils.h"
#endif

static QHash<QString, int> m_widgetTypeCache;

SuperWidget::~SuperWidget()
{
    delete m_manifest;
    delete m_widgetMap;
    delete m_widgetProperties;
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
    delete m_storage;
#endif
    m_widgetTypeCache.clear();
    delete m_widgetInstaller;
}

// initializes the widget to some defaults
// set up widget map
// create secure storage
//
void SuperWidget::initialize(QString& rootDirectory)
{
    LOG("SuperWidget::initialize");
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    setWidgetUnZipPath("/var/tmp/");
    setWidgetBundlePath("/var/tmp");
#else
    QString tempLocation = getTempLocation();
    LOG("SuperWidget::initialize.. tempLocation : "<<tempLocation);
    if (tempLocation.isEmpty())
      return;

    QString path = QDir::toNativeSeparators(tempLocation);
    setWidgetBundlePath(path);
    if (path.endsWith(QDir::separator()))
        setWidgetUnZipPath(path+"widgetUnZipPath");
    else
        setWidgetUnZipPath(path+QDir::separator()+"widgetUnZipPath");
#endif
    LOG("SuperWidget::initialize() - Widget install path" << rootDirectory);

    setWidgetInstallPath(rootDirectory);

#if defined(Q_OS_SYMBIAN)
     // Set the globals for this install
    // Extract the install dir from m_rootPath, assuming it is of format c:\private\uid or e:\private\\uid
    m_rootPath = m_widgetInstallPath;
    m_rootPath.chop(16);
    // installDir is the uid
    m_installDir =  m_rootPath;
    m_installDir.remove(0,11);

    m_sharedLibPath = (m_widgetInstallPath + "\\"+SHARED_LIBRARY_FOLDER+"\\");;
    m_dataPath = (m_rootPath + "\\" + QString(DATA_FOLDER) + "\\");

#else
    // Maemo and Linux use the hardcodeed vars below as defined in the constants directory
    m_sharedLibPath = SHARED_LIBRARY_PATH;
    m_dataPath = DATA_PATH;
    m_installDir = m_widgetInstallPath;
#endif

    QString certPath = getCertificatePath();
    // FIXME myWidgetCert.pem hard-coded for now
    certPath = certPath + QDir::separator()+"myWidgetCert.pem";
    setWidgetCertificatePath(certPath);
#if !defined(Q_OS_MAEMO5) && !defined(Q_OS_MAEMO6) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
    setWidgetRootPath(widgetUnZipPath());
#endif

    m_manifest = 0;
    m_widgetProperties = 0;
    m_widgetPropertiesCacheValid = false;

    m_widgetMap = new W3c2PlistMap();
    m_widgetMap->insert(Identifier, W3CSettingsKey::WIDGET_ID);
    m_widgetMap->insert(DisplayName, W3CSettingsKey::WIDGET_NAME);
    m_widgetMap->insert(MainHTML, W3CSettingsKey::WIDGET_CONTENT);
    m_widgetMap->insert(Version, W3CSettingsKey::WIDGET_VERSION);
    m_widgetMap->insert(DeltaVersion, W3CSettingsKey::WIDGET_DELTA);
    m_widgetMap->insert(MiniViewEnabled, "");
    m_widgetMap->insert(AllowNetworkAccess, "");

    m_widgetMap->insert(SharedLibraryFolder, 
            ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_FOLDER);
    m_widgetMap->insert(SharedLibraryWidget, 
            ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_WIDGET);
    m_widgetMap->insert(AllowBackgroundTimers, 
            ProprietarySettingsKey::WIDGET_BACKGROUND_TIMERS);

    m_size = 0;

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
    m_storage = WAC::Storage::createInstance(WIDGET_STORAGE);
    m_widgetInstaller = new WidgetInstaller(this, m_storage);
#else
    m_widgetInstaller = new WidgetInstaller(this, NULL);
#endif
}

void SuperWidget::setInstallationAttributes(WAC::InstallationAttributes& attributes) {
    if (m_widgetInstaller)
        m_widgetInstaller->setInstallationAttributes(attributes);
}

bool SuperWidget::createDir(const QString& path)
{

  QDir dir(path);

  if (!dir.exists()){
    if (!dir.mkdir(dir.absolutePath())) {
      LOG("Could not create dir : "<<path);
      return false;
      }
    }
    return true;
}

unsigned long SuperWidget::uncompressedSize(const QString& widgetPath) const
{
    unsigned long size = 0;
#ifdef Q_OS_SYMBIAN
    TChar driveLetter = m_widgetUnZipPath[0].toAscii();
    TRAP_IGNORE(size = WidgetUnzipUtilityS60::uncompressedSizeL(widgetPath, driveLetter));
#else
    size = Zzip::uncompressedSize(widgetPath);
#endif

    return size;
}

bool SuperWidget::checkDiskSpaceForInstallation(const QString& widgetPath) const
{
#ifdef Q_OS_SYMBIAN
    // Symbian has different zip handling.

    TInt error( 0 );

    // Precondition: widgetPath starts with a drive letter
    // get Symbian drive number from drive letter
    TChar driveLetter = m_widgetUnZipPath[0].toAscii();
    TInt driveNumber( 0 );
    error = RFs::CharToDrive(driveLetter, driveNumber);
    if (KErrNone != error)
        return false;
    // get free space on drive
    TInt64 driveSpace( 0 );
    error = driveInfo(driveNumber, driveSpace);
    if (KErrNone != error)
        return false;

    // calculate uncompressed size of the widget to be installed
    TUint64 uncompressedSize(0);
    TRAP_IGNORE(uncompressedSize = WidgetUnzipUtilityS60::uncompressedSizeL(widgetPath, driveLetter));

    // add 500 kBs safe buffer
    uncompressedSize += 500000;

    if (uncompressedSize > UINT_MAX)
        return false;

    TBool belowCriticalLevel(false);
    TRAP_IGNORE( belowCriticalLevel = SysUtil::DiskSpaceBelowCriticalLevelL(NULL, uncompressedSize, driveNumber));

    // compare space needed to space available
    if (uncompressedSize > driveSpace || belowCriticalLevel)
        return false;
    else
        return true;    // all ok

#else

    // check if there's enough disk space
    // check that uncompressed widget fits to file system
    // get uncompressed zip size...
    unsigned long uncompressedSize = Zzip::uncompressedSize(widgetPath);
    LOG("Widget uncompressed size is:" << uncompressedSize);

    unsigned long long availableSize = 0;

#ifdef Q_OS_LINUX
    // get available disk space...
    // there's no Qt way to do this, so this is platform dependent implementation
    struct statfs stats;
    statfs( m_widgetUnZipPath.toLocal8Bit(), &stats);
    availableSize = ((unsigned long long)stats.f_bavail) *
        ((unsigned long long)stats.f_bsize);
#else
    // Free size check not implemented for other platforms
    // TODO: Implement disk size check for other platforms
    return true;
#endif

    // Approx. 4 x uncompressed widget size should be enough in maemo;
    // The package is unzipped twice, and also widget & debian files are copied to
    // several places during the installation process
#ifdef __MAEMO__
    if (( availableSize/4) < uncompressedSize)
#else
    // 2 x widget size is enough for other platforms?
    if (( availableSize/2) < uncompressedSize)
#endif
    {
        qCritical() << "Widget package cannot be unzipped. Not enough free space";
        return false;
    }
    return true;
#endif
}

QString SuperWidget::getTempLocation()
  {
  QString tempLocation("");

#ifdef Q_OS_SYMBIAN
  // private path of the process
  tempLocation = QCoreApplication::applicationDirPath();
  // append 'Cache'
  tempLocation.append("\\Cache\\");
  LOG("SuperWidget::getTempLocation - Path after appending cache: " << tempLocation);
#else
  tempLocation =  QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
#endif

  return tempLocation;
  }

QString SuperWidget::getCertificatePath()
{
  QString certPath = "";
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
#ifdef Q_OS_SYMBIAN
  certPath = m_rootPath;
#else
  QString dataLocation =  QDesktopServices::storageLocation(QDesktopServices::DataLocation);
  certPath = QDir::toNativeSeparators(dataLocation);
#endif
  certPath = certPath+QDir::separator()+WIDGET_FOLDER;
  certPath = QDir::toNativeSeparators(certPath);
  if (! createDir(certPath)) {
    LOG("SuperWidget::getWidgetInstallPath - Could not create certificate path dir");
    return QString();
    }
  certPath = certPath+QDir::separator()+"widgetCertificates";
  certPath = QDir::toNativeSeparators(certPath);
  if (! createDir(certPath)) {
    LOG("SuperWidget::getWidgetInstallPath - Could not create certificate path dir");
    return QString();
    }
#else
  certPath = CERTIFICATE_PATH;
#endif

  return certPath;
}

// function to determine widget type
// parameters:
//     widgetPath    path to widget bundle
//     contentType   contentType from server
// return
//     WidgetType    type of widet
//
// Currently we are relaxing the contentType check in order to
// support modes where contentType is not known.
//
WidgetType SuperWidget::getWidgetType(const QString& widgetPath,
                                      const QString& contentType)
{
    LOG("SuperWidget::getWidgetType. widgetPath : "<<
            widgetPath<<" contentType : "<<contentType);

    if (widgetPath.isEmpty())
        return WidgetTypeUnknown;

    if (contentType.isEmpty() && QFileInfo(widgetPath).isDir())
        return getWidgetType(widgetPath);

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(MEEGO)
    if ((contentType.isEmpty() || (0 == QString::compare(CONTENT_TYPE_WGZ, contentType, 
            Qt::CaseInsensitive)))
        && (widgetPath.endsWith(EXTENSION_WGZ, Qt::CaseInsensitive))) {
        return WidgetTypeWgz;
    }
#endif

    if (!contentType.isEmpty() && 0 != QString::compare(CONTENT_TYPE_WGT, contentType, 
            Qt::CaseInsensitive))
        return WidgetTypeUnknown;

    QFile file(widgetPath);
    file.open(QIODevice::ReadOnly);

    if (0 == qstrncmp(WIDGET_PACKAGE_MAGIC_NUMBER_WGT, file.read(4).data(), 4))
        {
        file.close();
        return WidgetTypeW3c;
        }
    file.close();
    return WidgetTypeUnknown;
}

// function to unzip widget bundle
// parameters:
//     path    path to widget
// return
//     bool   true is good
//            false is bad
// widet will be unzipped to widgetUnZipPath()
// Currently using zlib to unzip widgets
// This method can be overrided to provide platform specific
// implementation.
//
bool SuperWidget::unZipBundle(const QString& path)
{
    LOG("OLDCODE: SuperWidget::unZipBundle with path arg" << path);
    
    unsigned long size = 0;
    QString widgetPath(path);

    LOG("OLDCODE: WidgetUnzipPath is : " << widgetUnZipPath());
    LOG("OLDCODE: WidgetBundlePath is : " << widgetBundlePath());
    
    // clean up unzip path
    if (!rmDir(widgetUnZipPath())) return false;
    // unzip bundle
    if (widgetPath.isEmpty())
        widgetPath = widgetBundlePath();
    LOG("UnZipBundle: widgetunzippath: "<<widgetUnZipPath());
#ifdef Q_OS_SYMBIAN
    if (WidgetUnzipUtilityS60::unzip(widgetPath, widgetUnZipPath(), size)) {
#else
    if (Zzip::unzip(widgetPath, widgetUnZipPath(), size) == 0) {
    	// Creating dummy folder as a hack to unblock CI
    	QDir unzippedWidgetPath(widgetUnZipPath());
    	if(!unzippedWidgetPath.exists("dummy"))
    		unzippedWidgetPath.mkdir("dummy");
#endif
        m_size = size;
        LOG("OLDCODE: unzip returned true");
        return true;
    } else {
        return false;
    }
}

// function to delete all files in a directory
// parameters:
//     path    path to directory that needs to be deleted
//
// Used to delete unzip directory
//
bool SuperWidget::rmDir(const QString& path)
{
#if defined(Q_OS_SYMBIAN)
    // CFileMan API is much faster than iterating!
    QString delPath(path);
    if(!path.endsWith('\\')) {
        delPath+='\\';
    }
    bool rtn(false);
    TRAP_IGNORE(
        RFs fs;
        if( KErrNone == fs.Connect() ) {
            CleanupClosePushL(fs);
            CFileMan* fsMan = CFileMan::NewL(fs);
            CleanupStack::PushL(fsMan);
            TPtrC symPath( reinterpret_cast<const TUint16*>(delPath.utf16()) );
            fsMan->RmDir(symPath);
            CleanupStack::PopAndDestroy(fsMan);
            CleanupStack::PopAndDestroy(&fs); // fs
            rtn = true;
        }
    )
    return rtn;
#else
    // get all files and folders list in the current folder
    QDirIterator it(path, QDir::Files|QDir::AllDirs|QDir::Hidden|QDir::NoDotAndDotDot);
    QDir currentDir(path);

    QEventLoop loop;
    while (it.hasNext()) {
        loop.processEvents(QEventLoop::AllEvents, 100);
        it.next();
        // get file/dir name
        QString cleanitem = QDir::cleanPath(it.fileInfo().absoluteFilePath());

        if ( it.fileInfo().isFile() ) {
            // delete file
            currentDir.remove(cleanitem);
        }
        else {
            // recursevly keep deleting the content
            rmDir(cleanitem);
            // now delete the empty folder
            currentDir.rmdir(cleanitem);
        }
    }
    return true;
#endif
}

// function to check if widget type is valid
// return:
//     bool    true if widget type is valid
//             false if widget is invalid
//
bool SuperWidget::isValidWidgetType()
{
    return (getWidgetType()!=WidgetTypeUnknown
            && getContentType() != CONTENT_TYPE_UNKNOWN);
}

// overloaded function to check if widget type is valid
// parameters:
//    widgetPath    path to widget
//    contentType   contentType provided by HTTP or obtained from MIME type
// return:
//     bool    true if widget type is valid
//             false if widget is invalid
//
bool SuperWidget::isValidWidgetType(const QString& widgetPath,
                                    const QString& contentType)
{
    LOG("SuperWidget::isValidWidgetType. widgetPath : "<<widgetPath<<" contentType : "<<contentType);
    return (getWidgetType(widgetPath,contentType) != WidgetTypeUnknown);
}

void SuperWidget::tryContinueInstallation() {
    m_continueInstallation = true;
}

void SuperWidget::tryCancelInstallation() {
    LOG(Q_FUNC_INFO);
    emit installationCancelled();
}

// function to register a widget with WebAppRegistry
//
// return
//    bool   true if successful
bool SuperWidget::registerWidget(const QString& startFile)
{
    // FIXME registerApp needs a better interface to pass values and
    // the widget processing should produce a struct with all required
    // values ready to pass like old code did
    WidgetProperties* widgetProps = widgetProperties();
    bool status = false;
    if (widgetProps) {
        LOG("registerWidget");
            QString startFileFullPath = widgetProps->installPath()+QDir::separator()+startFile;
            LOG("The title's direction attribute(ltr/rtl/lro/rlo):" << widgetProps->titleDir());
            // Add Text Direction attribute as arguments when platform provides support.
            // Following elements will provide support: Name, Author, Description & License
#ifdef OPTIMIZE_INSTALLER
        QSet<QString>langs = widgetProps->languages();
        QString version = (widgetProps->plistValue(W3CSettingsKey::WIDGET_VERSION)).toString();
        QString author = (widgetProps->plistValue(W3CSettingsKey::WIDGET_AUTHOR)).toString();
#else
	          QSet<QString> langs = m_manifest->languages();
              QString version = m_manifest->value(W3CSettingsKey::WIDGET_VERSION);
              QString author = m_manifest->value(W3CSettingsKey::WIDGET_AUTHOR);
#endif
    
	
	        status
                = (WebAppRegistry::instance()->registerApp(widgetProps->id(),
                                                           widgetProps->title(),
                                                           widgetProps->installPath(),
                                                           widgetProps->iconPath(),
                                                           widgetProps->plist(),
                                                           widgetProps->type(),
                                                           widgetProps->size(),
                                                           startFileFullPath,
                                                           widgetProps->hideIcon(),
                                                           langs,
                                                           version,
                                                           author));

#if !defined(Q_OS_LINUX)
            // Save SecSession
            if (status) {
                LOG("saving secsession");
                QString resourcesDir = widgetProps->resourcePath();
                if (!resourcesDir.isNull() && !resourcesDir.isEmpty()) {
                    QDir dir;
                    dir.mkpath(resourcesDir);
                    QString secSessionFileName(QDir::toNativeSeparators(resourcesDir));
                    secSessionFileName += QDir::separator();
                    secSessionFileName += SECSESSION_FILE;
                    QFile secSessionFile(secSessionFileName);
                    if (secSessionFile.open(QIODevice::WriteOnly)) {
                        QTextStream stream(&secSessionFile);
                        stream << widgetProps->secureSessionString(); //codescanner::leave
                        secSessionFile.close();
                    }

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
                    // TODO:
                    // Add to secure storage
                    m_storage->add(secSessionFile);
#endif
                }
            }
#endif
    }
    return status;
}

// function to uninstall the widget
// parameters:
//     widgetProps    widget properties
//     removeData     remove wigdet data flag
// return:
//     WidgetUninstallError       WidgetUninstallSuccess or error reason
//
// removes widget from registry and deletes all widget files
//
WidgetUninstallError SuperWidget::uninstall(const QString &uniqueIdentifier, 
        const bool removeData, 
        WidgetManager *wm)
{
    WebAppInfo info;
    if (WebAppRegistry::instance()->isRegistered(uniqueIdentifier, info)) {
        if (info.isPresent()) {
            if (wm)
                wm->uninstallProgress(25);

            if (!WidgetInstaller::uninstall(info.appPath(), uniqueIdentifier)) {
                return WidgetUninstallFailed;
            }

            if (wm)
                wm->uninstallProgress(50);

#if !defined(Q_OS_LINUX)
            // Remove SecSession
            QString resourcesDir = resourcePath(info.appPath());
            if (!resourcesDir.isNull() && !resourcesDir.isEmpty()) {
                if (removeData) {
                    // Remove all widget data
                    rmDir(resourcesDir);
                }
                else {
                    // Leave widget data, remove only secsession
                    QString secSessionFileName(QDir::toNativeSeparators(resourcesDir));
                    secSessionFileName += QDir::separator();
                    secSessionFileName += SECSESSION_FILE;
                    QFile::remove(secSessionFileName);
                }
                QDir dir;
                dir.rmpath(resourcesDir);

                if (wm)
                    wm->uninstallProgress(60);
            }
#endif
        }

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
        // TODO:
        // Remove from secure storage
#endif
        if (wm)
            wm->uninstallProgress(75);

        LOG("widget uninstall " << info.appPath());
        if (WebAppRegistry::instance()->unregister(uniqueIdentifier)) {
#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
            TRAP_IGNORE(SuperWidget::NotifyAppArcOfUninstallL(info.uid()));
#endif
            if (wm)
                wm->uninstallProgress(100);

            return WidgetUninstallSuccess;
        }
    }
    return WidgetUninstallFailed;
}

QString SuperWidget::resourcePath(const QString& installPath) {
    QDir dir(installPath);
    QString resourcePath;
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    resourcePath = DATA_PATH;
#else
    if (installPath.section(QDir::separator(),4,4) == SHARED_LIBRARY_FOLDER)
        resourcePath = installPath.left(installPath.indexOf(WIDGET_FOLDER, 
                Qt::CaseInsensitive))+ 
                        QString(DATA_FOLDER) + QDir::separator() + 
                        SHARED_LIBRARY_FOLDER + QDir::separator();
    else
        resourcePath = installPath.left(installPath.indexOf(WIDGET_FOLDER, 
                Qt::CaseInsensitive))+ 
        QString(DATA_FOLDER) + QDir::separator();
#ifdef Q_OS_SYMBIAN
    // Ensure that the resource path is on C drive
    if (resourcePath[1] == ':') {
        resourcePath.replace(0, 1, 'C');
    }
#endif
#endif
    QString resourcesDir = resourcePath + dir.dirName();
    resourcesDir = QDir::toNativeSeparators(resourcesDir);
    resourcesDir += QDir::separator();
    return resourcesDir;
}

// function to get widget properties
// parameters:
//     pkgPath             path to widget files
// return:
//     WidgetProperties*   pointer to widget properties
//
// parses widget manifest and returns widget properties
//
WidgetProperties* SuperWidget::getProperties(const QString& pkgPath)
{

    LOG("SuperWidget::getProperties path="<<pkgPath);

    setWidgetRootPath(pkgPath);
    if (!parseManifest(pkgPath))
    {
        LOG("getProperties : failed");
        return 0;
    }

    WidgetProperties* props = widgetProperties();

    return props;


}

#ifdef Q_OS_MAEMO6
bool SuperWidget::createErrorDebian(const WidgetInstallError& error)
{
    QString packageName = "wrt-widget-error";
    QString installationPath = "/var/tmp/";

    QString source = widgetUnZipPath();
    QString outputPath = "";
    QString productId = "";
    bool no_install(false);

    QStringList arguments = QCoreApplication::arguments();

    for (int i = 0; i < arguments.size(); ++i) {
        if (arguments.at(i) == "--path") {
            if(i+1 < arguments.size())
                outputPath = arguments.at(i+1);
        } else if (arguments.at(i) == "--ovi-productid") {
            if(i + 1 < arguments.size()) {
                productId = arguments.at(i+1);
                packageName = OVISTORE_WEBAPPS_PACKAGE_NAME_PREFIX + productId + "-error";
            }
        }
        else if(arguments.at(i) == "--no-install")
            no_install =  true; 
    }
    if(no_install){
        PackageUtils* packageUtils = new DebianUtils(this, packageName, source, 
                installationPath, packageName);
        if (!packageUtils->createErrorPackage()) {
            qCritical() << "Couldn't create package";
            return false;
        }
        
        if (outputPath.isEmpty())
            outputPath = QDir::currentPath();

        if (!outputPath.isEmpty()) {

            outputPath.append(QDir::separator() + packageUtils->packageFileName());

            if (QFile::exists(outputPath))
                QFile::remove(outputPath);

            if (!QFile::rename(packageUtils->packageFilePath(), outputPath)) {
                qCritical() << "Couldn't move the package";
                return false;
            }
            return true;
        }
    }
    return false;
}
#endif // maemo6 specific code 


// function to clean up widget files
// parameters:
//    path   path to cleanup (optional)
// return:
//    bool   true if successful
//
// removes all files from unzip path
//
#ifdef Q_OS_MAEMO6
bool SuperWidget::cleanup( const WidgetInstallError& error,const QString& path )
#else //!Q_OS_MAEMO6
bool SuperWidget::cleanup( const WidgetInstallError& /*error*/,const QString& path )
#endif //Q_OS_MAEMO6
{
    QString rmPath(path);
    if (rmPath.isEmpty())
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    {
#ifdef Q_OS_MAEMO6        
       if(error != WidgetInstallSuccess)
            createErrorDebian(error);
#endif
        QDir tmpDir(widgetUnZipPath());
        tmpDir.cdUp();
#if USE(AEGIS)
        tmpDir.cdUp();
#endif
        rmPath=tmpDir.absolutePath();
    }
#else
    rmPath = widgetUnZipPath();
#endif
    if (!rmDir(rmPath)) {
        LOG("Widget cleanup failed");
        return false;
    }
    LOG("Widget cleanup done");
    return true;
}

    
// overloaded
 bool SuperWidget::cleanup( const WidgetInstallError& error,const bool& removed, const QString& path )
 {
   if (!removed)
       return cleanup(error,path);
   return true;
 }

bool SuperWidget::parseManifest( const QString&, const bool )
{
    return false;
}

bool SuperWidget::isValidDigitalSignature( WidgetInstallError &,
                                           const QString&,
                                           bool,
                                           bool )
{
    return false;
}

bool SuperWidget::findFeatures( WidgetFeatures&, const QString& )
{
    return false;
}

//function to get widget type from path
// parameters:
//    path         path to unzipped widget files
// return:
//    WidgetType   type of widget
//
WIDGETUTILS_EXPORT WidgetType SuperWidget::getWidgetType(const QString& path)
{
    // This ensures that all paths in the cache end with a separator
    // in order to prevent duplicate entries.
    QString sep = QDir::separator();
    QString sPath(QDir::toNativeSeparators(path));
    if (!path.endsWith(sep)) {
        sPath += sep;
    }

    // Check cache first for faster file access
    QHash<QString,int>::const_iterator it = m_widgetTypeCache.find(sPath);
    if (it != m_widgetTypeCache.end())
    {
        return (WidgetType)it.value();
    }

    WidgetType type(WidgetTypeUnknown);

    // W3C packaging spec: "A widget package has exactly one
    // configuration document located at the root of the widget
    // package."  Also: "A valid configuration document file name is
    // the string 'config.xml'." Only lowercase is legal.
    if (QFile::exists(sPath+"config.xml")) {
        type = WidgetTypeW3c;
    }
    // S60 WRT: Doc at http://library.forum.nokia.com/ "Web
    // Developer's Library 1.7 / Web Runtime Widgets / Developing
    // widgets / Widget component files / Creating the info.plist
    // file" says: "Save the XML document under the widget project
    // root folder with the name info.plist."
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
    else if (QFile::exists(sPath+"info.plist") ||
             QFile::exists(sPath+"Info.plist") ||
             QFile::exists(sPath+"INFO.plist")) {
        // FIXME: The Forum Nokia documentation may not agree with the
        // existing S60 WRT code which is both case insensitive and
        // maybe doesn't require info.plist at root.
        type = WidgetTypeWgz;
    }
#endif
    m_widgetTypeCache.insert(sPath, type);
    return type;
}

// function to split parts of widget file path
// parameters:
//    rootParts     has two parts, first is root of widget,
//                  second is relative path
//    path          path to file in widget
// return:
//    bool          true if successful
//
bool SuperWidget::getWidgetPathParts(QStringList& rootParts, const QString& path)
{

    if (path.isEmpty()) return false;
    QRegExp part(WIDGET_FOLDER);
    QString part2 = path.section(part,1,1);
    QString urlSeparator("/");
    if (part2.isEmpty())
    {
        // If part2 is empty, it means we are not coming from networkaccessmanager.
        // We have to just send back the path.
        rootParts.append(path.section(part,0,0));
    }
    else {
        // We spit the absolute path into two parts: <widget-root> and <widget-relative> path
        // absolute path: <widget-root>/<widget-relative>
        // widget-root  : <some-base-path>/<widget-folder>/<widget-id>

        if (part2.startsWith(urlSeparator + SHARED_LIBRARY_FOLDER)) {
            // part 1 is <sharedlib-root>
            rootParts.append(path.section(part,0,0)+WIDGET_FOLDER+urlSeparator+
                                part2.section(urlSeparator,0,1,QString::SectionSkipEmpty));
            // part 2 is <sharedlib-relative>
            rootParts.append(part2.section(urlSeparator,2,-1,QString::SectionSkipEmpty));
        }
        else {
            // part 1 is <widget-root>
            rootParts.append(path.section(part,0,0)+WIDGET_FOLDER+urlSeparator+
                                part2.section(urlSeparator,0,0,QString::SectionSkipEmpty));
            // part 2 is <widget-relative>
            rootParts.append(part2.section(urlSeparator,1,-1,QString::SectionSkipEmpty));
        }
    }
    LOG("full path = " << path);
    LOG("relative file = " << part2.section(urlSeparator,1,-1,QString::SectionSkipEmpty));
    LOG("root = " << rootParts.at(0));
    return (rootParts.count()>0);
}

#ifdef Q_OS_SYMBIAN
TInt SuperWidget::driveInfo( TInt aDrive, TInt64& aDiskSpace ) const
{
    RFs fs;
    User::LeaveIfError(fs.Connect()); // not possible to continue without RFs
    CleanupClosePushL(fs);
    _LIT( KFat, "Fat" );

    // Check if the drive is already mounted
    TFullName name;
    TInt error(fs.FileSystemName(name, aDrive));
    if (error) {
        CleanupStack::PopAndDestroy(&fs);
        return KErrNotReady;
    } else {
        // check if MMC already mounted
        if (name.Length() == 0) {
            // MMC drive isnt mounted at present, so try it now....
            error = fs.MountFileSystem( KFat, aDrive );

            // If it's a locked MMC and the password is already known it'll be
            // unlocked automatically when it's mounted., otherwise the mount will
            // return with KErrLocked.....
            if (error == KErrLocked) {
                CleanupStack::PopAndDestroy(&fs);
                return KErrLocked;
            }
        }
    }

    TDriveInfo driveInfo;
    error = fs.Drive( driveInfo, aDrive );
    if (!error) {
        // MMC is in slot
        if (driveInfo.iMediaAtt & KMediaAttLocked) {
            CleanupStack::PopAndDestroy(&fs);
            return KErrLocked;
        }

        TVolumeInfo volumeInfo;
        error = fs.Volume( volumeInfo, aDrive );
        if (!error) {
            aDiskSpace = volumeInfo.iFree;
            CleanupStack::PopAndDestroy(&fs);
            return KErrNone;    // all ok, diskspace set
        }
    }

    CleanupStack::PopAndDestroy(&fs);
    return KErrNotReady;
}
#endif


#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
// static
void SuperWidget::NotifyAppArcOfInstallL(int uid)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());

    RArray<TApaAppUpdateInfo> updates;
    updates.AppendL(TApaAppUpdateInfo(TUid::Uid(uid), TApaAppUpdateInfo::EAppPresent));
    session.UpdateAppListL(updates);
    updates.Close();
    session.Close();
}

// static
void SuperWidget::NotifyAppArcOfUninstallL(int uid)
{
    RApaLsSession session;
    User::LeaveIfError(session.Connect());

    RArray<TApaAppUpdateInfo> updates;
    updates.AppendL(TApaAppUpdateInfo(TUid::Uid(uid), TApaAppUpdateInfo::EAppNotPresent));
    session.UpdateAppListL(updates);
    updates.Close();
    session.Close();
}
#endif
