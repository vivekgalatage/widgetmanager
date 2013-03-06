#include "widgetstatemachine.h"
#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include <QEvent>
#include <QCoreApplication>

WidgetStateMachine::WidgetStateMachine( const QList<WidgetOperationType>& operationList, WidgetInformation &widgetInfo )
    : m_State( EStateIdle ),
      m_InitialOperations( operationList ),
      m_WidgetInfo( widgetInfo ),
      m_Index( -1 ),
      m_FinalizeIndex( -1 ),
      m_CurrentPercentage ( 0 ),
      m_Canceled( false )
{
}

WidgetStateMachine::~WidgetStateMachine()
{
    LOGIT("WidgetStateMachine::~WidgetStateMachine()");
    if( m_Canceled )
        cancel();
    else
        clear();
}

bool WidgetStateMachine::event( QEvent *event )
{
    bool ret = true;
    if( !m_Canceled )
    {
        EventType e = EEventInvalid;
        if( event )
            e = static_cast<EventType>( event->type() );
        LOGIT("WidgetStateMachine::event() eventtype" << e);
        switch( e )
        {
        case EEventStart:
            initialize();
            //coverity[unterminated_case]
        case EEventNext:
            m_State = EStateRunning;
            nextState();
            break;
        case EEventStop:
            m_Canceled = true;
            m_State = EStateCanceled;
            emit aborted(EErrorUserCancel);
            break;
        case EEventFinalize:
            if( m_FinalizeIndex == -1 )
                m_FinalizeIndex = 0;
            LOGIT("event EEventFinalize");
            finalize();
            break;
        case EEventCompleted:
            m_State = EStateCompleted;
            emit completed();
            break;
        case EEventAborted:
        {
            restore();
            WidgetErrorType errCode = static_cast<WidgetErrorType>( m_WidgetInfo[ EPropertyErrorCode ].toInt() );
            emit aborted( errCode );
            break;
        }
        case EEventNotifyProgress:
            if( m_FinalizeIndex != -1 )
            {
                if( m_OperationData[m_FinalizeIndex-1].m_operation && m_OperationData[m_FinalizeIndex-1].m_operation->isFinalized() )
                    {
                    if( (m_OperationData.size() - (m_FinalizeIndex-1)) > 0 )
                        m_CurrentPercentage += ( 100 - m_CurrentPercentage ) / ( m_OperationData.size() - (m_FinalizeIndex-1) ) ;
                    int index = m_FinalizeIndex < m_OperationData.size() ? m_FinalizeIndex : m_FinalizeIndex - 1 ;
                    LOGIT("PROGRESS " << m_CurrentPercentage );
                    emit progress( m_CurrentPercentage, m_OperationData[ index ].m_type );
                    }
            }
            else
            {    
                m_CurrentPercentage += ( 50 - m_CurrentPercentage ) / ( m_OperationData.size() - m_Index ) ;
                emit progress( m_CurrentPercentage, EOperationPreparing );
            }
            break;
        case EEventInvalid:
            break;
        default:
            ret = false;
        }
    }
    return ret;
}

void WidgetStateMachine::nextState()
{
    ++m_Index;
    
    bool isInteractive = m_WidgetInfo[ EPropertyInteractiveMode ].toBool();
    
    if( m_Index < m_OperationData.size() )
    {
        WidgetOperationData &data = m_OperationData[ m_Index ];
        WidgetOperation *&operation = data.m_operation;

        if( operation == NULL )
            operation = WidgetOperationManager::createOperation( data.m_type, m_WidgetInfo );

        if( operation )
        {
            operation->clearSubOperations();

            if( isInteractive ) 
            {
                operation->setMode( WidgetOperation::EModeInteractive );
                // Now as we are in interactive mode, please connect the interactive signal from operation 
                QObject::connect( operation,
                                  SIGNAL( interactiveRequest( QMap<WidgetPropertyType, QVariant> & )),
                                  this,
                                  SLOT( handleInteractiveRequest( QMap<WidgetPropertyType, QVariant> & )),
                                  Qt::UniqueConnection
                                 );
            }
            else 
                operation->setMode( WidgetOperation::EModeNonInteractive );

            QObject::connect( operation,
                              SIGNAL( completed() ),
                              this,
                              SLOT( handleCompleted() ),
                              Qt::UniqueConnection 
                             );
            
            QObject::connect( operation,
                              SIGNAL( aborted( WidgetErrorType ) ),
                              this,
                              SLOT( handleAborted( WidgetErrorType ) ),
                              Qt::UniqueConnection 
                             );
            
            operation->execute();
            
        }
        else
        {
            m_WidgetInfo[ EPropertyErrorCode ] = EErrorOperationNotRegistered;
            QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventAborted ) ) );
        }
    }
    else
    {
        // Adjust the index position as we reached end of list
        --m_Index;
        
       if( isInteractive )
            serveInteractiveRequests();

       if(m_WidgetInfo[EPropertyContinueInstallation].toBool())
           QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventFinalize ) ) );
       else
           QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>(EEventStop)), Qt::HighEventPriority );
    }
}

void WidgetStateMachine::start()
{
    LOGIT("WidgetStateMachine::State from TRUNK");
    if( m_State == EStateIdle )
    {
        QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>(EEventStart) ) );
        emit started();
    }
}

void WidgetStateMachine::stop()
{
    if( m_State == EStateRunning )
        QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>(EEventStop)), Qt::HighEventPriority );
}

void WidgetStateMachine::serveInteractiveRequests()
{
    QMap<WidgetPropertyType,QVariant> interactiveProperties;

    interactiveProperties[EPropertyContinueInstallation] = false;

    for( int i=0; i < m_OperationData.size(); i++ )
    {
        WidgetOperationData& data = m_OperationData[i];
        if( data.m_operation )
            data.m_operation->interactiveProperties( interactiveProperties );
    }
    
    if( interactiveProperties.size() >  0 ) 
    {
        emit interactiveRequest( interactiveProperties );
        bool continueInstall = interactiveProperties[EPropertyContinueInstallation].toBool();
        if(continueInstall) {
            QList<WidgetPropertyType> props =  interactiveProperties.keys();
            for( int i=0; i<props.size(); i++ )
                m_WidgetInfo[ props[i] ] = interactiveProperties.value( props[i] );
        }
        else {
            m_WidgetInfo[EPropertyContinueInstallation]=false;
        }
    }
}

void WidgetStateMachine::finalize()
{
    if( m_FinalizeIndex < m_OperationData.size() )
    {
        WidgetOperationData &data = m_OperationData[m_FinalizeIndex];
        if( data.m_operation )
        {
            WidgetErrorType err = data.m_operation->complete();
            m_WidgetInfo[ EPropertyErrorCode ] = err; 
            if( err == EErrorNone )
            {
                QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventNotifyProgress ) ) );
                QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventFinalize ) ) );
            }
            else
                QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventAborted ) ) );
        }
        m_FinalizeIndex++;
    }
    else
        QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventCompleted ) ) );
}

void WidgetStateMachine::restore( bool cancel )
{
    int i = m_Index;
    LOGIT("WidgetStateMachine::restore() m_Index " << m_Index << m_OperationData.size() );
    while( i >= 0 )
    {
        WidgetOperationData data = m_OperationData[ i ];
        WidgetOperation *operation = data.m_operation;
        
        if( operation && m_OperationData.indexOf( data ) == i )
        {
            LOGIT( "GOING TO DELETE SOMEBODY" << data.m_type );
            if( cancel )
                operation->cancel();
            else
                operation->restore();
        }
        i--;
    }
    clear();
}

void WidgetStateMachine::cancel()
{
    restore( true );
}

void WidgetStateMachine::initialize()
{
    for( int i=0; i < m_InitialOperations.size(); i++ )
    {
        WidgetOperationData data( m_InitialOperations[i], NULL );
        m_OperationData.append( data );
    }
    m_Index = -1;
    m_CurrentPercentage = 0;
    m_Canceled = false;
}

void WidgetStateMachine::clear()
{
    LOGIT("WidgetStateMachine::clear()" << m_Index << m_OperationData.size());
    while( m_Index >= 0 )
    {
        WidgetOperationData data = m_OperationData[m_Index];

        if( data.m_operation && ( m_OperationData.indexOf( data ) == m_Index ) )
            delete data.m_operation;
        
        m_Index--;
    }
    LOGIT("Deleted everything");
    m_OperationData.clear();
}



void WidgetStateMachine::handleInteractiveRequest( QMap<WidgetPropertyType, QVariant> &properties )
{
    emit interactiveRequest( properties );
}

void WidgetStateMachine::handleCompleted()
{
    m_WidgetInfo[ EPropertyErrorCode ] = EErrorNone;
    
    WidgetOperationData &data = m_OperationData[ m_Index ];

    const QList<WidgetOperationData>& subOperations = data.m_operation->subOperations();
    int subSize = subOperations.size();
    
    if( subSize > 0 )
    {
        for( int i=subSize-1; i>=0; i--)
        {
            WidgetOperationData subData = subOperations[i];
            m_OperationData.insert( m_Index+1, subData );
        }
    }
    QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventNotifyProgress ) ) );
    QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventNext ) ) );
    
}
    
void WidgetStateMachine::handleAborted( WidgetErrorType error )
{
    m_WidgetInfo[ EPropertyErrorCode ] = error;
    QCoreApplication::postEvent( this, new QEvent( static_cast<QEvent::Type>( EEventAborted ) ) );
}
