#ifndef WIDGETFILEEXTRACTOR_H
#define WIDGETFILEEXTRACTOR_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"

#include <f32file.h> 

class RFs;
class CZipFile;
class CZipFileMember;
class RZipFileMemberReaderStream;

class WidgetFileExtractor: public WidgetOperation
{
public:
    WidgetFileExtractor( WidgetInformation &widgetInfo,const QString& fileToBeExtracted, RFs &rfs, CZipFile &zipFile, CZipFileMember &zipFileMember, const QString &destination = ""  );

    ~WidgetFileExtractor();

public:
    void execute();

    void restore();
  
private:
    WidgetFileExtractor();
    explicit WidgetFileExtractor( const WidgetFileExtractor &);
    WidgetFileExtractor & operator = ( const WidgetFileExtractor &);
    
    const QString m_fileToBeExtracted; 
    
    RFs &m_Rfs;
    CZipFile &m_ZipFile;
    CZipFileMember &m_ZipFileMember;
    RZipFileMemberReaderStream *m_Stream;
    
    RFile m_File;

    QString m_destination;
    int m_DataToBeRead;
};

#endif //WIDGETFILEEXTRACTOR_H
