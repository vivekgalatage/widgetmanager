#include "widgetcontextcreator.h"
#include "widgetinformation.h"

WidgetContextCreator::WidgetContextCreator()
{
    
}

WidgetContextCreator& WidgetContextCreator::instance()
{
    static WidgetContextCreator inst;
    return inst;
}

WidgetContext* WidgetContextCreator::createContext( const WidgetInformation &widgetInfo )
{
    return instance().newContext( widgetInfo );
}

bool WidgetContextCreator::registerContext( const WidgetContextType& contextType, const ContextCreator& creator )
{
    contextRegistry[ contextType ] = creator;
    return true;
}

WidgetContext* WidgetContextCreator::newContext( const WidgetInformation &widgetInfo )
{
    WidgetContext* context = NULL;
    ContextCreator fPtr = contextRegistry.value( widgetInfo.widgetContext(), NULL );
    if( fPtr )
        context = (*fPtr)( widgetInfo );
    return context;
}
