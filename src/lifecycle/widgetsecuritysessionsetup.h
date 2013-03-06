#ifndef WIDGETSECURITYSESSIONSETUP_H
#define WIDGETSECURITYSESSIONSETUP_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include "wacWidgetProperties.h"
#include "featuremapping.h"

#include <wacsecsession.h>

using namespace WAC;

typedef QMap<QString, QString> WidgetFeatures;

class WidgetSetupSecuritySession : public WidgetOperation
{
public:
    explicit WidgetSetupSecuritySession( WidgetInformation &widgetInfo );
    ~WidgetSetupSecuritySession();
    
    void execute();
    
    void restore();
    
    void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );
    
    WidgetErrorType finalize();
    
private:
    WidgetErrorType setupSecuritySession();
    WidgetFeatures getFeaturesFromFeatureList();
    void setSecuritySessionString(WAC::SecSession* secureSession);
    bool createSecuritySessionFile();
    void setProcessUidInProps();
    
    WidgetProperties *m_widgetProps;
    
    FeatureMapping m_mapping;
    
    bool m_hasFeatures;
};

#endif //WIDGETSECURITYSESSIONSETUP_H
