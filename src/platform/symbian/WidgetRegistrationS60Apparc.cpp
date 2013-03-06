//
// ==============================================================================
//  Name        : WidgetRegistrationS60Apparc.cpp
//  Part of     : SW Installer UIs / WidgetInstallerUI
//  Description : This is defines CWidgetRegistrationS60Apparc which handles
//                registration of widgets
//  Version     : 3.1
//
// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
//
// This file is part of Qt Web Runtime.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
//

// INCLUDE FILES
#include "WidgetRegistrationS60Apparc.h"
#include <apgcli.h>
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <apgicnfl.h>
#else
#include <apgicnfl.h>
#include <apgicnflpartner.h> // new file introduced by xSymbian
#endif
#include <S32MEM.H>

#ifdef QTWRT_USE_USIF
const TInt KWidgetRegistryVal = KMaxFileName;
#else
#include <WidgetRegistryConstants.h>
#endif

#include <apacmdln.h>

#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <apgicnflpartner.h>
#endif //SYMBIAN_ENABLE_SPLIT_HEADERS


// CONSTANTS
_LIT(KMBMExt, ".mbm");

using namespace SwiUI;

// MEMBER FUNCTION DECLARATIONS

// ============================================================================
// CWidgetRegistrationS60Apparc::NewL()
// two-phase constructor
//
// @since 3.1
// ============================================================================
//
CWidgetRegistrationS60Apparc* CWidgetRegistrationS60Apparc::NewL( RFs& aFs )
    {
    CWidgetRegistrationS60Apparc *self =
        new ( ELeave ) CWidgetRegistrationS60Apparc( aFs );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ============================================================================
// CWidgetRegistrationS60Apparc::CWidgetRegistrationS60Apparc()
// C++ constructor
//
// @since 3.1
// ============================================================================
//
CWidgetRegistrationS60Apparc::CWidgetRegistrationS60Apparc(
    RFs& aFs )
    : iFs( aFs )
    {
    }

// ============================================================================
// CWidgetRegistrationS60Apparc::~CWidgetRegistrationS60Apparc()
// destructor
//
// @since 3.1
// ============================================================================
//
CWidgetRegistrationS60Apparc::~CWidgetRegistrationS60Apparc()
    {
    }

// ============================================================================
// CWidgetRegistrationS60Apparc::CWidgetRegistrationS60Apparc()
// Symbian second phase constructor
//
// @since 3.1
// ============================================================================
//
void CWidgetRegistrationS60Apparc::ConstructL()
    {
    }

// ============================================================================
// CWidgetRegistrationS60Apparc::DeregisterWidgetL()
// Deregister installed widget as non native app
//
// @since 3.1
// ============================================================================
//
void CWidgetRegistrationS60Apparc::DeregisterWidgetL( const TUid& aUid )
    {
    RApaLsSession apparcSession;
    User::LeaveIfError( apparcSession.Connect() );

    apparcSession.PrepareNonNativeApplicationsUpdatesL();
    apparcSession.DeregisterNonNativeApplicationL( aUid );
    apparcSession.DeregisterNonNativeApplicationTypeL( aUid );
    apparcSession.CommitNonNativeApplicationsUpdatesL();
    apparcSession.Close();
    }

// ============================================================================
// CWidgetRegistrationS60Apparc::RegisterWidgetL()
// Register installed widget as non native app
//
// It will leave if registration is impossible or fails.  It will not
// leave if icon path is KNullDesC or unusable since a default icon
// will be used.
//
// @since 3.1
// ============================================================================
//
void CWidgetRegistrationS60Apparc::RegisterWidgetL(
    const TDesC& aMainHTML,
    const TDesC& aBundleDisplayName,
    const TDesC& aIconPath,
    const TDesC& aDriveName,
    const TUid& aUid,
    bool hideIcon)
    {
    RApaLsSession apparcSession;
    CleanupClosePushL( apparcSession );
    User::LeaveIfError( apparcSession.Connect() );
//    apparcSession.PrepareNonNativeApplicationsUpdatesL();
//    apparcSession.DeregisterNonNativeApplicationL( KUidWidgetLauncher );
//    apparcSession.DeregisterNonNativeApplicationTypeL( KUidWidgetLauncher );
//    apparcSession.CommitNonNativeApplicationsUpdatesL();

    // reasonably like an acceptable file name
    TBuf<KWidgetRegistryVal> appName;
    appName.Append( aMainHTML );

    CApaRegistrationResourceFileWriter* writer =
        CApaRegistrationResourceFileWriter::NewL(
            aUid,
            appName,
            TApaAppCapability::ENonNative );
    CleanupStack::PushL( writer );

    TBuf8<KWidgetRegistryVal> opaqueData;
    RDesWriteStream writeStream( opaqueData );

    // le data opaque
    writeStream.WriteUint32L( aUid.iUid );
    writeStream.WriteUint32L( aMainHTML.Length() );
    writeStream.WriteL( aMainHTML );

    writeStream.CommitL();
    writer->SetOpaqueDataL( opaqueData );

    if(hideIcon)
    {
        writer->SetAppIsHiddenL(ETrue);
    }
    
    // avec nom de plume
    CApaLocalisableResourceFileWriter* lWriter =
        CApaLocalisableResourceFileWriter::NewL(
            KNullDesC,
            aBundleDisplayName,
            1 ,
            KNullDesC );
    CleanupStack::PushL( lWriter );

    // This call deletes any pending registrations which are not commited
    // in previous installations (ex: power off case).
    // This must be the first call after session connect,
    // and before Prepare... call.
    // This function returns with no effect, if no pending registrations in
    // previous install.
    apparcSession.RollbackNonNativeApplicationsUpdates();

    // Prepare for Registration & Deregistration.
    // Actual Registration & Deregistration will be done at Commit call
    // CommitNonNativeApplicationsUpdatesL.
    apparcSession.PrepareNonNativeApplicationsUpdatesL();

    RFile appArcIconFile;
    CleanupClosePushL( appArcIconFile );
    RFile* toIconFile = NULL;

    // la petit image (ou non)
    if ( aIconPath.Length() )
        {
        TFileName mbmIcon;
        mbmIcon.Append( aIconPath );
        mbmIcon.Append( aUid.Name() );
        mbmIcon.Append( KMBMExt() );
        TInt error =
            appArcIconFile.Open(
                iFs, mbmIcon,
                EFileShareReadersOrWriters|EFileWrite );
        if ( KErrNone == error )
            {
            toIconFile = &appArcIconFile;
            }
        }


    const TUid KUidWidgetLauncher = { 0x200267DC };
    _LIT( KLauncherApp, "wgtwidgetlauncher.exe" );


    apparcSession.RegisterNonNativeApplicationL(
        KUidWidgetLauncher,
        aDriveName,
        *writer,
        lWriter,
        toIconFile );

    TRAP_IGNORE( apparcSession.RegisterNonNativeApplicationTypeL(
                     KUidWidgetLauncher,
                     KLauncherApp() ) );

    apparcSession.CommitNonNativeApplicationsUpdatesL();

    // appArcIconFile, writer, lWriter, apparcSession
    CleanupStack::PopAndDestroy( 4 );
    }

void CWidgetRegistrationS60Apparc::closeWidgetL( const TUid& aUid )
    {
    RApaLsSession apparcSession;
    User::LeaveIfError( apparcSession.Connect() );

    TInt size = 3*sizeof( TUint32 );

    CApaCommandLine* cmd( CApaCommandLine::NewLC() );
    HBufC8* opaque( HBufC8::NewLC( size ) );

    RDesWriteStream stream;
    TPtr8 des( opaque->Des() );

    stream.Open( des );
    CleanupClosePushL( stream );

    // Generate the command.
    stream.WriteUint32L( aUid.iUid );
    stream.WriteUint32L( 0 );

    // Number 2 is for EDeactivate WidgetOperation.
    stream.WriteInt32L( 2 );
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
    }

//  End of File
