#ifndef WIDGETCONTENTEXTRACTION_H
#define WIDGETCONTENTEXTRACTION_H

#include "widgetoperationmanager.h"
#include "widgetoperation.h"

#include "widgettypes.h"
class WidgetContentExtractionPrivate;

class WidgetContentExtraction : public WidgetOperation
{
public:
    
    explicit WidgetContentExtraction( WidgetInformation& widgetInfo );
    ~WidgetContentExtraction();

    void execute();
    
    WidgetErrorType finalize();
    
    void restore();
    
    void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );
public:
    static int operationcount;
    
private:
    WidgetContentExtractionPrivate *d;
    
    void initializeData();
    
    void destroyData();
    
    quint64 availableSpace( const QString& destination );
    
    quint64 widgetSize( const QString& widgetFilePath );
    
    bool isSafeToInstall( quint64 uncompressedSize, quint64 driveSpace );
    
    WidgetErrorType extractContents();
    
};

#endif // WIDGETCONTENTEXTRACTION_H
