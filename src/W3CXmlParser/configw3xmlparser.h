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

#ifndef _CONFW3XMLPARSER_H
#define _CONFW3XMLPARSER_H

#include "wacWidgetUtils.h"
#include <QXmlStreamReader>
#include <QSet>
#include <QMap>
#include <QString>

#include "configw3xmlparser_p.h"

class W3CElement;
class QStringList;
class QXmlStreamReader;

#define XMLNS_STRING "http://www.w3.org/ns/widgets"
#define XMLNS_JIL_STRING "http://www.jil.org/ns/widgets1.2"

class WIDGETUTILS_EXPORT ConfigXmlParser
{
public:
    ConfigXmlParser(const QString &confFile = QString());
    ~ConfigXmlParser();
    void setFile(const QString &afile);
    void setContents(const QString &aContents);
    bool parseFile();
    
    QString namespaceUri();
    QString id();
    QString version();
    QString language() const;
    QString height() const;
    QString width() const;
    QString viewmodes() const;
    QString widgetType() const;
    QStringList allKeys();
    QSet<QString> languages() const;
    bool contains(const QString &aKey , QString *aLang = NULL) const;
    int count() const;
    int count(const QString &aKey , QString *lang = NULL) const;

    // methods used by USIF installation
    /**
     * Owner ship is not transfred for returned address
     * ConfigXML parser owns all the returned objects
     **/
    const W3CElement *getElement(const QString &aKey, const QString &aLang=QString("") ) const;
    const W3CElement *getElement(const QString &aKey, int pos, const QString &aLang= QString("")) const;

private:
//    ConfigXmlParser(const ConfigXmlParser &aParser);
//    ConfigXmlParser & operator=(const ConfigXmlParser &aParser);

private:
    ConfigXmlParserPrivate* d;
    friend class ConfigXmlParserPrivate;
};

#endif
