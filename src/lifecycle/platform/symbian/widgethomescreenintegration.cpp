#include "widgethomescreenintegration.h"
#include "wacWidgetProperties.h"

#include <apgcli.h>
#include <apacmdln.h> 
#include <s32mem.h>

WidgetHomeScreenIntegration::WidgetHomeScreenIntegration( WidgetInformation &widgetInfo , TUid& aUid )
    : WidgetOperation( widgetInfo ),
      m_uid( aUid )
{
    
}

WidgetHomeScreenIntegration::~WidgetHomeScreenIntegration()
{
    
}

void WidgetHomeScreenIntegration::execute()
{
    emit completed();
    return;
}

void WidgetHomeScreenIntegration::restore()
{
    
}

WidgetErrorType WidgetHomeScreenIntegration::finalize()
{
    RApaLsSession apparcSession;
    if( (apparcSession.Connect() ) != KErrNone )
    {
        return EErrorUpdateGeneral;
    }
    
    const TInt size( 3*sizeof( TUint32 ) );
    
    CApaCommandLine* cmd( CApaCommandLine::NewLC() );
    HBufC8* opaque( HBufC8::NewL( size ) );
    
    RDesWriteStream stream;
    TPtr8 des( opaque->Des() );
    stream.Open( des );
    
    // Generate the command.
    stream.WriteUint32L( m_uid.iUid );
    stream.WriteUint32L( 0 );
    stream.WriteInt32L( 8 );
    stream.Close();
    
    // Generate command.
    cmd->SetCommandL( EApaCommandBackgroundAndWithoutViews );
    cmd->SetOpaqueDataL( *opaque );
    delete opaque;
    
    _LIT( KLauncherApp, "wgtwidgetlauncher.exe" );
    cmd->SetExecutableNameL( KLauncherApp );
    if( (apparcSession.StartApp( *cmd ) ) != KErrNone )
        return EErrorUpdateGeneral; 
    
    delete cmd;
    apparcSession.Close();
    return EErrorNone;
}
