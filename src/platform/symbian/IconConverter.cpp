//
// ============================================================================
//  Name     : IconConverter.cpp
//  Part of  : Wrt Widget Installer
//
//  Description:
//     Icon convert to convert icon for png to mbm format
//
//  Version     : WRT 1.0
//
// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
//
// This file is part of Qt Web Runtime.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
//

// INCLUDE FILES
#include <QString>
#include <fbs.h>
#include <ImageConversion.h>
#include <BitmapTransforms.h>
#include "IconConverter.h"
#include "WidgetUtilsLogs.h"

#include <QEventLoop>
#include <QDebug>


// CONSTANTS
_LIT( KTempPath,"c:\\system\\temp\\" );

const TInt KIconSizeLarge = 88;
const TInt KIconSizeMedium = 32;
const TInt KIconSizeSmall = 24;



// ============================================================================
// CIconConverter::NewL()
// two-phase constructor
//
// @since 3.1
// @param aController - controller for callback to notify the completion
// @param aFs - file session
// @return pointer to CIconConverter
// ============================================================================
//
CIconConverter* CIconConverter::NewL(QEventLoop* loop)
    {
    CIconConverter* self =
        new(ELeave) CIconConverter( loop );
    CleanupStack::PushL( self );

    self->ConstructL();

    CleanupStack::Pop( self );
    return self;
    }

// ============================================================================
// CIconConverter::CIconConverter()
// C++ default constructor
//
// @since 3.1
// ============================================================================
CIconConverter::CIconConverter( QEventLoop* loop ) :
        CActive( EPriorityStandard )
        , m_loop(loop)
    {
    CActiveScheduler::Add( this );
    }

// ============================================================================
// CIconConverter::ConstructL()
// Symbian default constructor
//
// @since 3.1
// ============================================================================
void CIconConverter::ConstructL()
    {
    User::LeaveIfError( iFs.Connect() );
    User::LeaveIfError( iFs.ShareProtected() );
    User::LeaveIfError( RFbsSession::Connect() );

    // create the destination bitmap
    iOriginalBitmap = new ( ELeave ) CFbsBitmap;
    iOriginalBitmapMask = new ( ELeave ) CFbsBitmap;

    iTempBitmap = new ( ELeave ) CFbsBitmap;
    iTempBitmapMask = new ( ELeave ) CFbsBitmap;
    iTempPath = KTempPath().AllocL();
    iIconSizes = new CArrayFixFlat<TSize>( 3 );
    iIconSizes->InsertL( 0, TSize( KIconSizeLarge, KIconSizeLarge ) );
    iIconSizes->InsertL( 1, TSize( KIconSizeMedium, KIconSizeMedium ) );
    iIconSizes->InsertL( 2, TSize( KIconSizeSmall, KIconSizeSmall ) );
    }


// ============================================================================
// CIconConverter::~CIconConverter()
// destructor
//
// @since 3.1
// ============================================================================
CIconConverter::~CIconConverter()
    {
    Cancel();

    // CImageDecoder must be deleted first otherwise a related thread might panic
    if ( iImageDecoder )
        {
        delete iImageDecoder;
        }
    if ( iOriginalBitmap )
        {
        delete iOriginalBitmap;
        }
    if ( iOriginalBitmapMask )
        {
        delete iOriginalBitmapMask;
        }
    if ( iOutputFileName )
        {
        delete iOutputFileName;
        }
    if ( iTempBitmap )
        {
        delete iTempBitmap;
        }
    if ( iTempBitmapMask )
        {
        delete iTempBitmapMask;
        }
    if ( iScaler )
        {
        delete iScaler;
        }
    if ( iTempPath )
        {
        delete iTempPath;
        }
    iIconFile.Close();
    iIconPngFile.Close();
    RFbsSession::Disconnect();
    if ( iIconSizes )
        {
        iIconSizes->Reset();
        delete iIconSizes;
        }

    iFs.Close();
    }

bool CIconConverter::Convert(
    const QString& aInputFileName,
    const QString& aOutputFileName )
    {
        TPtrC16 inFile(reinterpret_cast<const TUint16*>(aInputFileName.constData()));
        TPtrC16 outFile(reinterpret_cast<const TUint16*>(aOutputFileName.constData()));

        TRAPD(err, StartToDecodeL(inFile,outFile));
        if (err != KErrNone) {
            return false;
        }
        return true;
    }
// ============================================================================
// CIconConverter::StartToDecodeL
// use image decoder to decode the image
//
// @since 3.1
// ============================================================================
void CIconConverter::StartToDecodeL(
    const TDesC& aInputFileName,
    const TDesC& aOutputFileName )
    {
    iState = EConvertingFile;
    delete iImageDecoder;
    iImageDecoder = NULL;

    delete iOutputFileName;
    iOutputFileName = 0;

    iOutputFileName = aOutputFileName.AllocL();

    // create the decoder
    iImageDecoder = CImageDecoder::FileNewL( iFs, aInputFileName );
    LOG (" after CImageDecoder::FileNewL");

    // Extract information about the image, now we've read the header
    TFrameInfo info = iImageDecoder->FrameInfo( 0 );
    LOG (" after CImageDecoder::FrameInfo");

    iOriginalBitmap->Create( info.iOverallSizeInPixels, info.iFrameDisplayMode );
    LOG (" after iOriginalBitmap->Create");

    // If the PNG has a built in transparency, use it to build the mask
    if ( info.iFlags & TFrameInfo::ETransparencyPossible )
        {
        // If we have a full alpha channel, use that
        if ( info.iFlags & TFrameInfo::EAlphaChannel )
            {
            LOG (" bf iOriginalBitmap->Create 1");
            User::LeaveIfError( iOriginalBitmapMask->Create(
                info.iOverallSizeInPixels,
                EGray256 ) );
            }
        else
            {
            LOG (" bf iOriginalBitmap->Create 2");
            User::LeaveIfError( iOriginalBitmapMask->Create(
                info.iOverallSizeInPixels,
                EGray2 ) );
            }

        LOG (" bf iImageDecoder->Convert");
        iImageDecoder->Convert(
            &iStatus, *iOriginalBitmap, *iOriginalBitmapMask );
        LOG (" af iImageDecoder->Convert 1");
        }
    else
        {
        iImageDecoder->Convert( &iStatus, *iOriginalBitmap );
        LOG (" af iImageDecoder->Convert 1");
        }

    // start conversion to bitmap
    SetActive();
    LOG (" af SetActive()");
    }

// ============================================================================
// CIconConverter::RunL()
// Handle various stages of icon conversion
//
// @since 3.1
// ============================================================================
void CIconConverter::RunL()
    {
    // If there is an error in the previous stage, then leave. Otherwise,
    // call the handle function
    LOG ("CIconConverter::RunL() status = " << iStatus.Int());
    User::LeaveIfError( iStatus.Int() );

    switch ( iState )
        {
    case EConvertingFile:
        LOG ("  bf DoProcessMaskL()");
        DoProcessMaskL();
        break;

    case EScalingIcon:
        LOG ("  bf DoMaskScalingL()");
        DoMaskScalingL();
        break;

    case EScalingMask:
    case EFinalize:
        LOG ("  bf DoIconStoreL()");
        DoIconStoreL();
        break;

    default:
        LOG ("  default");
        User::Leave( KErrNotSupported );
        break;
        };

    }

// ============================================================================
// CIconConverter::RunError()
// Notify client with error
//
// @since 3.1
// ============================================================================
TInt CIconConverter::RunError( TInt aError )
    {
    LOG ("CIconConverter::RunError() err = " << aError);
    // If any error occurred, then complete the client with the error.
    if ( iClientStatus )
        {
        User::RequestComplete( iClientStatus, aError );
        }

    // There is nothing more to do if notifyCompletion leaves.
    if (m_loop->isRunning())
        m_loop->exit();

    return KErrNone;
    }

// ============================================================================
// CIconConverter::DoCancel()
// cancel icon conversion
//
// @since 3.1
// ============================================================================
void CIconConverter::DoCancel()
    {
    LOG ("CIconConverter::DoCancel()");
    switch (iState)
        {
    case EConvertingFile:
        if ( iImageDecoder )
            {
            iImageDecoder->Cancel();
            }

        break;

    case EScalingIcon:
    case EScalingMask:
        if ( iScaler )
            {
            iScaler->Cancel();
            }
        break;

        };

    if ( iClientStatus )
        {
        User::RequestComplete( iClientStatus, KErrCancel );
        }

    // no need to call notifyCompletion() because cancel can only be
    // caused by the client
    if (m_loop->isRunning())
        m_loop->exit();
    }

// ============================================================================
// CIconConverter::DoProcessMaskL()
// process the bitmap mask
//
// @since 3.1
// ============================================================================
void CIconConverter::DoProcessMaskL()
    {
    LOG ("CIconConverter::DoProcessMaskL()");
    // we use white to mean transparent at this stage, simply for efficiency
    // since all the canvases we will copy in to begin as white

    if ( iOriginalBitmapMask->Handle() == 0 )
        {
        // Create a mask that shows the whole bitmap as an icon
        // (all black)
        User::LeaveIfError( iOriginalBitmapMask->Create(
            iOriginalBitmap->SizeInPixels(), EGray2 ) );
        CFbsBitmapDevice* device =
            CFbsBitmapDevice::NewL( iOriginalBitmapMask );
        CleanupStack::PushL( device );

        CFbsBitGc* gc;
        User::LeaveIfError( device->CreateContext( gc ) );
        gc->SetBrushStyle( CGraphicsContext::ESolidBrush );
        gc->SetDrawMode( CGraphicsContext::EDrawModePEN );
        gc->SetBrushColor( KRgbBlack );
        // Create a big black image
        gc->Clear();
        delete gc;
        CleanupStack::PopAndDestroy( device );
        }
    else
        {
        // Invert the mask obtained from the PNG
        CFbsBitmapDevice* device =
            CFbsBitmapDevice::NewL( iOriginalBitmapMask );
        CleanupStack::PushL(device);
        CFbsBitGc* gc;
        User::LeaveIfError( device->CreateContext( gc ) );
        gc->SetDrawMode( CGraphicsContext::EDrawModeNOTSCREEN );
        gc->Clear();
        delete gc;
        CleanupStack::PopAndDestroy( device );
        }
    LOG (" af iOriginalBitmapMask->Handle()");

    // Scale the icon to the sizes required
    iCurrentSizeIndex = 0;
    DoIconScalingL();
    LOG ("CIconConverter::DoProcessMaskL() ...Done");
    }

// ============================================================================
// CIconConverter::DoIconScalingL()
// Scale the bitmap
//
// @since 3.1
// ============================================================================
void CIconConverter::DoIconScalingL()
    {
    LOG ("CIconConverter::DoIconScalingL()");
    // free any current icons to prevent memory leaks
    iTempBitmap->Reset();
    LOG (" af iTempBitmap->Reset()");
    // current target size
    TSize size = iIconSizes->At( iCurrentSizeIndex );
    LOG (" af iIconSizes->At()");

    iState = EScalingIcon;
    // Create a canvas to hold the scaled icon, of the same depth
    User::LeaveIfError(
        iTempBitmap->Create( size, iOriginalBitmap->DisplayMode() ) );
    LOG (" af iOriginalBitmap->DisplayMode()");
    DoScalingL( *iOriginalBitmap, *iTempBitmap );
    }

// ============================================================================
// CIconConverter::DoMaskScalingL()
// Scale the bitmap mask
//
// @since 3.1
// ============================================================================
void CIconConverter::DoMaskScalingL()
    {
    LOG ("CIconConverter::DoMaskScalingL()");
    // Reset the mask to prevent memory leaks
    iTempBitmapMask->Reset();
    // current target size
    TSize size = iIconSizes->At( iCurrentSizeIndex );

    iState = EScalingMask;
    // Create a canvas to hold the scaled icon, of 8 bit colour depth
    User::LeaveIfError( iTempBitmapMask->Create( size, EGray256 ) );
    DoScalingL( *iOriginalBitmapMask, *iTempBitmapMask );
    }

// ============================================================================
// CIconConverter::DoScalingL()
// Scale
//
// @since 3.1
// ============================================================================
void CIconConverter::DoScalingL(
    CFbsBitmap& aBitmapSource, CFbsBitmap& aBitmapTarget )
    {
    LOG ("CIconConverter::DoScalingL()");
    ScalerL().Scale( &iStatus, aBitmapSource, aBitmapTarget, ETrue );
    LOG ("CIconConverter::DoScalingL() ...Done");
    SetActive();
    }

// ============================================================================
// CIconConverter::ScalerL()
// Create bitmap scalar
//
// @since 3.1
// ============================================================================
CBitmapScaler& CIconConverter::ScalerL()
    {
    LOG ("CIconConverter::ScalerL()");
    if ( iScaler == NULL )
        {
        iScaler = CBitmapScaler::NewL();
        LOG ("CBitmapScaler::NewL()");
        // always use highest quality scaling
        User::LeaveIfError( iScaler->SetQualityAlgorithm( CBitmapScaler::EMaximumQuality ) );
        LOG ("SetQualityAlgorithm()");
        }
    LOG ("CIconConverter::ScalerL() ...Done");
    return *iScaler;
    }

// ============================================================================
// CIconConverter::DoIconStoreL()
// Store icon and mask files
//
// @since 3.1
// ============================================================================
void CIconConverter::DoIconStoreL()
    {
    LOG ("CIconConverter::DoIconStoreL()");
    // Store the icon and its mask in temporary files until we are ready
    // to create the final icon

    // Icon is stored at index n, mask at index n+1
    TInt iconIndex = iCurrentSizeIndex * 2;
    TFileName iconFile = *iTempPath;
    GetTempIconName( iconIndex++, iconFile );
    LOG (" af GetTempIconName() 1");

    TFileName maskFile = *iTempPath;
    GetTempIconName( iconIndex, maskFile );
    LOG (" af GetTempIconName() 2");

    // invert the masks before saving

    CFbsBitmapDevice* device = CFbsBitmapDevice::NewL( iTempBitmapMask );
    CleanupStack::PushL( device );
    LOG (" af CFbsBitmapDevice::NewL()");

    CFbsBitGc* gc;
    User::LeaveIfError( device->CreateContext( gc ) );
    gc->SetDrawMode( CGraphicsContext::EDrawModeNOTSCREEN );
    gc->Clear();
    LOG (" af device->CreateContext()");

    delete gc;
    CleanupStack::PopAndDestroy( device );

    // save the bitmaps
    User::LeaveIfError( iTempBitmap->Save( iconFile ) );
    LOG (" af iTempBitmap->Save()");
    User::LeaveIfError( iTempBitmapMask->Save( maskFile ) );
    LOG (" af iTempBitmapMask->Save()");

    if ( ++iCurrentSizeIndex < iIconSizes->Count() )
        {
        // do the next icon size
        LOG (" bf DoIconScalingL()");
        DoIconScalingL();
        }
    else
        {
        LOG (" bf DoCreateFinalIconL()");
        DoCreateFinalIconL();
        }
    LOG ("CIconConverter::DoIconStoreL() ...Done");
    }

// ============================================================================
// CIconConverter::DoCreateFinalIconL()
// Create the final icon
//
// @since 3.1
// ============================================================================
void CIconConverter::DoCreateFinalIconL()
    {
    LOG ("CIconConverter::DoCreateFinalIconL()");
    TInt i, elements = 0;
    // one icon, one mask per size
    TInt bitmapCount = iIconSizes->Count() * 2;
    LOG (" bitmapCount: " << bitmapCount);

    TFileName** filenames = new ( ELeave ) TFileName*[bitmapCount];
    CleanupStack::PushL( filenames );
    TInt32* uniqueIds = new ( ELeave ) TInt32[bitmapCount];
    CleanupStack::PushL( uniqueIds );

    TInt err = KErrNone;

    for ( i = 0; i < bitmapCount; ++i )
        {
        filenames[i] = NULL;
        filenames[i] = new TFileName( *iTempPath );
        elements = i;

        if ( filenames[i] == NULL )
            {
            // we need to cleanup this structure
            err = KErrNoMemory;
            goto cleanup;
            }
        GetTempIconName( i, *filenames[i] );
        uniqueIds[i] = 0;

        }

    TRAP( err, CFbsBitmap::StoreL(
        *iOutputFileName, bitmapCount, ( const TDesC** )filenames, uniqueIds ) );

cleanup:
    for ( i = 0; i <= elements; ++i )
        {
        if ( filenames[i] == NULL )
            {
            // if we failed to allocate a filename, then we would not have continued
            break;
            }
        else
            {
            delete filenames[i];
            }
        }

    CleanupStack::PopAndDestroy( 2, filenames );

    // There is no recovery on a leave and we don't want to trigger
    // RunError here since that will also call notifyCompletion.
    if (m_loop->isRunning())
        m_loop->exit();
    LOG ("CIconConverter::DoCreateFinalIconL() ...Done   err = " << err);
    }

// ============================================================================
// CIconConverter::GetTempIconName()
// Get temporary icon name
//
// @since 3.1
// ============================================================================
void CIconConverter::GetTempIconName( const TInt& aIndex, TFileName& aIconName )
    {
    _LIT( KIcon, "ICON" );
    _LIT( KBmp, ".MBM" );
    aIconName.Append( KIcon );
    aIconName.AppendNum( static_cast<TInt64>( aIndex ) );
    aIconName.Append( KBmp );
    }


IconConverterWrapper::IconConverterWrapper(QEventLoop *loop, QObject *parent)
    : QObject(parent),
      m_eventLoop(loop),
      m_converter(0) {
    }

IconConverterWrapper::~IconConverterWrapper() {
    delete m_converter;
}

bool IconConverterWrapper::Init(const QString& inputFileName, const QString& outputFileName) {
    TRAPD(error, m_converter = CIconConverter::NewL(m_eventLoop));
    if (error != KErrNone)
        return false;

    bool ret = true;
    ret = m_converter->Convert(inputFileName, outputFileName);
    return ret;
}

