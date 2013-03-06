#include "widgetdirectorybackup.h"
#include "wacWebAppRegistry.h"
#include "wacwebappinfo.h"
#include "widgetconstants.h"
#include "widgetutilities.h"

#include <QDir>

class WidgetDirectoryBackupWorker 
{
public:
    WidgetDirectoryBackupWorker( WidgetInformation &widgetInfo, const QString &widgetInstalledPath );
    ~WidgetDirectoryBackupWorker();
    
    bool createBackupDirectory();
    
    bool backupWidgetDirectory();
    
    bool backupDataDirectory();
    
    bool restoreWidgetDirectory();
    
    bool restoreDataDirectory();
    
    bool removeWidgetBackupDirectory();
    
private:
    WidgetDirectoryBackupWorker();
    WidgetDirectoryBackupWorker( const WidgetDirectoryBackupWorker & );
    WidgetDirectoryBackupWorker & operator = ( const WidgetDirectoryBackupWorker & );
    
private:
    
    WidgetInformation &m_WidgetInfo;
    
    QString m_WidgetInstalledPath;

};

WidgetDirectoryBackupWorker::WidgetDirectoryBackupWorker( WidgetInformation &widgetInfo, const QString &widgetInstalledPath )
    :m_WidgetInfo( widgetInfo ),
     m_WidgetInstalledPath( widgetInstalledPath )
{
    
}

WidgetDirectoryBackupWorker::~WidgetDirectoryBackupWorker()
{
    
}

bool WidgetDirectoryBackupWorker::createBackupDirectory()
{
    QString newPath = m_WidgetInfo[ EPropertyBackupDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString() + QDir::separator();
        
    if( QFile::exists( newPath ) )
        WidgetUtilities::removeDirectory( newPath );
    
    QDir().mkpath( newPath );    
    return true;
}

bool WidgetDirectoryBackupWorker::backupWidgetDirectory()
{
    if( QFile::exists( m_WidgetInstalledPath ) )
    {
        QString widgetBackupPath = m_WidgetInfo[ EPropertyBackupDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString() + QDir::separator() + WidgetBackupFolderName;
        return QDir().rename( m_WidgetInstalledPath, widgetBackupPath );
    }
    return false;
}

bool WidgetDirectoryBackupWorker::backupDataDirectory()
{    
    QString resourcesDir = m_WidgetInstalledPath.left( m_WidgetInstalledPath.indexOf( WidgetFolder, Qt::CaseInsensitive ) )+ DataFolderName + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
    
    if (!resourcesDir.isNull() && !resourcesDir.isEmpty() && QFile::exists( resourcesDir ) ) 
    {
        QString dataBackupPath = m_WidgetInfo[ EPropertyBackupDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString() + QDir::separator() + DataBackupFolderName;
        return QDir().rename( resourcesDir, dataBackupPath );
    }
    return false;
}

bool WidgetDirectoryBackupWorker::restoreWidgetDirectory()
{
    QString widgetBackupPath  = m_WidgetInfo[ EPropertyBackupDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString() + QDir::separator() + WidgetBackupFolderName;

    if( QFile::exists( widgetBackupPath ) )
        return QDir().rename( widgetBackupPath, m_WidgetInstalledPath );

    return false;
}


bool WidgetDirectoryBackupWorker::restoreDataDirectory()
{    
    QString resourcesDir = m_WidgetInstalledPath.left( m_WidgetInstalledPath.indexOf( WidgetFolder, Qt::CaseInsensitive ) )+ DataFolderName + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
    QString dataBackupPath = m_WidgetInfo[ EPropertyBackupDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString() + QDir::separator() + DataBackupFolderName;
    
    if ( QFile::exists( dataBackupPath ) ) 
        return QDir().rename( dataBackupPath, resourcesDir );
    else
        return false;
}

bool WidgetDirectoryBackupWorker::removeWidgetBackupDirectory()
{
    QString widgetBackupPath = m_WidgetInfo[ EPropertyBackupDirectory ].toString() + QDir::separator() + m_WidgetInfo[ EPropertyWidgetId ].toString();
    
    if( QFile::exists( widgetBackupPath ) )
        QDir().rmdir( widgetBackupPath );
    
    return true;
}

WidgetDirectoryBackup::WidgetDirectoryBackup( WidgetInformation &widgetInfo )
    :WidgetOperation( widgetInfo )
{
    
}

WidgetDirectoryBackup::~WidgetDirectoryBackup()
{
    
}

void WidgetDirectoryBackup::execute()
{

    
    emit completed();
    return;
}

void WidgetDirectoryBackup::restore()
{
    if( isFinalized() )
    {
        QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();
        WebAppInfo info;
        if (WebAppRegistry::instance()->isRegistered(widgetId, info)) 
            if (info.isPresent()) 
            {
               WidgetDirectoryBackupWorker worker( m_WidgetInfo, info.appPath() );
               worker.restoreWidgetDirectory();
               worker.restoreDataDirectory();
               worker.removeWidgetBackupDirectory();
            }
    }
}

WidgetErrorType WidgetDirectoryBackup::finalize()
{
    LOGIT("WidgetDirectoryBackup::finalize()");
    
    QString widgetId = m_WidgetInfo[ EPropertyWidgetId ].toString();
    WebAppInfo info;
    if (WebAppRegistry::instance()->isRegistered(widgetId, info)) 
        if (info.isPresent()) 
        {
            WidgetDirectoryBackupWorker worker( m_WidgetInfo, info.appPath() );
            if( worker.createBackupDirectory() && worker.backupWidgetDirectory() && worker.backupDataDirectory() )
            {
                return EErrorNone;
            }
        }
    return EErrorUpdateGeneral;
}
