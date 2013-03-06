#include "widgetprepareforinstall.h"
#include "wacWidgetProperties.h"
#include "wacWebAppRegistry.h"
#include "wacwebappinfo.h"
#include "symbianutils.h"
#include "widgetconstants.h"
#include "widgetdirectorybackup.h"
#include "widgetprepareinstallationdirectories.h"

#include <QDir>

REGISTER_OPERATION( EOperationLegacyPrepareForInstall, WidgetPrepareForInstall )

WidgetPrepareForInstall::WidgetPrepareForInstall( WidgetInformation &widgetInfo )
    : WidgetOperation( widgetInfo )
{
    
}

WidgetPrepareForInstall::~WidgetPrepareForInstall()
{
    
}

void WidgetPrepareForInstall::execute()
{
    QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();
    
    if( (widgetId.isEmpty()) )
    {
        emit aborted( EErrorUpdateGeneral );
        return;
    }
    
    if (WebAppRegistry::instance()->isRegistered(widgetId)) 
    {
        
        //set the property which tells widget is already installed
        m_WidgetInfo.setWidgetContext( EContextUpdate );
        
        addSuboperation( new WidgetDirectoryBackup( m_WidgetInfo ) );
        
    }
    //create installation directories
    addSuboperation( new WidgetPrepareInstallationDirectories( m_WidgetInfo) );
    
    emit completed();
    return;
}

void WidgetPrepareForInstall::restore()
{
    
}

void WidgetPrepareForInstall::interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties )
{
    if( m_WidgetInfo.widgetContext() == EContextUpdate )
        properties[ EPropertyAllowOverwrite ] = m_WidgetInfo[ EPropertyAllowOverwrite ];
}

WidgetErrorType WidgetPrepareForInstall::finalize()
{
    
    if ( ( m_WidgetInfo[ EPropertyAllowOverwrite ].toBool() ) == false )
       return EErrorUserCancel;
    
    return EErrorNone;
}
