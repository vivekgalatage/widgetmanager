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

#ifndef RPMUTILS_H
#define RPMUTILS_H


#include "packageutils.h"

#include <QFile>

class Archiver;
class SuperWidget;
class WidgetProperties;

class QDirIterator;

struct rpmlead {
    unsigned char magic[4];
    unsigned char major, minor;
    short type;
    short archnum;
    char name[66];
    short osnum;
    short signature_type;
    char reserved[16];
};

struct indexentry {
    unsigned int tag, type, offset, count;
    void *data;
    unsigned int datasize;
};

struct rpmheader {
    unsigned char magic[3], headerversion, mustbezero[4];
    unsigned int numberofentries, signaturesize;
    QList<struct indexentry> entries;
};

struct filesdata {
    QString filename;
    QString path;
    QString owner;
    unsigned int filesize;
    unsigned short filemode;
};


/*!
  \class PackageUtils
  \brief A class used to create RPM packages for widgets.
  */
class RpmUtils : public PackageUtils
{
    Q_DISABLE_COPY(RpmUtils)

public:
    /*!
     Constructs an object that can be used to create a Debian package named \a packageName. The unzipped widget files locate at \a sourcePath, and they are
     supposed to be installed to \a installationPath. The attributes of widgets can be seen from \a widget, while the unique ID is given by \a appId.
     */
    explicit RpmUtils(SuperWidget* widget, const QString packageName, const QString& sourcePath, const QString& installationPath, const QString& appId);

    /*!
      Destroys the object.
    */
    virtual ~RpmUtils();

    /*!
      Create the package.
     */
    virtual bool createPackage();

private:
    void populateLead();
    void populateRpmHeader(struct rpmheader &head);
    void calculateAlign(const int type);
    bool addEntryToHeader(struct rpmheader &header, int tag, int type, void *data, unsigned int datasize, int count = 1);
    bool addEntryFilesSize();
    bool addEntryFilesMode();
    void sortDirsAndFiles();
    bool addDirectoryRecursively(Archiver& pack, const QString& root, const QString& newPath);
    void addFile(QDirIterator &it, const QString &dir);
    bool createCpioFile(const QString& rpmRootDir, const QString& outputPath);
    int calculateOffsetForImmutable(int number_of_entries);
    bool addCpioData();
    int* createDirIndex(int size);
    bool addLead();
    void fixRpmName();
    bool addHeader(struct rpmheader header);

    rpmlead m_lead;
    rpmheader m_header, m_signature;

    QFile m_fd;
    QList<struct filesdata> m_filesData;
    QString m_dirs;
    QString m_files;
    QString m_cpioData;

    int m_index;
    int m_rpmSize;
};

#endif // RPMUTILS_H
