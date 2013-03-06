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

#ifndef WIDGETUTILITY_H
#define WIDGETUTILITY_H

//  INCLUDES
#include <e32base.h>
#include <f32file.h>
#include <QString>

// FORWARD DECLARATIONS
class CZipFile;
class CZipFileMemberIterator;

class WidgetUnzipUtilityS60: public CBase
    {
    public:
        static bool unzip(const QString& aFileName, const QString& aTempFilePath, 
		unsigned long& size);
        static QString fileContents(const QString& aZipFileName, const QString& aFileName, 
		const Qt::CaseSensitivity& cs = Qt::CaseInsensitive);
        static WidgetUnzipUtilityS60* NewL(const TDesC& aFileName, const TDesC& aTempFilePath, 
		const TBool& aAsync = ETrue);
        virtual ~WidgetUnzipUtilityS60();
        TBool RunUnzip();
        TUint size() { return iUncompressedSize; }
        static TUint32 uncompressedSizeL(const QString& aZipFileName, const TChar& aDriveLetter);

    protected:
        WidgetUnzipUtilityS60();
        void ConstructL(const TDesC& aFileName, const TDesC& aTempFilePath, const TBool& aAsync);
        static QString fileContentsL(const QString& aZipFileName, const QString& aFileName, 
		const Qt::CaseSensitivity& cs = Qt::CaseInsensitive);

    private:
        HBufC* iZipFileName;
        HBufC* iTempZipPath;
        CZipFile* iZipFile;
        CZipFileMemberIterator* iMembers;
        RFs iFs;
        TUint iUncompressedSize;
        TBool RunUnzipL();

        CIdle* iIdle;
    };

#endif
