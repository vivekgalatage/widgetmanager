/*
* ============================================================================
*  Name         : IconConverter.h
*  Part of      :
*
*  Description  : Icon convert to convert icon for png to mbm format
*  Version      :
*
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

#ifndef __ICONCONVERTER_H__
#define __ICONCONVERTER_H__

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <QObject>

QT_BEGIN_NAMESPACE
class QEventLoop;
class QString;
QT_END_NAMESPACE

// FORWARD DECLARATION
class CFbsBitmap;
class CBitmapScaler;
class CImageDecoder;

/*
* Inherited CActive, performs a asynchronous conversion operation
*
* @lib .lib
* @since
*/
class CIconConverter : public CActive
    {
    // states for this engine
    enum TState
        {
        EIdle = 0,
        EDecoding,
        EConvertingFile,
        EScalingIcon,
        EScalingMask,
        EStoringIcon,
        EFinalize
        };

    public:

        public: // contructors/destructors

        /*
        * NewL
        *
        * Create a CIconConverter object and return a pointer to it.
        *
        * Params:
        *      aController Pointer to a MConverterController interface.
        *      The engine uses NotifyCompletionL callback from this interface
        *      to notify the controller about completions of coding or
        *      encoding requests.
        *
        * Returns:
        *       A pointer to the created engine
        *
        */
        static CIconConverter* NewL( QEventLoop* loop );

        /*
        * ~CIconConverter
        *
        * Destructor
        *
        */
        ~CIconConverter();

    public: // interface methods

        // convert png to mbm
        bool Convert( const QString& aInputFileName, const QString& aOutputFileName );

    private:
        void StartToDecodeL( const TDesC& aInputFileName, const TDesC& aOutputFileName );


    protected: // implementation of CActive

        /*
        * From CActive
        *
        * Handle the cancel operation
        *
        */
        void DoCancel();

        /*
        * From CActive
        *
        * Handle the conversion process
        *
        */
        void RunL();

        /*
        * From CActive
        * Handle the error situations
        */
        TInt RunError( TInt aError );

    private: // internal methods, constructors

        /*
        * CIconConverter
        *
        * C++ Constructor
        *
        */
        CIconConverter( QEventLoop* loop );

        /*
        * ConstructL
        *
        * 2-phase constructor
        *
        * @Returns:
        *       Nothing
        */
        void ConstructL();

        /*
        * GetTempIconName
        *
        * Get temporary icon name
        *
        * @Returns:
        *       Nothing
        */
        void GetTempIconName( const TInt& aIndex, TFileName& aIconName );

        /*
        * DoCreateFinalIconL
        *
        * Create the final icon
        *
        * @Returns:
        *       Nothing
        */
        void DoCreateFinalIconL();

        /*
        * DoIconStoreL
        *
        * Store icon and mask files
        *
        * @Returns:
        *       Nothing
        */
        void DoIconStoreL();

        /*
        * ScalerL
        *
        * Create bitmap scalar
        *
        * @Returns:
        *       CBitmapScaler
        */
        CBitmapScaler& ScalerL();

        /*
        * DoScalingL
        *
        * Scale
        *
        * @Returns:
        *       Nothing
        */
        void DoScalingL( CFbsBitmap& aBitmapSource, CFbsBitmap& aBitmapTarget );

        /*
        * DoMaskScalingL
        *
        * Scale the mask
        *
        * @Returns:
        *       Nothing
        */
        void DoMaskScalingL();

        /*
        * DoIconScalingL
        *
        * Scale the bitmap
        *
        * @Returns:
        *       Nothing
        */
        void DoIconScalingL();

        /*
        * DoProcessMaskL
        *
        * Process the mask for the bitmap
        *
        * @Returns:
        *       Nothing
        */
        void DoProcessMaskL();

    private: // internal data

        // The client's pending status
        TRequestStatus*         iClientStatus;
        RFs                     iFs; // for opening/saving images from/to files, owned
        CImageDecoder*          iImageDecoder; // decoder from ICL API, owned
        HBufC*                  iOutputFileName; // the resulted file name, owned
        TState                  iState; // state of the conversion
        CArrayFixFlat<TSize>*   iIconSizes;
        // Original bitmap pointers garnered from the PNG in the midlet
        CFbsBitmap*             iOriginalBitmap;
        CFbsBitmap*             iOriginalBitmapMask;
        // Scaled target bitmaps
        CFbsBitmap*             iTempBitmap;
        CFbsBitmap*             iTempBitmapMask;
        // Bitmap scaler
        CBitmapScaler*          iScaler;
        // The current size icon being converted
        TInt                    iCurrentSizeIndex;
        // Icon file
        RFile                   iIconFile;
        RFile                   iIconPngFile;
        // Path to create Temp icons
        const HBufC*            iTempPath;

        QEventLoop* m_loop;
    bool iSuccess;

    };

class IconConverterWrapper : public QObject {

public:
    explicit IconConverterWrapper(QEventLoop *loop, QObject *parent=0);
    ~IconConverterWrapper();

    bool Init(const QString& inputFileName, const QString& outputFileName);

private:
    QEventLoop* m_eventLoop;
    CIconConverter* m_converter;
};

#endif // #ifndef __ICONCONVERTER_H__
