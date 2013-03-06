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

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDirIterator>
#include <QRegExp>

#include <archive.h>
#include <archive_entry.h>
#include <sys/stat.h>

#include "archiver.h"

// how much memory is used when writing archive
#define CHUNK_SIZE 5 * 1024 * 1024

/** Constructs a new instance of @class Archiver that is able to store files
  * in to an archive with a type @param type. Supported formats are BSD ar, tar
  * and gzip.
  *
  * @param target Archive name.
  * @param type Enumeration @enum ArchiveType.
  * @param parent Parent.
  */
Archiver::Archiver(const QString& target, ArchiveType type)
    : mArchive(NULL), mEntry(NULL), mValid(false), mArchiveType(type)
{
    mArchive = archive_write_new();
    mEntry = archive_entry_new();

    switch (type) {
    case Ar:
        if (archive_write_set_format_ar_bsd(mArchive)) {
            qCritical() << "Cannot set format to BSD ar.";
        }
        break;

    case TarGz:
        if (archive_write_set_compression_gzip(mArchive)) {
            qCritical() << "Cannot set compression method to gzip.";
        }
        if (archive_write_set_format_pax_restricted(mArchive)) {
            qCritical() << "Cannot set format to pax restricted.";
            return;
        }
        break;

    case Tar:
        if (archive_write_set_format_pax_restricted(mArchive)) {
            qCritical() << "Cannot set format to pax restricted.";
            return;
        }
        break;

    case Cpio:
        if (archive_write_set_compression_gzip(mArchive)) {
            qCritical() << "Cannot set compression method to gzip.";
        }
        if (archive_write_set_format_cpio_newc(mArchive)) {
            qCritical() << "Cannot set format to cpio.";
            return;
        }
        break;

    default:
        Q_ASSERT(false);
        qCritical() << "Unsupported archive format.";
    }

    if (archive_write_open_filename(mArchive, target.toAscii())) {
        qCritical() << "Cannot open" << target << "for writing.";
        return;
    }
    mValid = true;
}

Archiver::~Archiver()
{
    flush();
}

/** Flushes all the changes to disk.
  * After a call to this function all the contents are written from memory
  * buffers and stored to disk.
  * @return True if succesfully flushed, false otherwise.
  */
bool Archiver::flush()
{
    bool bRet = true;
    if (mEntry) {
        //TODO: add error checking for ret value
        archive_entry_free(mEntry);
        mEntry = NULL;
    }
    if (mArchive) {
        //TODO: add error checking for ret value
        archive_write_finish(mArchive);
        mArchive = NULL;
    }
    return bRet;
}

/** Used to query the state of the engine.
  * This is used internally and externally before operations are executed.
  *
  * @return True if the state is valid, false otherwise.
  */
bool Archiver::isValid() const
{
    if (!mValid) {
        qCritical() << "Archiver not in valid state.";
    }
    return mValid;
}

/** Adds directory recursively in to archive.
  *
  * @param directory Directory to be added.
  * @newPath New root for files found.
  * @return True if directory was succesfully added, false otherwise.
  */
bool Archiver::addDirectoryRecursively(const QString& directory, const QString& newPath)
{
    if (!isValid()) {
        return false;
    }
    QFileInfo fi(directory);
    if (!fi.isDir()) {
        qCritical() << directory << "is not a directory.";
        return false;
    }
    QDirIterator it(directory, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();

        // add entry as file
        if (it.fileInfo().isFile()) {
            QString storePath;
            storePath = it.filePath();
            storePath = newPath + storePath.mid(directory.size());
            //qDebug() << "Storing" << it.filePath() << "as" << storePath;
            addFile(it.filePath(), storePath);
        }
        // add entry as directory
        else if (it.fileInfo().isDir()) {
            QString storePath;
            storePath = it.filePath();
            storePath = storePath.mid(directory.size());
            storePath = newPath + storePath;
            addDirectory(it.filePath(), storePath);
        }
        // unsupported type
        else {
            qDebug() << "Archiver: Unsupported file entry." << it.fileName();
        }
    }
    return true;
}

/** Adds directory in to archive.
  *
  * @param directory Directory to be added.
  * @newName Prepended as new root for the directory.
  * @return True if directory was succesfully added, false otherwise.
  */
bool Archiver::addDirectory(const QString& directory, const QString& newName)
{
    if (!isValid()) {
        return false;
    }

    QString combinedPath(newName);

    stripPath(combinedPath);

    archive_entry_clear(mEntry);

    archive_entry_set_pathname(mEntry, combinedPath.toAscii());

    struct stat st;
    stat(directory.toAscii(), &st);

    archive_entry_copy_stat(mEntry, &st);

    // set file entry size
    archive_entry_set_size(mEntry, 4096);

    setOwner();

    // set mode from original
    archive_entry_set_mode(mEntry, st.st_mode);

    // set as directory
    archive_entry_set_filetype(mEntry, AE_IFDIR);

    writeHeader();

    writeFinish();

    return true;
}

/** Adds one file from file system to archive.
  * File is stored with its basename prepended by @param newPath.
  * Path is stored always relative thus a path like /var/tmp/foo.txt
  * will be stored as var/tmp/foo.txt in the archive.
  *
  * @param file File to be added to the archive.
  * @param newName Used as new path to the file.
  * @return True if file was succesfully added, false otherwise.
  */
bool Archiver::addFile(const QString& file, const QString& newName)
{
    if (!isValid()) {
        return false;
    }
    // open the file
    QFile f(file);
    bool status = f.open(QFile::ReadOnly);
    if (!status) {
        qCritical() << "Cannot open" << file << "for reading.";
        return false;
    }

    struct stat st;
    stat(f.fileName().toAscii(), &st);

    QString combinedPath(newName);

    stripPath(combinedPath);

    archive_entry_clear(mEntry);

    archive_entry_set_pathname(mEntry, combinedPath.toAscii());
    archive_entry_copy_stat(mEntry, &st);

    // set file entry size
    archive_entry_set_size(mEntry, f.size());

    setOwner();

    // set mode from original
    archive_entry_set_mode(mEntry, st.st_mode);

    writeHeader();

    // read the file in chunks to save memory when dealing with huge files
    //
    QByteArray buff = f.readAll();
    qint64 written = archive_write_data(mArchive, buff.constData(), buff.size());
    //qDebug() << "Wrote" << written << "bytes.";

/*
    char data[CHUNK_SIZE];
    qint64 size = 0;
    while ((size = f.read(data, CHUNK_SIZE)) > 0) {
        qint64 written = archive_write_data(mArchive, data, size);
        // qDebug() << "Wrote" << written << "bytes.";
    }
*/

    return writeFinish();
}

/** Adds one file from a memory buffer to the archive.
  *
  * @buffer Bytes containing the file contents.
  * @param filename Full path and filename to be used for the file.
  * @param mode Octal value of the POSIX file permissions (e.g 0644) for the
  * new file.
  */
bool Archiver::addFileFromBuffer(const QByteArray& buffer, const QString& filename, int mode)
{
    if (!isValid()) {
        return false;
    }

    // reuse the same for performance reasons
    archive_entry_clear(mEntry);

    QString newName(filename);
    stripPath(newName);

    archive_entry_set_pathname(mEntry, newName.toAscii());

    // set file entry size
    archive_entry_set_size(mEntry, buffer.size());

    // create regular file
    archive_entry_set_filetype(mEntry, AE_IFREG);

    // set permissions
    archive_entry_set_perm(mEntry, mode);

    setOwner();
    setTime();

    writeHeader();

    qint64 written = archive_write_data(mArchive, buffer.constData(), buffer.size());
    if (-1 == written) {
        qCritical() << "Couldn't write data to archive.";
        return false;
    }
    // qDebug() << "Wrote" << written << "bytes from buffer of" << buffer.size() << "bytes.";

    return writeFinish();
}

/** Makes sure path does not start with forward slash(es) and that the path is
  * otherwise valid. QFile can handle most of the problems but standard C doesn't.
  * @param path String to be cleaned to be valid path in this context.
  */
void Archiver::stripPath(QString& path) const
{
    // removes leading / from the path
    QRegExp r("^/+");
    path.replace(r, "");
    // replaces two or more / in the middle of the path with single one
    r.setPattern("/+");
    path.replace(r, "/");
}

/** Sets the uid and gid of the file inside archive.
  * If the uid or gid is longer than 6 digits then '1000' is
  * used if the archive format is ar. This is because
  * ar format limits the length of uid or gid to 6 decimal
  * digits.
  */
void Archiver::setOwner()
{
    // set textual user and group id of the entry
    archive_entry_set_gname(mEntry, "nokia");
    archive_entry_set_uname(mEntry, "nokia");
    
    // check if we are creating an ar archive and truncate uid and gid if
    // either one of them is too long for ar to handle
    // based on /usr/include/ar.h the limitation is 6 digits
    if(Ar == mArchiveType) {
        uid_t uid = archive_entry_uid(mEntry);
        gid_t gid = archive_entry_gid(mEntry);
        if(uid > 999999) {
            archive_entry_set_uid(mEntry, 1000);
        }
        if(gid > 999999) {
            archive_entry_set_gid(mEntry, 1000);
        }
    }
}

bool Archiver::writeFinish()
{
    // finally we write the entry to the archive
    int ret = archive_write_finish_entry(mArchive);
    if (ret < ARCHIVE_OK && ret != ARCHIVE_WARN) {
        qCritical() << "Couldn't finish writing the archive entry.";
        return false;
    }
    return true;
}

bool Archiver::writeHeader()
{
    // write the header for the entry
    int ret = archive_write_header(mArchive, mEntry);
    if (ret < ARCHIVE_OK && ret != ARCHIVE_WARN){
        qCritical() << "Couldn't write header.";
        return false;
    }
    return true;
}

void Archiver::setTime()
{
    // set timestamps
    QDateTime time = QDateTime::currentDateTime();
    archive_entry_set_ctime(mEntry, time.toTime_t(), 0);
    archive_entry_set_atime(mEntry, time.toTime_t(), 0);
    archive_entry_set_mtime(mEntry, time.toTime_t(), 0);
    archive_entry_set_birthtime(mEntry, time.toTime_t(), 0);
}
