#ifndef WIDGETCONTEXTCREATOR_H
#define WIDGETCONTEXTCREATOR_H

#include "widgettypes.h"

class WidgetInformation;
class WidgetContext;

typedef WidgetContext * (*ContextCreator)( const WidgetInformation &widgetInfo );

class WidgetContextCreator 
{
public:
    static WidgetContextCreator& instance();
    
    static WidgetContext* createContext( const WidgetInformation &widgetInfo );

    bool registerContext( const WidgetContextType& contextType, const ContextCreator& creator );

    WidgetContext* newContext( const WidgetInformation &widgetInfo );

private:
    WidgetContextCreator();
    explicit WidgetContextCreator( const WidgetContextCreator & );
    WidgetContextCreator & operator=( const WidgetContextCreator & );
    
private:
    QMap<WidgetContextType, ContextCreator> contextRegistry;
};

template<typename T>
WidgetContext* createContext( const WidgetInformation& widgetInfo )
{
    return new T( widgetInfo );
}

#define REGISTER_CONTEXT( Type, Context ) \
    static bool reg##Type = WidgetContextCreator::instance().registerContext( Type, createContext<Context> );

#endif // WIDGETCONTEXTCREATOR_H
