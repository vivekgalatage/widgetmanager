#include "widgetcontext.h"

WidgetContext::WidgetContext( const WidgetInformation& widgetInfo )
    : m_WidgetInfo( widgetInfo )
{
}

WidgetContext::~WidgetContext()
{
    
}

const WidgetInformation& WidgetContext::widgetInfo() const
{
    return m_WidgetInfo;
}
