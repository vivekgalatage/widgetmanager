#include "widgetinformation.h"

#include <QString>
#include <QFile>
#include <QFileInfo>

const QString WidgetExtensionWGT = ".wgt";
const char WidgetPackageMagicNumber[] = {0x50, 0x4B, 0x03, 0x04};

WidgetInformation::WidgetInformation()
    : m_ContentType( EContentInvalid ),
      m_ContextType( EContextInvalid )
{
}

WidgetInformation::WidgetInformation( const QString& filePath, const WidgetContextType& op )
    : m_ContentType( EContentInvalid ),
      m_ContextType( op )
{
    // Set the property filePath
    m_Properties[ EPropertyFilePath ] = filePath;
    
    if( QFileInfo( filePath ).isDir() )
    {
        //TODO: Need to check the required feature.
        // if required we need to implement SuperWidget::getWidgetType(const QString& path)
    }
    else if ( filePath.endsWith( WidgetExtensionWGT, Qt::CaseInsensitive ) )
    {
        QFile file( filePath );
        file.open( QIODevice::ReadOnly );

        if ( 0 == qstrncmp( WidgetPackageMagicNumber, file.read(4).data(), 4 ) )
            m_ContentType = EContentWgt;
        
        file.close();
    }
    else
    {
        m_ContentType = EContentInvalid;
    }
}

WidgetInformation::WidgetInformation( const WidgetInformation& rhs )
{
    *this = rhs;
}

WidgetInformation& WidgetInformation::operator = ( const WidgetInformation& rhs )
{
    // Prevent self assignment
    if( this != &rhs )
    {
        this->m_ContentType = rhs.m_ContentType;
        this->m_ContextType = rhs.m_ContextType;
        this->m_Properties  = rhs.m_Properties;
    }
    return *this;
}

WidgetInformation::~WidgetInformation()
{
}

WidgetContentType WidgetInformation::widgetType() const
{
    return m_ContentType;
}

WidgetContextType WidgetInformation::widgetContext() const
{
    return m_ContextType;
}

void WidgetInformation::setWidgetContext( const WidgetContextType& context )
{
    m_ContextType = context;
}

QVariant& WidgetInformation::operator []( WidgetPropertyType propType )
{
    return m_Properties[ propType ];
}
