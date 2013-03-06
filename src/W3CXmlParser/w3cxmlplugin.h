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



#ifndef __W3CXMLPLUGIN1_H__
#define __W3CXMLPLUGIN1_H__

#include <QMetaType>
#include <QVariant>

#include "wacWidgetInfo.h"
#include "configw3xmlparser.h"

class QString;
class WidgetInfo;
class ConfigXmlParser;
class W3CElement;

class W3cXmlPlugin : public WidgetInfo
{
public:
    W3cXmlPlugin();
    ~W3cXmlPlugin();
    bool process(const QString *file) ;
    bool processContents(const QString& contents);
    QString installedPath() const { return m_dir.absolutePath();}
    void setdir (const QDir &dir) ;
    bool isValid() { return true ;}
    const AttributeMap& getDictionary() const;
    virtual void setDictionary(AttributeMap* dict){ m_data = dict;};
    virtual QString value(const QString &aKey , const QString &aAttr = QString("")
                           , const QString &alang=QString("") ) ;
    virtual QString value(const QString &aKey , int pos ,
                           const QString &aAttr=QString(""), const QString &alang=QString("")) ;
    virtual int  count(const QString &aKey);
    virtual void WriteManifestFile(const QString &aPath);
    virtual QSet<QString> languages();

private:
    void setAttributeMap();

private:
    QString m_confFile;
    QDir    m_dir;
    AttributeMap *m_data ;
    ////////////////////////////
    //object owned by W3cXmlPlugin
    ////////////////////////////
    AttributeMap *m_metadata ;
    ConfigXmlParser *m_parser;
};
Q_DECLARE_METATYPE(const W3CElement *)
#endif

