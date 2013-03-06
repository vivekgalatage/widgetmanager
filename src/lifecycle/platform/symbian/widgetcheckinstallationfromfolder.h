#ifndef WIDGETCHECKINSTALLATIONFROMFOLDER_H
#define WIDGETCHECKINSTALLATIONFROMFOLDER_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "widgettypes.h"

class WidgetCheckInstallationFromFolder : public WidgetOperation
{
public:

    explicit WidgetCheckInstallationFromFolder( WidgetInformation& widgetInfo );
    ~WidgetCheckInstallationFromFolder();

    void execute();

    void restore();

};

#endif //WIDGETCHECKINSTALLATIONFROMFOLDER_H
