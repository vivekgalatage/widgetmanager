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


#include <QXmlStreamReader>
#include <QMap>
#include <QFile>
#include <QDir>
#include <wacWidgetProperties.h>
#include "wacWidgetInfo.h"
#include "WidgetLinkResolver.h"
#include "WidgetPListParser.h"

#include "WidgetUtilsLogs.h"

WidgetInfoPList::WidgetInfoPList ()
{
}

WidgetInfoPList::WidgetInfoPList (const QString &infoPlistPath)
{
    m_dir = QDir(infoPlistPath);
}

WidgetInfoPList::~WidgetInfoPList ()
{
}

/**
sets the directory which points to where the plist info
file can be found
*/
void WidgetInfoPList::setdir (const QDir &dir)
{
    LOG("WidgetInfoPList::setdir: " << dir.absolutePath());
    m_dir = dir;

}


/**
virtual function which is can be overriden by various platform
to do more processing than the usual.
*/
bool WidgetInfoPList::process (const QString *infoPlistFile)
{
    if (infoPlistFile == NULL && m_dir.exists()) {
        LOG("WidgetInfoPList::process : find info.plist in files");
        QFileInfoList tempExtract = m_dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        foreach (const QFileInfo &isWidgetDir, tempExtract )
        {
            if (!isWidgetDir.fileName().compare("info.plist", Qt::CaseInsensitive))
            {
                QString filePath = isWidgetDir.absoluteFilePath();
                LOG("WidgetInfoPList::process: " << filePath);
                if (!filePath.isEmpty())
                {
                    LOG("found info.plist");
                    LOG("WidgetInfoPList::process : found info.plist: " << filePath);
                    m_dir = QDir (filePath);
                    return processInfoPList(filePath);
                }
            }
        }
        LOG("WidgetInfoPList::process : find info.plist in dir");
        LOG("WidgetInfoPList::process : find info.plist" );
        tempExtract = m_dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
        foreach (const QFileInfo &isWidgetDir, tempExtract )
        {
            QString widgetPath = isWidgetDir.absoluteFilePath();
            QString filePath = WidgetLinkResolver::resolveLink (widgetPath, "info.plist");
            if (!filePath.isEmpty()) {
                LOG("WidgetInfoPList::process : found info.plist: " << filePath);
                m_dir = QDir (widgetPath);
                return processInfoPList(filePath);
            }
        }

        return false;
    }

    if (infoPlistFile == NULL) return false;
    return processInfoPList(*infoPlistFile);
}

bool WidgetInfoPList::processContents(const QString& contents)
{
    return processInfoPList(contents, true);
}

const AttributeMap& WidgetInfoPList::getDictionary() const
{
    return data;
}


bool WidgetInfoPList::isValid()
{
    if (
        data.value (MainHTML).toString().isNull() || data.value (MainHTML).toString().isEmpty() ||
        data.value (DisplayName).toString().isNull() || data.value (DisplayName).toString().isEmpty() ||
        data.value (Identifier).toString().isNull() || data.value (Identifier).toString().isEmpty()
       )
    {
        LOG("WidgetInfoPList::isValid : INVALID widget:: mandatory fields missing ");
        return false;
    }

    // check if MainHTML is present
    QFileInfo mainHtml (WidgetLinkResolver::resolveLink (m_dir.absolutePath(), data.value (MainHTML).toString()));
    if (!mainHtml.exists()) {
        LOG("WidgetInfoPList::isValid : INVALID widget: main html not found!! -" << mainHtml.absoluteFilePath());
        return false;
    }

    LOG("WidgetInfoPList::isValid : VALID widget ");
    return true;
}

bool WidgetInfoPList::processInfoPList (const QString &infoPlistFile, bool isContents)
{
    QXmlStreamReader reader;
    QFile file(infoPlistFile);

    if (!isContents) {
        LOG("infoplist = " << infoPlistFile);

        if (file.open(QIODevice::ReadOnly)) {
            reader.setDevice(&file);
        } else {
            return true;
        }
    } else {
        reader.addData(infoPlistFile);
    }

    QString key;
    bool isKey = false;
    bool validPlist = false;
    
    //coverity[dead_error_line]
    bool yetToStore = false;
    LOG(" ******** opened infoPlistfile *******");

    while ( !reader.atEnd() )
        {
            reader.readNext();
            switch (reader.tokenType())
                {
                case QXmlStreamReader::StartElement:
                    {
                        if (validPlist && "key" == reader.name())
                            isKey = true;
                        else if (validPlist && isKey )
                            isKey = false;
                        else if ("plist" == reader.name())
                            validPlist = true;

                        break;
                    }

                case QXmlStreamReader::Characters:
                    {
                        QString temp (reader.text().toString().trimmed());

                        if (temp.isEmpty()) break;

                        if (validPlist && isKey)
                            {
                                yetToStore = true;
                                key = temp;
                            }
                        //coverity[dead_error_line]
                        else if (validPlist && yetToStore && !isKey)
                            {
                                yetToStore = false;
                                LOG("dat = " << temp);
                                data [key] = temp;
                            }
                        break;
                    }

                case QXmlStreamReader::EndElement:
                    {
                        if (validPlist && yetToStore && !isKey)
                            {
                                yetToStore = false;
                                if (reader.name().toString() == "true" || reader.name().toString() == "false")
                                    {
                                        data [key] = reader.name().toString();
                                        LOG("dat = " << reader.name().toString());
                                    }

                            }
                        break;
                    }

                default: break;
                }
        }

    reader.clear();

    if (file.isOpen())
        file.close();

    LOG("WidgetInfoPList::processInfoPList result \nIdentifier:" << data[Identifier]);
    LOG("DisplayName:" << data[DisplayName]);
    LOG("MainHTML:" << data[MainHTML]);
    LOG("Version:" << data[Version]);
    //LOG("OriginURL:" << data[OriginUrl]);

    LOG("WidgetInfoPList::processInfoPList result \nIdentifier:" << data[Identifier]);
    LOG("DisplayName:" << data[DisplayName]);
    LOG("MainHTML:" << data[MainHTML]);
    LOG("Version:" << data[Version]);


    return true;
}

QSet<QString> WidgetInfoPList::languages()
{
    QSet<QString> s;
    return s;
}
