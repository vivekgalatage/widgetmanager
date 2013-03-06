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
 
#if defined(Q_OS_MAEMO6)
#include "packagemanager.h"
#include "packagemanagerpendingcallwatcher.h"
#include "WgtWidget.h"
#endif

#include "widgetinstaller.h"
#include "widgetinstaller_p.h"
#include "debianutils.h"
#include "featuremapping.h"
#include "rpmutils.h"
#include "storage.h"
#include "wacw3csettingskeys.h"
#include "wbenchmark.h"

#if defined(Q_OS_MAEMO5)
#include <dbus/dbus.h>
#include <hildon-mime.h>
#endif

#include <QDateTime>
#include <QProcess>
#include <QCoreApplication>
#include <sys/stat.h>
#include <sys/vfs.h>

#if defined(Q_OS_MAEMO6)
static const int PKGMGR_INSTALL_TIMEOUT = 60000;
static const int PKGMGR_INSTALL_INTERVAL = 100;
static const int PKGMGR_WAITFIRST_TIMEOUT = 20000;
static const int PKGMGR_WAITFREE_TIMEOUT = 120000;
static const int PKGMGR_WAITFREE_INTERVAL = 1000;
#endif


/*!
    \internal
    \class WidgetInstallerPrivate
    \brief Platform specific installations.

    It's only a QObject for Maemo6 as it requires to listen to the signals from Package Managers.
 */

/*!
    \internal
    \enum WidgetInstallerPrivate::InstallationStatus
    \value Success The installation is successful. Note that as we can't track the return value of Application Manager on Maemo 5, it may still return this
           even if the installation actually fails.
    \value PackageCreationFailed Can't create the package.
    \value PackageInstallationFailed Can't install the generated package.
    \value PackageMovingFailed Can't move the generated package to the directory specified by the user.
    \value SecureStorageSetupFailed Can't put the installed widgets in secure storage.
    \value NotEnoughSpace Don't have enough space for the package.
    \value OperationCancelled The installation is cancelled by the user.
 */

/*!
    \internal
    Constructs a WidgetInstallerPrivate object to install the \a widget.
 */
WidgetInstallerPrivate::WidgetInstallerPrivate(SuperWidget* widget)
#if defined(Q_OS_MAEMO6)
     : QObject(NULL), m_packageManager( NULL), m_widget(widget), m_packageUtils(NULL),
       m_noInstall(false), m_userCancelled(false), m_userInformedWait(false), m_fetcher(NULL)
#else
    : m_widget(widget), m_packageUtils(NULL), m_noInstall(false)
#endif
{
}

/*!
    \internal
    Destroys the object.
 */
WidgetInstallerPrivate::~WidgetInstallerPrivate()
{
#if defined(Q_OS_MAEMO6)
    delete m_packageManager;
    m_packageManager = NULL;
#endif
    delete m_packageUtils;
    m_packageUtils = NULL;
}

/*!
    \internal
    Install the unzipped widgets at \a source to \a target. The Unique ID is given as \a appId.
 */
WidgetInstallerPrivate::InstallationStatus WidgetInstallerPrivate::install(QString source, QString target, QString appId)
{
    WBM("Maemo specific installation");

    m_appId = appId;
    m_packageName = m_appId + "-wrt-widget";
    
    QStringList arguments = QCoreApplication::arguments();
    for (int i = 0; i < arguments.size(); ++i) {
        if (arguments.at(i) == "--path") {
            if(i+1 < arguments.size())
                m_outputPath = arguments.at(i+1);
        } else if (arguments.at(i) == "--ovi-productid") {
            if(i + 1 < arguments.size()) {
                m_productId = arguments.at(i+1);
                m_packageName = OVISTORE_WEBAPPS_PACKAGE_NAME_PREFIX + m_productId;
            }
        }
    }

    m_installationPath = target;
    if (!m_installationPath.endsWith(QDir::separator()))
        m_installationPath.append(QDir::separator());

#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6)
    m_packageUtils = new DebianUtils(m_widget, m_packageName, source, m_installationPath, m_appId);
    
#if ENABLE(AEGIS_LOCALSIG)
    //If coming from from trusted zone set the the flag to true for the
    //debian utils and it will create the local signature
    if (m_widget->getWidgetType() == WidgetTypeW3c || m_widget->getWidgetType() == WidgetTypeJIL) {
        WgtWidget* wgtwidget = static_cast<WgtWidget*>(m_widget);
        if (wgtwidget && wgtwidget->isFromTrustedDomain()) {
            static_cast<DebianUtils*>(m_packageUtils)->installFromTrustedOrigin(true);
        }
    }
#endif  //AEGIS_LOCALSIG
    
#elif defined(Q_OS_MEEGO)
    m_packageUtils = new RpmUtils(m_widget, m_packageName, source, m_installationPath, m_appId);
#else
    // TODO: fix for other platforms
#endif

#if defined(Q_OS_MAEMO6)
     if(m_userCancelled)
         return OperationCancelled;
#endif

    if (!m_packageUtils->createPackage()) {
        qCritical() << "Couldn't create package";
        emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
        return PackageCreationFailed;
    }

#if defined(Q_OS_MAEMO6)
    QCoreApplication::processEvents();
    if(m_userCancelled)
        return OperationCancelled;
    m_packageManager = new PackageManager();
#endif

    int pathIndex = arguments.indexOf("--installation-info-file") + 1;
    if (pathIndex) {
        QFile file(arguments.at(pathIndex));
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "debcreated: " << m_packageUtils->packageFilePath() << "\n";
            out << "installpath: " << m_installationPath;
            file.close();
        } else
            qCritical() << "Could not open infofile (" << arguments.at(pathIndex) << ") for writing";
    }

    if (m_outputPath.isEmpty() && m_noInstall)
        m_outputPath = QDir::currentPath();

    if (!m_outputPath.isEmpty()) {
        if (!hasEnoughSpace(m_packageUtils->packageFilePath(), m_outputPath)) {
            qCritical() << "Not enough space";
            emit m_widget->installationError(WidgetInsufficientDiskSpace);
            return NotEnoughSpace;
        }

        m_outputPath.append(QDir::separator() + m_packageUtils->packageFileName());
        if (QFile::exists(m_outputPath))
            QFile::remove(m_outputPath);

        if (!QFile::rename(m_packageUtils->packageFilePath(), m_outputPath)) {
            qCritical() << "Couldn't move the package";
            emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
            return PackageMovingFailed;
        }

        return Success;
    }
    
#if defined(Q_OS_MAEMO6)
    QCoreApplication::processEvents();
    if(m_userCancelled) {
        return OperationCancelled;
    }
#endif

    if (!installPackage()) {
#if defined(Q_OS_MAEMO6)
        if (m_userCancelled)
            return OperationCancelled;
#endif
        qCritical() << "Couldn't install Debian package";
        return PackageInstallationFailed;
    }

    return Success;
}

/*!
    \internal
    Set the \a attributes for installation.
 */
void WidgetInstallerPrivate::setInstallationAttributes(WAC::InstallationAttributes attributes)
{
    if (attributes & WAC::NoInstall)
        m_noInstall = true;
}

#if defined(Q_OS_MAEMO5)
/*!
    \internal

    This should be removed once we use QDesktopServices::openUrl to open the Debian package.
 */
static gboolean quit_mainloop(gpointer data)
{
    g_main_loop_quit((GMainLoop*)data);
    return true;
}
#endif

/*!
    \internal

    Installs the generated package.
 */
bool WidgetInstallerPrivate::installPackage()
{
#if defined(Q_OS_MAEMO6)
    qDebug() << "Install package with Package Manager";

    PackageManagerPendingCallWatcher* watcher = NULL;

    // First check package manager state, is it free

    m_pkgmgrInstallationSuccess = false;
    m_pkgmgrMutex.lock();
    m_pkgmgrInstallationTime = PKGMGR_WAITFIRST_TIMEOUT/PKGMGR_WAITFREE_INTERVAL;

    bool isLockObtained = false;
    QCoreApplication::processEvents();

    while (m_pkgmgrInstallationTime > 0 && !isLockObtained  && !m_userCancelled) {
        if(!m_pkgmgrInstallationSuccess && !fetchPkgmgrState())
            return false;
        isLockObtained = m_pkgmgrMutex.tryLock(PKGMGR_WAITFREE_INTERVAL);
        QCoreApplication::processEvents();
        m_pkgmgrInstallationTime--;
    }

    if(m_userCancelled)
        return false;

    if(!m_pkgmgrInstallationSuccess) {
        if (m_pkgmgrInstallationTime <= 0) {
            if(m_userInformedWait)
                qCritical() << "Couldn't get Package Manager free state, is it too busy?";
            else
                qCritical() << "Couldn't get Package Manager state, is it running?";
            emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
        }
        return false;
    }

    m_pkgmgrMutex.unlock();

    // So pkgmgrd is free, start installation itself
    connect(m_packageManager, SIGNAL(operationComplete(const QString&, const QString&, const QString&, const QString&, const bool)),
            SLOT(pkgmgrOperationComplete(const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);
    connect(m_packageManager, SIGNAL(operationProgress(const QString&, const QString&, const QString&, int)),
            SLOT(pkgmgrOperationProgress(const QString&, const QString&, const QString&, int)), Qt::QueuedConnection);
    connect(m_packageManager, SIGNAL(operationAborted(const QString &, const QString &, const QString &, const QString &)),
            SLOT(pkgmgrOperationAborted(const QString &, const QString &, const QString &, const QString &)));

    if(m_userCancelled)
        return false;

    m_pkgmgrInstallationSuccess = false;
    watcher = m_packageManager->installFileUi(m_packageUtils->packageFilePath());
    if(m_userCancelled)
        return false;
    if (watcher != NULL) {
        if (watcher->errorCode() != PackageManager::ErrorNone) {
            qCritical() << "Package Manager failed: " << watcher->errorName() << watcher->errorMessage();
            emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
            return false;
        }

        connect(watcher, SIGNAL(dbusError(PackageManagerPendingCallWatcher*)),
                SLOT(pkgmgrDbusError(PackageManagerPendingCallWatcher*)), Qt::QueuedConnection);
    } else {
        qCritical() << "Couldn't create Package Manager watcher";
        emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
        return false;
    }

    m_pkgmgrMutex.lock();
    m_pkgmgrInstallationTime = PKGMGR_INSTALL_TIMEOUT / PKGMGR_INSTALL_INTERVAL;
    isLockObtained = false;
    while (m_pkgmgrInstallationTime > 0 && !isLockObtained && !m_userCancelled) {
        isLockObtained = m_pkgmgrMutex.tryLock(PKGMGR_INSTALL_INTERVAL);
        QCoreApplication::processEvents();
        m_pkgmgrInstallationTime--;
    }
    m_pkgmgrMutex.unlock();

    if(!m_pkgmgrInstallationSuccess && !m_userCancelled)
        emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);

    return m_pkgmgrInstallationSuccess && !m_userCancelled;
#elif defined(Q_OS_MAEMO5)
    // TODO should use QDesktopServices::openUrl to open the Debian package

    qDebug() << "Install package with Application Manager launched by Hildon framework";

    DBusConnection* dbusConnection;
    DBusError dbusError;
    GMainLoop* mainloop = g_main_loop_new(NULL, FALSE);

    if (mainloop == NULL) {
      qCritical() << "Couldn't create mainloop";
      return false;
    }

    dbus_error_init(&dbusError);
    dbusConnection = dbus_bus_get(DBUS_BUS_SESSION, &dbusError);

    if (dbusConnection == NULL) {
        qCritical() << "Couldn't get DBus connection: " << dbusError.message;
        dbus_error_free(&dbusError);
        return false;
    }

    if (hildon_mime_open_file(dbusConnection, m_packageUtils->packageFilePath().toAscii().constData()) != 1) {
        qCritical() << "Cannot launch Application Manager";
        return false;
    }

    g_timeout_add(1000, quit_mainloop, (void*)mainloop);
    g_main_loop_run(mainloop);
#elif defined(Q_OS_MEEGO)
    // TODO should use QDesktopServices::openUrl

    QProcess process;
    process.start("/usr/bin/gpk-install-local-file", QStringList() << m_packageUtils->packageFilePath());
    process.waitForFinished();
    return (process.exitCode() == 0);
#endif

    return true;
}

#if defined(Q_OS_MAEMO6)
/*!
    \internal
 */
bool WidgetInstallerPrivate::fetchPkgmgrState()
{
    if(m_fetcher)
        return true;

    m_fetcher = m_packageManager->fetchCurrentOperationData();
    if (m_fetcher != NULL) {
        if (m_fetcher->errorCode() != PackageManager::ErrorNone) {
            qCritical() << "Package Manager state fetch failed: " << m_fetcher->errorName() << m_fetcher->errorMessage();
            emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
            return false;
        }
        connect(m_fetcher, SIGNAL(dataFetched(const QMap<QString, QVariant>&)), this,
                SLOT(pkgmgrDataFetched(const QMap<QString, QVariant>&)), Qt::DirectConnection);
    } else {
        qCritical() << "Couldn't create Package Manager fetcher";
        emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
        return false;
    }
    return true;
}

/*!
    \internal
 */
void WidgetInstallerPrivate::installationCancelled()
{
    qDebug() << "\nINSTALLATION CANCELLED!\n";
    m_pkgmgrInstallationSuccess = false;
    m_pkgmgrInstallationTime = 0;
    m_userCancelled = true;
    emit m_widget->installationError(WidgetInstallCancelled);
}

/*!
    \internal
 */
void WidgetInstallerPrivate::pkgmgrDataFetched(const QMap<QString, QVariant>& fetcher)
{
    // If fetcher is empty, we can continue installation
    // otherwise pkgmgr is doing something else and is not available this time
    if (fetcher.isEmpty()) {
        m_pkgmgrInstallationSuccess = true;
        qDebug() << "Pkgmgrd state:    FREE";
        emit m_widget->installationError(PackageManagerStarted); // Hide preinstall widget
        m_pkgmgrMutex.unlock();
    } else if(!m_userInformedWait) {
        emit m_widget->installationError(PackageManagerBusy);
        m_userInformedWait = true;
        m_pkgmgrInstallationTime = PKGMGR_WAITFREE_TIMEOUT/PKGMGR_WAITFREE_INTERVAL;
        qDebug() << "Pkgmgrd state:    BUSY - started to wait, timeout is" << PKGMGR_WAITFREE_TIMEOUT/1000 << "s";
    }
    m_fetcher = NULL;
}

/*!
    \internal
 */
void  WidgetInstallerPrivate::pkgmgrOperationComplete(const QString& operation, const QString& packageName, const QString& version, const QString& result)
{
    Q_UNUSED(version)

    if (m_packageUtils->packageFilePath() == packageName) {
        qDebug() << Q_FUNC_INFO;
        qDebug() << "  Operation:" << operation;
        qDebug() << "  Package:" << packageName;
        qDebug() << "  Result:" << result;

        m_pkgmgrInstallationSuccess = !result.contains("error", Qt::CaseInsensitive);
        m_pkgmgrMutex.unlock();
    }
}

/*!
    \internal
 */
void WidgetInstallerPrivate::pkgmgrOperationAborted(const QString &operation, const QString &packageidentifier, const QString &version, const QString &reason)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "  Package:" << packageidentifier;
    qDebug() << "  Reason:" << reason;

    m_pkgmgrInstallationSuccess = false;
    m_pkgmgrInstallationTime = 0;
}

/*!
    \internal
 */
void WidgetInstallerPrivate::pkgmgrDbusError( PackageManagerPendingCallWatcher* watcher)
{
    if (watcher == NULL)
        return;

    qDebug() << Q_FUNC_INFO;
    qDebug() << "  Error name:" << watcher->errorName();
    qDebug() << "  Error message:" << watcher->errorMessage();

    m_pkgmgrInstallationSuccess = false;
    m_pkgmgrInstallationTime = 0;
}

/*!
    \internal
 */
void WidgetInstallerPrivate::pkgmgrOperationProgress(const QString& operation, const QString& packageName, const QString& version, int percentage)
{
    Q_UNUSED(operation)
    Q_UNUSED(version)

    if (m_packageUtils->packageFilePath() == packageName) {
        m_pkgmgrInstallationTime = qMax(m_pkgmgrInstallationTime, PKGMGR_INSTALL_TIMEOUT / PKGMGR_INSTALL_INTERVAL / 2);
        qDebug() << "Pkgmgrd progress:" << percentage << "%" ;
    } else {
        emit m_widget->installationError(WidgetPlatformSpecificInstallFailed);
        m_pkgmgrInstallationSuccess = false;
        m_pkgmgrInstallationTime = 0;
    }
}
#endif

/*!
    \internal

    Checks if we have enough space for installation.
 */
bool WidgetInstallerPrivate::hasEnoughSpace(const QString& file, const QString& path) const
{
    unsigned long fileSize = QFile(file).size();

    unsigned long long availableSize = 0;
    struct statfs stats;
    statfs(path.toLocal8Bit(), &stats);
    availableSize = ((unsigned long long)stats.f_bavail) * ((unsigned long long)stats.f_bsize);

    if (availableSize < fileSize)
        return false;

    return true;
}
