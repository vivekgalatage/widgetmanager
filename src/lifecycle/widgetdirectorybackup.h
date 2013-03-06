#ifndef WIDGETDIRECTORYBACKUP_H
#define WIDGETDIRECTORYBACKUP_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "widgetinformation.h"

class WidgetDirectoryBackup : public WidgetOperation
{
public:
    explicit WidgetDirectoryBackup( WidgetInformation &widgetInfo );
    ~WidgetDirectoryBackup();
    
    void execute();
    
    void restore();
    
    WidgetErrorType finalize();
    
};

#endif //WIDGETDIRECTORYBACKUP_H
