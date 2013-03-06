#include "widgetcontentextraction.h"

#include "widgetcheckinstallationfromfolder.h"
#include "widgetfileextractor.h"
#include "symbianutils.h"
#include "widgetconstants.h"
#include "widgetutilities.h"

#include <sysutil.h>
#include <QFileInfo>
#include <QStringList>
#include <QList>
#include <QDir>
#include <zipfile.h>

class WidgetContentExtractionPrivate 
{
public:
    ~WidgetContentExtractionPrivate();
    
public:
    RFs m_Rfs;
    CZipFile *m_ZipFile;
    QList<CZipFileMember *> m_ZipFileMembers;
};

WidgetContentExtractionPrivate::~WidgetContentExtractionPrivate()
{
    for( int i=0; i< m_ZipFileMembers.size(); i++ )
    {
        CZipFileMember *member = m_ZipFileMembers[i];
        if( member )
            delete member;
    }
    
    if( m_ZipFile )
    {
        m_ZipFile->Close();
        delete m_ZipFile;
    }
    
    m_Rfs.Close();
}

void WidgetContentExtraction::initializeData()
{
    d = new WidgetContentExtractionPrivate();
}

void WidgetContentExtraction::destroyData()
{
    if( d )
        delete d;
}

WidgetErrorType WidgetContentExtraction::extractContents()
{
    if( !d )
        return EErrorContentExtractionGeneral;
    
    QString filePath = m_WidgetInfo[ EPropertyFilePath ].toString();
    
    QString unzipPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();

    WidgetUtilities::removeDirectory( unzipPath );
    
    if( d->m_ZipFileMembers.size() <= 0 )
        return EErrorContentExtractionGeneral;
    
    for( int i=0; i< d->m_ZipFileMembers.size(); i++ )
    {
        CZipFileMember* member = d->m_ZipFileMembers[i];
        
        TBuf<KMaxFileName> nameBuf = *member->Name();
        nameBuf.ZeroTerminate();
        
        QString fileName( (QChar*)nameBuf.Ptr(), nameBuf.Length());
        
        WidgetFileExtractor* fileExtractor;
        if( fileName.endsWith("wgt") )
        {
            fileExtractor = new WidgetFileExtractor( m_WidgetInfo, fileName, d->m_Rfs, *( d->m_ZipFile ), *member, WidgetExtractionPath ); 
            addSuboperation( fileExtractor );
        }
        fileExtractor = new WidgetFileExtractor( m_WidgetInfo, fileName, d->m_Rfs, *( d->m_ZipFile ), *member, m_WidgetInfo[ EPropertyContentDirectory].toString() );
        
        addSuboperation( fileExtractor );
    } 
   return EErrorNone; 
}

quint64 WidgetContentExtraction::availableSpace( const QString& /*destination*/ )
{
    int error( 0 );

    // Precondition: widgetPath starts with a drive letter
    // get Symbian drive number from drive letter
    QString unzipPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();
    TChar driveLetter = unzipPath[0].toAscii();
    
    TInt driveNumber( 0 );
    error = RFs::CharToDrive(driveLetter, driveNumber);
    if (KErrNone != error)
        return false;
    
    // get free space on drive
    quint64 driveSpace = 0;
    error = SymbianUtils::driveInfo(driveNumber, driveSpace);
    if (KErrNone != error)
        return false;

    // calculate uncompressed size of the widget to be installed
    TUint64 uncompressedSize(0);
    TRAP_IGNORE(uncompressedSize = SymbianUtils::getUncompressedSizeL(m_WidgetInfo[ EPropertyFilePath ].toString(), driveLetter));

    // add 500 kBs safe buffer
    uncompressedSize += 500000;
 
    return (quint64)uncompressedSize;
}

bool WidgetContentExtraction::isSafeToInstall( quint64 uncompressedSize, quint64 driveSpace )
{
    if (uncompressedSize > UINT_MAX)
        return false;
    
    TInt error( 0 );
    
    QString unzipPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();
    TChar driveLetter = unzipPath[0].toAscii();
    
    TInt driveNumber( 0 );
    error = RFs::CharToDrive(driveLetter, driveNumber);
    if (KErrNone != error)
        return false;

    TBool belowCriticalLevel(false);
    TRAP_IGNORE( belowCriticalLevel = SysUtil::DiskSpaceBelowCriticalLevelL(NULL, uncompressedSize, driveNumber));

    // compare space needed to space available
    if (uncompressedSize > driveSpace || belowCriticalLevel)
        return false;
    else
        return true;    // all ok
}


quint64 WidgetContentExtraction::widgetSize( const QString& /*widgetFilePath*/ )
{
    QString aZipFileName = m_WidgetInfo[ EPropertyFilePath ].toString();
    TInt err( 0 );

    QString unzipPath = m_WidgetInfo[ EPropertyContentDirectory ].toString();
    TChar driveLetter = unzipPath[0].toAscii();
    
    TInt driveNumber( 0 );
    err = RFs::CharToDrive(driveLetter, driveNumber);
    if (KErrNone != err)
        return false;
        
    TRAPD( error , d->m_Rfs.Connect()); // not possible to continue without RFs
    if( error )
        return 0;

    TUint64 uncompressedSize(0);
    TInt allocUnitSize(8192); // use 8kb cluster size when no information is available
    
    TInt drive(0);
    err = d->m_Rfs.CharToDrive(driveLetter, drive);
    // get cluster size to calculate uncompressed widget size more accurately
    if (!err) 
    {
       TVolumeIOParamInfo ioInfo;
       err = d->m_Rfs.VolumeIOParam(drive, ioInfo);
    
       // VolumeIOParam.iClusterSize contains also possible error code
       if (!err && ioInfo.iClusterSize > 0) 
       {
           allocUnitSize = ioInfo.iClusterSize;
       }
    }
    
    QString zipFileName = QDir::toNativeSeparators(aZipFileName);
    
    TPtrC16 zName(reinterpret_cast<const TUint16*>(zipFileName.utf16()));
    TRAPD( error1, d->m_ZipFile = CZipFile::NewL(d->m_Rfs, zName) );
    if( error1 )
        return 0;

    CZipFileMemberIterator *members = NULL;
    TRAPD( error2, members = d->m_ZipFile->GetMembersL() );
    if( error2 )
        return 0;
    
    CZipFileMember* member = NULL;
    TRAPD( error3, member = members->NextL() );
    if( error3 )
        return 0;

    while (member != NULL) 
    {
       (d->m_ZipFileMembers) << member;
       TUint32 fileSize(member->UncompressedSize());
       if(fileSize != UINT_MAX)
           fileSize = (fileSize + (allocUnitSize - (fileSize % allocUnitSize)));
       uncompressedSize += fileSize;
       //delete member;
       TRAPD( error, member = members->NextL() );
       if( error )
           return 0;
    }
    
    if(uncompressedSize > UINT_MAX)
       return UINT_MAX;
    return (quint64)uncompressedSize;
}
