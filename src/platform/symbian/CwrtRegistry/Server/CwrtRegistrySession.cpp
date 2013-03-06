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

#include <S32MEM.H>

// User includes
#include "CwrtRegistryData.h"
#include "wacWebAppRegistry.h"
#include "CwrtRegistryServer.h"
#include "CwrtRegistrySession.h"

// ============================================================================
// CCwrtRegistryServerSession::CCwrtRegistryServerSession()
// C++ constructor
//
// ============================================================================
//
CCwrtRegistryServerSession::CCwrtRegistryServerSession()
    {
    }

// ============================================================================
// CCwrtRegistryServerSession::~CCwrtRegistryServerSession()
// Destructor
//
// ============================================================================
//
CCwrtRegistryServerSession::~CCwrtRegistryServerSession()
    {
    CCwrtRegistryServer* server( static_cast<CCwrtRegistryServer*>(
        const_cast<CServer2*>( Server() ) ) );

    if ( server && server->SessionCount() <= 0 )
        {
        // do cleanup here
        }
    }


// ============================================================================
// CCwrtRegistryServerSession::CreateL()
// Called by the CServer framework
//
// ============================================================================
//
void CCwrtRegistryServerSession::CreateL()
    {
    CCwrtRegistryServer* server;

    server = static_cast<CCwrtRegistryServer*>(
        const_cast<CServer2*>( Server() ) );
    User::LeaveIfNull ( server );

    server->AddSession();
    }

// ============================================================================
// CCwrtRegistryServerSession::ConstructL()
// ConstructL method
//
// ============================================================================
//
void CCwrtRegistryServerSession::ConstructL(
    CCwrtRegistryServer& /*aServer*/,
    const RMessage2& /*aMessage*/ )
    {

    }

// ============================================================================
// CCwrtRegistryServerSession::NewL()
//
// ============================================================================
//
CCwrtRegistryServerSession* CCwrtRegistryServerSession::NewL(
    CCwrtRegistryServer& aServer, const RMessage2& aMessage )
    {
    CCwrtRegistryServerSession* self;

    self = new ( ELeave ) CCwrtRegistryServerSession();
    CleanupStack::PushL( self );
    self->ConstructL( aServer, aMessage );
    CleanupStack::Pop( self );
    return self;
    }

// ============================================================================
// CCwrtRegistryServerSession::ServiceL()
// Handles messages from client
//
// ============================================================================
//
void CCwrtRegistryServerSession::ServiceL( const RMessage2& aMessage )
    {
    TBool retBool( EFalse );
    bool ret( false );
    TInt len = 0;
    TInt uid = 0;
    TInt count = 0;
    TInt policyId = 0;
    const TInt function = aMessage.Function();

    // TBD seems like we should trap leaves and return error codes so
    // as not to exit the server
    switch ( function )
        {
        case EOpCodeRegisterWidget:
            ret = OpRegisterWidgetL( aMessage );
            aMessage.Complete( ret );
            break;
        case EOpCodeDeRegisterWidget:
            ret = OpDeRegisterWidgetL( aMessage );
            aMessage.Complete( ret );
            break;
        case EOpCodeGetWebAttribute:
            OpGetWebAttributeL( aMessage );
            aMessage.Complete( KErrNone );
            break;
        case EOpCodeSetWebAttribute:
            ret = OpSetWebAttributeL( aMessage );
            aMessage.Complete( ret );
            break;
        case EOpCodeNativeIdToAppId:
            OpNativeIdToAppIdL( aMessage );
            aMessage.Complete( KErrNone );
            break;
        case EOpCodeWidgetRegistryDisconnect:
            OpDisconnect( aMessage );
            aMessage.Complete( KErrNone );
            break;
        default:
            User::Leave( KErrNotSupported );
            break;
        }
    }

// ============================================================================
// CCwrtRegistryServerSession::ServiceError()
// Handles leaves from ServiceL()
//
// ============================================================================
//
void CCwrtRegistryServerSession::ServiceError( const RMessage2& aMessage,
                                                 TInt& aError )
    {
    const TInt function = aMessage.Function();

    // supply the correct error return message params for each opcode
    switch ( function )
        {
        case EOpCodeRegisterWidget:
            aMessage.Complete( EFalse );
            break;
        case EOpCodeDeRegisterWidget:
            aMessage.Complete( EFalse );
            break;
        case EOpCodeGetWebAttribute:
            aMessage.Complete( aError );
            break;
        case EOpCodeSetWebAttribute:
            aMessage.Complete( EFalse );
            break;
        case EOpCodeNativeIdToAppId:
            aMessage.Complete( 0 );
            break;
        case EOpCodeWidgetRegistryDisconnect:
            aMessage.Complete( aError );
            break;
        default:
            aMessage.Complete( aError );
            break;
        }
    }

// ============================================================================
// CCwrtRegistryServerSession::OpRegisterWidgetL()
// Registers the widget
//
// ============================================================================
//
bool CCwrtRegistryServerSession::OpRegisterWidgetL(
    const RMessage2& aMessage )
    {
    // ARGS: 0 -> descriptor to buf of serialized property values
    TInt l = aMessage.GetDesMaxLength( 0 );
    HBufC8* msgData = HBufC8::NewLC( l );
    TPtr8 readPtr( msgData->Des() );
    aMessage.ReadL( 0, readPtr );
    RDesReadStream stream( *msgData );
    CleanupClosePushL( stream );

    QString appId = DeserializeStringL( stream );
    QString appTitle = DeserializeStringL( stream );
    QString appPath = DeserializeStringL( stream );
    QString iconPath = DeserializeStringL( stream );
    AttributeMap attrMap;
    DeserializeMapL( stream, attrMap );
    QString type = DeserializeStringL( stream );
    int size = DeserializeIntL( stream );
    QString startPath = DeserializeStringL( stream );

    CleanupStack::PopAndDestroy( 2 ); //stream, msgData,

    bool ret = WebAppRegistry::instance()->registerApp( appId, appTitle, appPath, iconPath,
                                                        attrMap, type, size, startPath);
    return ret;
    }

// ============================================================================
// CCwrtRegistryServerSession::OpDeRegisterWidgetL()
// Deregisters the widget
//
// ============================================================================
//
bool CCwrtRegistryServerSession::OpDeRegisterWidgetL( const RMessage2& aMessage )
    {
    TInt l = aMessage.GetDesMaxLength( 0 );
    HBufC8* msgData = HBufC8::NewLC( l );
    TPtr8 readPtr( msgData->Des() );
    aMessage.ReadL( 0, readPtr );
    RDesReadStream stream( *msgData );
    CleanupClosePushL( stream );

    QString appId = DeserializeStringL( stream );
    int update = DeserializeIntL( stream );

    CleanupStack::PopAndDestroy( 2 ); //stream, msgData,

    bool ret = WebAppRegistry::instance()->unregister( appId, (bool)update );
    return ret;
    }

// ============================================================================
// CCwrtRegistryServerSession::OpGetWebAttributeL()
// Returns the attribute value for the widget
//
// ============================================================================
//
TInt CCwrtRegistryServerSession::OpGetWebAttributeL( const RMessage2& aMessage )
    {
    TInt l = aMessage.GetDesMaxLength( 0 );
    HBufC8* msgData = HBufC8::NewLC( l );
    TPtr8 readPtr( msgData->Des() );
    aMessage.ReadL( 0, readPtr );
    RDesReadStream stream( *msgData );
    CleanupClosePushL( stream );
    QString appId = DeserializeStringL( stream );
    QString attribute = DeserializeStringL( stream );
    QString defaultValue = DeserializeStringL( stream );
    CleanupStack::PopAndDestroy( 2 ); //stream, msgData,

    QVariant attrValue =  WebAppRegistry::instance()->getAttribute( appId, attribute, QVariant( defaultValue ) );

    TInt maxLength = aMessage.GetDesMaxLength( 1 );
    CBufFlat* buf = CBufFlat::NewL( maxLength );
    CleanupStack::PushL( buf );
    RBufWriteStream wstream( *buf ); // stream over the buffer
    CleanupClosePushL( wstream );
    SerializeStringL( wstream, attrValue.toString() );
    CleanupStack::PopAndDestroy( &wstream );
    aMessage.WriteL( 1, buf->Ptr(0) );
    CleanupStack::PopAndDestroy( buf ); // buf

    return KErrNone;
    }

// ============================================================================
// CCwrtRegistryServerSession::OpSetWebAttributeL()
// Sets the attribute value for the widget
//
// ============================================================================
//
bool CCwrtRegistryServerSession::OpSetWebAttributeL( const RMessage2& aMessage )
    {
    TInt l = aMessage.GetDesMaxLength( 0 );
    HBufC8* msgData = HBufC8::NewLC( l );
    TPtr8 readPtr( msgData->Des() );
    aMessage.ReadL( 0, readPtr );
    RDesReadStream stream( *msgData );
    CleanupClosePushL( stream );
    QString appId = DeserializeStringL( stream );
    QString attribute = DeserializeStringL( stream );
    QString value = DeserializeStringL( stream );
    CleanupStack::PopAndDestroy( 2 ); //stream, msgData,

    bool ret = WebAppRegistry::instance()->setWebAppAttribute( appId, attribute, QVariant( value ) );

    return ret;
    }

// ============================================================================
// CCwrtRegistryServerSession::OpNativeIdToAppIdL()
// Returns the appId of the widget
//
// ============================================================================
//
TInt CCwrtRegistryServerSession::OpNativeIdToAppIdL( const RMessage2& aMessage )
    {
    TInt IdNum = aMessage.Int0();
    QString id;
    id.setNum(IdNum);

    QString appId = WebAppRegistry::instance()->nativeIdToAppId( id );

    TInt maxLength = aMessage.GetDesMaxLength( 1 );
    CBufFlat* buf = CBufFlat::NewL( maxLength );
    CleanupStack::PushL( buf );
    RBufWriteStream stream( *buf ); // stream over the buffer
    CleanupClosePushL( stream );

    SerializeStringL( stream, appId );

    CleanupStack::PopAndDestroy( &stream );

    aMessage.WriteL( 1, buf->Ptr(0) );

    CleanupStack::PopAndDestroy( buf ); // buf

    return KErrNone;
    }

// ============================================================================
// CCwrtRegistryServerSession::OpDisconnect()
// Disconnects session with server
//
// ============================================================================
//
TInt CCwrtRegistryServerSession::OpDisconnect(
    const RMessage2& /*aMessage*/ )
    {
    TInt ret = KErrNone;
    CCwrtRegistryServer* server;

    server = static_cast<CCwrtRegistryServer*>(
        const_cast<CServer2*>( Server() ) );

    if ( server )
        {
        server->RemoveSession();
        }

    return ret;
    }

// ============================================================================
// CCwrtRegistryServerSession::CwrtRegistryServer()
// Returns server object
//
// ============================================================================
//
CCwrtRegistryServer& CCwrtRegistryServerSession::CwrtRegistryServer()
    {
    return static_cast<CCwrtRegistryServer&>(
        *const_cast<CServer2*>( Server() ) );
    }


// End of File




