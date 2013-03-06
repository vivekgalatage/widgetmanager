// enable user specified configuration properties
#define ENABLE_CONFIG_PROPERTIES
#include "widgetinstallcontext.h"
#include "widgetinformation.h"

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "widgetstatemachine.h"

#include <QList>

WidgetInstallContext::WidgetInstallContext( const WidgetInformation& widgetInfo )
    : WidgetContext( widgetInfo ),
      m_StateMachine( NULL )
{
    QList<WidgetOperationType> operationsList;

    operationsList << EOperationInitialize \
                   << EOperationLegacyContentExtraction \
                   << EOperationLegacyValidateDigitalSignature \
                   << EOperationLegacyParseManifest \
                   << EOperationLegacyPrepareForInstall \
				   << EOperationLegacySetupSecuritySession \
                   << EOperationLegacyFinalizeInstallation;

    m_StateMachine = new WidgetStateMachine( operationsList, m_WidgetInfo );

    QObject::connect( m_StateMachine,
                      SIGNAL( progress(int, WidgetOperationType ) ),
                      this,
                      SLOT( handleProgress(int, WidgetOperationType ) )
                    );

    QObject::connect( m_StateMachine,
                      SIGNAL( started() ),
                      this,
                      SLOT( handleStarted() )
                    );
    QObject::connect( m_StateMachine,
                      SIGNAL( completed() ),
                      this,
                      SLOT( handleCompleted() )
                    );
    QObject::connect( m_StateMachine,
                      SIGNAL( aborted( WidgetErrorType ) ),
                      this,
                      SLOT( handleAborted( WidgetErrorType ) )
                    );

    QObject::connect( m_StateMachine,
                      SIGNAL( interactiveRequest( QMap<WidgetPropertyType, QVariant> & ) ),
                      this,
                      SLOT( handleInteractiveRequest( QMap<WidgetPropertyType, QVariant> & ) )
                    );

}

WidgetInstallContext::~WidgetInstallContext()
{
    delete m_StateMachine;
}

void WidgetInstallContext::initialize( const QMap< QString, QVariant > &configDetails )
{
#ifdef ENABLE_CONFIG_PROPERTIES
    const QList<QString> &props = configDetails.keys(); 
    for( int i=0; i<props.size(); i++ )
    {
        WidgetPropertyType type = configProperties.value( props[i], EPropertyInvalid );
        
        if( type != EPropertyInvalid )
        {
            m_WidgetInfo[ type ] = configDetails.value( props[i] );
        }
    }
#endif //ENABLE_CONFIG_PROPERTIES
    
    
    if( !m_WidgetInfo[ EPropertyInteractiveMode ].isValid() )
        m_WidgetInfo[ EPropertyInteractiveMode ] = false;
}

void WidgetInstallContext::start()
{
    if( m_StateMachine )
        m_StateMachine->start();
}

void WidgetInstallContext::stop()
{
    if( m_StateMachine )
        m_StateMachine->stop();
}

void WidgetInstallContext::handleProgress( int percent, WidgetOperationType operationType )
{
    emit progress( percent, operationType );
}

void WidgetInstallContext::handleStarted()
{
    emit started();
}

void WidgetInstallContext::handleCompleted()
{
    emit completed();
}

void WidgetInstallContext::handleAborted( WidgetErrorType errCode )
{
    emit aborted( errCode );
}

void WidgetInstallContext::handleInteractiveRequest( QMap<WidgetPropertyType, QVariant> &properties )
{
    emit interactiveRequest( properties );
}
