#ifndef WIDGETREGISTRATION_H
#define WIDGETREGISTRATION_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

class WidgetRegistration : public WidgetOperation
{
public:
    explicit WidgetRegistration( WidgetInformation &widgetInfo );
    ~WidgetRegistration();
    
    void execute();
    
    void restore();
    
    WidgetErrorType finalize();
    
    void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );
};

#endif // WIDGETREGISTRATION_H
