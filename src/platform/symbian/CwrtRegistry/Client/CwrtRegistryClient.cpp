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
#include <e32base.h>
#include <f32file.h>
#include <s32mem.h>
#include "wacRegistry.h"
#include "CwrtRegistryData.h"
#include "wacRegistryClient.h"

// ============================================================================
// Starts the CWRT registry server
//
// ============================================================================
//
TInt StartServer()
    {
    const TUidType KServerUidType(
        KNullUid, KNullUid, KWACRegistryServerUid );

    RProcess server;
    TInt ret = server.Create(
        KWACRegistryImage, KNullDesC, KServerUidType );

    // Did we manage to create the thread/process?
    if (ret == KErrNone)
        {
        // Wait to see if the thread/process died during construction
        TRequestStatus serverDiedRequestStatus;
        server.Rendezvous( serverDiedRequestStatus );

        // do we have to abort?
        if ( serverDiedRequestStatus != KRequestPending )
            {
            // abort the startup here
            server.Kill(0);
            }
        else
            {
            // start server
            server.Resume();
            }

        User::WaitForRequest( serverDiedRequestStatus );

        // Determine the reason for the server exit.
        const TInt exitReason = ( server.ExitType() == EExitPanic ) ?
            KErrGeneral : serverDiedRequestStatus.Int();
        ret = exitReason;

        // Finished with process handle
        server.Close();
        }

    return ret;
    }

// ============================================================================
// RCwrtRegistryClientSession::RCwrtRegistryClientSession()
// C++ constructor
//
// ============================================================================
//
EXPORT_C RWACRegistryClientSession::RWACRegistryClientSession()
    {
    }

// ============================================================================
// RCwrtRegistryClientSession::Connect()
// Connect to widgetregistry server
//
// ============================================================================
//
EXPORT_C TInt RWACRegistryClientSession::Connect()
    {
    TInt startupAttempts = KWACRegistryServerStartupAttempts;

    for ( ;; )
        {
        TInt ret = CreateSession( KWACRegistryName, Version(),
            KWACRegistryServerAsynchronousSlotCount );

        if ( ret != KErrNotFound && ret != KErrServerTerminated )
            {
            return ret;
            }

        if ( startupAttempts-- == 0 )
            {
            return ret;
            }

        ret = StartServer();
        if ( ret != KErrNone && ret != KErrAlreadyExists )
            {
            return ret;
            }
        }
    }

// ============================================================================
// RCwrtRegistryClientSession::Version()
// Returns the version
//
// ============================================================================
//
EXPORT_C TVersion RWACRegistryClientSession::Version() const
    {
    const TVersion version(
        KWACRegistryClientVersionMajor,
        KWACRegistryClientVersionMinor,
        KWACRegistryClientVersionBuild );

    return version;
    }


// ============================================================================
// RCwrtRegistryClientSession::Disconnect()
// Disconnect from widgetregistry server
//
// ============================================================================
//
EXPORT_C TInt RWACRegistryClientSession::Disconnect()
    {
    SendReceive( EOpCodeWidgetRegistryDisconnect, TIpcArgs() );
    RSessionBase::Close();
    return KErrNone;
    }

// ============================================================================
// RCwrtRegistryClientSession::RegisterWidgetL()
// Registers the widget
//
// ============================================================================
EXPORT_C bool RWACRegistryClientSession::RegisterWidgetL(
                        const QString& appId, const QString& appTitle,
                        const QString& appPath, const QString& iconPath,
                        const AttributeMap& attributes, const QString& type,
                        unsigned long size, const QString& startPath)
    {
    CBufFlat* buf = CBufFlat::NewL( 5120 );
    CleanupStack::PushL( buf );

    RBufWriteStream stream( *buf );
    CleanupClosePushL( stream );

    SerializeStringL( stream, appId );
    SerializeStringL( stream, appTitle );
    SerializeStringL( stream, appPath );
    SerializeStringL( stream, iconPath );
    SerializeMapL( stream, attributes );
    SerializeStringL( stream, type );
    SerializeIntL( stream, size );
    SerializeStringL( stream, startPath );

    CleanupStack::PopAndDestroy( &stream );

    TPtr8 p( buf->Ptr(0) );
    bool ret = SendReceive( EOpCodeRegisterWidget, TIpcArgs( &p ) );

    CleanupStack::PopAndDestroy( buf );

    return ret;
    }

// ============================================================================
// RCwrtRegistryClientSession::DeRegisterWidgetL()
// Deregister the widget
//
// ============================================================================
EXPORT_C bool RWACRegistryClientSession::DeRegisterWidgetL( const QString& appId, bool update )
    {
    CBufFlat* buf = CBufFlat::NewL( 5120 );
    CleanupStack::PushL( buf );

    RBufWriteStream stream( *buf );
    CleanupClosePushL( stream );

    SerializeStringL( stream, appId );
    SerializeBoolL( stream, update );

    CleanupStack::PopAndDestroy( &stream );

    TPtr8 p( buf->Ptr(0) );
    bool ret = SendReceive( EOpCodeRegisterWidget, TIpcArgs( &p ) );

    CleanupStack::PopAndDestroy( buf );

    return ret;
    }

// ============================================================================
// RCwrtRegistryClientSession::getAttributeL()
// Returns the attribute value for the widget
//
// ============================================================================
EXPORT_C QVariant RWACRegistryClientSession::getAttributeL( const QString& appId,
                                                           const QString& attribute,
                                                           const QVariant& defaultValue )
    {
    const TInt maxSize = 5120;

    CBufFlat* sendBuf = CBufFlat::NewL( maxSize );
    CleanupStack::PushL( sendBuf );
    RBufWriteStream stream( *sendBuf );
    CleanupClosePushL( stream );
    SerializeStringL( stream, appId );
    SerializeStringL( stream, attribute );
    SerializeStringL( stream, defaultValue.toString() );
    CleanupStack::PopAndDestroy( &stream );

    CBufFlat* recvBuf = CBufFlat::NewL( maxSize );
    CleanupStack::PushL( recvBuf );
    recvBuf->ExpandL( 0, maxSize );

    TPtr8 sendArg( sendBuf->Ptr(0) );
    TPtr8 recvArg( recvBuf->Ptr(0) );

    User::LeaveIfError(
            SendReceive( EOpCodeGetWebAttribute, TIpcArgs( &sendArg, &recvArg ) )
            );

    // deserialize
    RDesReadStream rstream( recvArg );
    CleanupClosePushL( rstream );
    QString attrValue = DeserializeStringL( rstream );
    CleanupStack::PopAndDestroy( 3, sendBuf ); // rstream, recvBuf, sendBuf

    return ( QVariant( attrValue ) );
    }

// ============================================================================
// RCwrtRegistryClientSession::setAttributeL()
// Sets the attribute value for the widget
//
// ============================================================================
EXPORT_C bool RWACRegistryClientSession::setAttributeL(const QString& appId,
                                                       const QString& attribute,
                                                       const QVariant& value)
    {
    CBufFlat* buf = CBufFlat::NewL( 5120 );
    CleanupStack::PushL( buf );

    RBufWriteStream stream( *buf );
    CleanupClosePushL( stream );

    SerializeStringL( stream, appId );
    SerializeStringL( stream, attribute );
    SerializeStringL( stream, value.toString() );

    CleanupStack::PopAndDestroy( &stream );

    TPtr8 p( buf->Ptr(0) );
    bool ret = SendReceive( EOpCodeSetWebAttribute, TIpcArgs( &p ) );

    CleanupStack::PopAndDestroy( buf );

    return ret;
    }

// ============================================================================
// RCwrtRegistryClientSession::nativeIdToAppIdL()
// Returns the AppId
//
// ============================================================================
EXPORT_C QString RWACRegistryClientSession::nativeIdToAppIdL( const QString& Id )
    {
    const TInt maxSize = 512;
    CBufFlat* buf = CBufFlat::NewL( maxSize );
    CleanupStack::PushL( buf );
    buf->ExpandL( 0, maxSize );
    TPtr8 p( buf->Ptr(0) );

    User::LeaveIfError(
            SendReceive( EOpCodeNativeIdToAppId, TIpcArgs( Id.toInt(), &p ) )
            );

    // deserialize
    RDesReadStream stream( p );
    CleanupClosePushL( stream );

    QString appId = DeserializeStringL( stream );

    CleanupStack::PopAndDestroy( 2, buf ); // stream, buf

    return appId;
    }

// End of File
