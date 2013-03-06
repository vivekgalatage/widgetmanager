#include "symbianutils.h"
#include "widgetconstants.h"

#include <zipfile.h>
#include <sysutil.h>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>

quint32 SymbianUtils::getUncompressedSizeL( const QString& aZipFileName, const char aDriveLetter )
{
    RFs fs;
    User::LeaveIfError(fs.Connect()); // not possible to continue without RFs
    CleanupClosePushL(fs);
    
    TUint64 uncompressedSize(0);
    TInt allocUnitSize(8192); // use 8kb cluster size when no information is available
    
    TInt drive(0);
    TInt err = fs.CharToDrive(aDriveLetter, drive);
    // get cluster size to calculate uncompressed widget size more accurately
    if (!err) {
       TVolumeIOParamInfo ioInfo;
       err = fs.VolumeIOParam(drive, ioInfo);
    
       // VolumeIOParam.iClusterSize contains also possible error code
       if (!err && ioInfo.iClusterSize > 0) {
           allocUnitSize = ioInfo.iClusterSize;
       }
    }
    
    QString zipFileName = QDir::toNativeSeparators(aZipFileName);
    
    TPtrC16 zName(reinterpret_cast<const TUint16*>(zipFileName.utf16()));
    CZipFile *zipfile = CZipFile::NewL(fs, zName);
    CleanupStack::PushL(zipfile);
    
    CZipFileMemberIterator *members = zipfile->GetMembersL();
    CleanupStack::PushL(members);
    
    CZipFileMember* member = members->NextL();
    while (member != NULL) {
       
       TUint32 fileSize(member->UncompressedSize());
       if(fileSize != UINT_MAX)
           fileSize = (fileSize + (allocUnitSize - (fileSize % allocUnitSize)));
       uncompressedSize += fileSize;
       delete member;
       member = members->NextL();
    }
    CleanupStack::PopAndDestroy(3);  // members, zipfile, fs.connect
    
    if(uncompressedSize > UINT_MAX)
       return UINT_MAX;
    return (TUint32)uncompressedSize;
}


int SymbianUtils::driveInfo( int aDrive, quint64& aDiskSpace ) 
{
    RFs fs;
    User::LeaveIfError(fs.Connect()); // not possible to continue without RFs
    CleanupClosePushL(fs);
    TInt error;
    TDriveInfo driveInfo;
    error = fs.Drive( driveInfo, aDrive );
    if (!error) {
        // MMC is in slot
        if (driveInfo.iMediaAtt & KMediaAttLocked) {
            CleanupStack::PopAndDestroy(&fs);
            return KErrLocked;
        }

        TVolumeInfo volumeInfo;
        error = fs.Volume( volumeInfo, aDrive );
        if (!error) {
            aDiskSpace = volumeInfo.iFree;
            CleanupStack::PopAndDestroy(&fs);
            return KErrNone;    // all ok, diskspace set
        }
    }
    CleanupStack::PopAndDestroy(&fs);
    return KErrNotReady;
}
