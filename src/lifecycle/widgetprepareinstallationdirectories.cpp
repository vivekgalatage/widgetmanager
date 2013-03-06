#include "widgetprepareinstallationdirectories.h"
#include "widgetconstants.h"

#include <QDir>

WidgetPrepareInstallationDirectories::WidgetPrepareInstallationDirectories( WidgetInformation &widgetInfo )
    : WidgetOperation( widgetInfo )
{
    
}

WidgetPrepareInstallationDirectories::~WidgetPrepareInstallationDirectories()
{
    
}

void WidgetPrepareInstallationDirectories::execute()
{
    emit completed();
    return;
}

void WidgetPrepareInstallationDirectories::restore()
{
    //add code here to cleanup operations done in execute
    
    //cleanup operations done in finalize
    if( isFinalized() )
    {
        QString rootDir = m_WidgetInfo[ EPropertyInstallDirectory ].toString();
        QString widgetInstallPath = rootDir + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
        if( QFile::exists(widgetInstallPath) )
        {
            QDir().rmdir( widgetInstallPath );
        }
        QString widgetSettingsPath = rootDir.left( rootDir.indexOf( WidgetFolder, Qt::CaseInsensitive ) )+ DataFolderName + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
                
        if( QFile::exists(widgetSettingsPath) )
        {
            QDir().rmdir( widgetSettingsPath );
        }
    }
}

WidgetErrorType WidgetPrepareInstallationDirectories::finalize()
{
    LOGIT("WidgetPrepareInstallationDirectories::finalize()");
    QString rootDir = m_WidgetInfo[ EPropertyInstallDirectory ].toString();
            
    QString widgetInstallPath = rootDir + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
    if( !( QFile::exists(widgetInstallPath) ) )
    {
        QDir path(widgetInstallPath);
        if( !path.mkpath( widgetInstallPath ) )
            return EErrorUpdateGeneral;
    }
    QString widgetSettingsPath = rootDir.left( rootDir.indexOf( WidgetFolder, Qt::CaseInsensitive ) )+ DataFolderName + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
            
    if( !( QFile::exists(widgetSettingsPath) ) )
    {
        QDir path(widgetSettingsPath);
        if( !path.mkpath( widgetSettingsPath ) )
            return EErrorUpdateGeneral;
    }
    return EErrorNone;
}
