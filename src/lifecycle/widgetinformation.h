#ifndef WIDGETINFORMATION_H
#define WIDGETINFORMATION_H

#include "widgettypes.h"

#include <QMap>
#include <QVariant>

enum WidgetContentType
{
    EContentInvalid = -1,
    EContentWgt
};

class WidgetInformation
{
public:
    WidgetInformation();
    WidgetInformation( const QString& filePath, const WidgetContextType& op );
    
    explicit WidgetInformation( const WidgetInformation& rhs );
    WidgetInformation& operator = ( const WidgetInformation& rhs );
    
    ~WidgetInformation();
    
    WidgetContentType widgetType() const;
    WidgetContextType widgetContext() const;
    
    void setWidgetContext( const WidgetContextType& context );
   
    QVariant& operator[]( WidgetPropertyType type );
    
private:
    WidgetContentType m_ContentType;
    WidgetContextType m_ContextType;
    
    QMap< WidgetPropertyType, QVariant > m_Properties;
};

#endif // WIDGETINFORMATION_H
