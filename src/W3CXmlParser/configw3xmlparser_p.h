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

#ifndef _CONFW3XMLPARSERPRIVATE_H
#define _CONFW3XMLPARSERPRIVATE_H

#include "wacWidgetUtils.h"
#include <QXmlStreamReader>
#include <QSet>
#include <QMap>
#include <QString>

class W3CElement;
class QStringList;
class QXmlStreamReader;

class ConfigXmlParser;

typedef QMap<QString , W3CElement *> ConfigContainer;
//typedef QMap<QString , QVariant> WidgetAttributesMap;

class ConfigXmlParserPrivate
{
public:
    explicit ConfigXmlParserPrivate(ConfigXmlParser*);
    ~ConfigXmlParserPrivate();

public:
    // internal utility functions
    bool parseWidgetAccessElement();
    bool parseFeatureElement();
    bool parseParamElement();
    bool parsePreference();
    bool parseAuthorElement();
    bool parseIconElement();
    bool parseServerElement();
    bool parseDeltaElement();
    bool parseNameElement();
    bool parseDescriptionElement();
    bool parseLicenseElement();
    bool parseContentElement();
    bool parseSharedLibraryElement();
    bool parseBackgroundTimerElement();
    bool parseViewmodeSettingsElement();
    bool isLocalizable(const QString &aString) const;
    bool parseHiddenElement();
    bool parsePreInstallElement();
    bool parseCategoriesElement();

public:
    QString m_lang;
    QString m_version;
    QString m_id;
    QString m_namespace ;
    QString m_height;
    QString m_width;
    QString m_viewmodes;
    QString m_widgetType;

    QString m_file ;
    QString m_contents;
    QString m_services;
    QXmlStreamReader m_xmlreader;
    ConfigContainer m_container;
    bool m_parseFromFile;
    bool m_isFeatureTagPrevElement;
    bool m_isSharedLibraryTagPrevElement;
    bool m_isViewModeSettingsTagPrevElement;
    bool m_isAllUrls;
    bool m_isFirstJilAccessTag;
    QSet<QString> m_languages;

    //occurence counts
    int m_accessCount;
    int m_featureCount;
    int m_paramCount;
    int m_paramTotalCount;
    int m_preferenceCount;
    int m_iconCount;
    
private:
    ConfigXmlParser* q;
};

#endif // _CONFW3XMLPARSERPRIVATE_H
