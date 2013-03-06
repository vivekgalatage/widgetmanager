#include "widgetutilities.h"

#include <QDir>

bool WidgetUtilities::removeDirectory( const QString &path )
{
    QDir dir ( path );
    if( QFile::exists( path ) )
    {
        QFileInfoList fileInfoList = dir.entryInfoList();
        for( int i =0; i < fileInfoList.size(); i++ )
        {
            if( fileInfoList[i].isDir() )
                WidgetUtilities::removeDirectory( fileInfoList[ i ].filePath() );
            else if( fileInfoList[i].isFile() )
                dir.remove( fileInfoList[ i ].filePath() );
        }
        dir.rmdir( path );
    }
    return true;
}
