#ifndef WIDGETFINALIZEINSTALLATION_H
#define WIDGETFINALIZEINSTALLATION_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

class WidgetFinalizeInstallation : public WidgetOperation
{
public:
    explicit WidgetFinalizeInstallation( WidgetInformation &widgetInfo );
    ~WidgetFinalizeInstallation();
    
    void execute();
    
    void restore();
    
};

#endif // WIDGETFINALIZEINSTALLATION_H
