#include "widgetoperationmanager.h"

WidgetOperationManager::WidgetOperationManager()
{
}

WidgetOperationManager& WidgetOperationManager::instance()
{
    static WidgetOperationManager inst;
    return inst;
}

WidgetOperation* WidgetOperationManager::createOperation( const WidgetOperationType& type, WidgetInformation& widgetInfo )
{
    return instance().newOperation( type, widgetInfo );
}

bool WidgetOperationManager::registerOperation( const WidgetOperationType& type, const OperationCreator& operationCreator )
{
    operationRegistry[ type ] = operationCreator;
    return true;
}

WidgetOperation* WidgetOperationManager::newOperation( const WidgetOperationType& type, WidgetInformation& widgetInfo )
{
    WidgetOperation* operation = NULL;
    OperationCreator fPtr = operationRegistry.value( type, NULL );
    if( fPtr )
    {
        operation = (*fPtr)( widgetInfo );
    }
    return operation;
}
