#ifndef WIDGETPREPAREINSTALLATIONDIRECTORIES_H
#define WIDGETPREPAREINSTALLATIONDIRECTORIES_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

class WidgetPrepareInstallationDirectories : public WidgetOperation
{
public:
    explicit WidgetPrepareInstallationDirectories( WidgetInformation &widgetInfo );
    ~WidgetPrepareInstallationDirectories();
    
    void execute();
    
    void restore();
    
    WidgetErrorType finalize();
};

#endif //WIDGETPREPAREINSTALLATIONDIRECTORIES_H
