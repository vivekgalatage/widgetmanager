#ifndef WIDGETINSTALLCONTEXT_H
#define WIDGETINSTALLCONTEXT_H

#include "widgettypes.h"

#include "widgetcontext.h"
#include "widgetcontextcreator.h"

class WidgetOperation;
class WidgetStateMachine;

class WidgetInstallContext: public WidgetContext
{
    Q_OBJECT
public:
    explicit WidgetInstallContext( const WidgetInformation& widgetInfo );
    virtual ~WidgetInstallContext();
    
    void initialize( const QMap< QString, QVariant > &configDetails );
    
    void start();
    void stop();


public slots:
    void handleProgress( int percentageComplete, WidgetOperationType operationType );

    void handleStarted();
    void handleCompleted();
    void handleAborted( WidgetErrorType errCode );

    void handleInteractiveRequest( QMap<WidgetPropertyType, QVariant>& );


private:
    WidgetStateMachine *m_StateMachine;
};

REGISTER_CONTEXT( EContextInstall, WidgetInstallContext )

#endif // WIDGETINSTALLCONTEXT_H
