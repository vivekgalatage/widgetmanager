#ifndef WIDGETPREPAREFORINSTALL_H
#define WIDGETPREPAREFORINSTALL_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "widgetinformation.h"
#include "widgettypes.h"

class WidgetPrepareForInstall : public WidgetOperation
{
public:
    explicit WidgetPrepareForInstall( WidgetInformation &widgetInfo );
    ~WidgetPrepareForInstall();
    
    void execute();
    
    void restore();
    
    void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );
    
    WidgetErrorType finalize();
    
};

#endif //WIDGETPREPAREFORINSTALL_H
