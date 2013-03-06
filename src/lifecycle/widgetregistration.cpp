#include "widgetregistration.h"

#include "wacWidgetProperties.h"
#include "wacWebAppRegistry.h"

#include <QStringList>
#include <QDir>

const QString WidgetVersion = "widget/version";
const QString WidgetAuthor = "widget/author";

WidgetRegistration::WidgetRegistration( WidgetInformation &widgetInfo )
    : WidgetOperation( widgetInfo )
{
    
}

WidgetRegistration::~WidgetRegistration()
{
    
}

void WidgetRegistration::execute()
{
    QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();     
       
    WebAppInfo info;
    if( m_WidgetInfo.widgetContext() == EContextUpdate && WebAppRegistry::instance()->isRegistered( widgetId, info ) )
    {
       AttributeMap attrMap = info.attributes();
       if( attrMap.size() > 0 )
           m_WidgetInfo[ EPropertyInstalledVersion ] = attrMap.value( WidgetVersion ).toString();
    }
    emit completed();
}

void WidgetRegistration::restore()
{
    LOGIT("WidgetRegistration::restore()");
    if( isFinalized() )
    {
        QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();     
            
        if( WebAppRegistry::instance()->isRegistered( widgetId ) )
            WebAppRegistry::instance()->unregister( widgetId );
    }
}

WidgetErrorType WidgetRegistration::finalize()
{
    LOGIT("WidgetRegistration::finalize()");
        
    QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();     
    
    if( m_WidgetInfo.widgetContext() == EContextUpdate && WebAppRegistry::instance()->isRegistered( widgetId ) )
        if( ! ( WebAppRegistry::instance()->unregister( widgetId ) ) )
        {
            return EErrorRegistrationGeneral;
        }
    
    QVariant v = m_WidgetInfo[ EPropertyWidgetProperties ];
    WidgetProperties *widgetProps = (WidgetProperties *)v.value<void *>();                                               

    if( widgetProps )
    {
        QString startFilePath = m_WidgetInfo[ EPropertyInstallDirectory].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString() + QDir::separator() + widgetProps->startFile();
        
        QSet<QString> languages = widgetProps->languages();
        QString version = ( widgetProps->plistValue( WidgetVersion ) ).toString();
        QString author = ( widgetProps->plistValue( WidgetAuthor ) ).toString();

        bool status = WebAppRegistry::instance()->registerApp( widgetProps->id(),
                                                               widgetProps->title(),
                                                               widgetProps->installPath(),
                                                               widgetProps->iconPath(),
                                                               widgetProps->plist(),
                                                               widgetProps->type(),
                                                               widgetProps->size(),
                                                               startFilePath,
                                                               widgetProps->hideIcon(),
                                                               languages,
                                                               version,
                                                               author );
        if( !status )
            return EErrorRegistrationGeneral;
        
        QStringList certificateAKIs = m_WidgetInfo[ EPropertyCertificateAKIs ].toStringList();
        QString commaSeparatedAKIs;
        if( !certificateAKIs.isEmpty() )
            commaSeparatedAKIs = certificateAKIs.join(",");

        if( ! commaSeparatedAKIs.isEmpty() )
            WebAppRegistry::instance()->setCertificateAki( widgetProps->id(), commaSeparatedAKIs );

        return EErrorNone;
    }
    return EErrorRegistrationGeneral;
}

void WidgetRegistration::interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties )
{
    if( m_WidgetInfo.widgetContext() == EContextUpdate )
        properties[ EPropertyInstalledVersion ] = m_WidgetInfo[ EPropertyInstalledVersion ];
}
