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

#include <QLocale>
#include <QStringList>

#include "wacw3celement.h"
#include "wacw3csettingskeys.h"
#include "wacwidgetmanagerconstants.h"
#include "wacWidgetUtils.h"
#include "WidgetUtilsLogs.h"
#include "wacwebappinfo.h"
#include "webapplocalizer.h"
#include "wacsettings.h"

QString WebAppInfo::appId () const {
    return m_appId;
}

QString WebAppInfo::appTitle () const {
    return m_appTitle;
}


QString WebAppInfo::appPath () const {
    return m_appPath;
}

QString WebAppInfo::iconPath () const {
    return m_iconPath;
}

QString WebAppInfo::type () const {
    return m_widgetType;
}

QString WebAppInfo::startPath () const {
    return m_startPath;
}

QString WebAppInfo::dataPath () const {
    return m_dataPath;
}

bool WebAppInfo::capabilityCheck() const {
    return m_capabilityCheck;
}

QString WebAppInfo::certificateAki() const {
    return m_certificateAki;
}

int WebAppInfo::widgetType() const
{
    int type(WidgetUnknown);

    if (m_widgetType == WIDGET_PACKAGE_FORMAT_WGT) {
        type = WidgetW3C;
    }
    else if (m_widgetType == WIDGET_PACKAGE_FORMAT_WGZ) {
        type = WidgetNokia;
    }
    else if (m_widgetType == WIDGET_PACKAGE_FORMAT_SHARED_LIBRARY) {
        type = WidgetSharedLibraryWidget;
    }
    else if (m_widgetType == WIDGET_PACKAGE_FORMAT_JIL) {
        type = WidgetJIL;
    }
    // ToDo: It should be created new WebAppInfo type for OviAppPackages
    else if (m_widgetType == WIDGET_PACKAGE_FORMAT_OVIAPP) {
        type = WidgetW3C;
    }
    return type;
}

QString WebAppInfo::nativeId() const
{
    return m_nativeId;
}

QUrl WebAppInfo::url() const
{

    QString contentSrc = m_data.value("widget/content:src").toString();
    QString startFile = WebAppLocalizer::findStartFile(contentSrc, m_appPath);

    return QUrl("widget://" + m_appId + '/' + startFile);
}

/*!
 \return a localized name of the widget
 */
QString WebAppInfo::name() const
{
    // Cache the name when first time asked
    if (m_name.isEmpty()) {
        QList<W3CElement*> w3cElementsList = getElement(W3CSettingsKey::WIDGET_NAME);
        if (w3cElementsList.count() != 1)
            return ""; //Only one Name for local lang

        W3CElement* w3celem = w3cElementsList.at(0);
        if (!w3celem)
            return "";

        m_name = w3celem->readElementText();
        delete w3celem;
        return m_name;
    } else {
        return m_name;
    }
}

const AttributeMap& WebAppInfo::attributes() const {
    return m_data;
}

/*!
 \return list of all matching W3CElements, note that reciever responsible to free the W3CElements
*/
QList<W3CElement*> WebAppInfo::getElement(const QString &aKey,
                                          const W3CElement *parent,
                                          const QString &aLang) const
{
    QString rawKey, lang;
    QList<W3CElement*> w3cElementsList;
    
    QStringList rawKeys = m_data.keys();
    
     if (aLang.isEmpty()){
         lang = WebAppLocalizer::platformLanguage();
     }
     else {
         lang = aLang;
     }

    // Use standard W3C localization for these main localizable items.
    if ((aKey == W3CSettingsKey::WIDGET_NAME)
    || (aKey == W3CSettingsKey::WIDGET_DESCRIPTION)
    || (aKey == W3CSettingsKey::WIDGET_LICENSE)
    || (aKey == W3CSettingsKey::WIDGET_ICON)) {
        
        // Break up the language by '-'
        // Go through all combinations of fallback and default. 
        QStringList langs = lang.split("-");
        for( int i=langs.count(); i>=0; --i ) {
            // Build lang fallbacks, remove extra '-' at the end
            rawKey = "";
            for( int j=0; j<=i-1; ++j ) {
                rawKey += langs[j];
                rawKey += '-';
            }
            rawKey.chop(1);
            if( rawKey.length() )
                rawKey += QString("/");
            rawKey += aKey;
            
            QStringList keys = rawKeys.filter(rawKey, Qt::CaseInsensitive);
            if( m_data.contains(rawKey) ) {
                // If localized name is found, break out of the localization loop
                W3CElement *element = new W3CElement(rawKey);
                element->setText(m_data.value(rawKey).toString());
                QStringList attrs = keys.filter(rawKey + ':');
                foreach(const QString &key, attrs) {
                    QString attrName = key.right(key.length() - key.lastIndexOf(":") -1);
                    element->addAttribute(attrName, m_data.value(key).toString());
                }
                w3cElementsList.append(element);
                break;
            }
        }
        if(!w3cElementsList.count()) {
            // If nothing found from fallback, also check default/attribute key
            // because default is stored under default key
            rawKey = W3CSettingsKey::WIDGET_LANGUAGE_KEY + '/' + aKey;
            if( m_data.contains(rawKey) ) {
                // find the element
                W3CElement *element = new W3CElement(rawKey);
                element->setText(m_data.value(rawKey).toString());
                QStringList keys = rawKeys.filter(rawKey, Qt::CaseInsensitive);
                QStringList attrs = keys.filter(rawKey + ':');
                foreach(const QString &key, attrs) {
                     QString attrName = key.right(key.length() - key.lastIndexOf(":") -1);
                     element->addAttribute(attrName, m_data.value(key).toString());
                  }
                w3cElementsList.append(element);
            }
        }
    }
    else {
        rawKey = aKey;
        QStringList keys = rawKeys.filter(rawKey, Qt::CaseInsensitive);
        LOG("Keys found -> " << keys);
        
        QString keyValue;
        int count = 1;
        
        if (parent) {
            QString parentNum = parent->elementName();
            parentNum = parentNum.right(parentNum.length() - parentNum.lastIndexOf("/") - 1);
            
            QString temp1, temp2;
            temp1 = rawKey.left(rawKey.lastIndexOf("/") + 1);
            temp2 = rawKey.right(rawKey.length() - rawKey.lastIndexOf("/"));
            
            keyValue = temp1 + parentNum + temp2;
        }
        else {
            keyValue = rawKey;
        }
        
        QString newKey;
        if (m_data.contains(keyValue)) // Exact key present. Only one element.
            newKey = keyValue;
        else
            newKey = keyValue + '/' + QString::number(count);

        while (m_data.contains(newKey)) {
            W3CElement *element = new W3CElement(newKey);
            element->setText(m_data.value(newKey).toString());
            QStringList attrs = keys.filter(newKey + ':');
            foreach(const QString &key, attrs) {
                QString attrName = key.right(key.length() - key.lastIndexOf(":") -1);
                element->addAttribute(attrName, m_data.value(key).toString());
            }

            w3cElementsList.append(element);

            count++;
            newKey = keyValue + '/' + QString::number(count);
        }
    }
    return w3cElementsList;
}

QVariant WebAppInfo::value (const QString& key) const {
    return m_data[key];
}

bool WebAppInfo::operator==(const WebAppInfo &other) const {
        return (m_appId == other.appId())? true : false;
}

bool WebAppInfo::isPresent() const
{
    // Present if either:
    // 1) there is no WEBAPPREGISTRY_ATTRIBUTE_NOTPRESENT attribute, or
    // 2) the value of the attribute is not "yes"
    if (m_data.contains(WEBAPPREGISTRY_ATTRIBUTE_NOTPRESENT) &&
        !m_data.value(WEBAPPREGISTRY_ATTRIBUTE_NOTPRESENT)
               .toString().compare("yes",Qt::CaseInsensitive))
        return false;
    else
        return true;
}

int WebAppInfo::uid () const {
    return m_uid;
}
