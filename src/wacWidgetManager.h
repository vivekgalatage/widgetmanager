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


#ifndef _WAC_WIDGET_MANAGER_H_
#define _WAC_WIDGET_MANAGER_H_

#include "wacWidgetUtils.h"
#include "wacSuperWidget.h"

#ifdef OPTIMIZE_INSTALLER

#include "widgettypes.h"

#if defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)
#include <aknnotewrappers.h>
#include "CUIDialogs.h"

using namespace SwiUI;

#endif // defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)

#endif // OPTIMIZE_INSTALLER

class HbAction;
class WidgetProperties;
class WidgetManagerPrivate;

namespace WAC {
    namespace Maemo {
        class WidgetInstallerTestHelper;
    }
}

/* Widget managers own widget installation and launching:
 *  Manage widget storage
 *  Own widget preferences
 *  Use widget factory at install and launch
 */

/* Use the widget engine and package management components */
class WIDGETUTILS_EXPORT WidgetManager : public QObject
{
    Q_OBJECT

public:
    explicit WidgetManager(QWidget* parent,
                  bool isChromeless=false);
    ~WidgetManager();

    QString launcherPath(const QString& pkgPath);
    bool isRegistered(const QString& pkgPath);

#ifdef OPTIMIZE_INSTALLER
    int installWidget( const QString& pkgPath, const QMap<QString, QVariant>& configDetails );
#endif //OPTIMIZE_INSTALLER

    WidgetInstallError install(const QString& pkgPath,
                               QString& widgetId,
                               bool silent = false,
                               bool update = false,
                               const QString& rootDirectory = "");
#ifdef OPTIMIZE_INSTALLER
    void cancel( int trxId );
#endif // OPTIMIZE_INSTALLER
    void asyncInstall(const QString& pkgPath,
                      bool silent,
                      const QString& rootDirectory = "",
                      bool update=false);
    WidgetUninstallError uninstall(const QString& uniqueIdentifier,
                                   bool silent=false);
    WidgetProperties* getProperties(const QString& pkgPath);
    WidgetProperties* getProperties();
    bool isValidWidgetType(const QString& widgetPath, const QString& contentType);
    bool isValidWidgetType(const QString& widgetPath);
    void setContentType(const QString& contentType);
    bool getContentType(QString& contentType);
    void setInstallationAttributes(const WAC::InstallationAttributes& attributes);

    //Backup-restore widget install.
    bool backupRestoreInstall(const QString& pkgPath,
                              bool disableUnsigWidgetSignCheck);

    void removeWidgetFromCache(const QString& pkgPath);

    QString installSisx(const QString& pkgPath,
                        const QString& rootDirectory = "");
    WidgetUninstallError uninstallSisx(const QString& uniqueIdentifier,
                                       bool aRemoveFromList);
    QHash<QString,uint> readSisxWidgetList();
    void writeSisxWidgetList(QHash<QString,uint>& aWidgetList);
    QString sisxWidgetFile();

Q_SIGNALS:
    void queryConfirmation();
    void installComplete(QString widgetId,
                         WidgetInstallError status);
    void installProgress( int percentComplete );
#ifdef OPTIMIZE_INSTALLER 
    void progress( int transId, int percentComplete, WidgetOperationType operationType );
#endif // OPTIMIZE_INSTALLER

    /**
     * This signal is emitted whenever an existing widget is about to be replaced during installation
     * phase.
     *
     * If you intend to replace the widget when this message is signaled, you should
     * tryContinueInstallation() method when you're handling this signal.
     */
    void aboutToReplaceExistingWidget(QString widgetTitle);

    /**
     * This signal is emitted whenever a widget destination is to be selected
     *
     * If you intend to continue installation of the widget when this message is signaled,
     * you should tryContinueInstallation) method when you're handling this signal.
     */
    void queryInstallationDestination(unsigned long spaceRequired, bool allowRemovableInstallation);

     /**
     * This signal is emitted whenever a widget with features is about to be installed.
     *
     *
     * If you intend to install the widget when this message is signaled, you should
     * tryContinueInstallation() method when you're handling this signal.
     */
    void aboutToInstallWidgetWithFeatures(QList<QString> capList);

    /**
     * This signal is emitted whenever an error occurs during installation phase.
     *
     * If you intend to continue installation when error is signaled, you should
     * invoke tryContinueInstallation() method when you're handling installation error.
     *
     */
    void installationError(WidgetInstallError error);

    /**
     * This signal is emitted when a not signed widget is being installed.
     *
     * If you intend to install the widget when this message is signaled, you should invoke
     * tryContinueInstallation() method when you're handling this signal.
     *
     * @param errorCode tells reasons for installed widget being untrusted
     */
    void aboutToInstallUntrustedWidget(WidgetInstallError errorCode);

    /**
     * This signal is emitted whenever an existing widget is about to be replaced during installation
     * phase.
     */
    void installationSucceed();

    /**
     * This signal is emitted when installation process is cancelled by the user
     *
     */
    void installationCancelled();

#ifdef OPTIMIZE_INSTALLER
    void aborted( int transId, WidgetErrorType errCode );

    void interactiveRequest( int transId, QMap<WidgetPropertyType, QVariant> & );
    
    void completed( int transId );

#endif // OPTIMIZE_INSTALLER

public Q_SLOTS:
    /**
     * Try to continue installation process when and installation is about to do something.
     *
     * @see aboutToReplaceExistingWidget(QString)
     * @see aboutToInstallUntrustedWidget(WidgetInstallError)
     * @see aboutToInstallWidgetWithFeatures(QList<QString> capList)
     */
    void tryContinueInstallation();
    void cancelInstallation();
    
#ifdef OPTIMIZE_INSTALLER
    void handleProgress( int transId, int percentCompleted, WidgetOperationType operationType );

    void handleAborted( int transId, WidgetErrorType errCode );

    void handleInteractiveRequest( int transId, QMap<WidgetPropertyType, QVariant> & );
    
    void handleCompleted( int transId );

#endif //OPTIMIZE_INSTALLER


protected:
    // TODO: Should really be private method in WidgetManagerPrivate
    WidgetInstallError install(const QString& pkgPath,
                               QString& widgetId,
                               const QString& contentType,
                               const QString& rootDirectory = "",
                               bool silent = false,
                               bool update = false,
                               const QString& sigId = "");
    // END TODO: Should really be private method in WidgetManagerPrivate

    SuperWidget* getWidgetFromCache(const QString& pkgPath);
    void setWidgetIntoCache(const QString& pkgPath,
                            SuperWidget* widget);
    QString getWidgetInstallPath(const QString& rootDirectory);
    bool createDir(const QString& path);

public slots:

    //void widgetFileUnsecure(QString&);

private:
    WidgetManagerPrivate* d; // owned, must be deleted

public:
    void uninstallProgress(int progress);

    friend class WAC::Maemo::WidgetInstallerTestHelper;
    friend class WidgetManagerPrivate;
};

#endif // _WAC_WIDGET_MANAGER_H_

