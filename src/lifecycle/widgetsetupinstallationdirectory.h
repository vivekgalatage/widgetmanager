#ifndef WIDGETSETUPINSTALLATIONDIRECTORY_H
#define WIDGETSETUPINSTALLATIONDIRECTORY_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

class WidgetSetupInstallationDirectory : public WidgetOperation
{
public:
    explicit WidgetSetupInstallationDirectory( WidgetInformation &widgetInfo );
    ~WidgetSetupInstallationDirectory();
    
    void execute();
    
    void restore();
    
    WidgetErrorType finalize();
    
};

#endif // WIDGETSETUPINSTALLATIONDIRECTORY_H
