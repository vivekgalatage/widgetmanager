#include "widgetfinalizeinstallation.h"
#include "widgetsetupinstallationdirectory.h"
#include "widgetregistration.h"
#include "widgetcleanup.h"
#include "widgethomescreenintegration.h"
#include "wacWebAppRegistry.h"

#include <WidgetRegistryClient.h>

REGISTER_OPERATION( EOperationLegacyFinalizeInstallation, WidgetFinalizeInstallation )

WidgetFinalizeInstallation::WidgetFinalizeInstallation( WidgetInformation &widgetInfo )
    : WidgetOperation ( widgetInfo )
{
    
}

WidgetFinalizeInstallation::~WidgetFinalizeInstallation()
{
    
}

void WidgetFinalizeInstallation::execute()
{
    QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();
        
    if( (widgetId.isEmpty()) )
    {
        emit aborted( EErrorUpdateGeneral );
        return;
    }
    
    if (WebAppRegistry::instance()->isRegistered(widgetId)) 
    {
        TInt64 isInMiniview;        
        TBuf<120> bundleId(widgetId.utf16());
        TUid aUid;
    
        RWidgetRegistryClientSession clientReg;
        if( (clientReg.Connect() ) != KErrNone )
        {
            emit aborted( EErrorUpdateGeneral );
            return;
        }
        
        aUid.iUid = clientReg.GetWidgetUidL(bundleId);
        isInMiniview = clientReg.IsWidgetInMiniView(aUid);
    
        if (isInMiniview) 
            addSuboperation( new WidgetHomeScreenIntegration( m_WidgetInfo, aUid ) );
    }
    
    addSuboperation( new WidgetSetupInstallationDirectory( m_WidgetInfo ) );
    
    addSuboperation( new WidgetRegistration( m_WidgetInfo ) );
    
    addSuboperation( new WidgetCleanup( m_WidgetInfo ) );
    
    emit completed();
}

void WidgetFinalizeInstallation::restore()
{
    
}

 
