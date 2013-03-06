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



#include <QString>
#include <QDir>
#include <QVariant>
#include <QDesktopServices>

#include "wacWebAppRegistry.h"
#include "wacWidgetInfo.h"
#include "configw3xmlparser.h"
#include "WidgetLinkResolver.h"
#include "w3cxmlplugin.h"
#include "wacw3celement.h"
#include "wacw3csettingskeys.h"
#include "proprietarysettingskeys.h"

#include "private/WidgetUtilsLogs.h"

W3cXmlPlugin::W3cXmlPlugin()
: m_confFile(QString(""))
, m_data(NULL)
{
    qRegisterMetaType<const W3CElement *>("const W3CElement *");
    m_parser = new ConfigXmlParser(QString());
    m_metadata = new AttributeMap();
}

W3cXmlPlugin::~W3cXmlPlugin()
{
    delete m_parser;
    delete m_data;
    delete m_metadata;
}

void W3cXmlPlugin::setdir(const QDir &ConfDir)
{
    if (!ConfDir.exists()) {
        return;
    }
    m_dir = ConfDir;
}

/*************************************************************
 * Description : Function to fetch the contents of attribute
 *               data parsed from config.xml file
 *@params :   aKey Key of the element ,aLang Language locale,
              pos position
 * if @aAttr is empty then element inner text is retur ned
 ************************************************************/

QString W3cXmlPlugin::value(const QString &aKey, const QString &aAttr, const QString &aLang)
{
    LOG("W3cXmlPlugin::value getting key" << aKey << " language" << aLang);

  // Check if the query is for widget headers
  bool isHeaderKey = false ;

  if (aKey == W3CSettingsKey::WIDGET_VERSION
      || aKey == W3CSettingsKey::WIDGET_LANG
      || aKey == W3CSettingsKey::WIDGET_ID
      || aKey == W3CSettingsKey::WIDGET_HEIGHT
      || aKey == W3CSettingsKey::WIDGET_WIDTH
      || aKey == W3CSettingsKey::WIDGET_VIEWMODES

      || aKey == ProprietarySettingsKey::WIDGET_TYPE)
      isHeaderKey = true;

  if (isHeaderKey)   {
      return m_metadata->value(aKey).toString();
  } else {
      // Fetch the w3celement and then its value
      LOG("W3cXmlPlugin::value getting key: " << aKey << ", language: " << aLang);
      const W3CElement *w3celem = m_parser->getElement(aKey, aLang);

      if (w3celem)   {
          if (!aAttr.isEmpty())   {
              LOG("W3cXmlPlugin::value returning attribute" << w3celem->attributeValue(aAttr));
              return w3celem->attributeValue(aAttr);
          }

          LOG("W3cXmlPlugin::value returning text" << w3celem->readElementText());
          return w3celem->readElementText();
      }
  }

  return QString();
}


/*************************************************************
 * Description : Function to fetch the contents of attribute
 *               data parsed from config.xml file
 *@params :   aKey Key of the element ,aLang Language locale,
 ************************************************************/

QString W3cXmlPlugin::value(const QString &aKey,
                            int pos,
                            const QString &aAttr,
                            const QString &aLang /*=""*/)
{
    LOG("W3cXmlPlugin::value getting value for pos key" << aKey  << " language" << aLang);

    const W3CElement *w3celem = m_parser->getElement(aKey, pos, aLang);
    if (w3celem) {
        if (!aAttr.isEmpty()) {
            return w3celem->attributeValue(aAttr);
        } else {
            return w3celem->readElementText();
        }
    }

    return QString();
}

/***************************************************************
 * Description : function thats gives the total count of given
 *                elements
 *@params      : aKey , input key
 *return       : returns total count of the elements
 ***************************************************************/
int W3cXmlPlugin::count(const QString &aKey)
{
  return m_parser->count(aKey);
}


void  W3cXmlPlugin::WriteManifestFile(const QString &aPath)
{
    QString newList("<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n <widget xmlns=\"http://www.w3.org/ns/widgets\" id=\"");
    newList.append(m_metadata->value(W3CSettingsKey::WIDGET_ID).toString());
    newList.append("\" version=\"");
    newList.append(m_metadata->value(W3CSettingsKey::WIDGET_VERSION).toString());
    newList.append("\" height=\"");
    newList.append(m_metadata->value(W3CSettingsKey::WIDGET_HEIGHT).toString());
    newList.append("\" width=\"");
    newList.append(m_metadata->value(W3CSettingsKey::WIDGET_WIDTH).toString());
    newList.append("\" xml:lang=\"");
    newList.append(m_metadata->value(W3CSettingsKey::WIDGET_LANG).toString());
    newList.append("\">\n");
    newList.append("<name>");
    newList.append(value(W3CSettingsKey::WIDGET_NAME));
    newList.append("</name>\n");
    newList.append("<content src=\"");
    newList.append(value(W3CSettingsKey::WIDGET_CONTENT , QString("src")));
    newList.append("\"/>\n");

    newList.append("</widget>");
    LOG("*********\n\n" << newList);

    QString fpath(aPath);
    if (aPath.isEmpty())
        fpath=QDesktopServices::storageLocation(QDesktopServices::TempLocation)+QDir::separator()+"config.xml";

    QFile file(fpath);
    file.open(QIODevice::WriteOnly);
    file.write(newList.toUtf8().data(),newList.length());
    file.close();
}

QSet<QString> W3cXmlPlugin::languages()
{
    return m_parser->languages();
}

bool W3cXmlPlugin::process(const QString *infoListfile)
{
    LOG("BEGIN W3cXmlPlugin::process infoListfile=" << infoListfile);

    if (infoListfile == NULL && m_dir.exists()) {
        LOG("W3cXmlPlugin::process search for config.xml");
        // Search for config.xml file

        QFileInfoList tempExtract = m_dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        LOG("W3cXmlPlugin::process got entryInfoList");

        foreach (const QFileInfo &isWidgetDir, tempExtract ) {
            if (!isWidgetDir.fileName().compare("config.xml", Qt::CaseSensitive)) {
                // We found the config.xml
                QString filePath = isWidgetDir.absoluteFilePath();
                if (!filePath.isEmpty()) {
                    QFileInfo f = filePath;
                    m_dir = f.dir();
                    LOG("W3cXmlPlugin::process found config.xml at " << filePath);
                    m_parser->setFile(filePath);
                    if (m_parser->parseFile()) {
                       setAttributeMap();
                       LOG("END W3cXmlPlugin::process xml parsing successful");
                       return true;
                    }
                }
            }
        }   // end of foreach
    }

    LOG("END W3cXmlPlugin::process - no xml parsing");
    return false;
}

bool W3cXmlPlugin::processContents(const QString &contents)
{
    m_parser->setContents(contents);
    if (m_parser->parseFile()) {
        setAttributeMap();
        return true;
    }

    return false;
}

const AttributeMap& W3cXmlPlugin::getDictionary() const
{
    return *m_metadata;
}


/********************************************************************
 *Function: setAttributeMap() , function to set the attribute map with
 *          parsed xml w3celements(private function)
 *
 ******************************************************************/

void W3cXmlPlugin::setAttributeMap()
{
    LOG("W3C Setting attribute");

    // First insert widget headers
    QVariant version   = m_parser->version();
    QVariant height    = m_parser->height();
    QVariant width     =  m_parser->width();
    QVariant id        =  m_parser->id();
    QVariant lang      =  m_parser->language();
    QVariant viewmodes = m_parser->viewmodes();
    QVariant widgetType = m_parser->widgetType();

    if (!version.isNull())
        m_metadata->insert(W3CSettingsKey::WIDGET_VERSION, version);

    if (!height.isNull())
        m_metadata->insert(W3CSettingsKey::WIDGET_HEIGHT, height);

    if (!width.isNull())
        m_metadata->insert(W3CSettingsKey::WIDGET_WIDTH, width);

    if (!id.isNull())
        m_metadata->insert(W3CSettingsKey::WIDGET_ID, id);

    if (!lang.isNull())
        m_metadata->insert(W3CSettingsKey::WIDGET_LANG, lang);

    if (!viewmodes.isNull())
        m_metadata->insert(W3CSettingsKey::WIDGET_VIEWMODES, viewmodes);
    if (!widgetType.isNull()) {
      m_metadata->insert(ProprietarySettingsKey::WIDGET_TYPE, widgetType);
    }

    QStringList w3ckeys = m_parser->allKeys();
    int keycount = w3ckeys.count() ;

    for (int iter = 0; iter < keycount; ++iter) {
        QString key = w3ckeys.at(iter);
        const W3CElement *elm = m_parser->getElement(key);
        if (NULL == elm)
            continue;
        LOG(Q_FUNC_INFO << "W3C Setting attribute"  << key << elm);

        // Insert Element text, Attribute name and Attribute value
        QVariant w3celemText = elm->readElementText();
        m_metadata->insert(key, w3celemText);

        QXmlStreamAttributes attributes = elm->attributes();
        for (int i = 0; i < attributes.size(); ++i) {
            QString name = attributes[i].name().toString();
            QString attributeKey = key + QString(":") + name;
            LOG(Q_FUNC_INFO << "W3C Setting attribute"  << attributeKey);

            QVariant attributeValue = attributes[i].value().toString();
            m_metadata->insert(attributeKey, attributeValue);
        }
    }

    QVariant test = m_metadata->value(W3CSettingsKey::WIDGET_CONTENT);

    if (qVariantCanConvert<const W3CElement *>(test))   {
        const W3CElement *elm  = qVariantValue<const W3CElement*>(test);
        if (elm) {
          LOG(elm->attributeValue("src") << "yes");
        } else {
          LOG("Error null element");
        }
    } else {
       LOG("Cannot convert");
    }
}
