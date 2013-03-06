#include "widgetcontentextraction.h"
#include "widgetutilities.h"

#include <QDir>

REGISTER_OPERATION( EOperationLegacyContentExtraction, WidgetContentExtraction )

int WidgetContentExtraction::operationcount = 1;

WidgetContentExtraction::WidgetContentExtraction( WidgetInformation& widgetInfo )
    : WidgetOperation( widgetInfo ),
      d( NULL )
{
    initializeData();
}

WidgetContentExtraction::~WidgetContentExtraction()
{
    destroyData();
}

void WidgetContentExtraction::execute()
{
    quint64 requiredSpace = widgetSize( m_WidgetInfo[ EPropertyFilePath ].toString() );
    if( requiredSpace == 0 )
    {
        emit aborted( EErrorContentReadError );
        return;
    }
    
    m_WidgetInfo[ EPropertyWidgetSize ] = requiredSpace;
    
    //emit and get the drive value
    QMap< WidgetPropertyType, QVariant > propertyMap;
    interactiveProperties( propertyMap );

    m_WidgetInfo[ EPropertyContentDirectory ] = m_WidgetInfo[ EPropertyContentDirectory ].toString() + QString("%1").arg(operationcount++);
    
    quint64 freeSpace = availableSpace( m_WidgetInfo[ EPropertyContentDirectory ].toString() );
    
    if( !( isSafeToInstall( requiredSpace, freeSpace ) ) )
    {
        emit aborted( EErrorNoEnoughSpace );
        return;
    }
    
    //unzip by removing the directory if it exists
    WidgetErrorType error = extractContents();
    if( error == EErrorNone )
        emit completed();
    else
        emit aborted( error );
    return;
}

void WidgetContentExtraction::interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties )
{
    properties[ EPropertyContentDirectory ] = m_WidgetInfo[ EPropertyContentDirectory ];
}


void WidgetContentExtraction::restore()
{
    if( !m_Finalized )
    {
        QString unzipPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();        
        //QDir().rmdir(unzipPath);
        WidgetUtilities::removeDirectory( unzipPath );
    }
}

WidgetErrorType WidgetContentExtraction::finalize()
{
    return EErrorNone;    
}
