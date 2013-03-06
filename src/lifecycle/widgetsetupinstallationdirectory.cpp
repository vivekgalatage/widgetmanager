#include "widgetsetupinstallationdirectory.h"
#include "widgetutilities.h"

#include <QDir>

WidgetSetupInstallationDirectory::WidgetSetupInstallationDirectory( WidgetInformation &widgetInfo )
    : WidgetOperation( widgetInfo )
{
    
}

WidgetSetupInstallationDirectory::~WidgetSetupInstallationDirectory()
{
    LOGIT("WidgetSetupInstallationDirectory::~WidgetSetupInstallationDirectory()");
}

void WidgetSetupInstallationDirectory::execute()
{
    emit completed();
    return;
}

void WidgetSetupInstallationDirectory::restore()
{
    if( isFinalized() )
    {
        QString widgetContentExtractedPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();
        QString widgetInstallationPath = m_WidgetInfo[ EPropertyInstallDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();

        if (widgetContentExtractedPath != widgetInstallationPath)
        {
            if ( QFile::exists(widgetContentExtractedPath) ) 
            {
            WidgetUtilities::removeDirectory(widgetContentExtractedPath);
            }
            bool status = QDir().rename( widgetInstallationPath, widgetContentExtractedPath );
        }
    }
}

WidgetErrorType WidgetSetupInstallationDirectory::finalize()
{
    QString widgetContentExtractedPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();
    QString widgetInstallationPath = m_WidgetInfo[ EPropertyInstallDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();

    if (widgetContentExtractedPath != widgetInstallationPath)
    {
        if ( QFile::exists(widgetInstallationPath) ) 
        {
            if ( !( WidgetUtilities::removeDirectory(widgetInstallationPath) ) )
                return EErrorGeneral; 
        }
        if( !( QDir().rename( widgetContentExtractedPath, widgetInstallationPath ) ) )
            return EErrorGeneral; 
    }
    return EErrorNone;
}
