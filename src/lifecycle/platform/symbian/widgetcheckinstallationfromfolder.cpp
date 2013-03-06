#include "widgetcheckinstallationfromfolder.h"
#include "widgetconstants.h"

#include <QFileInfo> 

WidgetCheckInstallationFromFolder::WidgetCheckInstallationFromFolder( WidgetInformation& widgetInfo )
    :WidgetOperation( widgetInfo )
{
}

WidgetCheckInstallationFromFolder::~WidgetCheckInstallationFromFolder()
{
}

void WidgetCheckInstallationFromFolder::execute()
{
    QString filePath = m_WidgetInfo[ EPropertyFilePath ].toString();
    
    if ( !( QFileInfo(filePath).isFile() ) ) 
    {
        RProcess me;
        TUidType uidType(me.Type());
        TUint32 uid3 = uidType[2].iUid;

        if (uid3 != BackupRestoreUid && uid3 != MediaManagerUid) 
        {
            emit aborted( EErrorPermissionDenied );
            return;
        }
    }

    emit completed();
    return; 
}

void WidgetCheckInstallationFromFolder::restore()
{
}
