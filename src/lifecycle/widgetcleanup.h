#ifndef WIDGETCLEANUP_H
#define WIDGETCLEANUP_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

class WidgetCleanup : public WidgetOperation
{
public:
    explicit WidgetCleanup( WidgetInformation &widgetInfo );
    ~WidgetCleanup();
    
    void execute();
    
    void restore();
    
    WidgetErrorType finalize();
    
};
#endif //WIDGETCLEANUP_H
