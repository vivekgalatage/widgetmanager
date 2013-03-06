#ifndef WIDGETOPERATIONMANAGER_H
#define WIDGETOPERATIONMANAGER_H

#include "widgettypes.h"

#include <QMap>

class WidgetOperation;
class WidgetInformation;

typedef WidgetOperation* (*OperationCreator)( WidgetInformation& widgetInfo );

class WidgetOperationManager
{
public:
    static WidgetOperationManager& instance();
    
    static WidgetOperation* createOperation( const WidgetOperationType& type, WidgetInformation& widgetInfo );
    
    bool registerOperation( const WidgetOperationType& type, const OperationCreator& operationCreator );
    
    WidgetOperation* newOperation( const WidgetOperationType& type, WidgetInformation& widgetInfo );

private:
    WidgetOperationManager();
    explicit WidgetOperationManager( const WidgetOperationManager& );
    WidgetOperationManager& operator=( const WidgetOperationManager& );
    
private:
    QMap<WidgetOperationType, OperationCreator> operationRegistry;
};

template<typename T>
WidgetOperation* createOperation( WidgetInformation& widgetInfo )
{
    return new T( widgetInfo );
}

#define REGISTER_OPERATION( Type, Operation ) \
    static bool reg##Type = WidgetOperationManager::instance().registerOperation( Type, createOperation<Operation> );

#endif // WIDGETOPERATIONMANAGER_H
