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


#include "WidgetManager_p.h"
#include "wacwidgetmanagerconstants.h"
#include "private/WidgetUtilsLogs.h"

#include "webappuniquenesshelper.h"
#include "wacWidgetManager.h"



#include "WgtWidget.h"
#include "AsyncInstall.h"

#if defined(Q_OS_SYMBIAN)
#include <QWidget>
#include <SWInstApi.h>

static const char KSIGNATUREID[]            = "58:BF:92:E1:B3:C6:15:69:8C:6C:5E:F5:9C:25:C4:2D:DF:1F:D8:C5";
static const char KINDEXFILE[]              = "installedWidgets.txt";
_LIT8(KSISMIMETYPE, "x-epoc/x-sisx-app");

#define ROOT_DIRECTORY_LENGTH 11
#endif  // Q_OS_SYMBIAN

#ifdef QTWRT_USE_USIF
#include <sifui.h>
#include <sifuiappinfo.h>
#include <swi/msisuihandlers.h>
#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/sif/sifnotification.h>
#endif

#ifdef OPTIMIZE_INSTALLER
#if defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)
#include <StringLoader.h>
#include <0x200267B7.rsg>
#endif //defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)

#endif //OPTIMIZE_INSTALLER

#define NO_NAME "NoName"


/*!
 \class WidgetManager
 \brief  Install and uninstall web applications. Public class for installing widgets for 3rd party.
*/

/*!
Widget manager deletes all temporary created files on destructor

\a widget pointer for displaying confirmation
*/
Q_DECL_EXPORT WidgetManager::WidgetManager(QWidget *parent, bool isChromeless)
    : QObject(parent)
    , d(new WidgetManagerPrivate(parent, this))
{
    (void)isChromeless; // Argument required for compatability with old branch
    d->m_widgetType = WidgetTypeUnknown;
    d->m_contentType = CONTENT_TYPE_UNKNOWN;
    d->m_installingWidget = 0;

#if defined(QTWRT_USE_USIF)
    d->iSifProgressGlobalComponentId = 0;
    d->iSifUi = 0;
    TRAP_IGNORE(d->iSifUi = CSifUi::NewL());
    d->iSifProgressPublisher = 0;
    TRAP_IGNORE(d->iSifProgressPublisher = Usif::CPublishSifOperationInfo::NewL());
    d->iProgressValue = 0;
    d->iProgressDisplayed = false;
    d->iAppInfo = 0;
#endif
    
#ifdef OPTIMIZE_INSTALLER
    connect( d,
             SIGNAL( progress( int, int, WidgetOperationType ) ),
             this,
             SLOT( handleProgress( int, int, WidgetOperationType ) )
            );
    connect( d,
             SIGNAL( aborted( int, WidgetErrorType ) ),
             this,
             SLOT( handleAborted( int, WidgetErrorType ))
            );

    connect( d,
             SIGNAL( interactiveRequest( int, QMap< WidgetPropertyType, QVariant> & ) ),
             this,
             SLOT( handleInteractiveRequest( int, QMap< WidgetPropertyType, QVariant> & ) )
           );
    
    connect( d,
             SIGNAL( completed( int ) ),
             this,
             SLOT( handleCompleted( int ) )
           );
    
#endif // OPTIMIZE_INSTALLER
}


Q_DECL_EXPORT WidgetManager::~WidgetManager()
{
    delete d->m_installingWidget;

    qDeleteAll(d->m_widgetCache.values());

#if defined(QTWRT_USE_USIF)
    if (d->iAppInfo)
        delete d->iAppInfo;
#endif

    delete d;
}

/*!
 already installed web applications executable file/path

 \a pkgPath package file path
 \return returns launcher file path (html file path for widgets) or link for web applications
*/
Q_DECL_EXPORT QString WidgetManager::launcherPath(const QString &pkgPath)
{
    if (!d->checkCallerPermissions()) {
        return "";
    }

    LOG("BEGIN WidgetManager::launcherPath(" << pkgPath << ')');
    // Get the widget
    SuperWidget* widget = getWidgetFromCache(pkgPath);
    if (!widget) {
        // We don't have a widget for this path yet, create one
        widget = d->createWidget(pkgPath);
    }
    if (widget) {
        LOG("END WidgetManager::launcherpath returns" << widget->launcherPath(pkgPath));
        return widget->launcherPath(pkgPath);
    }

    LOG("END WidgetManager::launcherPath returns ' '");
    return "";
}

/*!
 \a pkgPath package file path
 \return returns true if pkgPath is already installed
*/
Q_DECL_EXPORT bool WidgetManager::isRegistered(const QString &pkgPath)
{
    if (!d->checkCallerPermissions()) {
        return false;
    }

    WidgetProperties *widgetProp = d->getWidgetProperties(pkgPath);
    if (widgetProp) {
        LOG("WidgetManager::isRegistered : pkgPath: " <<
                pkgPath << WebAppRegistry::instance()->isRegistered (widgetProp->id()));
        return WebAppRegistry::instance()->isRegistered (widgetProp->id());
    }

    return false;
}

/*!
 install's the package file passed. Registers the application in registry if package file is valid
o
 \a pkgPath package file path
 \return returns true if installation is successful
*/
#ifdef OPTIMIZE_INSTALLER
int WidgetManager::installWidget(const QString& pkgPath, const QMap<QString, QVariant>& configDetails )
{
    return d->installWidget( pkgPath, configDetails );
}
#endif //OPTIMIZE_INSTALLER
    
WidgetInstallError WidgetManager::install(const QString& pkgPath,
                                          QString& widgetId,
                                          bool silent,
                                          bool update,
                                          const QString& rootDirectory)
{
    if (!d->checkCallerPermissions()) {
        return WidgetInstallPermissionFailed;
    }

    LOG("WidgetManger::install silent =" << silent << "update =" << update);

    if (!d->validateDriveLetter(rootDirectory)) {
        return WidgetDriveLetterValidationFailed;
    }
    if (update) {
        return install(pkgPath, widgetId, "", d->m_rootDirectory, true, true);
    }
    return install(pkgPath, widgetId, "", d->m_rootDirectory, silent, false);
}


#ifdef OPTIMIZE_INSTALLER
void WidgetManager::cancel( int trxId )
{
    LOG("OPTINSTALLER: WidgetManager::cancel()");
    if( d )
        d->cancel( trxId );
}

#endif // OPTIMIZE_INSTALLER


/*!
 uninstall's the web app. Removes the entry from registry.

 \a pkgPath package file path
 \return returns true if uninstallation is successful
*/
Q_DECL_EXPORT WidgetUninstallError WidgetManager::uninstall(const QString &uniqueIdentifier, 
        bool silent)
{
    if (!d->checkCallerPermissions()) {
        return WidgetUninstallPermissionFailed;
    }

    WebAppInfo widget;
    if (!WebAppRegistry::instance()->isRegistered(uniqueIdentifier, widget)) {
        return WidgetUninstallFailed;
    }

    QString widgetName = ((widget.appTitle() == NO_NAME) ? uniqueIdentifier : widget.appTitle());

#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    // Create the global identifier for SIF uninstall progress notifications
    d->iSifProgressGlobalComponentId = 0;
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(uniqueIdentifier.utf16()));
    TRAPD(err, d->iSifProgressGlobalComponentId = HBufC16::NewL(ptr.Length()));
    if (err == KErrNone)
        d->iSifProgressGlobalComponentId->Des().Copy(ptr);

    // send the uninstall start notification
    if (d->iSifProgressGlobalComponentId) {
        Usif::CSifOperationStartData *startData;
        RPointerArray<HBufC> appNames;
        RPointerArray<HBufC> appIcons;

        TPtrC16 titleptr(reinterpret_cast<const TUint16*>(widgetName.utf16()));
        HBufC *title;
        TRAP(err, title = HBufC16::NewL(titleptr.Length()));
        if (err == KErrNone)
            title->Des().Copy(titleptr);

        TRAP(err, startData =
                    Usif::CSifOperationStartData::NewL(*d->iSifProgressGlobalComponentId, *title,
                                                       appNames, appIcons, d->iAppSize, KNullDesC,
                                                       KNullDesC, Usif::KSoftwareTypeWidget,
                                                       Usif::EUninstalling);
                    d->iSifProgressPublisher->PublishStartL(*startData);
                    User::Free(startData));
        if (err != KErrNone) {
            LOG("Uninstall Start Progress publishing error: " << err);
        }
    }

    bool removable = true;

    // Check to see if the widget is non-removable
    Usif::RSoftwareComponentRegistry scr;

    err = scr.Connect();
    if (err == KErrNone) {
        HBufC *appid;
        TPtrC16 ptr(reinterpret_cast<const TUint16*>(uniqueIdentifier.utf16()));
        TRAP(err, appid = HBufC16::NewL(ptr.Length()));
        if (err == KErrNone) {
            appid->Des().Copy(ptr);
            TInt compID;
            TRAP(err, compID = scr.GetComponentIdL(*appid, Usif::KSoftwareTypeWidget));
            if (err == KErrNone) {
                Usif::CComponentEntry *component;
                TRAP(err, component = Usif::CComponentEntry::NewL());
                if (err == KErrNone) {
                    TRAP(err, scr.GetComponentL(compID, *component));
                    if (err == KErrNone) {
                        removable = component->IsRemovable();
                    }

                    User::Free(component);
                }
            }

            User::Free(appid);
        }
    }

    if (!removable) {
        d->uninstallComplete(WidgetUninstallPermissionFailed);
        return WidgetUninstallPermissionFailed;
    }
#endif

    bool accepted = false;
    WidgetUninstallError err = WidgetUninstallSuccess;
    
#ifdef OPTIMIZE_INSTALLER
#if defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)
    SwiUI::CommonUI::CCUIDialogs* commonDialogs = NULL;
    if(!silent)
        TRAP_IGNORE(commonDialogs = SwiUI::CommonUI::CCUIDialogs::NewL());
#endif //defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)
#endif //OPTIMIZE_INSTALLER

    if (WebAppRegistry::instance()->isRegistered(uniqueIdentifier, widget)) {
        if (!silent)
        {
#ifdef OPTIMIZE_INSTALLER
#if defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)
            d->loadSymbianResourceFile();
            
            if( commonDialogs )
            {
                TPtrC16 widgetNamePtr(reinterpret_cast<const TUint16*>(widgetName.utf16()));
                HBufC* buf = StringLoader::LoadLC(R_UNINSTALL_CONFIRM, widgetNamePtr);
                QString prompt = QString ((QChar*)buf->Des().Ptr(),buf->Length()); //+ widgetName+" ?";
                TPtrC16 promptTextPtr(reinterpret_cast<const TUint16*>(prompt.utf16()));
                CleanupStack::PopAndDestroy(buf);
                accepted = commonDialogs->ShowConfirmationQueryL( promptTextPtr, 
                        R_AVKON_SOFTKEYS_YES_NO );
                if( accepted )
                    commonDialogs->ShowWaitDialogL(R_UNINSTALL_PREPARING,NULL,
                            R_AVKON_SOFTKEYS_EMPTY );
            }
#endif //defined(Q_OS_SYMBIAN) && defined(USE_NATIVE_DIALOG)
#else // !OPTIMIZE_INSTALLER
            accepted = d->showQuestion(TR_WM_WIDGET_UNINSTALL,
                                       TR_WM_WIDGET_REMOVE_QUERY + widgetName + '?',
                                       d->WidgetManagerPromptButtonYes,
                                       d->WidgetManagerPromptButtonNo);
#endif // OPTIMIZE_INSTALLER
        }
        if (silent || accepted) {
            emit installProgress(0);
            uninstallProgress(0);
            LOG("WidgetManager::uninstall : " << uniqueIdentifier);
            err = SuperWidget::uninstall(uniqueIdentifier, true, this);

#if defined(Q_OS_SYMBIAN) && !defined(QTWRT_USE_USIF)
            if (err == WidgetUninstallSuccess) {
                bool removeFromList(silent);
                err = uninstallSisx(uniqueIdentifier, removeFromList);
            }
#endif  // Q_OS_SYMBIAN && !USIF

            d->uninstallComplete(err);
        }
        else {
            d->uninstallComplete(WidgetUninstallCancelled);
            err = WidgetUninstallCancelled;
        }
    }
    else {
        d->uninstallComplete(WidgetUninstallFailed);
        err = WidgetUninstallFailed;
    }
    
#ifdef OPTIMIZE_INSTALLER
#if defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)
        if(commonDialogs) {
            if (!silent && accepted) {
                commonDialogs->CloseWaitDialogL();  //wait dialog shown only on confirmation of uninstallation
            }
            delete commonDialogs;
        }
        if(d->iResourceFileOffset) {
            CCoeEnv::Static()->DeleteResourceFile(d->iResourceFileOffset);
        }
#endif //defined(Q_OS_SYMBIAN) && defined (USE_NATIVE_DIALOG)
#endif //OPTIMIZE_INSTALLER
    return err;
}

void WidgetManager::tryContinueInstallation() {
    if (d->m_installingWidget) {
        d->m_installingWidget->tryContinueInstallation();
    }
}

void WidgetManager::cancelInstallation() {
    if( d->m_installingWidget) {
        d->m_installingWidget->tryCancelInstallation();
    } else {
        emit installationCancelled();
        qWarning() << "m_installingWidget is NULL!";
    }
}

Q_DECL_EXPORT WidgetProperties* WidgetManager::getProperties(const QString &pkgPath)
{
    if (!d->checkCallerPermissions()) {
        return 0;
    }

    return d->getWidgetProperties(pkgPath);
}

SuperWidget* WidgetManager::getWidgetFromCache(const QString& pkgPath)
{
    LOG("BEGIN WidgetManager::getWidgetFromCache");
    QString uniqueName;
    if (WebAppUniquenessHelper::getUniqueNameFromPath(pkgPath, uniqueName)) {
        LOG("WidgetManager::getWidgetFromCache uniqueName =" << uniqueName);

        if (d->m_widgetCache.contains(uniqueName)) {
            SuperWidget* widget = d->m_widgetCache.value(uniqueName);
            LOG("END WidgetManager::getWidgetFromCache found a widget =" << widget);
            return widget;
        }
    }

    LOG("END WidgetManager::getWidgetFromCache can't find widget");
    return 0;
}

void WidgetManager::setWidgetIntoCache(const QString& pkgPath, SuperWidget* widget)
{
    LOG("BEGIN WidgetManager::setWidgetIntoCache");
    QString uniqueName;
    if (WebAppUniquenessHelper::getUniqueNameFromPath(pkgPath, uniqueName)) {
        LOG("WidgetManager::setWidgetIntoCache uid =" << uniqueName);

        if (!d->m_widgetCache.contains(uniqueName)) {
            d->m_widgetCache.insert(uniqueName, widget);
            LOG("WidgetManager::setWidgetIntoCache saved widget =" << widget);
        }
    }
    LOG("END WidgetManager::setWidgetIntoCache");
}

void WidgetManager::removeWidgetFromCache(const QString& pkgPath)
{
    QString uniqueName;
    if (WebAppUniquenessHelper::getUniqueNameFromPath(pkgPath, uniqueName)) {

        if (d->m_widgetCache.contains(uniqueName)) {
            SuperWidget* widget = d->m_widgetCache.value(uniqueName);
            delete widget;
            d->m_widgetCache.remove(uniqueName);
        }
    }
}

bool WidgetManager::isValidWidgetType(const QString& widgetPath, const QString& contentType)
{
    if (!d->checkCallerPermissions()) {
        return false;
    }

    return SuperWidget::isValidWidgetType(widgetPath,contentType);
}

void WidgetManager::setContentType(const QString& contentType)
{
    if (!d->checkCallerPermissions()) {
        return;
    }
    d->m_contentType = contentType;
}

void WidgetManager::setInstallationAttributes(const WAC::InstallationAttributes& attributes) {
    d->m_attributes = attributes;
}

bool WidgetManager::isValidWidgetType(const QString& widgetPath)
{
    if (!d->checkCallerPermissions()) {
        return false;
    }

    QString contentType;
    if (getContentType(contentType)) {
        bool ret = isValidWidgetType(widgetPath,contentType);
        if (!ret)
            d->handleInstallError(WidgetUnZipBundleFailed);
        return ret;
    }
    else {
        return false;
    }
}

bool WidgetManager::getContentType(QString& contentType)
{
    if (!d->checkCallerPermissions()) {
        return false;
    }

    contentType = d->m_contentType;
    return true;
}

void WidgetManager::asyncInstall(const QString &pkgPath, bool silent, 
        const QString& rootDirectory, bool update)
{
    Q_UNUSED(rootDirectory);
    if (!d->checkCallerPermissions()) {
        return;
    }

    LOG(">> WidgetManager::asyncInstall");
    if ( !SuperWidget::isValidWidgetType(pkgPath, QString()) )
    {
        LOG("Invalid Widget type");
        emit installationError(WidgetTypeValidationFailed);
        return;
    }

    AsyncInstall *asyncInstall = new AsyncInstall( pkgPath, silent, update, 
            this, d->m_rootDirectory);
    QThreadPool::globalInstance()->start( asyncInstall );
    LOG("<< WidgetManager::asyncInstall");
}

/*!
 install's the SISX package file passed.
 Registers the application in registry if package file is valid
 \a pkgPath package file path
 \return UID of installed widget
*/
Q_DECL_EXPORT QString WidgetManager::installSisx(const QString &pkgPath, 
        const QString& rootDirectory)
{
#if defined(Q_OS_SYMBIAN)
    QString id;
    if (!d->checkCallerPermissions()) {
        return id;
    }

    LOG(">> WidgetManger::installSisx(" << pkgPath << ')');

    if (!d->validateDriveLetter(rootDirectory)) {
        return id;
    }

    // First check that only trusted clients can call this method
    RProcess process;
    if (process.VendorId() == TVendorId(VID_DEFAULT) &&
        process.HasCapability(ECapabilityAllFiles)) {

        // Install the widget
        QString widgetId;
        if (WidgetInstallSuccess == install(QDir::toNativeSeparators(pkgPath),
                                            widgetId,
                                            "",
                                            d->m_rootDirectory,
                                            true,
                                            false,
                                            KSIGNATUREID)) {

            // Get the ID
            if (d->m_installingWidget) {
                WidgetProperties *props(d->m_installingWidget->getProperties());
                if (props) {
                    id = props->id();
                    // the id and widgetId shall be the same at this point, need to verify

#ifndef QTWRT_USE_USIF
                    // Save the ID and the current process SID
                    // for use during uninstallation
                    TSecureId sid(process.SecureId());
                    TUint32 sidInt(sid.iId);
                    QHash<QString,uint> widgetList = readSisxWidgetList();
                    widgetList.insert(id, (uint)sidInt);
                    writeSisxWidgetList(widgetList);
#endif
                }
            }
        }
    }
    LOG("<< WidgetManger::installSisx(" << pkgPath << ')');
    return id;
#else
    return QString();
#endif
}

/*!
 Uninstall's the SISX package file passed.
 \a uniqueIdentifier ID of the widget to uninstall
 \a aRemoveFromlist Flag to remove widget from installed list
 \return returns An error code from WidgetUninstallError enum
*/
WidgetUninstallError WidgetManager::uninstallSisx(const QString &uniqueIdentifier,
                                                  bool aRemoveFromList)
{
#if defined(Q_OS_SYMBIAN)
    LOG(">> WidgetManager::uninstallSisx(" << uniqueIdentifier << ", " << 
            aRemoveFromList << ')');
    WidgetUninstallError result(WidgetUninstallSuccess);

    // First check if widget was installed as a SISX wrapper
    QHash<QString,uint> widgetList = readSisxWidgetList();
    uint sid = widgetList.value(uniqueIdentifier, -1);
    if (-1 != sid) {
        SwiUI::RSWInstLauncher swinst;
        if (KErrNone == swinst.Connect()) {
            SwiUI::TUninstallOptionsPckg options;
            TInt err(swinst.SilentUninstall(TUid::Uid(sid), options, KSISMIMETYPE));
            if (SwiUI::KSWInstErrBusy == err) {
                result = WidgetUninstallSuccessButIncomplete;
            } else if (KErrNone != err) {
                result = WidgetUninstallFailed;
            }
            swinst.Close();
            if (aRemoveFromList) {
                widgetList.remove(uniqueIdentifier);
                writeSisxWidgetList(widgetList);
            }
        }
    }
    return result;
#else
    // TODO: check value
    return WidgetUninstallFailed;
#endif
}

/*!
 Reads the SISX widget list from file
*/
QHash<QString, uint> WidgetManager::readSisxWidgetList()
{
    QHash<QString, uint> widgetList;
#if defined(Q_OS_SYMBIAN)
    QFile file(sisxWidgetFile());
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream dataStream(&file);
        dataStream >> widgetList; //codescanner::leave
        file.close();
    }
#endif
    return widgetList;
}

/*!
 Writes the SISX widget list to file
*/
void WidgetManager::writeSisxWidgetList(QHash<QString, uint>& aWidgetList)
{
#if defined(Q_OS_SYMBIAN)
    QString fileName(sisxWidgetFile());
    if (!QFile::exists(fileName)) {
        QDir dir = QFileInfo(fileName).dir();
        dir.mkpath(dir.path());
    }
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream dataStream(&file);
        dataStream << aWidgetList; //codescanner::leave
        file.close();
    }
#endif
}

/*!
 Gets the name of the SISX widget list file
*/
QString WidgetManager::sisxWidgetFile()
{
#if defined(Q_OS_SYMBIAN)
    QString indexFileName(DEFAULT_ROOT_DIRECTORY);
    indexFileName += QDir::separator();
    indexFileName += KINDEXFILE;
    indexFileName = QDir::toNativeSeparators(indexFileName);
    return indexFileName;
#else
    return QString();
#endif
}

// function to install widgets
// parameters:
//     widgetPath          path to widget bundle
//     widgetContentType   content type of widget (optional)
//     silent              true for silent install (optional)
//                         false by default
// return
//     bool                true if successful
//
// creates appropriate widget and installs it
WidgetInstallError WidgetManager::install(const QString& widgetPath,
                                          QString &widgetId,
                                          const QString& widgetContentType,
                                          const QString& rootDirectory,
                                          const bool silent,
                                          const bool update,
                                          const QString &sigId)
{
    Q_UNUSED(sigId);
    d->m_silent = silent;

    WidgetType widgetType = SuperWidget::getWidgetType(widgetPath, widgetContentType);

    if (WidgetTypeUnknown == widgetType) {
        LOG("WidgetManager::install - Invalid Widget type");
        d->handleInstallError(WidgetTypeValidationFailed);
        emit installationError(WidgetTypeValidationFailed);
        return WidgetTypeValidationFailed;
    }

    if (d->m_installingWidget) {
        delete d->m_installingWidget;
        d->m_installingWidget = 0;
    }

    // Create a new installing widget
    QString rootDir = rootDirectory;

    switch (widgetType) {

    case WidgetTypeW3c:
        d->m_installingWidget = new WgtWidget(rootDir);
        d->m_installingWidget->disableBackupRestoreValidation(d->m_disableUnsignWidgetSignCheck);
        d->m_installingWidget->setInstallationAttributes(d->m_attributes);
        break;
    default:
        LOG("WidgetManager::install Invalid Widget type");
        return WidgetTypeValidationFailed;
    }

    LOG("WidgetManager::install widget installed path: " << 
            d->m_installingWidget->widgetInstallPath());

    if (!d->m_installingWidget) {
        return WidgetSystemError;
    }

    // FIXME : Silent / not silent should not visible. SIGNALS or interfacing for this class

#if defined(Q_OS_SYMBIAN)
    connect(d->m_installingWidget, SIGNAL(queryConfirmation()), 
            this, SIGNAL(queryConfirmation()));
    connect(d->m_installingWidget, SIGNAL(queryConfirmation()), 
            d, SLOT(handleConfirmation()), Qt::DirectConnection);
#else
    // Symbian OS installs can prompt after in-memory config.xml parsing in order
    // to be able to show widget name, etc., rather than filename
    if (!silent) {
        bool cancelled = !d->showQuestion(TR_WM_WIDGET_INSTALL,
                                          TR_WM_WIDGET_INSTALL_QUERY + 
                                          QFileInfo(widgetPath).fileName() + " ?",
                                          d->WidgetManagerPromptButtonYes,
                                          d->WidgetManagerPromptButtonNo);

        if (cancelled)
        {
            d->showInformation(TR_WM_WIDGET_INSTALL, TR_WM_WIDGET_INSTALL_CANCELLED);

            return WidgetUserConfirmFailed;
        }
    }
#endif

    connect(d->m_installingWidget, SIGNAL(installProgress(int)), 
            this, SIGNAL(installProgress(int)));
    connect(d->m_installingWidget,
            SIGNAL(installProgress(int)),
            d,
            SLOT(handleInstallProgress(int)),
            Qt::DirectConnection);

    // Connect all signals from a widget to dialgos handles of this class.
    connect(d->m_installingWidget, SIGNAL(installationError(WidgetInstallError)),
            d, SLOT(handleInstallError(WidgetInstallError)));
    connect(d->m_installingWidget, SIGNAL(aboutToReplaceExistingWidget(QString)),
            d, SLOT(handleWidgetReplacement(QString)), Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(queryInstallationDestination(unsigned long, bool)),
            d, SLOT(handleDestinationSelection(unsigned long, bool)), Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(aboutToInstallWidgetWithFeatures(QList<QString>)),
            d, SLOT(handleFeatureInstallation(QList<QString>)),Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(aboutToInstallUntrustedWidget(WidgetInstallError)),
            d, SLOT(handleUntrustedWidgetInstallation(WidgetInstallError)), Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(installationSucceed()),
            d, SLOT(handleInstallationSuccess()));

    // Widget installation signals forwarded out of this class.
    // This provides one way to rid of qmessageboxes from this class and applications that are
    // using this would be in charge of showing correct ui dialogs.
    // However, silentInstall is only thingie which could really utilize this.
    connect(d->m_installingWidget, SIGNAL(installationError(WidgetInstallError)),
            this, SIGNAL(installationError(WidgetInstallError)));
    connect(d->m_installingWidget, SIGNAL(aboutToReplaceExistingWidget(QString)),
            this, SIGNAL(aboutToReplaceExistingWidget(QString)), Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(queryInstallationDestination(unsigned long, bool)),
            this, SIGNAL(queryInstallationDestination(unsigned long, bool)), Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(aboutToInstallUntrustedWidget(WidgetInstallError)),
            this, SIGNAL(aboutToInstallUntrustedWidget(WidgetInstallError)), Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(aboutToInstallWidgetWithFeatures(QList<QString>)),
            this, SIGNAL(aboutToInstallWidgetWithFeatures(QList<QString>)),Qt::DirectConnection);
    connect(d->m_installingWidget, SIGNAL(installationSucceed()),
            this, SIGNAL(installationSucceed()));

#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    d->iSifProgressGlobalComponentId = 0;
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(widgetPath.utf16()));
    TRAPD(err, d->iSifProgressGlobalComponentId = HBufC16::NewL(ptr.Length()));
    if (err == KErrNone) {
        d->iSifProgressGlobalComponentId->Des().Copy(ptr);
    }
#endif

    enum WidgetInstallError status=WidgetInstallSuccess;
    d->m_installingWidget->setWidgetBundlePath(widgetPath);
    if (update) {
        status = d->m_installingWidget->install(true);
        disconnect(d->m_installingWidget,
                   SIGNAL(installProgress(int)),
                   this,
                   SIGNAL(installProgress(int)));
        if (status == WidgetInstallSuccess) {
            widgetId = d->m_installingWidget->getProperties()->id();
            removeWidgetFromCache(d->m_installingWidget->getProperties()->installPath());
        }

        return status;
    }

    if (!sigId.isNull() && !sigId.isEmpty() && widgetType == WidgetTypeW3c) {
#ifdef Q_OS_SYMBIAN
        WgtWidget *widget = static_cast<WgtWidget*>(d->m_installingWidget);
        status = widget->install(sigId);
#else
        status = d->m_installingWidget->install();
#endif
    } else {
        status = d->m_installingWidget->install();
    }

    if (status == WidgetInstallSuccess) {
        widgetId = d->m_installingWidget->getProperties()->id();
    }

    disconnect(d->m_installingWidget,
               SIGNAL(installProgress(int)),
               this,
               SIGNAL(installProgress(int)));

#if defined(QTWRT_USE_USIF) && defined(Q_OS_SYMBIAN)
    if (d->iSifProgressGlobalComponentId) {
        User::Free(d->iSifProgressGlobalComponentId);
    }
#endif
    return status;
}

/*
 * Returns the installing widget properties
 */
WidgetProperties* WidgetManager::getProperties()
{
    if (!d->checkCallerPermissions()) {
        return 0;
    }

    if (d->m_installingWidget)
    {
        return d->m_installingWidget->getProperties();
    }
    return 0;
}

/*
 * Constructs the widgets installation path
 * TODO: This method actually gets the data location, not installation path.
 *       Should be renamed getWidgetDataPath().
 */
QString WidgetManager::getWidgetInstallPath(const QString& rootDirectory)
{
    Q_UNUSED(rootDirectory);
    QString installPath("");

#ifdef Q_OS_SYMBIAN
    installPath = QDir::toNativeSeparators(rootDirectory);
    if (installPath.startsWith("Z"))
    installPath.replace(0, 1, "C");
    QString privatePath = installPath[0]+":"+QDir::separator()+"private";
    if (! createDir(privatePath)) {
        LOG("SuperWidget::getWidgetInstallPath - Could not create private dir");
        return QString();
      }
    if (! createDir(installPath)) {
        LOG("SuperWidget::getWidgetInstallPath - Could not create widget UI private dir");
        return QString();
    }
    installPath = installPath + QDir::separator() + WIDGET_FOLDER;
#elif defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6)
    installPath = WIDGET_INSTALL_PATH;
#elif defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    installPath = WIDGET_INSTALL_PATH;
#else   // win32 || mac
    QString dataLocation =  QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    installPath = dataLocation + installPath + QDir::separator() + WIDGET_FOLDER;
#endif

    if (! createDir(installPath)) {
        LOG("SuperWidget::getWidgetInstallPath - Could not create widget dir:" << installPath);
        return QString();
    }
  return installPath;
}

/*
 * Directory Creation for Widget Installation
 */
bool WidgetManager::createDir(const QString& path)
{
  QDir dir(path);
  if (!dir.exists()){
    if (!dir.mkpath(dir.absolutePath())) {
      LOG("WidgetManager::createDir could not create dir");
      return false;
      }
    }
    return true;
}

bool WidgetManager::backupRestoreInstall(const QString &pkgPath, bool disableUnsignWidgetSignCheck)
{
#ifdef Q_OS_SYMBIAN
        // for security reasons, any process other than backup-restore/MediaManager should not be
        // allowed to use this API.
        RProcess myUID;
        TUidType uidType(myUID.Type());
        TUint32 uid = uidType[2].iUid;
        LOG("WidgetManager::backupRestoreInstall()");
        if ((uid == BACKUP_RESTORE_UID || uid ==  MEDIA_MANAGER_UID) 
                && pkgPath.length() > ROOT_DIRECTORY_LENGTH) {
            //set BUR process
            d->m_disableUnsignWidgetSignCheck = disableUnsignWidgetSignCheck;
            QString widgetId;
            int len = pkgPath.indexOf(QDir::separator(),ROOT_DIRECTORY_LENGTH); // first '\' after "T:\Private\#"
            QString rootDirectory(pkgPath.left(len));
            return (install(pkgPath, widgetId, true, false, rootDirectory) 
                    == WidgetInstallSuccess);
        } else {
            LOG("WidgetManager::backupRestoreInstall() - Aborting install. This is not Backup-Restore process");
            return false;
        }
#else
      return false;
#endif
}

#ifdef QTWRT_USE_USIF
void WidgetManager::uninstallProgress(int progress)
#else // !QTWRT_USE_USIF
void WidgetManager::uninstallProgress(int /*progress*/)
#endif
{
#ifdef QTWRT_USE_USIF
    if (d->iSifProgressGlobalComponentId) {
        Usif::CSifOperationProgressData *progressData;
        TRAP_IGNORE(progressData =
                    Usif::CSifOperationProgressData::NewL(*d->iSifProgressGlobalComponentId,
                                                          Usif::EUninstalling,
                                                          Usif::ENoSubPhase, progress,
                                                          100);
                    d->iSifProgressPublisher->PublishProgressL(*progressData);
                    User::Free(progressData));
    }
#endif
}
#ifdef OPTIMIZE_INSTALLER
void WidgetManager::handleProgress( int transId, int percentComplete, 
        WidgetOperationType operationType )
{
    emit progress( transId, percentComplete, operationType );
}
#endif //OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManager::handleAborted( int transId, WidgetErrorType errCode )
{
    emit aborted( transId, errCode );
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManager::handleInteractiveRequest( int transId, QMap<WidgetPropertyType, 
        QVariant> &properties )
{
    emit interactiveRequest( transId, properties );
}
#endif // OPTIMIZE_INSTALLER

#ifdef OPTIMIZE_INSTALLER
void WidgetManager::handleCompleted( int transId )
{
    emit completed( transId ); 
}
#endif // OPTIMIZE_INSTALLER


