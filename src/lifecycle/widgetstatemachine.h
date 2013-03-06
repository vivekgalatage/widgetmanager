#ifndef WIDGETSTATEMACHINE_H
#define WIDGETSTATEMACHINE_H

#include "widgettypes.h"
#include "widgetoperation.h"

#include <QObject>
#include <QEvent>

#include <QList>
#include <QLinkedList>
#include <QSharedPointer>

class WidgetContext;
class WidgetInformation;
class WidgetState;

class WidgetStateMachine : public QObject
{
    Q_OBJECT
public:
    enum EventType
    {
        EEventInvalid = -1,
        EEventStart = QEvent::User,
        EEventStop,
        EEventNext,
        EEventNotifyProgress,
        EEventFinalize,
        EEventCompleted,
        EEventAborted
    };

    enum State
    {
        EStateIdle,
        EStateRunning,
        EStateCanceled,
        EStateCompleted
    };

    WidgetStateMachine( const QList<WidgetOperationType>& operationList, WidgetInformation &widgetInfo );
    ~WidgetStateMachine();

    void start();

    void stop();
    
    void restore( bool cancel = false );

    bool event( QEvent *event );

public slots:
    void nextState();

    void handleInteractiveRequest(QMap<WidgetPropertyType, QVariant>& );
    
    void handleCompleted();
    
    void handleAborted( WidgetErrorType );

signals:
    void started();
    void progress( int percentCompleted, WidgetOperationType operationType );
    void completed();
    void aborted( WidgetErrorType errCode );

    void interactiveRequest( QMap<WidgetPropertyType, QVariant>& );

private:
    WidgetStateMachine();
    explicit WidgetStateMachine( const WidgetStateMachine& );
    WidgetStateMachine& operator=( const WidgetStateMachine& );

private:
    void cancel();
    void initialize();
    void clear();
    
    void serveInteractiveRequests();
    
    void finalize();

private:
    State m_State;
    QList<WidgetOperationType> m_InitialOperations;
    QList<WidgetOperationData> m_OperationData;
    WidgetInformation &m_WidgetInfo;

    int m_Index;
    int m_FinalizeIndex;
    int m_CurrentPercentage;

    bool m_Canceled;
};

#endif // WIDGETSTATEMACHINE_H
