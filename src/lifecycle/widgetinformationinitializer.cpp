#include "widgetinformationinitializer.h"

REGISTER_OPERATION( EOperationInitialize, WidgetInformationInitializer )

WidgetInformationInitializer::WidgetInformationInitializer( WidgetInformation &widgetInfo )
    : WidgetOperation( widgetInfo )
{
}

WidgetInformationInitializer::~WidgetInformationInitializer()
{
}

void WidgetInformationInitializer::execute()
{
    if( m_WidgetInfo.widgetType() == EContentInvalid )
    {
        emit aborted( EErrorInvalidWidget );
        return;
    }
    
    WidgetErrorType err;
    
    if( ( err = initialize() ) != EErrorNone )
    {
        emit aborted( err );
        return;
    }
    if( ( err = setDefaultAllowUntrustedWidgetInstallation() ) != EErrorNone )
    {
        emit aborted( err );
        return;
    }
    
    if( ( err = platformInitialization() ) != EErrorNone )
    {
        emit aborted( err );
        return;
    }
    
    emit completed();
    return;    
}

WidgetErrorType WidgetInformationInitializer::setDefaultAllowUntrustedWidgetInstallation()
{
#ifdef BLOCK_UNSIGNED_WIDGETS
    m_WidgetInfo[EPropertyAllowUntrustedWidget]=false;
#else
    //By default allow unsigned widget installation
    if( ! m_WidgetInfo[EPropertyAllowUntrustedWidget].isValid() )
        m_WidgetInfo[EPropertyAllowUntrustedWidget]=true;
#endif
    
    if( !m_WidgetInfo[EPropertyContinueInstallation].isValid() )
        m_WidgetInfo[EPropertyContinueInstallation] = true;
    
    
    return EErrorNone;
}

WidgetErrorType WidgetInformationInitializer::initialize()
{
    //by default widget is not overwritten
    if( !m_WidgetInfo[ EPropertyAllowOverwrite ].isValid() )
    {
        switch( m_Mode )
        {
        case EModeInteractive:
            m_WidgetInfo[ EPropertyAllowOverwrite ] = false;        
            break;
        case EModeNonInteractive:
            m_WidgetInfo[ EPropertyAllowOverwrite ] = true;
            break;
        default:
            m_WidgetInfo[ EPropertyAllowOverwrite ] = false;
            break;
        }
    }
    
    //by default we do not know if widget already exists, so assume it does not exist
    if( !m_WidgetInfo[ EPropertyAlreadyExists ].isValid() )
        m_WidgetInfo[ EPropertyAlreadyExists ] = false;
    
    return EErrorNone;
}

void WidgetInformationInitializer::restore()
{
}
