#include "widgetinformationinitializer.h"
#include "widgetconstants.h"

QString getRootDirectory( QString rootDirectory );
bool createDir(const QString& path);

class WidgetInfoInitWorker
{
public:
    WidgetInfoInitWorker( WidgetInformation& widgetInfo );
    ~WidgetInfoInitWorker();
    
    WidgetErrorType validateProcessCapabilities();
    
private:
    QString getRootDirectory( QString rootDirectory );
    
    bool createDir(const QString& path);
    
private:
    WidgetInformation& m_WidgetInfo;
    
};

WidgetErrorType WidgetInformationInitializer::platformInitialization()
{
    m_WidgetInfo [ EPropertyMaxReadBytes ] = 1048576;
    
    WidgetInfoInitWorker* worker = new WidgetInfoInitWorker( m_WidgetInfo );
    WidgetErrorType error = EErrorNone;
    
    if( ( error = worker->validateProcessCapabilities() ) == EErrorNone )
    {
        QString rootDir = DefaultDrive + QDir::separator() + PrivateDirectory + QDir::separator() + InstallerUid ;
        m_WidgetInfo[ EPropertyContentDirectory ] =  rootDir + QDir::separator() + WidgetUnzipFolder;
        m_WidgetInfo[ EPropertyBackupDirectory ] =  rootDir + QDir::separator() + WidgetBackupFolder;
        
        delete worker;
        return EErrorNone;
    }
    delete worker;
    return error;
}

WidgetInfoInitWorker::WidgetInfoInitWorker( WidgetInformation& widgetInfo )
    : m_WidgetInfo( widgetInfo )
{
    
}

WidgetInfoInitWorker::~WidgetInfoInitWorker()
{
    
}

WidgetErrorType WidgetInfoInitWorker::validateProcessCapabilities()
{
    RProcess process;
    
    static _LIT_SECURITY_POLICY_V0(myVidPolicy, VID_DEFAULT);
    TBool error = myVidPolicy().CheckPolicy(process);    

    if (error  == EFalse)
        return EErrorPermissionDenied;
    
    return EErrorNone;
}

