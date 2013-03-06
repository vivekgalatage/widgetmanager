/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This file is part of Qt Web Runtime.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

//  INCLUDES
#include "WidgetUnzipUtilityS60.h"
#include "zipfile.h"
#include <QEventLoop>
#include <QtCore/QDir>


//-------------------------------------------------------------------------------
// Animate
// C-style TCallback function
//-------------------------------------------------------------------------------
TBool Unzip(
    TAny* aAny )
    {
    WidgetUnzipUtilityS60* util = static_cast<WidgetUnzipUtilityS60*> (aAny);
    TBool result = util->RunUnzip();
    return result;
    }

WidgetUnzipUtilityS60* WidgetUnzipUtilityS60::NewL(const TDesC& aFileName, 
        const TDesC& aTempFilePath, const TBool& aAsync)
    {
    WidgetUnzipUtilityS60* self = new (ELeave) WidgetUnzipUtilityS60();
    CleanupStack::PushL(self);
    self->ConstructL(aFileName, aTempFilePath, aAsync);
    CleanupStack::Pop();
    return self;
    }


WidgetUnzipUtilityS60::~WidgetUnzipUtilityS60()
    {
    delete iZipFileName;
    delete iTempZipPath;

    delete iMembers;
    delete iZipFile;

    if ( iIdle )
        {
        iIdle->Cancel();
        }
    delete iIdle;
    iFs.Close();
    }

WidgetUnzipUtilityS60::WidgetUnzipUtilityS60()
    :iUncompressedSize(0)
    {

    }

void WidgetUnzipUtilityS60::ConstructL(const TDesC& aFileName, const TDesC& aTempFilePath, const TBool& aAsync)
    {
    User::LeaveIfError( iFs.Connect() );
    iZipFileName = aFileName.AllocL();
    iTempZipPath = aTempFilePath.AllocL();

    iZipFile = CZipFile::NewL(iFs, aFileName);
    iMembers = iZipFile->GetMembersL();

    if (aAsync)
        {
    //coverity[size_error]
    //coverity[buffer_alloc]
        iIdle = CIdle::NewL( CActive::EPriorityHigh );
        //__asm int 3;
        iIdle->Start(TCallBack(&Unzip,this));
        }
    }

TBool WidgetUnzipUtilityS60::RunUnzip()
    {
    TBool result = EFalse;
    TRAP_IGNORE(result = this->RunUnzipL());
    return result;
    }

TBool WidgetUnzipUtilityS60::RunUnzipL()
    {
    CZipFileMember* member = iMembers->NextL();
    if (member)
        {
        CleanupStack::PushL(member);
        RZipFileMemberReaderStream* stream;
        User::LeaveIfError(iZipFile->GetInputStreamL(member, stream));
        CleanupStack::PushL(stream);

        HBufC8* buffer = HBufC8::NewLC(member->UncompressedSize());
        TPtr8 bufferPtr(buffer->Des());
        User::LeaveIfError(stream->Read(bufferPtr, member->UncompressedSize()));

        TFileName text;
        text.Copy(*iTempZipPath);
        text.Append(_L('\\'));
        text.Append(*member->Name());

        HBufC* zipedFileNameAndPath = text.AllocLC();

        TInt index = zipedFileNameAndPath->LocateReverse('\\');

        if (index >= 0)
            {
            TPtrC path = zipedFileNameAndPath->Left(index + 1);
            iFs.MkDirAll(path); // errors are ok
            }

        if (zipedFileNameAndPath->Right(1).Compare(_L('\\')) != 0)
            {
            RFile file;
            CleanupClosePushL(file);
            file.Replace(iFs,*zipedFileNameAndPath,EFileWrite);
            User::LeaveIfError(file.Write(*buffer));
            CleanupStack::PopAndDestroy(); // file
            }
        iUncompressedSize += member->UncompressedSize();
        CleanupStack::PopAndDestroy(4);
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

// static
bool WidgetUnzipUtilityS60::unzip(const QString& aFileName, const QString& aTempFilePath, unsigned long& size)
    {
    // Qt handles paths using /-separator so make sure that paths are in native format.
    QString fileName = QDir::toNativeSeparators(aFileName);
    QString tempFilePath = QDir::toNativeSeparators(aTempFilePath);

    // perform syncronous unzip
    TPtrC16 fName(reinterpret_cast<const TUint16*>(fileName.utf16()));
    TPtrC16 tPath(reinterpret_cast<const TUint16*>(tempFilePath.utf16()));
    WidgetUnzipUtilityS60* wUnzip = NULL;
    TRAP_IGNORE(wUnzip = WidgetUnzipUtilityS60::NewL(fName, tPath, EFalse));
    if (wUnzip) {
        QEventLoop loop;
        while (wUnzip->RunUnzip()) {
            loop.processEvents(QEventLoop::AllEvents, 100);
        }
        size = wUnzip->size();
    }
    delete wUnzip;
    return (size > 0);
    }

// static
QString WidgetUnzipUtilityS60::fileContentsL(const QString& aZipFileName, const QString& aFileName, 
const Qt::CaseSensitivity& cs)
{
    QString zipFileName = QDir::toNativeSeparators(aZipFileName);
    QString contents;

    TPtrC16 zName(reinterpret_cast<const TUint16*>(zipFileName.utf16()));

    RFs iFs;

    User::LeaveIfError(iFs.Connect());
    CZipFile *zipfile = CZipFile::NewL(iFs, zName);
    CleanupStack::PushL(zipfile);

    CZipFileMemberIterator *members = zipfile->GetMembersL();
    CZipFileMember* member;
    while ((member = members->NextL()) != 0) {
        QString filename = QString::fromUtf16(member->Name()->Ptr(), member->Name()->Length());

        // If the member is the desired file, extract it
        if (!filename.compare(aFileName, cs)) {
            RZipFileMemberReaderStream *stream;
            zipfile->GetInputStreamL(member, stream);
            CleanupStack::PushL(stream);

            HBufC8 *buffer = HBufC8::NewLC(member->UncompressedSize());
            TPtr8 bufferPtr(buffer->Des());
            User::LeaveIfError(stream->Read(bufferPtr, member->UncompressedSize()));

            QByteArray data(reinterpret_cast<const char*>(buffer->Ptr()), buffer->Length());
            QString ret(data);

            CleanupStack::PopAndDestroy(3); // zipfile, stream, buffer
            return ret;
        }
    }

    CleanupStack::PopAndDestroy();
    return contents;
}

// static
QString WidgetUnzipUtilityS60::fileContents(const QString& aZipFileName, const QString& aFileName, 
const Qt::CaseSensitivity& cs)
{
    QString contents;
    TRAP_IGNORE(contents = fileContentsL(aZipFileName, aFileName, cs));

    return contents;
}

//static
TUint32 WidgetUnzipUtilityS60::uncompressedSizeL(const QString& aZipFileName, const TChar& aDriveLetter)
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

