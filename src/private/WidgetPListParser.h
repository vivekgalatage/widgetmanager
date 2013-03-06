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

#ifndef _INFOPLIST_PARSER_H_
#define _INFOPLIST_PARSER_H_

#include <QMap>
#include <QDir>

#include <wacWidgetProperties.h>

class WidgetInfo ;
class WidgetInfoPList : public WidgetInfo
{
public:

    WidgetInfoPList ();
    explicit WidgetInfoPList (const QString &infoPlistPath);

    ~WidgetInfoPList ();

    void setdir (const QDir &dir);
    const AttributeMap& getDictionary() const;
    bool process (const QString* infoPlistFile = NULL );
    bool processContents(const QString& contents);

    QString value(const QString &aKey ,
                         const QString &/*aAttr*/ = QString(""),
                         const QString &/*alang*/=QString("en") ) { return data.value (aKey).toString();}
    bool contains (const QString& key) const { return data.contains (key); }
    bool isValid() ;
    QString installedPath() const { return m_dir.absolutePath(); }
    virtual void setDictionary(AttributeMap* dict){ data = *dict;};
    virtual QSet<QString> languages();

protected:
    bool processInfoPList (const QString &infoPlistFile, bool isContents = false);

private:
    AttributeMap data;
    QDir m_dir; // this is the dir where the plist file is searched
};

#endif //_INFOPLIST_PARSER_H_
