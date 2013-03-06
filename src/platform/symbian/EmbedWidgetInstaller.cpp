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

#include "EmbedWidgetInstaller.h"
#include "../../private/WidgetUtilsLogs.h"

/*_LIT(KSilentInstallCommand, "install silent");
_LIT(KSilentUninstallCommand, "uninstall silent");*/


/***CProcessMonitor***/
CProcessMonitor::CProcessMonitor( RProcess& aProcess,
                                  MProcessMonitorObserver &aObserver  )
: CActive( EPriorityStandard ) // Standard priority
, m_process( aProcess )
, m_observer( aObserver )
{
}

CProcessMonitor* CProcessMonitor::NewLC( RProcess& aProcess,
                                         MProcessMonitorObserver &aObserver  )
{
    CProcessMonitor* self = new ( ELeave ) CProcessMonitor( aProcess, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
}

CProcessMonitor* CProcessMonitor::NewL( RProcess& aProcess,
                                        MProcessMonitorObserver &aObserver  )
{
    CProcessMonitor* self = CProcessMonitor::NewLC( aProcess, aObserver );
    CleanupStack::Pop(); // self;
    return self;
}

void CProcessMonitor::ConstructL()
{
    CActiveScheduler::Add( this );              // Add to scheduler
}

CProcessMonitor::~CProcessMonitor()
{
    Cancel(); // Cancel any request, if outstanding
}

void
CProcessMonitor::MonitorProcess( )
{
    m_process.Logon( iStatus );
    iStatus = KRequestPending;
    SetActive();
    m_process.Resume();
}

void
CProcessMonitor::CancelMonitorProcess()
{
    m_process.LogonCancel( iStatus );
}

void CProcessMonitor::DoCancel()
{
    CancelMonitorProcess();
}

void CProcessMonitor::RunL()
{
    TInt reason = m_process.ExitReason();
    TExitType type = m_process.ExitType();
    CallObserver(reason);
}

void
CProcessMonitor::CallObserver( TInt& aReason )
{
    m_observer.ProcessDied( aReason );
}

TInt CProcessMonitor::RunError( TInt aError )
{
    return aError;
}


EmbedWidgetInstallerPrivate:: EmbedWidgetInstallerPrivate()
{
}

EmbedWidgetInstallerPrivate:: ~EmbedWidgetInstallerPrivate()
{
}

TUint32 EmbedWidgetInstallerPrivate::getProcUid() const
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
    RProcess me;
    TUidType uidType(me.Type());
    TUint32 uid3 = uidType[2].iUid;

    LOG("UID3 of this process : " << uid3);
    return uid3;
#else
    return 0;
#endif
}
#if defined(QTWRT_USE_EMBEDDEDINSTALLER) && defined(QTWRT_USE_USIF)
int EmbedWidgetInstallerPrivate::getSymbianUidFromWidgetUniqueIdL(QString& widgetId)
#else
int EmbedWidgetInstallerPrivate::getSymbianUidFromWidgetUniqueIdL(QString& /*widgetId*/)
#endif 
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER) && defined(QTWRT_USE_USIF)
	Usif::TComponentId componentId;
	Usif::CIntPropertyEntry *propEntry = NULL;
	User::LeaveIfError(m_SCR.Connect());
	CleanupClosePushL(m_SCR);
	HBufC16 *theBuf = HBufC16::NewLC(120);
	TPtrC16 tptr(reinterpret_cast<const TUint16*>(widgetId.utf16()));
	theBuf->Des().Append(tptr);
	componentId = m_SCR.GetComponentIdL(*theBuf, Usif::KSoftwareTypeWidget);
	HBufC16 *theBuf1 = HBufC16::NewLC(SCR_PROP_UID.length());
	TPtrC16 tptr1(reinterpret_cast<const TUint16*>(SCR_PROP_UID.utf16()));
	theBuf1->Des().Append(tptr1);
	propEntry  = (Usif::CIntPropertyEntry *)m_SCR.GetComponentPropertyL(componentId, *theBuf1);
	CleanupStack::PopAndDestroy(3);
	if (propEntry)
	    return propEntry->IntValue();
	else
	    return NULL;
#else
    return NULL;
#endif
}


EmbedWidgetInstaller:: EmbedWidgetInstaller()
    : d(new EmbedWidgetInstallerPrivate())
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
    d->m_installError = WidgetInstallFailed;
    d->m_uninstallError = WidgetUninstallFailed;
    d->m_widgetComponentIdProperty = TUid::Uid(d->getProcUid());
    _LIT_SECURE_ID(installerAppId, 0x200267D2);
    TSecurityPolicy installerWritePolicy = TSecurityPolicy(installerAppId, ECapabilityWriteDeviceData);
    TSecurityPolicy installerReadPolicy = TSecurityPolicy(TSecurityPolicy::EAlwaysPass);
    RProperty::Define(d->m_widgetComponentIdProperty, EWidgetUniqueIdProperty,
                                 RProperty::EByteArray, installerReadPolicy, installerWritePolicy,
                                 RProperty::KMaxPropertySize);
    d->m_property.Attach(d->m_widgetComponentIdProperty, EWidgetUniqueIdProperty);
#endif
}

EmbedWidgetInstaller:: ~EmbedWidgetInstaller()
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
    RProperty::Delete(d->m_widgetComponentIdProperty, EWidgetUniqueIdProperty);
#endif
    delete d;
}

#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
WidgetInstallError EmbedWidgetInstaller::installL(QString& bundlePath, QString& widgetId )
#else
WidgetInstallError EmbedWidgetInstaller::installL(QString& /*bundlePath*/, QString& /*widgetId*/ )
#endif
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
     // Set The UID of WidgetInstallerApp
     const TInt KAppUid = 0x200267D2;
     TUid id( TUid::Uid( KAppUid ) );
     TUint32 callerProcUid = d->getProcUid();
     RApaLsSession session;
     User::LeaveIfError( session.Connect() );
     CleanupClosePushL( session );
     TApaAppInfo info;
     User::LeaveIfError(session.GetAppInfo(info, id));

     CleanupStack::PopAndDestroy(); // session

     TFileName fullName;

     QString qRootDirectory = "c:\\private\\" + QString::number(callerProcUid, 16);;

     HBufC* command;
     command = HBufC::NewMaxLC(KSilentInstallCommand().Length() + (KMaxFileName * 2) + 100);
     command->Des().Copy(KSilentInstallCommand);

     command->Des().Append(_L(" \""));

     TPtrC16 tptr(reinterpret_cast<const TUint16*>(bundlePath.utf16()));
     command->Des().Append(tptr);
     command->Des().Append(_L('\"'));

     command->Des().Append(_L(" \""));

     TPtrC16 ptr(reinterpret_cast<const TUint16*>(qRootDirectory.utf16()));
     command->Des().Append(ptr);
     command->Des().Append(_L("\" "));

     TBuf<32> uidStr;
     uidStr.Num(d->m_widgetComponentIdProperty.iUid);

     command->Des().Append(uidStr);

     TInt err = d->m_newProc.Create(info.iFullName, *command);

     d->m_processMonitor = CProcessMonitor::NewL(d->m_newProc,*this);
     d->m_processMonitor->MonitorProcess();
     d->m_eLoop.exec();

     if (d->m_installError == WidgetInstallSuccess)
         {
          TBuf<RProperty::KMaxPropertySize> widgetUniqueId;
          if (d->m_property.Get(widgetUniqueId) == KErrNone)
          	{
          widgetId = QString::fromUtf16(widgetUniqueId.Ptr(), widgetUniqueId.Length());
          }
         }
        CleanupStack::PopAndDestroy();
        return d->m_installError;
#else
        // TODO: Check for UNSUPPORTED code
        return WidgetInstallFailed;
#endif
}

#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
void EmbedWidgetInstaller::ProcessDied( TInt& aReason )
#else
void EmbedWidgetInstaller::ProcessDied( TInt& /*aReason*/ )
#endif
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
    d->m_eLoop.quit();

    // WidgetInstallerApp currently only returns the error code 0 or 1
    if (aReason == 0)
        {
        d->m_installError = WidgetInstallSuccess;
        LOG("Widget Install success");
        }
    else
        {
        d->m_installError = WidgetInstallFailed;
        LOG("Widget Install Failure");
        }
#endif
}
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
WidgetUninstallError EmbedWidgetInstaller::uninstallL(QString& widgetId)
#else
WidgetUninstallError EmbedWidgetInstaller::uninstallL(QString& /*widgetId*/)
#endif
{
#if defined(QTWRT_USE_EMBEDDEDINSTALLER)
     // Set The UID of WidgetInstallerApp
     const TInt KAppUid = 0x200267D2;
     TUid id( TUid::Uid( KAppUid ) );
     RApaLsSession session;
     User::LeaveIfError( session.Connect() );
     CleanupClosePushL( session );

     TApaAppInfo info;
     User::LeaveIfError(session.GetAppInfo(info, id));

     CleanupStack::PopAndDestroy(); // session

     TFileName fullName;

     HBufC* command;
     command = HBufC::NewMaxLC(KSilentUninstallCommand().Length() + 100);
     command->Des().Copy(KSilentUninstallCommand);

     command->Des().Append(_L(" "));

     int uid;
     uid = d->getSymbianUidFromWidgetUniqueIdL(widgetId);
     if(uid == NULL)
         {
             CleanupStack::PopAndDestroy();
             return WidgetUninstallFailed;
         }

     command->Des().AppendNum(uid);

     TInt err = d->m_newProc.Create(info.iFullName, *command);

     d->m_processMonitor = CProcessMonitor::NewL(d->m_newProc,*this);
     d->m_processMonitor->MonitorProcess();
     d->m_eLoop.exec();

     CleanupStack::PopAndDestroy();

     if (d->m_installError == WidgetInstallSuccess)
         return WidgetUninstallSuccess;
     else
         return WidgetUninstallFailed;
#else
        // TODO: Check for UNSUPPORTED code
        return WidgetUninstallFailed;
#endif
}
