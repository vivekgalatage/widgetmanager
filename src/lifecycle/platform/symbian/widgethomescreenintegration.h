#ifndef WIDGETHOMESCREENINTEGRATION_H
#define WIDGETHOMESCREENINTEGRATION_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "widgetinformation.h"
#include "widgettypes.h"

class WidgetHomeScreenIntegration : public WidgetOperation
{
public:
    WidgetHomeScreenIntegration( WidgetInformation &widgetInfo, TUid& aUid );
    ~WidgetHomeScreenIntegration();
    
    void execute();
    
    void restore();
    
    WidgetErrorType finalize();
    
private:
    TUid m_uid;
    
};

#endif //WIDGETHOMESCREENINTEGRATION_H
