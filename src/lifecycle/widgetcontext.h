#ifndef WIDGETCONTEXT_H
#define WIDGETCONTEXT_H

#include "widgetinformation.h"

#include <QObject>
#include <QMap>
#include <QVariant>

class WidgetContext : public QObject
{
    Q_OBJECT
public:
    explicit WidgetContext( const WidgetInformation &widgetInfo );
    virtual ~WidgetContext();
    
    virtual void initialize( const QMap<QString, QVariant> &configDetails ) = 0;
    
    virtual void start() = 0;
    
    virtual void stop() = 0;

    const WidgetInformation& widgetInfo() const;

signals:
    void progress( int percentageComplete, WidgetOperationType operationType );

    void started();
    void completed();
    void aborted( WidgetErrorType errCode );

    void interactiveRequest( QMap<WidgetPropertyType, QVariant>& );

protected:
    WidgetContext();
    explicit WidgetContext( const WidgetContext& rhs );
    WidgetContext& operator =( const WidgetContext& rhs );
    
protected:
    WidgetInformation m_WidgetInfo;
};

#endif // WIDGETCONTEXT_H
