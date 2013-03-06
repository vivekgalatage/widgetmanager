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

#ifndef _WAC_WIDGET_INFO_1_H_
#define _WAC_WIDGET_INFO_1_H_

#include <QMap>
#include <QDir>

#include <wacWidgetProperties.h>

/*****************************************************************************
 * Interface class for xml parsing (Plist/or ConfigXML Parsing)
 *
 * *************************************************************************/

class WidgetInfo
{
public:
    virtual ~WidgetInfo() {}
    virtual void setdir (const QDir &dir) = 0;
    virtual bool process (const QString* infoPlistFile = NULL)  = 0  ;
    virtual bool processContents(const QString& contents) = 0;
    virtual QString value(const QString &/*aKey*/ ,
                           const QString &/*aAttr*/ = QString(""),
                           const QString &/*alang*/=QString("") ) {return QString();}
    virtual QString value(const QString &/*aKey*/ ,
                           int /*pos*/ ,
                           const QString &/*aAttr*/ =QString("") ,
                           const QString &/*alang*/=QString("")) {return QString();}
    virtual void WriteManifestFile(const QString &/*path*/) {return;}
    virtual QString installedPath() const = 0  ;
    virtual bool isValid() = 0;
    virtual const AttributeMap& getDictionary() const  = 0;
    virtual void setDictionary(AttributeMap*)   = 0;
    virtual int count (const QString &/*aKey*/) { return 0 ;}
    virtual QSet<QString> languages() = 0;
};
#endif
