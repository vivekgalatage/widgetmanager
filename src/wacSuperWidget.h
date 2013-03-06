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

#ifndef _WAC_SUPER_WIDGET_H_
#define _WAC_SUPER_WIDGET_H_


/*****************************************************************************
 * Super class for all Widget types
 *
 * *************************************************************************/

#include <QString>
#include <QDebug>
#include "wacwidgetmanagerconstants.h"
#include "private/wacWidgetInfo.h"
#include "wacWidgetUtils.h"

class WidgetManager;

typedef QMap<QString, QString> W3c2PlistMap;
typedef QMap<QString, QString> WidgetFeatures;

// Widget types
enum WidgetType
    {
        WidgetTypeWgz = 0,
        WidgetTypeW3c,
        WidgetTypeJIL,
        WidgetTypeUnknown
    };
typedef WidgetInfo WidgetManifestInfo;


enum WidgetInstallError
    {
        WidgetInstallSuccess = 0,
        WidgetValidSignature,
        WidgetUnZipBundleFailed,
        WidgetFindSignatureFailed,
        WidgetSignatureParsingFailed,
        WidgetSignatureOrSignedInfoMissing,
        WidgetSignatureRefExistFailed,
        WidgetSignatureRefValidationFailed,
        WidgetCertValidationFailed,
        WidgetSignatureValidationFailed,
        WidgetParseManifestFailed,
        WidgetRegistrationFailed,
        WidgetReplaceFailed,
        WidgetRmDirFailed,
        WidgetCapabilityNotAllowed,
        WidgetPlatformSpecificInstallFailed,
        WidgetCorrupted,
        WidgetSharedLibraryNotSigned,
        WidgetDriveLetterValidationFailed,
        WidgetTypeValidationFailed,
        WidgetSystemError,
        WidgetInstallPermissionFailed,
        WidgetUpdateFailed,
        WidgetUpdateVersionCheckFailed,
        WidgetUserConfirmFailed,
        WidgetInsufficientDiskSpace,
        WidgetInstallFailed,
        WidgetStartFileNotFound,
        WidgetIdInvalid,
        PackageManagerBusy,
        PackageManagerStarted,
        WidgetInstallCancelled
    };
Q_DECLARE_METATYPE(WidgetInstallError)

enum WidgetUninstallError
    {
        WidgetUninstallSuccess = 0,
        WidgetUninstallFailed,
        WidgetUninstallPermissionFailed,
        WidgetUninstallCancelled,
        WidgetUninstallSuccessButIncomplete
    };

namespace WAC {
    enum InstallationAttribute
    {
        NoAttibutes = 0x00,
        NoInstall = 0x001,
    };
    typedef QFlags<InstallationAttribute> InstallationAttributes;
}

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
namespace WAC {
     class Storage;
}
#endif

class WidgetInstaller;

class SuperWidget : public QObject
{
    Q_OBJECT
public:
    SuperWidget() {}
    explicit SuperWidget( QString& widgetPath ) { m_widgetBundlePath = widgetPath; }
    virtual ~SuperWidget();

public:
    virtual QString launcherPath(const QString&) = 0;
    WidgetInstallError install(const QString&){return WidgetInstallFailed;}
    static WidgetUninstallError uninstall(const QString &uniqueIdentifier,
                                          const bool removeData,
                                          WidgetManager *wm = NULL);
    static QString resourcePath(const QString& installPath);
    WidgetProperties* getProperties(const QString &pkgPath);
    WidgetProperties* getProperties() {return m_widgetProperties;};
    void setInstallationAttributes(WAC::InstallationAttributes& attributes);

#if defined (Q_OS_SYMBIAN) && defined (QTWRT_USE_USIF)
    static void NotifyAppArcOfInstallL(int m_uid);
    static void NotifyAppArcOfUninstallL(int m_uid);
#endif

    /**
     * Installs a web widget.
     *
     * Emits following signals:
     * @emit installationError(WidgetInstallError)
     * @emit aboutToReplaceExistingWidget(QString)
     * @emit aboutToInstallWidgetWithFeatures(QList<QString>)
     * @emit installationSucceed()
     *
     * @param update is installation update
     * @return true if installation is successful; otherwise false
     */
    virtual WidgetInstallError install( const bool update=false ) = 0;

    bool isValidWidgetType();

    virtual void writeManifest( const QString& path="" ) = 0;
    virtual bool unZipBundle( const QString& path="" ) = 0;
    virtual bool parseManifest( const QString& path="",
                                const bool force=false ) = 0;
    virtual bool allowRemovableInstallation() = 0;

    /**
     * Returns a metadata value by key and attribute
     * @param key a metadata element key
     * @attribute an attribute is an optional argument. With attribute you can get a value
     * of given attribute within specified key.
     */
    virtual QString value( const QString & key, 
            const QString & attribute = QString("")) = 0;

    /**
     * Returns true if the key with the attribute exists in widget metadata; otherwise false.
     * @param key the metadata element key
     * @attribute the attribute is an optional argument. With attribute you can test whether a value
     * of given attribute within specified key exists in widget metadata.
     */
    virtual bool contains( const QString & key, 
            const QString & attribute = QString("")) = 0;

public:
    static WidgetType getWidgetType(const QString&, const QString&);
    WIDGETUTILS_EXPORT static WidgetType getWidgetType(const QString&);
    static bool isValidWidgetType(const QString&, const QString&);
    static bool rmDir(const QString& );
    WIDGETUTILS_EXPORT static bool getWidgetPathParts(QStringList& , const QString&);

public:
    void setWidgetBundlePath(const QString& path) {m_widgetBundlePath = path;};
    void setWidgetUnZipPath(const QString& path) {m_widgetUnZipPath = path;};
    void setWidgetInstallPath(const QString& path) {m_widgetInstallPath = path;};
    void setWidgetCertificatePath(const QString& path){m_widgetCertificatePath = path;};
    void setWidgetManifestInfo(WidgetManifestInfo& mi){m_manifest = &mi;};
    QString widgetBundlePath() const {return m_widgetBundlePath;};
    QString widgetUnZipPath() const {return m_widgetUnZipPath;};
    QString widgetInstallPath() const {return m_widgetInstallPath;};
    QString widgetCertificatePath() const {return m_widgetCertificatePath;};
    QString& getContentType() {return m_contentType;};
    unsigned long getSize() const {return m_size;};
    virtual void disableBackupRestoreValidation(bool /*disableUnsignWidgetSignCheck*/){};

public Q_SLOTS:

    /**
     * Try to continue installation process when and installation is about to do something.
     *
     * @see aboutToReplaceExistingWidget(QString)
     */
    void tryContinueInstallation();
    void tryCancelInstallation();


Q_SIGNALS:
    void installProgress(int);

    /**
     * This signal is emitted whenever an error occurs during installation phase.
     *
     * If you intend to continue installation when error is signaled, you should
     * tryContinueInstallation() method when you're handling installation error.
     *
     */
    void installationError(WidgetInstallError error);

    /**
     * This signal is emitted whenever an existing widget is about to be replaced during installation
     * phase.
     *
     * If you intend to replace the widget when this message is signaled, you should invoke
     * tryContinueInstallation() method when you're handling this signal.
     */
    void aboutToReplaceExistingWidget(QString widgetTitle);

    void queryConfirmation();

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

    void promptFeatureList(QList<QString> capList);

    /**
     * This signal is emitted when installation is cancelled by user
     */
    void installationCancelled();

protected:
    virtual void initialize(QString& rootDirectory) = 0;

    bool isValidDigitalSignature( WidgetInstallError & error,
                                          const QString& widgetPath="",
                                          bool unzip=false,
                                          bool cleanup=false );

    bool findManifest( QString& ) { return false; }

    virtual WidgetProperties* widgetProperties(bool forceUpdate = false, 
            bool minimal = false) = 0;

    virtual bool findStartFile(QString& startFile, 
            const QString& widgetPath="") = 0;
    virtual bool findIcons(QStringList& icons, 
            const QString& widgetPath="") = 0;
    virtual bool findFeatures(WidgetFeatures& features, 
            const QString& widgetPath="") = 0;
    
    bool cleanup( const WidgetInstallError& error, const QString& path="" );
    bool cleanup( const WidgetInstallError& error,const bool& removed, 
	const QString& path="" );

    //creates a deb package that cannot be installed in case of installation failure in --no-install mode
#ifdef Q_OS_MAEMO6
    bool createErrorDebian(const WidgetInstallError& error);
#endif
    bool registerWidget(const QString& startFile);

    QString getTempLocation();
    QString getCertificatePath();
    bool createDir(const QString& path);

    // calculates uncompressed size of a widget
    unsigned long uncompressedSize(const QString& widgetPath) const;
    // checks if there is enough free space for widget installation
    bool checkDiskSpaceForInstallation( const QString& widgetPath) const;

    void setSize(unsigned long size) {m_size = size;};
#ifdef Q_OS_SYMBIAN
    // calculates free disk space and checks drive status
    // returns KErrNone if all ok
    // free disk space is returned in aDiskSpace
    TInt driveInfo( TInt aDrive, TInt64& aDiskSpace ) const;
#endif

protected:
    WidgetManifestInfo* getWidgetManifestInfo() const {return m_manifest;};
    WidgetType getWidgetType() const {return m_widgetType;};
    W3c2PlistMap* getWidgetMap()const {return m_widgetMap;};
    bool setWidgetRootPath(const QString& path=""){m_widgetRootPath = path; return true;};
    QString widgetRootPath() const {return m_widgetRootPath;};
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
    WAC::Storage* getStorage() const {return m_storage;};
#endif

protected :
    QString m_widgetBundlePath;          // path to the widget bundle.for eg: /home/user/Downloads
    QString m_widgetUnZipPath;           // path where widget can be unzipped for validation
    QString m_widgetInstallPath;         // path to install widget contents
    QString m_widgetCertificatePath;     // path to certificates
    QString m_widgetRootPath;            // path to widget root
    bool m_continueInstallation;         // on user prompting continuation status
    unsigned long m_size;
    bool m_widgetPropertiesCacheValid;

    WidgetType m_widgetType;
    QString m_contentType;
    WidgetManifestInfo *m_manifest;
    WidgetProperties *m_widgetProperties;
    W3c2PlistMap *m_widgetMap;
    WidgetInstaller * m_widgetInstaller;

#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
    WAC::Storage *m_storage;
#endif

    QString m_rootPath;
    QString m_installDir;
    QString m_sharedLibPath;
    QString m_dataPath;

#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
   friend class WidgetInstallerPrivate;
#endif

#if ENABLE(DESKTOPFILE)
   friend class DesktopFileWriter;
#endif
};

#endif
