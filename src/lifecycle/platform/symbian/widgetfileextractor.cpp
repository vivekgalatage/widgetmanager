#include "widgetfileextractor.h"

#include <QDir>

#include <f32file.h> 
#include <zipfile.h>

WidgetFileExtractor::WidgetFileExtractor( WidgetInformation &widgetInfo, const QString& fileToBeExtracted, RFs &rfs, CZipFile &zipFile, CZipFileMember &zipFileMember, const QString &destination )
: WidgetOperation( widgetInfo ),
  m_fileToBeExtracted( fileToBeExtracted ),
  m_Rfs( rfs ),
  m_ZipFile( zipFile ),
  m_ZipFileMember( zipFileMember ),
  m_Stream( NULL ),
  m_destination( destination )
{
    
    m_DataToBeRead = m_ZipFileMember.UncompressedSize();

    //get the temporary zip path
    TPtrC16 tempUnzipPath( reinterpret_cast<const TUint16*>( m_destination.utf16() ) );

    //get the destination FilePathName
    TFileName text;
    text.Copy( *( tempUnzipPath.AllocL()  ));
    text.Append(_L("\\"));
    text.Append( *m_ZipFileMember.Name() );

    HBufC* destFileNameAndPath = text.AllocL();
    
    TInt index = destFileNameAndPath->LocateReverse('\\');

    if (index >= 0)
    {
        TPtrC path = destFileNameAndPath->Left(index + 1);
        m_Rfs.MkDirAll(path); // errors are ok
    }

    if (destFileNameAndPath->Right(1).Compare(_L("\\")) != 0)
        m_File.Replace( m_Rfs,*destFileNameAndPath,EFileWrite);
    
    delete destFileNameAndPath;
}

WidgetFileExtractor::~WidgetFileExtractor()
{
    LOGIT( "OPTINSTALLER: WidgetFileExtractor::~WidgetFileExtractor()" );
    
    const TFileName* f = m_ZipFileMember.Name();
    QString fn( (QChar*)f->Ptr(), f->Length() );
    LOGIT( "->"<<fn<<"<-" );
    
    m_File.Close();
    
    if( m_Stream )
        delete m_Stream;
}

void WidgetFileExtractor::execute()
{
    if( !m_Stream )
    {
        TRAP_IGNORE( m_ZipFile.GetInputStreamL( &m_ZipFileMember, m_Stream ) );
        
        if( !m_Stream )
        {
            LOGIT("EErrorContentReadError");
            emit aborted( EErrorContentReadError );
            return;
        }
    }
    
    //read the contents to the buffer from the stream
    if( m_DataToBeRead > 0 ) 
    {
        int maxReadBytes = m_WidgetInfo[ EPropertyMaxReadBytes ].toInt();
        int readBytes = ( m_DataToBeRead < maxReadBytes ?  m_DataToBeRead : maxReadBytes );

        HBufC8* buffer = HBufC8::NewL( readBytes );
        TPtr8 bufferPtr( buffer->Des() );
        int readerr = m_Stream->Read( bufferPtr, readBytes );
        if( readerr != KErrNone )
        {
            delete buffer;
            emit aborted( EErrorContentReadError );
            return;
        }
    
        int writeerr = m_File.Write(*buffer);
        if( writeerr ) 
        {
            delete buffer;
            emit aborted( EErrorContentWriteError );
            return;
        }
        
        delete buffer;
        m_DataToBeRead -= readBytes;
        if( m_DataToBeRead > 0 )
            addSuboperation( this );
        else
        {
            m_File.Close();
            if( m_Stream ) {
                delete m_Stream;
                m_Stream = NULL;
            }
        }
    }
    emit completed();
    return; 
}

void WidgetFileExtractor::restore()
{
    LOGIT( "restore fileToBeExtracted" << m_fileToBeExtracted );
    QString fileName = m_WidgetInfo[ EPropertyContentDirectory ].toString() + QDir::separator() + m_fileToBeExtracted;
    
    QFile qFileName( fileName );
    if( qFileName.exists() )
    {
        LOGIT("file exists");
        qFileName.remove();
    }
    LOGIT("file does not exist");
}

