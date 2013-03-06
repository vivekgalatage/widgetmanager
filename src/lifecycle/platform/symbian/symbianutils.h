#ifndef SYMBIANUTILS_H
#define SYMBIANUTILS_H

#include <qglobal.h>

class SymbianUtils 
{
public:
    static quint32 getUncompressedSizeL( const QString& aZipFileName, const char aDriveLetter );
    static int driveInfo( int aDrive, quint64& aDiskSpace );
//    static QString resourcePath(const QString& installPath);
};

#endif //SYMBIANUTILS_H
