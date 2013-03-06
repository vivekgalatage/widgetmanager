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
#include <QFile>
#include <QXmlStreamAttributes>
#include <QXmlStreamNamespaceDeclaration>
#include <QVariant>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QMapIterator>
#include <QUrl>
#include <QLocale>

#include "wacsettings.h"
#include "w3ctags.h"
#include "proprietarytags.h"
#include "wacw3csettingskeys.h"
#include "proprietarysettingskeys.h"
#include "configw3xmlparser.h"
#include "wacw3celement.h"
#include "WidgetUtilsLogs.h"
#include "wacwidgetmanagerconstants.h"

/*const int  NAME_ATTR_COUNT =  3;
const int  AUTH_ATTR_COUNT =  4;
const int  DES_ATTR_COUNT  =  2;
const int  LIC_ATTR_COUNT  =  3;
const int  CNT_ATTR_COUNT  =  3;
const int  FTR_ATTR_COUNT  =  2;
const int  PRF_ATTR_COUNT  =  4;
const int  SLIB_ATTR_COUNT =  2;
*/

/**************************************************
 * Private methods of class ConfigXmlParserPrivate
 * ************************************************/

ConfigXmlParserPrivate::ConfigXmlParserPrivate(ConfigXmlParser* configXmlParser)
    : q(configXmlParser)
{
}

ConfigXmlParserPrivate::~ConfigXmlParserPrivate()
{
}

bool ConfigXmlParserPrivate::parseNameElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    QString key = simplifiedAttributes.value("lang").toString();
    if (key.isEmpty())
        key = W3CSettingsKey::WIDGET_LANGUAGE_KEY;
    else
        m_languages.insert(key.toLower());
    key += QString("/") + W3CSettingsKey::WIDGET_NAME;
    key = key.toLower().replace(QString("_"), QString("-"));

    // ignore if it's not the first one in this locale
    if (m_container.contains(key))
        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText(QXmlStreamReader::IncludeChildElements).simplified());
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(key, element);
    return true;
}


/***************************************************************************************************
 * parseDescriptionElement(): Parses the 'description' element from config.xml file
 * ContentModel: Any
 * Occurrences: Zero or more(one element is allowed per language)
 * W3Specs for description element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseDescriptionElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    QString key = simplifiedAttributes.value("lang").toString();
    if (key.isEmpty())
        key = W3CSettingsKey::WIDGET_LANGUAGE_KEY;
    else
        m_languages.insert(key.toLower());
    key += QString("/") + W3CSettingsKey::WIDGET_DESCRIPTION;
    key = key.toLower().replace(QString("_"), QString("-"));

    // ignore if it's not the first one in this locale
    if (m_container.contains(key))
        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText(QXmlStreamReader::IncludeChildElements).simplified());
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(key, element);
    return true;
}


/***************************************************************************************************
 * parseAuthorElement(): Parses 'author' element from config.xml file
 * ContentModel: Any
 * Occurrences: Zero or more(only first instance shall be used)
 * W3Specs for author element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseAuthorElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    // ignore if it's not the first one
    if (m_container.contains(W3CSettingsKey::WIDGET_AUTHOR))
        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText(QXmlStreamReader::IncludeChildElements).simplified());
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(W3CSettingsKey::WIDGET_AUTHOR, element);
    return true;
}


/***************************************************************************************************
 * parseLicenseElement(): Parses 'license' element from config.xml file
 * ContentModel: Any
 * Occurrences: Zero or more(one element is allowed per language)
 * W3Specs for license element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseLicenseElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    QString key = simplifiedAttributes.value("lang").toString();
    if (key.isEmpty())
        key = m_lang;
    m_languages.insert(key.toLower());
    key += QString("/") + W3CSettingsKey::WIDGET_LICENSE;
    key = key.toLower().replace(QString("_"), QString("-"));

    // ignore if it's not the first one in this locale
    if (m_container.contains(key))
        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText(QXmlStreamReader::IncludeChildElements).simplified());
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(key, element);
    return true;
}


/***************************************************************************************************
 * parseIconElement(): Parses 'icon' element from config.xml file
 * ContentModel: Empty
 * Occurrences: Zero or more(elements are grouped by language)
 * W3Specs for Name element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseIconElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    // ignore if the src attribute is missing
    if (simplifiedAttributes.value("src").isEmpty())
        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(W3CSettingsKey::WIDGET_ICON.arg(++m_iconCount), element);
    return true;
}


/***************************************************************************************************
 * parseServerElement(): Parses Server  element Tag from config.xml file
 * ContentModel: Empty
 * Occurrences:
 * W3Specs for Server element (refer http://dev.w3.org/2006/waf/widgets/  for more details.)
 * *************************************************************************************************/

bool ConfigXmlParserPrivate::parseServerElement()
{
  QXmlStreamAttributes attr = m_xmlreader.attributes();

  //This element is not localizable
  QString elemText  = m_xmlreader.readElementText();
  QString href = attr.value("href").toString();

  W3CElement *serverElement = new W3CElement();
  serverElement->setAttributes(attr);
  serverElement->setText(elemText);
  serverElement->setnamespaceUri(m_xmlreader.namespaceUri().toString());

  m_container.insert(W3CSettingsKey::WIDGET_SERVER, serverElement);

  return true;
}

/***************************************************************************************************
 * parseDeltaElement(): Parses Delta  element Tag from config.xml file
 * ContentModel: Empty
 * Occurrences:
 * W3Specs for delta element (refer http://dev.w3.org/2006/waf/widgets/  for more details.)
 * *************************************************************************************************/

bool ConfigXmlParserPrivate::parseDeltaElement()
{
  QXmlStreamAttributes attr = m_xmlreader.attributes();

  //This element is not localizable

  QString elemText  = m_xmlreader.readElementText();
  QString version = attr.value("version").toString();

  if (version.isEmpty())   {
     //as per specs src attributed is Required
     return false;
  }

  W3CElement *deltaElement = new W3CElement();
  deltaElement->setAttributes(attr);
  deltaElement->setnamespaceUri(m_xmlreader.namespaceUri().toString());

  m_container.insert(W3CSettingsKey::WIDGET_DELTA, deltaElement);

  return true;
}


/***************************************************************************************************
 * parseContentElement(): Parses 'content' element from config.xml file
 * ContentModel: Empty
 * Occurrences: Zero or more
 * W3Specs for content element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseContentElement()
{
    //ignore if this is not the first content element in the xml
    if (m_container.contains(W3CSettingsKey::WIDGET_CONTENT))
        return true;

    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    W3CElement *element = new W3CElement;
    element->setAttributes(simplifiedAttributes);
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(W3CSettingsKey::WIDGET_CONTENT, element);
    return true;
}


/***************************************************************************************************
 * parseAccessElement(): Parses Access  element Tag from config.xml file
 * ContentModel: Empty
 * Occurrences: Zero or more
 * W3Specs for Name element (refer http://dev.w3.org/2006/waf/widgets/  for more details.)
 * *************************************************************************************************/

bool ConfigXmlParserPrivate::parseWidgetAccessElement()
{
  QXmlStreamAttributes attrs = m_xmlreader.attributes();
  QString origin  = attrs.value("origin").toString().trimmed();
  QString network  = attrs.value("network").toString().trimmed();
  QString ns = m_xmlreader.prefix().toString().toLower();
  QUrl url(origin);
  bool nonW3cWidget = false;

  // is JIL access tag?
  if (!ns.isEmpty() && ns == "jil") {
      if (!m_isFirstJilAccessTag) {
          //only the first occurance of jil access tag is considered.
          return true;
      }
      nonW3cWidget = true;
      m_isFirstJilAccessTag = false;
  }

  if (!nonW3cWidget && m_isAllUrls) {
      //ignore all the consequent elements if any of the
      //elements origin has value as "*"
      return true;
  }

  if (!nonW3cWidget && (origin.isEmpty() || !url.isValid()))   {
     //This tag is invalid if origin attribute is empty
     //as per w3 specs
     return true;
  }

  QString subDomains = attrs.value("subdomains").toString();
  if (!nonW3cWidget && (!subDomains.isEmpty() && !(subDomains == "false" || subDomains == "true"))) {
      //this tag is invalid
      return true;
  }

  if (origin == "*")
    m_isAllUrls = true;


 ///////////////////////////////////////////////////////////
 // generated Key
 // widget/%1/access  (where %1 could be replaced with the
 // occurence number of access tag in config.xml file
 //////////////////////////////////////////////////////////

  W3CElement *accessElement = new W3CElement;
  accessElement->setAttributes(attrs);
  accessElement->setnamespaceUri(ns);
  m_container.insert(QString(W3CSettingsKey::WIDGET_ACCESS.arg(++m_accessCount)) , accessElement);
  return true;
}


/***************************************************************************************************
 * parseFeatureElement(): Parses 'feature' element from config.xml file
 * ContentModel: Empty
 * Occurrences: Zero or more(one element is allowed per language)
 * W3Specs for feature element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseFeatureElement()
{
    bool hasRequired = false;

    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        QString name = attribute.name().toString();
        QString value = attribute.value().toString().simplified();
        if (name == "required") {
            hasRequired = true;
            if (value != "false")
                value = "true";
        }
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    name,
                                    value);
    }

    if (!hasRequired)
        simplifiedAttributes.append("required", "true");

    QString name = simplifiedAttributes.value("name").toString().simplified();
    // ignore if the name attribute is missing
    if (name.isEmpty())
        return true;

    // ignore if already existed
    for (int i = 1; i <= m_featureCount; i++) {
        if (name == q->getElement(W3CSettingsKey::WIDGET_FEATURE, i)->attributeValue("name"))
            return true;
    }

    m_paramCount = 0;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(W3CSettingsKey::WIDGET_FEATURE.arg(++m_featureCount) , element);

    //set the m_isFeatureTagPrevElement flag so that the readNext does not get called twice.
    m_isFeatureTagPrevElement = true;
    m_xmlreader.readNext();

    while ((m_xmlreader.name() != W3CTags::FEATURE_TAG) || (!m_xmlreader.isEndElement())) {
        if (m_xmlreader.name() == W3CTags::PARAM_TAG) {
            // ignore and move to the next element on errors
            if (!parseParamElement())
               LOG("One of the Param elementn is invalid");
        }
        m_xmlreader.readNext();
    }

    return true;
}


/***************************************************************************************************
 * parseParamElement(): Parses the 'param' element from config.xml file
 * ContentModel: Empty
 * Occurrences: Zero or more
 * W3Specs for param element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parseParamElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    if (simplifiedAttributes.value("name").toString().simplified().isEmpty()
        || simplifiedAttributes.value("value").toString().simplified().isEmpty())

        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText(QXmlStreamReader::IncludeChildElements));
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(W3CSettingsKey::WIDGET_FEATURE_PARAM.arg(m_featureCount).arg(++m_paramCount), element);
    ++m_paramTotalCount;

    return true;
}

/***************************************************************************************************
 * parsePreferenceElement(): Parses the 'prefenrence' element from config.xml file
 * ContentModel: Empty
 * Occurrences: Zero or more(one element is allowed per language)
 * W3Specs for preference element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
bool ConfigXmlParserPrivate::parsePreference()
{
    bool hasReadonly = false;

    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        QString name = attribute.name().toString();
        QString value = attribute.value().toString().simplified();
        if (name == "readonly") {
            hasReadonly = true;
            if (value != "true")
                value = "false";
        }
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    name,
                                    value);
    }

    if (!hasReadonly)
        simplifiedAttributes.append("readonly", "false");

    QString name = simplifiedAttributes.value("name").toString().simplified();
    // ignore if the name attribute is missing
    if (name.isEmpty())
        return true;

    // ignore if already existed
    for (int i = 1; i <= m_preferenceCount; i++) {
        if (name == q->getElement(W3CSettingsKey::WIDGET_PREFERENCE, i)->attributeValue("name"))
            return true;
    }

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());
    element->setText(m_xmlreader.readElementText(QXmlStreamReader::IncludeChildElements));

    m_container.insert(W3CSettingsKey::WIDGET_PREFERENCE.arg(++m_preferenceCount), element);
    return true;
}


/***************************************************************************************************
 * parseSharedLibraryElement(): Parses shared library  element Tag from config.xml file
 * ContentModel: Empty
 * Occurrences:
 * *************************************************************************************************/

bool ConfigXmlParserPrivate::parseSharedLibraryElement()
{
    bool valid(false);
    QXmlStreamAttributes attr = m_xmlreader.attributes();

    W3CElement *sharedLibElement = new W3CElement();
    sharedLibElement->setAttributes(attr);
    sharedLibElement->setnamespaceUri(m_xmlreader.namespaceUri().toString());
    m_container.insert(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY, sharedLibElement);

    //set the m_isSharedLibraryTagPrevElement flag so that the readNext does not get called twice.
    m_isSharedLibraryTagPrevElement = true;
    m_xmlreader.readNext();

    while (!(m_xmlreader.name() == SharedLibraryTags::SHARED_LIBRARY_TAG
           && m_xmlreader.isEndElement())
           && !m_xmlreader.atEnd()
           && !m_xmlreader.hasError()) {
        if (m_xmlreader.isEndElement())   {
            m_xmlreader.readNext();
            continue;
        }
        if (m_xmlreader.name() == SharedLibraryTags::SHARED_LIBRARY_FOLDER_TAG)   {
            W3CElement *folderElement = new W3CElement();
            folderElement->setAttributes(m_xmlreader.attributes());
            folderElement->setText(m_xmlreader.readElementText().simplified());
            folderElement->setnamespaceUri(m_xmlreader.namespaceUri().toString());
            m_container.insert(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_FOLDER, folderElement);
            valid = true;
        }
        else if (m_xmlreader.name() == SharedLibraryTags::SHARED_LIBRARY_WIDGET_TAG)   {
            W3CElement *widgetElement = new W3CElement();
            widgetElement->setAttributes(m_xmlreader.attributes());
            widgetElement->setText(m_xmlreader.readElementText().simplified());
            widgetElement->setnamespaceUri(m_xmlreader.namespaceUri().toString());
            m_container.insert(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_WIDGET, widgetElement);
        }

        m_xmlreader.readNext();
    }
    return valid;
}

/***************************************************************************************************
 * parseTimerElement(): Parses timer  element Tag from config.xml file
 * ContentModel: Empty
 * Occurrences:
 * *************************************************************************************************/

bool ConfigXmlParserPrivate::parseBackgroundTimerElement()
{
    //ignore if this is not the first timer element in the xml
    if (m_container.contains(ProprietarySettingsKey::WIDGET_BACKGROUND_TIMERS))
        return true;

    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText().simplified());
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(ProprietarySettingsKey::WIDGET_BACKGROUND_TIMERS, element);

    return true;
}

/***************************************************************************************************
 * parseViewmodeSettingsElement(): Parses viewmode settings element Tag from config.xml file
 * ContentModel: Empty
 * Occurrences:
 * *************************************************************************************************/

bool ConfigXmlParserPrivate::parseViewmodeSettingsElement()
{
    bool valid(false);
    QXmlStreamAttributes attr = m_xmlreader.attributes();

    W3CElement *viewmodeElement = new W3CElement();
    viewmodeElement->setAttributes(attr);
    viewmodeElement->setnamespaceUri(m_xmlreader.namespaceUri().toString());
    m_container.insert(ProprietarySettingsKey::WIDGET_VIEWMODE_SETTINGS, viewmodeElement);

    //set the m_isViewModeSettingsTagPrevElement flag so that the readNext does not get called twice.
    m_isViewModeSettingsTagPrevElement = true;
    m_xmlreader.readNext();

    while (m_xmlreader.name() != ProprietaryTags::VIEWMODE_SETTINGS_TAG
           && !m_xmlreader.isEndElement()
           && !m_xmlreader.atEnd()
           && !m_xmlreader.hasError()) {
        if (m_xmlreader.name() == ProprietaryTags::VIEWMODE_MINIMIZED_SETTINGS_TAG)   {
            W3CElement *element = new W3CElement();
            element->setAttributes(m_xmlreader.attributes());
            element->setText(m_xmlreader.readElementText().simplified());
            element->setnamespaceUri(m_xmlreader.namespaceUri().toString());
            m_container.insert(ProprietarySettingsKey::WIDGET_VIEWMODE_MINIMIZED_SETTINGS, element);
            valid = true;
        }
        m_xmlreader.readNext();
    }
    return valid;
}

/******************************************************************************
 *  Internal utility function that determines if language locale can be used
 *  for a given  key
 *  aKey: input key for which the language local to be found
 *****************************************************************************/
bool ConfigXmlParserPrivate::isLocalizable(const QString &aKey) const
{
    if (W3CSettingsKey::WIDGET_NAME == aKey
        || W3CSettingsKey::WIDGET_DESCRIPTION == aKey
        || W3CSettingsKey::WIDGET_LICENSE == aKey)

        return true;

    return false;
}

bool ConfigXmlParserPrivate::parseHiddenElement() {

    W3CElement *element = new W3CElement();
    QString val = m_xmlreader.readElementText().simplified();
    if (val.compare("true",Qt::CaseInsensitive) != 0 && val.compare("false",Qt::CaseInsensitive) != 0) {
        delete element;
        return false;
    }
    element->setText(val);
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());
    m_container.insert(ProprietarySettingsKey::WIDGET_HIDDEN, element);
    return true;
}


bool ConfigXmlParserPrivate::parsePreInstallElement() {

    W3CElement *element = new W3CElement();
    QString val = m_xmlreader.readElementText().simplified();
    if (val.compare("true",Qt::CaseInsensitive) != 0 && val.compare("false",Qt::CaseInsensitive) != 0) {
        delete element;
        return false;
    }
    element->setText(val);
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());
    m_container.insert(ProprietarySettingsKey::WIDGET_PREINSTALL, element);
    return true;
}

bool ConfigXmlParserPrivate::parseCategoriesElement()
{
    QXmlStreamAttributes originalAttributes = m_xmlreader.attributes();
    QXmlStreamAttributes simplifiedAttributes;
    foreach (const QXmlStreamAttribute &attribute, originalAttributes) {
        simplifiedAttributes.append(attribute.namespaceUri().toString(),
                                    attribute.name().toString(),
                                    attribute.value().toString().simplified());
    }
    QString key = W3CSettingsKey::WIDGET_CATEGORIES;

    // ignore if it's not the first one in this locale
    if (m_container.contains(key))
        return true;

    W3CElement *element = new W3CElement();
    element->setAttributes(simplifiedAttributes);
    element->setText(m_xmlreader.readElementText().simplified());
    element->setnamespaceUri(m_xmlreader.namespaceUri().toString());

    m_container.insert(key, element);
    return true;
}

/**************************************************
 * Public exported methods of class ConfigXmlParser
 * ************************************************/

//Default constructor
ConfigXmlParser::ConfigXmlParser(const QString &confFile)
    : d(new ConfigXmlParserPrivate(this))
{
    d->m_file = confFile;
    d->m_parseFromFile = true;
    d->m_iconCount = 0;
    d->m_accessCount = 0;
    d->m_featureCount = 0;
    d->m_preferenceCount = 0;
    d->m_paramCount = 0;
    d->m_paramTotalCount = 0;
    d->m_isFeatureTagPrevElement = false;
    d->m_isSharedLibraryTagPrevElement = false;
    d->m_isViewModeSettingsTagPrevElement = false;
    d->m_isAllUrls = false;
    d->m_isFirstJilAccessTag = true;
    QLocale loc = QLocale::system();
    if( loc.language() == QLocale::C ) {
        // According to Qt doc, locale "C" is english US.
        QLocale us( QLocale::English, QLocale::UnitedStates );
        d->m_lang = us.name();
    }
    else {
        d->m_lang = loc.name();
    }
    d->m_lang = d->m_lang.toLower();
    d->m_lang.replace(QString("_"),QString("-"));
}

//Default destructor
ConfigXmlParser::~ConfigXmlParser()
{
    //delete all the elements of the container
    QMap<QString, W3CElement*>::iterator iter = d->m_container.begin();
    while (iter != d->m_container.end()) {
        delete iter.value();
        iter = d->m_container.erase(iter);
    }

    delete d;
}

/*****************************************************************************************************
 * ParseNameElement() : Parses the 'name' element from config.xml file
 * ContentModel: Any
 * Occurrences: Zero or more(one element is allowed per language)
 * W3Specs for name element (refer http://www.w3.org/TR/widgets/ for more details.)
 * *************************************************************************************************/
void ConfigXmlParser::setFile(const QString &afile) {
    d->m_file = afile;
    d->m_parseFromFile = true;
}

void ConfigXmlParser::setContents(const QString &aContents)
{
    d->m_parseFromFile = false;
    d->m_contents = aContents;
    //temporary fix for stripping BOM for UTF-8.
    if(aContents.left(3) == "\xef\xbb\xbf") {
        d->m_contents = aContents.mid(3);
    }
}


/************************************************************************************************************
 * parseFile(): Main function to parse all the essiantial tags of config.xml file
 * refer http://www.w3.org/TR/widgets/ for more details.
 * *********************************************************************************************************/
bool ConfigXmlParser::parseFile()
{
    QFile file;

    if (d->m_parseFromFile) {
        file.setFileName(d->m_file);

        if (!file.exists()) {
           LOG("File does not exist");
                return false;
        }
        if (file.open(QIODevice::ReadOnly) == false) {
          LOG("Failed to read from the device");
                return false;
            }
        d->m_xmlreader.setDevice(&file);
    } else {
        d->m_xmlreader.addData(d->m_contents);
    }

    bool isFirstElement = true;


    while (!d->m_xmlreader.atEnd()) {
        if (d->m_xmlreader.isStartElement()) {
            QStringRef name = d->m_xmlreader.name();
            LOG("ConfigXmlParser::parseFile() name:" << name);

            if (isFirstElement && (name != W3CTags::WIDGET_TAG))
                return false;
            else
                isFirstElement = false;

            if (name == W3CTags::WIDGET_TAG) {
                // read the attributes
                d->m_version = d->m_xmlreader.attributes().value("version").toString().simplified();
                d->m_height = d->m_xmlreader.attributes().value("height").toString().trimmed();
                d->m_width = d->m_xmlreader.attributes().value("width").toString().trimmed();
                d->m_id = d->m_xmlreader.attributes().value("id").toString().simplified();
                d->m_viewmodes = d->m_xmlreader.attributes().value("viewmodes").toString();

                QString lang = d->m_xmlreader.attributes().value("xml:lang").toString();
                if (!lang.isEmpty())
                    d->m_lang = lang.toLower();

                // check all namespaces instead of just the default one
                bool isNamespaceValid = false;
                foreach (const QXmlStreamNamespaceDeclaration &decl, d->m_xmlreader.namespaceDeclarations()) {
                    QString pref = decl.prefix().toString();
                    QString nsUri = decl.namespaceUri().toString();

                    if (pref.isEmpty()) {
                        // case 1: default namespace (empty prefix), must be XMLNS_STRING
                        if (nsUri.compare(XMLNS_STRING) != 0)
                            return false;
                        d->m_namespace = nsUri;
                        isNamespaceValid = true;
                    } else if (pref.compare("jil", Qt::CaseInsensitive) == 0) {
                        // case 2: jil namespace, must be XMLNS_JIL_STRING
                        if (nsUri.compare(XMLNS_JIL_STRING, Qt::CaseInsensitive) != 0)
                            return false;
                         d->m_widgetType = WIDGET_PACKAGE_FORMAT_JIL;
                    } else {
                        // TODO: some other namespaces, might need handling too!
                    }
                }
                if (!isNamespaceValid)
                    return false;

                d->m_services = d->m_xmlreader.attributes().value("xmlns:services").toString();
                if (!d->m_services.isEmpty())
                    d->m_xmlreader.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("xmlns:services", d->m_services));

            } else if (name == W3CTags::NAME_TAG) {
                d->parseNameElement();
            } else if (name == W3CTags::DESCRIPTION_TAG) {
                d->parseDescriptionElement();
            } else if (name ==  W3CTags::ACCESS_TAG) {
                d->parseWidgetAccessElement();
            } else if (name ==  W3CTags::FEATURE_TAG) {
                d->parseFeatureElement();
            } else if (name == W3CTags::LICENSE_TAG ) {
                d->parseLicenseElement();
            } else if (name == W3CTags::PREFERENCE_TAG) {
                d->parsePreference();
            } else if (name == W3CTags::CONTENT_TAG) {
                d->parseContentElement();
            } else if (name == W3CTags::AUTHOR_TAG) {
                d->parseAuthorElement();
            } else if (name == W3CTags::ICON_TAG) {
                d->parseIconElement();
            } else if (name == W3CTags::SERVER_TAG) {
                d->parseServerElement();
            } else if (name == W3CTags::DELTA_TAG) {
                d->parseDeltaElement();
            } else if (name ==  W3CTags::PARAM_TAG) {
                // ignore the param tag if it appears outside the feature tag
                LOG("param element found outside the feature element");
            } else if (name == SharedLibraryTags::SHARED_LIBRARY_TAG) {
                if (!d->parseSharedLibraryElement())
                    return false;
            } else if (name == ProprietaryTags::BACKGROUND_TIMER_TAG) {
                if (!d->parseBackgroundTimerElement())
                    return false;
            } else if (name == ProprietaryTags::VIEWMODE_SETTINGS_TAG) {
                if (!d->parseViewmodeSettingsElement())
                    return false;
            } else if (name == HiddenTags::HIDDEN_WIDGET_TAG) {
                if (!d->parseHiddenElement())
                    return false;
            } else if(name == PreinstallTags::PREINSTALL_WIDGET_TAG) {
                if (!d->parsePreInstallElement())
                    return false;
            } else if (name == W3CTags::CATEGORIES_TAG) {
                d->parseCategoriesElement();
            } else {
                LOG("Invalid tag");
            }
        }

        if (!d->m_isFeatureTagPrevElement &&
            !d->m_isSharedLibraryTagPrevElement &&
            !d->m_isViewModeSettingsTagPrevElement) {
            // for feature tag the readNext is already called while checking for param element
            d->m_xmlreader.readNext();
        } else {
            d->m_isFeatureTagPrevElement = false;
            d->m_isSharedLibraryTagPrevElement = false;
            d->m_isViewModeSettingsTagPrevElement = false;
        }

        if (d->m_xmlreader.hasError()) {
           LOG("XML Error :" << d->m_xmlreader.error()<<  "at line:"
                << d->m_xmlreader.lineNumber()<< "col:" << d->m_xmlreader.columnNumber()
                << d->m_xmlreader.errorString());
            return false;
        }
    }

    return true;
}


/******************************************************************************
 * Public Function to get element from the container(name, author, license,
 *         description)
 * aLang: language locale
 * aKey:  standard key for fetching Widget Element
 * ****************************************************************************/
const W3CElement* ConfigXmlParser::getElement(const QString &aKey, const QString &aLang) const
{
    if (d->isLocalizable(aKey)) {
        // localizable
        QString key;
        W3CElement *result = NULL;

        if (!aLang.isEmpty()) {
            key = aLang + QString("/") + aKey;
            result = d->m_container.value(key);
        } else if( (!aKey.compare(W3CSettingsKey::WIDGET_NAME))||(!aKey.compare(W3CSettingsKey::WIDGET_DESCRIPTION))) {
            key = W3CSettingsKey::WIDGET_LANGUAGE_KEY + QString("/") + aKey;
            result = d->m_container.value(key);
        } else {
            key = d->m_lang + QString("/") + aKey;
            result = d->m_container.value(key);
        }

        int index = -1;
        while (NULL == result) {
            // fallback from e.g. zh-hans-cn to zh-hans to zh
            if ((index = key.lastIndexOf("-")) >= 0) {
                key = key.left(index);
                key += QString("/") + aKey;
                result = d->m_container.value(key);
            } else {
                break;
            }
        }

        if (NULL == result)
            result = d->m_container.value(QString("c/") + aKey);

        return result;
    }

    // not localizable
    return d->m_container.value(aKey);
}


/******************************************************************************
 * Public Function to get element from the container(icon, feature, access
 *         preferences)
 *         Usefull for fetching elements that have multiple occurences in
 *         config.xml file (icon, feature etc)
 * aLang: language locale
 * aKey:  standard key for fetching Widget Element
 * ****************************************************************************/
const W3CElement* ConfigXmlParser::getElement(const QString &aKey, int pos, const QString &aLang) const
{
    Q_UNUSED(aLang);

    if (pos <= 0)
        return NULL;

    // no localization needed

    return d->m_container.value(aKey.arg(pos));

}


/********************************************************************************
 * Public Function to multiple elements from container(icon, content, preference,
 *         features, access etc)
 *
 * aLange: language locale
 * aKey : standard key for fetching Widget element
 *******************************************************************************/
bool ConfigXmlParser::contains(const QString &aKey, QString *aLang) const
{
    return count(aKey, aLang);
}


/****************************************************************************************
 * Public funtion to return number of occurences an item under a key
 * aKey: standard key
 * Useful for multipe occurance element like license, access , Feature and preference
 * ************************************************************************************/
int ConfigXmlParser::count(const QString &aKey, QString *lang) const
{
    if (aKey == W3CSettingsKey::WIDGET_ACCESS)
        return d->m_accessCount;

    if (aKey == W3CSettingsKey::WIDGET_FEATURE)
        return d->m_featureCount;

    if (aKey == W3CSettingsKey::WIDGET_PREFERENCE)
        return d->m_preferenceCount;

    if (aKey == W3CSettingsKey::WIDGET_ICON)
        return d->m_iconCount;

    if (aKey == W3CSettingsKey::WIDGET_FEATURE_PARAM)
        return d->m_paramTotalCount;

    QString key;
    if (d->isLocalizable(aKey)) {
        if (lang && !lang->isEmpty()) {
            key = *lang + QString("/");
        } else {
            if (!d->m_lang.isEmpty())
                key = d->m_lang + QString("/");
        }
    }
    key += aKey;

    return  d->m_container.contains(key);
 }


/****************************************************************************************
 * Public funtion to return all the keys based on the language
 * aLang: Language locale
 * Useful for fetching language specific values for element like license, description, name and preference
 * ************************************************************************************/
QStringList ConfigXmlParser::allKeys()
{
    return d->m_container.keys();
}

QString ConfigXmlParser::namespaceUri()
{
    return d->m_namespace;
}

QString ConfigXmlParser::id()
{
    return d->m_id;
}

QString ConfigXmlParser::version()
{
    return d->m_version;
}

QString ConfigXmlParser::language() const
{
    return d->m_lang;
}

QString ConfigXmlParser::height() const
{
    return d->m_height;
}

QString ConfigXmlParser::width() const
{
    return d->m_width;
}

QString ConfigXmlParser::viewmodes() const
{
    return d->m_viewmodes;
}

QString ConfigXmlParser::widgetType() const
{
    return d->m_widgetType ;
}

QSet<QString> ConfigXmlParser::languages() const
{
    return d->m_languages;
}

int ConfigXmlParser::count() const
{
    return d->m_container.count();
}
