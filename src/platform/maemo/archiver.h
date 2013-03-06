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

#ifndef ARCHIVER_H
#define ARCHIVER_H

#include <QObject>

class QString;
class QByteArray;

class archive;
class archive_entry;


/*!
  \class Archiver
  \brief A wrapper class of libarchive.

  Currently, Archiver only supports the generation of archives in the format of ar, tar, tar.gz, and cpio.
  */
class Archiver
{
    Q_DISABLE_COPY(Archiver)

public:
    /*!
      \enum Archiver::ArchiveType
      \value Ar Generate an archive in the ar format.
      \value Tar Generate an archive in the tar format.
      \value TarGz Generate an archive in the tar.gz format.
      \value Cpio Generate an archive in the cpio format.
     */
    enum ArchiveType {
        Ar,
        Tar,
        TarGz,
        Cpio
    };

    /*!
      Constructs an object that can be used to create an archive at \a target, of \a type.
    */
    explicit Archiver(const QString& target, ArchiveType type);

    /*!
      Destroys the object.
    */
    ~Archiver();

    /*!
      Add the \a directory and all its sub-directories and files to \a newPath.
     */
    bool addDirectoryRecursively(const QString& directory, const QString& newPath);

    /*!
      Add the \a directory to \a newName.
     */
    bool addDirectory(const QString& directory, const QString& newName);

    /*!
      Add the \a file to \a newName.
     */
    bool addFile(const QString& file, const QString& newName);

    /*!
      Add the \a buffer to \a filename with the access permission \a mode.
     */
    bool addFileFromBuffer(const QByteArray& buffer, const QString& filename, int mode);

    /*!
      Write to the file system.
     */
    bool flush();

    /*!
      Whether this archive is valid.
     */
    bool isValid() const;

private:
    void stripPath(QString& path) const;
    void setOwner();
    bool writeFinish();
    bool writeHeader();
    void setTime();

    archive* mArchive;
    archive_entry* mEntry;
    bool mValid;
    ArchiveType mArchiveType;
};

#endif // ARCHIVER_H
