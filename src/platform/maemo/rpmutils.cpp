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


#include "rpmutils.h"
#include "archiver.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>

#include <cstdlib>
#include <arpa/inet.h>
#include <sys/stat.h>


const int TYPE_ALIGN[16] = {
    1,	/*!< RPM_NULL_TYPE */
    1,	/*!< RPM_CHAR_TYPE */
    1,	/*!< RPM_INT8_TYPE */
    2,	/*!< RPM_INT16_TYPE */
    4,	/*!< RPM_INT32_TYPE */
    8,	/*!< RPM_INT64_TYPE */
    1,	/*!< RPM_STRING_TYPE */
    1,	/*!< RPM_BIN_TYPE */
    1,	/*!< RPM_STRING_ARRAY_TYPE */
    1,	/*!< RPM_I18NSTRING_TYPE */
};

#define RPMLEAD_SIZE 96
#define RPMHEADER_VERSION 1
#define RPMHEADER_SIGNATURES 62

#define RPMSIGTAG_SIZE 1000
#define RPMSIGTAG_MD5 1001
#define RPMSIGTAG_PGP 1002
#define RPMSIGTAG_SIZE 1000

#define RPMTAG_IMMUTABLE 63
#define RPMTAG_NAME 1000
#define RPMTAG_VERSION 1001
#define RPMTAG_DESCRIPTION 1005
#define RPMTAG_SIZE 1009
#define RPMTAG_VENDOR 1011
#define RPMTAG_GROUP 1016
#define RPMTAG_OS 1021
#define RPMTAG_ARCHITECTURE 1022
#define RPMTAG_PREINST 1023
#define RPMTAG_POSTINST 1024
#define RPMTAG_PREUN 1025
#define RPMTAG_POSTUN 1026
#define RPMTAG_FILESIZES 1028
#define RPMTAG_FILEMODES 1030
#define RPMTAG_DIRINDEXES 1116
#define RPMTAG_BASENAMES 1117
#define RPMTAG_DIRNAMES 1118

#define RPMTYPE_NULL 0x00
#define RPMTYPE_CHAR 0x01
#define RPMTYPE_INT8 0x02
#define RPMTYPE_INT16 0x03
#define RPMTYPE_INT32 0x04
#define RPMTYPE_INT64 0x05
#define RPMTYPE_STRING 0x06
#define RPMTYPE_BIN 0x07
#define RPMTYPE_STRING_ARRAY 0x08
#define RPMTYPE_I18NSTRING 0x09


RpmUtils::RpmUtils(SuperWidget* widget, const QString packageName, const QString& sourcePath, const QString& installationPath, const QString& appId)
    : PackageUtils(widget, packageName, sourcePath, installationPath, appId)
{
    m_packageFileName = m_packageName + ".rpm";
    m_packageFilePath = m_packageRootPath + QDir::separator() + m_packageFileName;

    m_index = 0;
    m_rpmSize = 0;
}

RpmUtils::~RpmUtils()
{
    foreach(struct indexentry entry, m_header.entries)
        free(entry.data);

    foreach(struct indexentry entry, m_signature.entries)
        free(entry.data);
}

bool RpmUtils::createPackage()
{
    if (!preCreatePackage())
        return false;

    populateLead();
    populateRpmHeader(m_header);
    populateRpmHeader(m_signature);

    m_fd.setFileName(m_packageRootPath + QDir::separator() + m_packageName + QString(".rpm"));
    if (!m_fd.open(QIODevice::WriteOnly)) {
        qCritical() << "Cannot create rpm file" << m_packageName << ".rpm";
        return false;
    }

    if (!createCpioFile(m_packageRootPath, m_packageRootPath)) {
        qCritical() << "Cannot create Cpio data";
        return false;
    }

    bool bRet = true;
    int *index = createDirIndex(m_filesData.size());
    unsigned int immutable[4];
    immutable[0] = htonl(0x3e);
    immutable[1] = htonl(0x7);
    immutable[2] = calculateOffsetForImmutable(1);
    immutable[3] = htonl(0x10);
    bRet = addEntryToHeader(m_signature, RPMHEADER_SIGNATURES, RPMTYPE_BIN, (void *)immutable, sizeof(immutable));

    immutable[0] = htonl(0x3f);
    immutable[2] = calculateOffsetForImmutable(16);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_IMMUTABLE, RPMTYPE_BIN, (void *)immutable,
                                    sizeof(immutable));
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_NAME, RPMTYPE_STRING, (void *)m_packageName.toLocal8Bit().data(),
                                    m_packageName.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_VERSION, RPMTYPE_STRING, (void *)DEFAULT_VERSION.toLocal8Bit().data(),
                                        DEFAULT_VERSION.size() + 1);

    if (m_appDescription.isEmpty())
        bRet = bRet && addEntryToHeader(m_header, RPMTAG_DESCRIPTION, RPMTYPE_I18NSTRING, (void *)DEFAULT_DESCRIPTION.toLocal8Bit().data(),
                                        DEFAULT_DESCRIPTION.size());
    else
        bRet = bRet && addEntryToHeader(m_header, RPMTAG_DESCRIPTION, RPMTYPE_I18NSTRING, (void *)m_appDescription.toLocal8Bit().data(),
                                        m_appDescription.size() + 1);

    bRet = bRet && addEntryToHeader(m_header, RPMTAG_SIZE, RPMTYPE_INT32, (void *)&m_rpmSize, sizeof(int), 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_VENDOR, RPMTYPE_STRING, (void *)DEFAULT_VENDOR.toLocal8Bit().data(),
                                    DEFAULT_VENDOR.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_GROUP, RPMTYPE_STRING, (void *)DEFAULT_GROUP.toLocal8Bit().data(),
                                    DEFAULT_GROUP.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_OS, RPMTYPE_STRING, (void *)DEFAULT_OS.toLocal8Bit().data(),
                                    DEFAULT_OS.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_ARCHITECTURE, RPMTYPE_STRING, (void *)DEFAULT_ARCHITECTURE.toLocal8Bit().data(),
                                    DEFAULT_ARCHITECTURE.size() + 1);

    bRet = bRet && addEntryToHeader(m_header, RPMTAG_POSTINST, RPMTYPE_STRING, (void *)m_postinstScript.data(), m_postinstScript.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_POSTUN, RPMTYPE_STRING, (void *)m_postrmScript.data(), m_postrmScript.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_PREUN, RPMTYPE_STRING, (void *)m_prermScript.data(), m_prermScript.size() + 1);
    bRet = bRet && addEntryFilesSize();
    bRet = bRet && addEntryFilesMode();
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_DIRINDEXES, RPMTYPE_INT32, (void *)index, m_filesData.size() * sizeof(int),
                                    m_filesData.size());
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_BASENAMES, RPMTYPE_STRING_ARRAY, (void *)m_files.toLocal8Bit().data(),
                                    m_files.size() + 1);
    bRet = bRet && addEntryToHeader(m_header, RPMTAG_DIRNAMES, RPMTYPE_STRING_ARRAY, (void *)m_dirs.toLocal8Bit().data(),
                                    m_dirs.size() + 1);

    delete index;

    if (!bRet) {
        qCritical() << "FAIL: creating rpm headers";
        m_fd.close();
        return false;
    }
    if (!addLead()) {
        qCritical() << "FAIL: adding struct Lead to rpm";
        m_fd.close();
        return false;
    }
    if (!addHeader(m_signature)) {
        qCritical() << "FAIL: adding signature area to rpm";
        m_fd.close();
        return false;
    }
    if (!addHeader(m_header)) {
        qCritical() << "FAIL: adding header area to rpm";
        m_fd.close();
        return false;
    }
    if (!addCpioData()) {
        qCritical() << "FAIL: adding cpio data to rpm";
        m_fd.close();
        return false;
    }

    m_fd.close();
    return true;

    return true;
}

void RpmUtils::populateLead()
{
    unsigned int magic = htonl(0xedabeedb);
    memcpy(m_lead.magic, &magic, 4);
    memset(m_lead.name, '\0', 66);

    if (m_appName.size() < 66) {
        memcpy(m_lead.name, m_appName.toAscii(), m_appName.size());
    } else {
        memcpy(m_lead.name, m_appName.toAscii().data(), 65);
    }

    m_lead.major = 0x03;
    m_lead.minor = 0x00;
    m_lead.type = 0x00;
    m_lead.archnum = 0x00;
    m_lead.osnum = 0x00;

    m_lead.signature_type = 0x0500; //Network byte order
    memset(m_lead.reserved, 0x0, 16);
}

void RpmUtils::populateRpmHeader(struct rpmheader &head)
{
    head.magic[0] = 0x8e;
    head.magic[1] = 0xad;
    head.magic[2] = 0xe8;
    head.headerversion = 0x01;
    memset(head.mustbezero, 0, 4);
    head.signaturesize = 0;
    head.numberofentries = 0;
}

void RpmUtils::calculateAlign(const int type)
{
    if (!(m_header.entries.size()))
        return;

    int off = (m_header.signaturesize) & (TYPE_ALIGN[type] - 1);
    off = (off) ? 4 - off : off;

    if (off) {
        char aligncontents[8] = {0};
        struct indexentry entry = m_header.entries.takeLast();
        entry.data = realloc(entry.data, entry.datasize + off);
        memcpy(((char *)entry.data + entry.datasize), (void *)aligncontents, off);
        m_header.signaturesize += off;
        entry.datasize += off;
        m_header.entries.append(entry);
    }
}

bool RpmUtils::addEntryToHeader(struct rpmheader &header, int tag, int type, void *data, unsigned int datasize, int count)
{
    calculateAlign(type);
    header.numberofentries++;

    struct indexentry entry;
    entry.tag = tag;
    entry.type = type;

    switch (type) {
        case RPMTYPE_STRING:
            entry.count = 0x01;
            break;
        case RPMTYPE_STRING_ARRAY:
            entry.count = 0;
            for (int i = 0; i < datasize; i++) {
                if ((char)*((char *)data + i) == ' ') {
                    entry.count++;
                    *((char *)data + i) = 0;
                }
            }
            entry.count++;
            break;
        case RPMTYPE_BIN:
            entry.count = 0x10;
            break;
        case RPMTYPE_I18NSTRING:
            entry.count = 0x01;
            break;
        default:
            entry.count = count;
            break;
    }

    entry.datasize = datasize;
    entry.data = malloc(datasize);
    if (!entry.data)
        return false;

    memcpy(entry.data, data, datasize);
    entry.offset = header.signaturesize;
    (header.entries).append(entry);
    header.signaturesize += datasize;

    return true;
}

bool RpmUtils::addEntryFilesSize()
{
    int size = m_filesData.size();
    if (!size)
        return true;

    int *sizes = new int[size];
    if (!sizes)
        return false;

    for (int i = 0; i < size; i++)
        sizes[i] = m_filesData.at(i).filesize;

    bool ret = true;
    ret = addEntryToHeader(m_header, RPMTAG_FILESIZES, RPMTYPE_INT32, (void *)sizes,
                           size * sizeof(int), size);
    delete sizes;
    return ret;
}

bool RpmUtils::addEntryFilesMode()
{
    int size = m_filesData.size();
    short *modes = new short[size];
    if (!modes)
        return false;

    for (int i = 0; i < size; i++)
        modes[i] = m_filesData.at(i).filemode;

    bool ret = true;
    ret = addEntryToHeader(m_header, RPMTAG_FILEMODES, RPMTYPE_INT16, (void *)modes,
                           size * sizeof(short), size);
    delete modes;
    return ret;
}

bool LessThan(const struct filesdata data1, const struct filesdata data2)
{
    return  (data1.path + data1.filename) < (data2.path + data2.filename);
}

void RpmUtils::sortDirsAndFiles()
{
    qSort(m_filesData.begin(), m_filesData.end(), LessThan);

    foreach(struct filesdata data, m_filesData) {
        m_dirs.append(data.path);
        m_dirs.append(" ");
        m_files.append(data.filename);
        m_files.append(" ");
    }

    m_dirs = m_dirs.left(m_dirs.size() - 1);
    m_files = m_files.left(m_files.size() - 1);
}

bool RpmUtils::addDirectoryRecursively(Archiver& pack, const QString& root, const QString& newPath)
{
    QFileInfo fi(root);
    if (!fi.isDir()) {
        qCritical() << root << "is not a root.";
        return false;
    }
    QDirIterator it(root, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();

        // add entry as file
        if (it.fileInfo().isFile()) {
            QString storePath, filename;
            filename = it.fileName();
            storePath = it.filePath();
            storePath = newPath + storePath.mid(root.size());
            addFile(it, storePath.mid(1, (storePath.size() - filename.size() - 1)));
            pack.addFile(it.filePath(), storePath.right(storePath.size() - 1));
        } else if (!it.fileInfo().isDir()) {
            qDebug() << "Packager: Unsupported file entry." << it.fileName();
        }
    }

    return true;
}

void RpmUtils::addFile(QDirIterator &it, const QString &dir)
{
    struct filesdata file;
    file.filename = it.fileName();
    file.owner = QString("root");
    file.path = dir;
    struct stat st;
    stat(it.filePath().toAscii(), &st);
    file.filemode = htons(st.st_mode);
    file.filesize = it.fileInfo().size();
    m_filesData.append(file);

    // Add the file size to rpm size in Kb
    m_rpmSize += htonl(file.filesize / 1024);
}

bool RpmUtils::createCpioFile(const QString& rpmRootDir, const QString& outputPath)
{
    bool bRet = true;
    m_cpioData = outputPath + "/data.cpio";
    Archiver data(m_cpioData, Archiver::Cpio);
    if (!data.isValid())
        return false;

    QStringList datafiles = QDir(rpmRootDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QString fileinfo, datafiles) {
        if (bRet && fileinfo != "." && fileinfo != "..") {
            bRet = addDirectoryRecursively(data, rpmRootDir + "/" + fileinfo,
                                           "./" + fileinfo);
        }
    }

    sortDirsAndFiles();
    bRet = bRet  && data.flush();
    return bRet;
}

int RpmUtils::calculateOffsetForImmutable(int number_of_entries)
{
    unsigned int off;
    off = 0xffffffff - ((number_of_entries * 16) - 1);
    return htonl(off);
}

bool RpmUtils::addCpioData()
{
    QFile data(m_cpioData);
    if (!data.open(QIODevice::ReadOnly))
        return false;

    while (!data.atEnd()) {
        QByteArray line = data.readLine();
        m_fd.write(line);
    }

    data.close();
    return true;
}

int* RpmUtils::createDirIndex(int size)
{
    int *index = new int[size];
    if (!index)
        return NULL;

    for (int i = 0; i < size; i++)
        index[i] = htonl(i);

    return index;
}

bool RpmUtils::addLead()
{
    if (!m_fd.isOpen())
        return false;

    qint64 leadsize = sizeof(m_lead);
    if (m_fd.write((char *)&m_lead, leadsize) != leadsize)
        return false;

    return true;
}

void RpmUtils::fixRpmName()
{
    for (int i = 0; i < m_packageName.size(); i++) {
        QChar c = m_packageName.at(i);
        if (!c.isLetterOrNumber()){
            m_packageName.replace(i, 1, "_");
        }
    }
}

bool RpmUtils::addHeader(struct rpmheader header)
{
    if (!m_fd.isOpen())
        return false;

    header.numberofentries = htonl(header.numberofentries);
    header.signaturesize = htonl(header.signaturesize);

    m_fd.write((char *)(header.magic), 3);
    m_fd.write((char *)&(header.headerversion), 1);
    m_fd.write((char *)(header.mustbezero), 4);
    m_fd.write((char *)&(header.numberofentries), 4);
    m_fd.write((char *)&(header.signaturesize), 4);

    //Writing the entries
    foreach (struct indexentry entry, header.entries) {
        entry.tag = htonl(entry.tag);
        entry.type = htonl(entry.type);
        entry.offset = htonl(entry.offset);
        entry.count = htonl(entry.count);
        m_fd.write((char *)&(entry.tag), 4);
        m_fd.write((char *)&(entry.type), 4);
        m_fd.write((char *)&(entry.offset), 4);
        m_fd.write((char *)&(entry.count), 4);
    }

    //Now write the data
    foreach (struct indexentry entry, header.entries)
        m_fd.write((char *)(entry.data), entry.datasize);

    return true;
}
