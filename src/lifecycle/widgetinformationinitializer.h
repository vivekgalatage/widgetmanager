#ifndef WIDGETINFORMATIONINITIALIZER_H
#define WIDGETINFORMATIONINITIALIZER_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "widgettypes.h"

#include <QDir>

const QString SuccessMsg = "Success";
const QString RootdirValidationFailedErrMsg = "Validation and creation of root directory of widget failed";

class WidgetInformationInitializer: public WidgetOperation
{
public:
    explicit WidgetInformationInitializer( WidgetInformation &widgetInfo );
    ~WidgetInformationInitializer();
    
public:
    
    void execute();
    
    void restore();

private:

    WidgetErrorType initialize();
    
    WidgetErrorType platformInitialization();
	WidgetErrorType setDefaultAllowUntrustedWidgetInstallation();
    
};

#endif //WIDGETINFORMATIONINITIALIZER_H
