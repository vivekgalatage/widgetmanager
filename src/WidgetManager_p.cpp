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

#include <QWidget>
#include <QMessageBox>
#include <QDir>
#include <QtAlgorithms>
#include <bautils.h>
#include <coemain.h>

#include "WidgetManager_p.h"
#include "wacwidgetmanagerconstants.h"
#include "proprietarytags.h"
#include "wacWidgetInfo.h"
#include "W3CXmlParser/w3cxmlplugin.h"
#include "private/WidgetLinkResolver.h"
#include "private/WidgetUtilsLogs.h"

#include "wacwebappinfo.h"
#include "webappuniquenesshelper.h"
#include "wacWebAppRegistry.h"
#include "wacWidgetProperties.h"
#include "wacWidgetManager.h"

#include "private/webappmanager_p.h"

#define PDATA ((WidgetManager_p*) m_data)

#include "signatureparser.h"
#include "wacSuperWidget.h"

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
#include "WgzWidget.h"
#endif

#include "WgtWidget.h"
#include "AsyncInstall.h"
#include "widgetpromptstrings.h"
#include <QDesktopServices>
#include <QThreadPool>

#if defined(Q_OS_SYMBIAN)
#include <QTimer>
#include <defaultcaps.hrh>
#include <e32std.h>
#include <e32base.h>
#include <SWInstApi.h>
#include <f32file.h>
#include "WidgetUnzipUtilityS60.h"
#endif  // __SYMBIAN32__

#ifdef QTWRT_USE_ORBIT
#include "hbmessagebox.h"
#include "hbinputdialog.h"
#include "hbeffect.h"
#include "hbaction.h"
#include <HbMessageBox>
#include <HbDeviceMessageBox>
#include <HbSelectionDialog>
#include <hbdevicemessageboxsymbian.h>
#endif

#ifdef QTWRT_USE_USIF
#include <sifui.h>
#include <sifuierrorinfo.h>
#include <sifuiappinfo.h>
#include <swi/msisuihandlers.h>
#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/sif/sifnotification.h>
#endif


#ifdef OPTIMIZE_INSTALLER
#include <QScopedPointer>
#include <QVariant>

#include "widgettypes.h"

#include "widgetcontext.h"
#include "widgetcontextcreator.h"

#endif // OPTIMIZE_INSTALLER


#include <EikAppUi.h>
#include <EikApp.h>


#define NO_NAME "NoName"

WidgetManager_p::WidgetManager_p(QWidget *parent):
    m_parent(parent)
{
}

WidgetManager_p::~WidgetManager_p()
{
}

WidgetManagerPrivate::WidgetManagerPrivate(QWidget *parent, WidgetManager* widgetManager)
    : m_silent(false)
    , m_disableUnsignWidgetSignCheck(false)
    , m_attributes(WAC::NoAttibutes)
    , m_parent(parent)
    , q(widgetManager)
{
    m_data = (void *) new WidgetManager_p(parent);
#if  defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)
    iResourceFileOffset = 0;
#endif //defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)    
}

WidgetManagerPrivate::~WidgetManagerPrivate()
{
    delete (WidgetManager_p*)m_data;
#ifdef OPTIMIZE_INSTALLER
    for( int i=0; i<m_WidgetContexts.size(); i++ )
    {
        WidgetContext *context = m_WidgetContexts[i];
        if( context )
            delete context;
    }

    m_WidgetContexts.clear();
#endif //OPTIMIZE_INSTALLER
}

//
// function to get widget properties from path. Will create a new widget object
// if one doesn't exist for the path
// parameters:
//     path       path to widget
// return
//     WidgetProperties* pointer to widget properteis
//
WidgetProperties* WidgetManagerPrivate::getWidgetProperties(const QString &pkgPath)
{
    LOG("BEGIN WidgetManagerPrivate::getWidgetProperties pkgpath =" << pkgPath);

    if (pkgPath.isEmpty()) {
        LOG("END WidgetManagerPrivate::getWidgetProperties pkgPath is empty");
        return 0;
    }

    SuperWidget* cachedWidget = q->getWidgetFromCache(pkgPath);
    if (cachedWidget) {
       LOG( "WidgetManagerPrivate::getWidgetProperties use cached widget to getProperties");
        WidgetProperties *props = cachedWidget->getProperties(pkgPath);
        if (props == 0) {
            LOG("END WidgetManagerPrivate::getWidgetProperties: cached widget has properties=0");
            return 0;
        }
        // Check if current widget path and properties path match. Use native
        // separators and remove trailing separator, because properties path
        // was saved using native separators.
        QString widgetPath = QDir::convertSeparators(pkgPath);

        QStringList widgetPathParts = widgetPath.split(QDir::separator(),QString::SkipEmptyParts);
        QString realWidgetPath = widgetPathParts.join(QDir::separator());
        if (widgetPath.startsWith(QDir::separator())) {
            realWidgetPath = QDir::separator() + realWidgetPath;
        }

        QStringList installPathParts = props->installPath().split(QDir::separator(),
                QString::SkipEmptyParts);
        QString realInstallPath = installPathParts.join(QDir::separator());
        if (props->installPath().startsWith(QDir::separator())) {
            realInstallPath = QDir::separator() + realInstallPath;
        }
        if (realInstallPath.compare(realWidgetPath) == 0) {
            LOG("END WidgetManagerPrivate::getWidgetProperties cached widget properties are valid");
            return props;
        }
    }
    else {
        // We couldn't find a cached widget, so create a new one
        SuperWidget* newWidget = createWidget(pkgPath);
        if (newWidget) {
            WidgetProperties *props = newWidget->getProperties(pkgPath);
            LOG("END WidgetManagerPrivate::getWidgetProperties using new widget to get properties");
            return props;
        }
    }

    LOG("END WidgetManagerPrivate::getWidgetProperties FAILED to get widget properties");
    return 0;

}

//
// function to create a widget, uses the path (unique folder name) as key to
// cache widget in hash
// parameters:
//     path       path to widget
// return
//     SuperWidget* pointer to new widget
//
SuperWidget* WidgetManagerPrivate::createWidget(const QString& pkgPath)
{
    // Generate the install path, use native separators and no ending separator
    QString installPath;
#if defined(Q_OS_SYMBIAN)
    QString path = pkgPath[0];
    installPath = QDir::toNativeSeparators(pkgPath).section(QDir::separator(),0,3);
    installPath[0] = path[0];
#elif defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    installPath = WIDGET_INSTALL_PATH;
#else   // win32
    QString dataLocation =  QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    installPath = QDir::toNativeSeparators(dataLocation);
    installPath = installPath + QDir::separator() + WIDGET_FOLDER;

#endif

    // Create a new widget
    SuperWidget* newWidget;
    switch (SuperWidget::getWidgetType(pkgPath))
        {
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
        case WidgetTypeWgz:
            newWidget = new WgzWidget(installPath);
            break;
#endif
        case WidgetTypeW3c:
            newWidget = new WgtWidget(installPath);
            break;
        default:
            LOG("END WidgetManagerPrivate::createWidget - Invalid Widget type");
            newWidget = 0;
            break;
    }

    if (newWidget) {
        q->setWidgetIntoCache(pkgPath, newWidget);
        return newWidget;
    }

    return 0;
}
#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
void WidgetManagerPrivate::handleInstallProgress(int p) 
#else 
void WidgetManagerPrivate::handleInstallProgress(int /*p*/) 
#endif 
{
#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    if (iSifProgressGlobalComponentId) {
        Usif::CSifOperationProgressData *progressData;
        TRAP_IGNORE(progressData =
                    Usif::CSifOperationProgressData::NewL(*iSifProgressGlobalComponentId, 
                                                            Usif::EInstalling,
                                                          Usif::ENoSubPhase, p, 100);
                    iSifProgressPublisher->PublishProgressL(*progressData);
                    User::Free(progressData));
    }

    if (!m_silent) {
        if (!iProgressDisplayed) {
            TRAP_IGNORE(iSifUi->ShowProgressL(*iAppInfo, 100));
            iProgressValue = 0;
            iProgressDisplayed = true;
        }

        TRAP_IGNORE(iSifUi->IncreaseProgressBarValueL(p - iProgressValue));
        iProgressValue = p;
    }
#else
#endif
}

#ifdef Q_OS_SYMBIAN
void WidgetManagerPrivate::handleConfirmation() {
#ifdef QTWRT_USE_USIF
    WidgetProperties* props = m_installingWidget->getProperties();
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(props->title().utf16()));
    HBufC *buffer;
    TRAPD(err, buffer = HBufC16::NewL(ptr.Length()));
    if (err != KErrNone)
        return;
    buffer->Des().Copy(ptr);

    if (iSifProgressGlobalComponentId) {
        Usif::CSifOperationStartData *startData;
        RPointerArray<HBufC> appNames;
        RPointerArray<HBufC> appIcons;
        TRAP_IGNORE(startData = Usif::CSifOperationStartData::NewL(*iSifProgressGlobalComponentId, *buffer,
                                                                   appNames, appIcons, iAppSize, KNullDesC,
                                                                   KNullDesC, Usif::KSoftwareTypeWidget);
                    iSifProgressPublisher->PublishStartL(*startData);
                    User::Free(startData));
    }
#endif

    if (m_silent) {
        m_installingWidget->tryContinueInstallation();
        return;
    }

#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    bool proceed = false;

    // WidgetProperties does not contain vendor or version currently

    // FIXME:  Who owns the CAppInfo and the contained strings
    iAppInfo = CSifUiAppInfo::NewL(buffer->Des(), KNullDesC, TVersion(), iAppSize, NULL);

    TRAP_IGNORE(proceed = iSifUi->ShowConfirmationL(*iAppInfo));

    if (proceed) {
        // Get the installation drive
        TInt driveNumber;
        TChar driveChar;

        // FIXME:  What if there's no drive selected due to silent, or only
        //         drive being drive C?
        err = iSifUi->SelectedDrive(driveNumber);
        if (err != KErrNone) {
            LOG("SelectedDrive returned errorcode = " << err);
        } else {
            RFs::DriveToChar(driveNumber, driveChar);

            QString installPath = m_installingWidget->widgetInstallPath();
            installPath.replace(0, 1, QString(QChar(TUint(driveChar))));
            m_installingWidget->setWidgetInstallPath(installPath);
        }

        m_installingWidget->tryContinueInstallation();
    }
    return;
#else
    m_installingWidget->tryContinueInstallation();
#endif
}
#endif

void WidgetManagerPrivate::handleInstallError(WidgetInstallError error) {
    // FIXME : We must localize message and use platform specific dialogs / notifications.
#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    Usif::TErrorCategory errCategory;

    switch (error) {
    case WidgetInstallSuccess:
    case WidgetValidSignature:
        errCategory = Usif::ENone;

        break;

    case WidgetUserConfirmFailed:
        errCategory = Usif::EUserCancelled;

        break;

    case WidgetFindSignatureFailed:
    case WidgetSignatureParsingFailed:
    case WidgetSignatureOrSignedInfoMissing:
    case WidgetSignatureRefExistFailed:
    case WidgetSignatureRefValidationFailed:
    case WidgetCertValidationFailed:
    case WidgetSignatureValidationFailed:
    case WidgetCapabilityNotAllowed:
    case WidgetSharedLibraryNotSigned:
    case WidgetInstallPermissionFailed:
        errCategory = Usif::ESecurityError;

        break;

    case WidgetInsufficientDiskSpace:
        errCategory = Usif::ELowDiskSpace;

        break;

    case WidgetCorrupted:
    case WidgetUnZipBundleFailed:
    case WidgetParseManifestFailed:
    case WidgetStartFileNotFound:
        errCategory = Usif::ECorruptedPackage;

        break;

    case WidgetRegistrationFailed:
    case WidgetReplaceFailed:
    case WidgetRmDirFailed:
    case WidgetPlatformSpecificInstallFailed:
    case WidgetDriveLetterValidationFailed:
    case WidgetTypeValidationFailed:
    case WidgetSystemError:
    case WidgetUpdateFailed:
    case WidgetUpdateVersionCheckFailed:
    case WidgetInstallFailed:
    case WidgetIdInvalid:
        errCategory = Usif::EUnexpectedError;

        break;

    default:
        errCategory = Usif::EUnknown;
    }

    if (iSifProgressGlobalComponentId) {
        Usif::CSifOperationEndData *endData;
        TRAP_IGNORE(endData = Usif::CSifOperationEndData::NewL(*iSifProgressGlobalComponentId,
                                                               errCategory, error, KNullDesC,
                                                               KNullDesC);
                    iSifProgressPublisher->PublishCompletionL(*endData);
                    User::Free(endData));
    }

    if (!m_silent && ((error != WidgetInstallSuccess) && (error!= WidgetUserConfirmFailed))) {
        QString message = TR_WM_WIDGET_INSTALL_FAILED;
        TPtrC16 ptr(reinterpret_cast<const TUint16*>(message.utf16()));
        HBufC *buffer;
        
        TRAPD(err, buffer = HBufC16::NewL(ptr.Length()));
        if (err != KErrNone)
            return;

        CSifUiErrorInfo* sifUiErrorInfo;

        buffer->Des().Copy(ptr);
        TRAP(err, sifUiErrorInfo = CSifUiErrorInfo::NewL(errCategory, error, 0, buffer->Des(), KNullDesC));
        if (err == KErrNone) {
            TRAP_IGNORE(iSifUi->ShowFailedL(*sifUiErrorInfo));
            User::Free(sifUiErrorInfo);
        }
        User::Free(buffer);
    }
#else
    if (!m_silent && ((error != WidgetInstallSuccess) && (error!= WidgetUserConfirmFailed))) {
        showInformation(TR_WM_WIDGET_INSTALL_ERROR, TR_WM_WIDGET_INSTALL_FAILED);
    }
#endif
}

void WidgetManagerPrivate::handleInstallationSuccess() {
// FIXME: for now we don't want to display any success messages,
// once the package should have being handled by application manager
// and we have no means to know when it has finished.
#ifndef Q_OS_MAEMO5
    QString message = TR_WM_WIDGET_INSTALL_SUCCESS;
#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    if (iSifProgressGlobalComponentId) {
        Usif::CSifOperationEndData *endData;
        TRAP_IGNORE(endData = Usif::CSifOperationEndData::NewL(*iSifProgressGlobalComponentId,
                                                               Usif::ENone, 0, KNullDesC,
                                                               KNullDesC);
                    iSifProgressPublisher->PublishCompletionL(*endData);
                    User::Free(endData));
        }

    if (!m_silent) {
        TRAP_IGNORE(iSifUi->ShowCompleteL());
    }
#else
    if (!m_silent) {
        showInformation(TR_WM_WIDGET_INSTALL, message);
    }
#endif
#endif
}

void WidgetManagerPrivate::handleWidgetReplacement(QString title) {
    if (!m_silent) {
        QString msg = TR_WM_WIDGET_REPLACE + " \" " + title + " \"?";
        if (!showQuestion(TR_WM_WIDGET_INSTALL,
                          msg,
                          WidgetManagerPromptButtonYes,
                          WidgetManagerPromptButtonNo))
        {
            showInformation(TR_WM_WIDGET_INSTALL, TR_WM_WIDGET_INSTALL_CANCELLED);
            return;
        }
    }
    m_installingWidget->tryContinueInstallation();
}

void WidgetManagerPrivate::handleFeatureInstallation(QList<QString> capList)
{
    if (!m_silent) {
        QString message;
        for (int i=0; i<capList.count(); i++) {
            message += '\n';
            message += QCoreApplication::translate("WidgetManager", 
                    QT_TRANSLATE_NOOP("WidgetManager",capList[i].toUtf8().constData()));
        }

        if (!showQuestion(TR_WM_SECURITY_WARNING,
                             TR_WM_WIDGET_INSTALL_ALLOW +
                             m_installingWidget->getProperties()->title() + 
                             TR_WM_WIDGET_INSTALL_TO_ACCESS + message,
                             WidgetManagerPromptButtonOkay,
                             WidgetManagerPromptButtonCancel))
        {
            showInformation(TR_WM_WIDGET_INSTALL, TR_WM_WIDGET_INSTALL_CANCELLED);
            return;
        }
    }
    m_installingWidget->tryContinueInstallation();
}

void WidgetManagerPrivate::handleDestinationSelection(unsigned long spaceRequired, 
        bool allowRemovable) {
    if (m_silent) {
        m_installingWidget->tryContinueInstallation();
        return;
    }

#if defined(Q_OS_SYMBIAN)
    QString selection;
    TDriveList drives;
    RFs iFs;
    QList<TInt> validDriveNumbers;
    QStringList validDrivesPrettyFormat; // actual text to display to users
    const int asciiCodeAdrive = 65; // ASCII representation for Symbian A:\

    TInt err = iFs.Connect();
    if (err != KErrNone) {
        // can't get the drive list, so can't prompt user
        m_installingWidget->tryContinueInstallation();
        return;
    }

    if (iFs.DriveList(drives) == KErrNone) {
        TInt driveNumber = EDriveA;

        for ( ; driveNumber <= EDriveZ ; driveNumber++) {
            if (drives[driveNumber]) {
                // bypass if drive is ROM
                if ((drives[driveNumber] & KDriveAttRom) ||
                    (drives[driveNumber] & KDriveAttSubsted))
                    continue;

                TDriveInfo driveInfo;
                iFs.Drive(driveInfo, driveNumber);

                // Don't allow installation on certain types of media,
                // or if media is not present
                if ((driveInfo.iType == EMediaNotPresent) ||
                    (driveInfo.iType == EMediaRam) ||
                    (driveInfo.iType == EMediaRom) ||
                    (driveInfo.iType == EMediaRemote))
                        continue;

                // skip drive if removable installation is not allowed and
                // drive is removable
                if ((drives[driveNumber] & KDriveAttRemovable) && !allowRemovable)
                    continue;

                // check if drive is writeable
                if (driveInfo.iMediaAtt & KMediaAttWriteProtected ||
                    driveInfo.iMediaAtt & KMediaAttLocked)
                    continue;

                // check if drive has enough free space
                TVolumeInfo volumeInfo;
                if( (iFs.Volume(volumeInfo, driveNumber)) != EErrorNone )
                {
                    m_installingWidget->tryContinueInstallation();
                    return;
                }
                    
                if (volumeInfo.iFree <= spaceRequired)
                    continue;
                
                // Store as int ... 
                validDriveNumbers.append(driveNumber);
                // .. and as a pretty formatted string 
                QString s = QString(QChar(asciiCodeAdrive + driveNumber)) + ':' ;
                validDrivesPrettyFormat.append(s);
            }
        }    // end of populating list of drives

        if (validDriveNumbers.size() != 0) {
            // if there's only one valid drive, we still prompt
            // if that drive is not C:
            if ((validDriveNumbers.size() == 1) &&
                (validDriveNumbers[0] == EDriveC)) {
                m_installingWidget->tryContinueInstallation();
                return;
            }

            // in ten one week 16+ drive selection is deferred until later
            // and handled by standard UI.
            // Just set the list of valid drives
            RArray<TInt> allowedDrives;

            for (int i = 0 ; i < validDriveNumbers.size() ; i++) {
                allowedDrives.Append(validDriveNumbers[i]);
            }

#if defined(QTWRT_USE_USIF)
            iAppSize = spaceRequired;
            TRAP_IGNORE(iSifUi->SetMemorySelectionL(allowedDrives));
            allowedDrives.Close();
            m_installingWidget->tryContinueInstallation();
#else
            // Pick first drive from already sensible drive list (avoid QtGUI look-n-feel issues) 
            int defaultChoiceIdx(0); 
           
            // Correct widget installation and unzip paths by replacing C: with another drive letter
            QString installPath = m_installingWidget->widgetInstallPath();
            QString newDriveLetter(QChar(uint(asciiCodeAdrive + 
                    validDriveNumbers[defaultChoiceIdx])));
            installPath.replace(0, 1, newDriveLetter);
            
            m_installingWidget->setWidgetInstallPath(installPath);
            QString unzipPath(m_installingWidget->widgetUnZipPath());
            unzipPath.replace(0, 1, newDriveLetter);
            m_installingWidget->setWidgetUnZipPath(unzipPath);
            
            // update flag to indicate to caller of slot that Cancel wasn't selected
            m_installingWidget->tryContinueInstallation();
                            
            

#endif            
        } // validDriveNumbers.size() > 0 block
    } // drivelist available block
    
#else // End of Q_OS_SYMBIAN
    m_installingWidget->tryContinueInstallation();
#endif
   
}

void WidgetManagerPrivate::handleUntrustedWidgetInstallation(WidgetInstallError errorCode) {

    if (m_silent) {
        // Same code as use to be in the WgtWidget installation.
        // In silent mode must enable installation of untrusted widget.
        // This is just about the error value.
        if (errorCode == WidgetFindSignatureFailed) {
            m_installingWidget->tryContinueInstallation();
        }
        return;
    }

    QString message = "";
    switch (errorCode) {
        case WidgetFindSignatureFailed:
        case WidgetSignatureParsingFailed:
        case WidgetSignatureOrSignedInfoMissing:
        case WidgetSignatureRefExistFailed:
        case WidgetCertValidationFailed:
        case WidgetSignatureValidationFailed:
        case WidgetSignatureRefValidationFailed:
            message = TR_WM_WIDGET_SIGNATURE_INVALID;
            break;
        default:
            message = TR_WM_WIDGET_UNKNOWN_ERROR;
            break;
    }

    if (showQuestion(TR_WM_WIDGET_INSTALL,
                     message + TR_WM_WIDGET_INSTALL_CONT_QUERY,
                     WidgetManagerPromptButtonYes,
                     WidgetManagerPromptButtonNo))
    {
            m_installingWidget->tryContinueInstallation();
    } else {
    	 showInformation(TR_WM_WIDGET_INSTALL, TR_WM_WIDGET_INSTALL_CANCELLED);
    }
}

/*
 * Checks for the rootDirectory value
 * If Null or Empty then it assigns the DEFAULT_ROOT_DIRECTORY
 */
QString WidgetManagerPrivate::setDefaultRootDirectory(const QString& rootDirectory)
{
    if (rootDirectory.isEmpty() || rootDirectory.isNull()){
        QString defaultRootDir = DEFAULT_ROOT_DIRECTORY;
        return(defaultRootDir);
    }
    QString& defaultRootDir = const_cast<QString&> (rootDirectory);
    return(defaultRootDir);
}

#ifdef __SYMBIAN32__
//
// function to construct S60 widget Install path
// parameters:
//     QChar      drive letter
// return
//     QString    Install Path
//
QString WidgetManagerPrivate::setS60RootDirectory(const QChar& driveLetter, 
        const QString& rootDirectory)
{

    // If no driveletter passed then use the default root directory
    if (driveLetter.isNull()){
       QString defaultRootDir = DEFAULT_ROOT_DIRECTORY;

        return(defaultRootDir);
    }
    
    // Handle edge case when only drive letter is available from rootDirectory (and no subdirectories) 
    if (rootDirectory.length() <= strlen("A:\\")) {
        QString rootPath = QDir::toNativeSeparators(QString(driveLetter) + PRIVATE_DIRECTORY + 
                QDir::separator() + DEFAULT_DIRECTORY); 
        return(rootPath); 
    }
            
    // Use the passed driveletter and root dir

    QString rootDrive = rootDirectory;
    if (!(rootDrive.startsWith("\\") || rootDrive.startsWith("/")))
        rootDrive.remove(0, 2);
   QString rootPath = "";
   rootPath.append(driveLetter);
   rootPath = rootPath+':'+rootDrive;
   rootPath = QDir::toNativeSeparators(rootPath);
   return(rootPath);

}
#endif  // __SYMBIAN32__

//
// function to validate the input rootDirectory string
// parameters:
//     QString    rootDirectory String
// return
//     bool    True if input is valid on Symbian
//
bool WidgetManagerPrivate::validateDriveLetter(const QString& rootDirectory)
{
#ifdef __SYMBIAN32__
    if ( rootDirectory.length() >= 1 ) {
        QChar driveLetter = rootDirectory[0];
        if (!driveLetter.isLetter())
            return false;
        m_rootDirectory = setS60RootDirectory(driveLetter, rootDirectory);
        m_rootDirectory = q->getWidgetInstallPath(m_rootDirectory);
        if (m_rootDirectory.isEmpty() || m_rootDirectory.isNull())
            return false;
    } else {
            m_rootDirectory = setS60RootDirectory();
            m_rootDirectory = q->getWidgetInstallPath(m_rootDirectory);
            if (m_rootDirectory.isEmpty() || m_rootDirectory.isNull())
                 return false;
    }


#else
    m_rootDirectory = setDefaultRootDirectory(rootDirectory);
    m_rootDirectory = q->getWidgetInstallPath(m_rootDirectory);
    if (m_rootDirectory.isEmpty() || m_rootDirectory.isNull())
        return false;
#endif  // __SYMBIAN32__

    return true;
}

bool WidgetManagerPrivate::checkCallerPermissions()
{
#ifdef Q_OS_SYMBIAN
    RProcess process;
    static _LIT_SECURITY_POLICY_V0(myVidPolicy, VID_DEFAULT);
    TInt error = myVidPolicy().CheckPolicy(process);
    if (error  == EFalse) {
        LOG("WidgetManagerPrivate::checkCallerPermissions() return false");
        return false;
    }
#endif
    return true;
}

#ifdef QTWRT_USE_ORBIT
void WidgetManagerPrivate::confirmCallback(HbAction* action)
{
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_dialogResult.m_dialogAccepted = (action->text() == m_acceptText);
        m_eventLoop->quit();
    }
}

void WidgetManagerPrivate::informationCallback(HbAction*)
{
    if (m_eventLoop && m_eventLoop->isRunning()) {
        m_eventLoop->quit();
    }
}
#endif

void WidgetManagerPrivate::showInformation(const QString& title, const QString& msg)
{
#ifdef QTWRT_USE_ORBIT
    Q_UNUSED(title);

    HbMessageBox *box = new HbMessageBox(msg);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setTimeout(-1);
    box->setDismissPolicy(HbDialog::NoDismiss);
    box->open(m_parent, SLOT(informationCallback(HbAction*)));
    m_eventLoop = new QEventLoop;
    m_eventLoop->exec();
    delete m_eventLoop;
    m_eventLoop = 0;
#else
    QMessageBox::information(PDATA->m_parent, title, msg);
#endif
}


#if defined(QTWRT_USE_ORBIT) && defined(Q_OS_SYMBIAN)
bool WidgetManagerPrivate::ShowQuestionL(const QString& msg, const QString& primary, const QString& secondary)
{
    TPtrC16 msgptr(reinterpret_cast<const TUint16*>(msg.utf16()));
    HBufC *msgbuffer;
    msgbuffer = HBufC16::NewLC(msgptr.Length());
    msgbuffer->Des().Copy(msgptr);

    TPtrC16 primptr(reinterpret_cast<const TUint16*>(primary.utf16()));
    HBufC *primbuffer;
    primbuffer = HBufC16::NewLC(primptr.Length());
    primbuffer->Des().Copy(primptr);

    TPtrC16 secptr(reinterpret_cast<const TUint16*>(secondary.utf16()));
    HBufC *secbuffer;
    secbuffer = HBufC16::NewLC(secptr.Length());
    secbuffer->Des().Copy(secptr);

    CHbDeviceMessageBoxSymbian::TButtonId answer =
        CHbDeviceMessageBoxSymbian::QuestionL(msgbuffer->Des(), primbuffer->Des(),
                                              secbuffer->Des());

    CleanupStack::PopAndDestroy(3); // msgbuffer, primbuffer, secbuffer

    return answer == CHbDeviceMessageBoxSymbian::EAcceptButton;
}
#endif

bool WidgetManagerPrivate::showQuestion(const QString& title,
                                        const QString& msg,
                                        const WidgetManagerPromptButtons& primaryButton,
                                        const WidgetManagerPromptButtons& secondaryButton)
{

#if defined(QTWRT_USE_ORBIT)
    Q_UNUSED(title);

    QString primary;
    QString secondary;

    switch (primaryButton) {
    case WidgetManagerPromptButtonYes:
        primary = m_parent->tr("Yes");
        break;
    case WidgetManagerPromptButtonNo:
        primary = m_parent->tr("No");
        break;
    case WidgetManagerPromptButtonCancel:
        primary = m_parent->tr("Cancel");
        break;
    case WidgetManagerPromptButtonOkay:
    default:
        primary = m_parent->tr("Okay");
        break;
    }

    switch (secondaryButton) {
    case WidgetManagerPromptButtonYes:
        secondary = m_parent->tr("Yes");
        break;
    case WidgetManagerPromptButtonNo:
        secondary = m_parent->tr("No");
        break;
    case WidgetManagerPromptButtonOkay:
        secondary = m_parent->tr("Okay");
        break;
    case WidgetManagerPromptButtonCancel:
    default:
        secondary = m_parent->tr("Cancel");
        break;
    }

    bool accepted;
    TRAPD(err, accepted = ShowQuestionL(msg, primary, secondary));
    if (err != KErrNone)
        return true;

    return accepted;
#else
    QMessageBox::StandardButtons buttons = QMessageBox::NoButton;
    QMessageBox::StandardButton primary = QMessageBox::NoButton;

    switch (primaryButton) {
    case WidgetManagerPromptButtonYes:
        primary = QMessageBox::Yes;
        break;
    case WidgetManagerPromptButtonNo:
        primary = QMessageBox::No;
        break;
    case WidgetManagerPromptButtonCancel:
        primary = QMessageBox::Cancel;
        break;
    case WidgetManagerPromptButtonOkay:
    default:
        primary = QMessageBox::Ok;
        break;
    }

    buttons |= primary;

    switch (secondaryButton) {
    case WidgetManagerPromptButtonYes:
        buttons |= QMessageBox::Yes;
        break;
    case WidgetManagerPromptButtonNo:
        buttons |= QMessageBox::No;
        break;
    case WidgetManagerPromptButtonOkay:
        buttons |= QMessageBox::Ok;
        break;
    case WidgetManagerPromptButtonCancel:
    default:
        buttons |= QMessageBox::Cancel;
        break;
    }

    return (primary == QMessageBox::question(PDATA->m_parent, title, msg, buttons));
#endif
}

#ifdef QTWRT_USE_USIF
void WidgetManagerPrivate::uninstallComplete(const WidgetUninstallError& errorCode)
#else //!QTWRT_USE_USIF
void WidgetManagerPrivate::uninstallComplete(const WidgetUninstallError& /*errorCode*/)
#endif
{
#ifdef QTWRT_USE_USIF
    Usif::TErrorCategory errCategory;

    switch (errorCode) {
    case WidgetUninstallSuccess:
    case WidgetUninstallSuccessButIncomplete:
        errCategory = Usif::ENone;

        break;

    case WidgetUninstallFailed:
        errCategory = Usif::EUnexpectedError;

        break;

    case WidgetUninstallPermissionFailed:
        errCategory = Usif::ESecurityError;

        break;

    case WidgetUninstallCancelled:
        errCategory = Usif::EUserCancelled;

        break;

    default:
        errCategory = Usif::EUnknown;

    }

    if (iSifProgressGlobalComponentId) {
        Usif::CSifOperationEndData *endData;
        TRAP_IGNORE(endData =
                    Usif::CSifOperationEndData::NewL(*iSifProgressGlobalComponentId,
                                                     errCategory, errorCode, KNullDesC,
                                                     KNullDesC);
                    iSifProgressPublisher->PublishCompletionL(*endData);
                    User::Free(endData));
    }
#endif
}

#ifdef OPTIMIZE_INSTALLER

int WidgetManagerPrivate::installWidget( const QString& pkgPath, const QMap<QString, 
        QVariant>& configDetails )
{
    if( !pkgPath.isEmpty() )
    {
        WidgetInformation widgetInfo( pkgPath, EContextInstall );
        
        WidgetContext* context = WidgetContextCreator::createContext( widgetInfo );
        
        if( context )
        {
            QObject::connect(context,
                             SIGNAL(progress( int, WidgetOperationType )),
                             this,
                             SLOT(handleProgress( int, WidgetOperationType )));

            QObject::connect(context,
                             SIGNAL(completed()),
                             this,
                             SLOT(handleCompleted()));

            QObject::connect(context,
                             SIGNAL(aborted( WidgetErrorType)),
                             this,
                             SLOT(handleAborted( WidgetErrorType)));

            QObject::connect(context,
                             SIGNAL(interactiveRequest( QMap< WidgetPropertyType, QVariant> & )),
                             this,
                             SLOT(handleInteractiveRequest( QMap< WidgetPropertyType, QVariant> & ))
                            );

            context->initialize( configDetails );
            context->start();

            m_WidgetContexts << context;
        }
    }
    else
    {
        //TODO: Do some handling :)
    }
    return m_WidgetContexts.size() - 1;
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManagerPrivate::cancel( int trxId )
{
    if( trxId >= 0 && trxId < m_WidgetContexts.size() )
        m_WidgetContexts[trxId]->stop();
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManagerPrivate::handleProgress( int percentCompleted, WidgetOperationType operationType )
{
    int transId = transactionId( QObject::sender() );
    emit progress( transId, percentCompleted, operationType );
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManagerPrivate::handleCompleted()
{
    int transId = transactionId( QObject::sender() );    
    emit completed( transId );
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManagerPrivate::handleAborted( WidgetErrorType errCode )
{
    int transId = transactionId( QObject::sender() );
    emit aborted( transId, errCode );
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManagerPrivate::handleInteractiveRequest( QMap< WidgetPropertyType, 
        QVariant> &properties )
{
    int transId = transactionId( QObject::sender() );
    emit interactiveRequest( transId, properties );
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
int WidgetManagerPrivate::transactionId( const QObject* sender )
{
    return m_WidgetContexts.indexOf( (WidgetContext*)sender );
}
#endif // OPTIMIZE_INSTALLER


#ifdef OPTIMIZE_INSTALLER

#if defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)
void WidgetManagerPrivate::loadSymbianResourceFile()
{
    const QString resourceFile = "0x200267B7.rsc";
    const QString resourcePath = "c:\\resource\\apps\\";

    QString resourceFileName(resourcePath+resourceFile);
    TPtrC resFileNameDescriptor (reinterpret_cast<const TText*>(resourceFileName.constData()), 
            resourceFileName.length());
    
    // Get the full path and name of the executable - using this to determine the resource file
    // path. Assumes that resource files are installed to the same dir as the exe.    
    CEikAppUi* pAppUI = (CEikAppUi*)(CCoeEnv::Static()->AppUi());
    TFileName dllName = pAppUI->Application()->DllName();
    QString dllNameString((QChar*)dllName.Ptr(),dllName.Length());
    
    TFileName resFile(resFileNameDescriptor);
    // Replace the drive letter with the letter from the dll path
    resFile[0] = dllName[0];
    BaflUtils::NearestLanguageFile(CCoeEnv::Static()->FsSession(), resFile);
    // Load the resource file and convert the potential leave to a exception
    iResourceFileOffset = CCoeEnv::Static()->AddResourceFileL(resFile);
}
#endif //defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)

#endif //OPTIMIZE_INSTALLER

