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
#include <QStringList>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QXmlStreamAttribute>

#include "featuremapping.h"

FeatureMapping::FeatureMapping()
{
    m_superFeatureInfoList.clear();
    m_featureSetInfoList.clear();
    m_featureInfoList.clear();
#if !defined (Q_OS_MAEMO5) && !defined (Q_OS_MAEMO6)
    m_settings = WAC::WrtSettings::createWrtSettings();
    m_xmlFilePath = m_settings->valueAsString("FeatureMapping");
#else
    //FIXME: All security related keys should be moved away from settings file
    m_xmlFilePath = "/etc/wrt/security/mapping.xml";
#endif
    m_widgetType = WidgetTypeW3c;
}

int FeatureMapping::capabilities(const QString& aFeature, QList<QString> &capList)
{
    int err1 = 0, err2 = 0, err3 = 0;

    if (m_featureSetInfoList.isEmpty()) {
        if (!parse())
            return -1;
    }
    QString feat = aFeature.trimmed();
    err1 = getCapsFromSuperFeature(feat,capList);
    if (capList.isEmpty()) {
        err2 = getCapsFromFeatureSet(feat,capList);
    }
    if (capList.isEmpty()) {
        err3 = getCapsFromFeature(feat,capList);
    } 
    if (capList.isEmpty() && (err1 == -1 && err2 == -1 && err3 == -1))
        return -1;  
   
    return 0;
}

int FeatureMapping::serviceNames(const QString& aFeature,QList<QString>& serviceNameList)
{
    QStringList serviceNames;
    QString feat = aFeature.trimmed();
    getServiceNameFromSuperFeature(feat,serviceNames);
    if (serviceNameList.isEmpty()) {
        getServiceNameFromFeatureSet(feat,serviceNames);
    }
    serviceNameList.append(serviceNames);
    return 0;
}

/*!
 * Method used to get the service names based on capabilities requested
 * 
 * @param capabilities The list of required capabilities
 * @param serviceNames The list of service names associated with the requested capabilities
 * 
 * @return false if feature mapping file is unable to be parsed
 */
bool FeatureMapping::getServiceNamesFromCapabilities(QList<QString>& capabilities, 
        QList<QString>& serviceNames)
{
    if (m_featureSetInfoList.isEmpty()) {
        if (!parse())
            return false;
    }
    
    // capabilities passed in may contain wildcards, i.e. pim.* or devicestatus.*
    QList<QRegExp> myCapList;
    for (int i = 0; i < capabilities.length(); i++) {
        myCapList.append(QRegExp(capabilities.at(i)));
    }

    // FIXME: optimize to find 1 service for each capability, instead of going through all services
    for (int i = 0; i < m_featureSetInfoList.length(); i++) {
        QMap<QString, QString> featureset = m_featureSetInfoList.at(i);
        // JIL featuresets include capabilities in mapping.xml, but Nokia featuresets do not.
        // However, mapping.xml may list multiple capabilities and multiple services for one JIL feature.
        // This isn't helpful in finding a 1 to 1 mapping of capability to service name, 
        // so we don't use JIL featuresets to help map capabilities to Service Names
        if (featureset.value(KWidgetType) == "Nokia") {
            QList<QString> features = featureset.value(KFeatures).split(",");
            for (int k = 0; k < features.length(); k++) {
                QList<QString> featureCaps;
                if ( getCapsFromFeature(features.at(k), featureCaps) == 0) {
                    for (int m = 0; m < featureCaps.length(); m++) {
                        for (int j = 0; j < myCapList.length(); j++){
                            if (myCapList.at(j).indexIn(featureCaps.at(m)) != -1) {
                                if (!serviceNames.contains(featureset.value(KService)))
                                    serviceNames.append(featureset.value(KService));
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

WidgetType FeatureMapping::getWidgetType()
{
    return m_widgetType;
}

FeatureMapping::~FeatureMapping()
{
    //delete m_settings;
}

int FeatureMapping::getCapsFromSuperFeature(const QString& aFeature,QList<QString>& capList)
{
    int result = -1;
    int cnt = m_superFeatureInfoList.count();
    for (int i=0; i<cnt; i++) {
        if (m_superFeatureInfoList[i].value(KName) == aFeature) {
            QString featureSets = m_superFeatureInfoList[i].value(KFeatureSets);
            QStringList featureSetList = featureSets.split(KMappingListSeparator,
                    QString::SkipEmptyParts);
            for (int j=0; j<featureSetList.count(); j++) {
                result = getCapsFromFeatureSet(featureSetList[j],capList);
                if (result == -1)
                    break;
            }
            break;
        }
    }
    return result;
}

int FeatureMapping::getCapsFromFeatureSet(const QString& aFeature,QList<QString>& capList)
{
    int result = -1;
    int cnt = m_featureSetInfoList.count();
    for (int i=0; i<cnt; i++) {
        if (m_featureSetInfoList[i].value(KName) == aFeature) {
            result = 0;
            if (m_featureSetInfoList[i].contains(KCapabilities)) {
                QString caps = m_featureSetInfoList[i].value(KCapabilities);
                QStringList capsArray = caps.split(KMappingListSeparator,
                        QString::SkipEmptyParts);
                capList.append(capsArray);
            } else if (m_featureSetInfoList[i].contains(KFeatures)) {
                QString features = m_featureSetInfoList[i].value(KFeatures);
                QStringList featureList = features.split(KMappingListSeparator,
                        QString::SkipEmptyParts);
                for (int j=0; j<featureList.count(); j++) {
                    result = getCapsFromFeature(featureList[j],capList);
                    if (result == -1)
                        break;
                }
            }
            if (m_featureSetInfoList[i].value(KWidgetType) == "JIL")
                m_widgetType = WidgetTypeJIL;
            break;
        }
    }
    return result;
}

int FeatureMapping::getCapsFromFeature(const QString& aFeature,QList<QString>& capList)
{
    int result = -1;
    int cnt = m_featureInfoList.count();
    for (int i=0; i<cnt; i++) {
        if (m_featureInfoList[i].value(KName) == aFeature) {
            QString caps = m_featureInfoList[i].value(KCapabilities);
            QStringList capsArray = caps.split(KMappingListSeparator,
                    QString::SkipEmptyParts);
            capList.append(capsArray);
            result = 0;
        }
    }
    return result;
}

int FeatureMapping::getServiceNameFromSuperFeature(const QString& aFeature,
        QList<QString>& serviceNameList)
{
    int result = -1;
    int cnt = m_superFeatureInfoList.count();
    for (int i=0; i<cnt; i++) {
        if (m_superFeatureInfoList[i].value(KName) == aFeature) {
            QString featureSets = m_superFeatureInfoList[i].value(KFeatureSets);
            QStringList featureSetList = featureSets.split(KMappingListSeparator,
                    QString::SkipEmptyParts);
            for (int j=0; j<featureSetList.count(); j++) {
                result = getServiceNameFromFeatureSet(featureSetList[j],serviceNameList);
                if (result == -1)
                    break;
            }
            break;
        }
    }
    return result;
}

int FeatureMapping::getServiceNameFromFeatureSet(const QString& aFeature,
QList<QString>& serviceNameList)
{
    int result = -1;
    int cnt = m_featureSetInfoList.count();
    for (int i=0; i<cnt; i++) {
        if ((m_featureSetInfoList[i].value(KName).compare(aFeature,Qt::CaseInsensitive) == 0)
             || (m_featureSetInfoList[i].value(KName).contains(aFeature+'.',Qt::CaseInsensitive))) {
            QStringList services = m_featureSetInfoList[i].value(KService).split(',',
                    QString::SkipEmptyParts);
            for (int j=0; j<services.count();j++) {
                if (!services[j].isEmpty())
                    serviceNameList.append(services[j]);
            }
            result = 0;
        }
    }
    return result;
}

bool FeatureMapping::parse()
{
    bool parseError = false;
    QFile xmlFile(m_xmlFilePath);
    if (xmlFile.exists()) {
        xmlFile.open(QIODevice::ReadOnly);
        QXmlStreamReader xmlReader(&xmlFile);
        while (!parseError && !xmlReader.atEnd()) {
            xmlReader.readNext();
            if (xmlReader.name().compare(MAPPING_TAG,Qt::CaseInsensitive) == 0) {
                if (!processMappingElement(xmlReader)) {
                    parseError = true;
                    break;
                }
            }
        }
    } else {
        parseError = true;
    }
    return !parseError;
}

bool FeatureMapping::processMappingElement(QXmlStreamReader& aXmlReader)
{
    Q_ASSERT(aXmlReader.isStartElement() && 
            (aXmlReader.name().compare(MAPPING_TAG,Qt::CaseInsensitive) == 0));
    bool parseError = false;

    while (!parseError && !aXmlReader.atEnd()) {
        aXmlReader.readNext();
        if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_SUPER_FEATURE_TAG,
                        Qt::CaseInsensitive)==0)) {
            if (!processSuperFeatureElement(aXmlReader)) {
                parseError = true;
                break;
            }
        } else if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURESET_TAG,
                        Qt::CaseInsensitive)==0)) {
            if (!processFeatureSetElement(aXmlReader)) {
                parseError = true;
                break;
            }
        }
    }
    return !parseError;
}

bool FeatureMapping::processSuperFeatureElement(QXmlStreamReader& aXmlReader)
{
    Q_ASSERT(aXmlReader.isStartElement() && 
            (aXmlReader.name().compare(MAPPING_SUPER_FEATURE_TAG,
                    Qt::CaseInsensitive)==0));
    bool parseError = false;
    QMap<QString,QString> superFeatureInfo;

    QXmlStreamAttributes attributes = aXmlReader.attributes();
    QString superFeatureName = attributes.value(KName).toString();
    superFeatureInfo.insert(KName,superFeatureName);
    while (!parseError && !aXmlReader.atEnd()) {
        aXmlReader.readNext();
        if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURESETS_TAG,
                        Qt::CaseInsensitive)==0)) {
            while (!parseError && !aXmlReader.atEnd()) {
                aXmlReader.readNext();
                if (aXmlReader.isStartElement() && 
                        (aXmlReader.name().compare(MAPPING_FEATURESET_TAG,
                                Qt::CaseInsensitive)==0)) {
                    QString fs = superFeatureInfo.value(KFeatureSets);
                    if (fs.isEmpty()) {
                        superFeatureInfo.insert(KFeatureSets,aXmlReader.readElementText());
                    } else {
                        fs.append(',');
                        fs.append(aXmlReader.readElementText());
                        superFeatureInfo.insert(KFeatureSets,fs);
                    }
                } else if (aXmlReader.isEndElement() && 
                        (aXmlReader.name().compare(MAPPING_FEATURESETS_TAG,
                                Qt::CaseInsensitive)==0)) {
                    break;
                }
            }
        } else if (aXmlReader.isEndElement() && 
                (aXmlReader.name().compare(MAPPING_SUPER_FEATURE_TAG,
                        Qt::CaseInsensitive)==0)) {
            break;
        }
    }
    m_superFeatureInfoList.append(superFeatureInfo);
    return !parseError;
}

bool FeatureMapping::processFeatureSetElement(QXmlStreamReader& aXmlReader)
{
    Q_ASSERT(aXmlReader.isStartElement() && 
            (aXmlReader.name().compare(MAPPING_FEATURESET_TAG,
                    Qt::CaseInsensitive)==0));
    bool parseError = false;
    QMap<QString,QString> featureSetInfo;

    QXmlStreamAttributes attributes = aXmlReader.attributes();
    QString type = attributes.value("type").toString();
    if (type.isEmpty())
        featureSetInfo.insert(KWidgetType,"Nokia");
    else
        featureSetInfo.insert(KWidgetType,type);

    while (!parseError && !aXmlReader.atEnd()) {
        aXmlReader.readNext();
        if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURESET_NAME_TAG,
                        Qt::CaseInsensitive)==0)) {
            featureSetInfo.insert(KName,aXmlReader.readElementText());
        } else if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_SERVICE_TAG,
                        Qt::CaseInsensitive)==0)) {
            featureSetInfo.insert(KService,aXmlReader.readElementText());
        } else if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_CAPABILITIES_TAG,
                        Qt::CaseInsensitive)==0)){
            featureSetInfo.insert(KCapabilities,aXmlReader.readElementText());
        } else if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_FEATUREFILE_TAG,
                        Qt::CaseInsensitive)==0)){
            featureSetInfo.insert(KFeatureFile,aXmlReader.readElementText());
        } else if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURE_TAG,
                        Qt::CaseInsensitive)==0)) {
            if (!processFeatureElement(aXmlReader,featureSetInfo))
                parseError = true;
        } else if (aXmlReader.isEndElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURESET_TAG,
                        Qt::CaseInsensitive)==0)) {
            break;
        }
    }
    m_featureSetInfoList.append(featureSetInfo);
     
    return !parseError;
}

bool FeatureMapping::processFeatureElement(QXmlStreamReader& aXmlReader, 
        QMap<QString,QString>& aFeatureSetInfo)
{
    Q_ASSERT(aXmlReader.isStartElement() && 
            (aXmlReader.name().compare(MAPPING_FEATURE_TAG,
                    Qt::CaseInsensitive)==0));
    bool parseError = false;
    QMap<QString,QString> featureInfo;

    while (!parseError && !aXmlReader.atEnd()) {
        aXmlReader.readNext();
        if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURE_NAME_TAG,
                        Qt::CaseInsensitive)==0)) {
            featureInfo.insert(KName,aXmlReader.readElementText());
        } else if (aXmlReader.isStartElement() && 
                (aXmlReader.name().compare(MAPPING_CAPABILITIES_TAG,
                        Qt::CaseInsensitive)==0)) {
            featureInfo.insert(KCapabilities,aXmlReader.readElementText());
        } else if (aXmlReader.isEndElement() && 
                (aXmlReader.name().compare(MAPPING_FEATURE_TAG,
                        Qt::CaseInsensitive)==0)) {
            break;
        }
    }
    QString feat = aFeatureSetInfo.value(KFeatures);
    if (feat.isEmpty()) {
        aFeatureSetInfo.insert(KFeatures,featureInfo.value(KName));
    } else {
        feat.append(KMappingListSeparator);
        feat.append(featureInfo.value(KName));
        aFeatureSetInfo.insert(KFeatures,feat);
    }
    m_featureInfoList.append(featureInfo);
    return !parseError;
}

// Method used to get the capability lists based on feature name
//
// parameters:
//      features the feature list for which the device-caps are to be determined
//      required_capabilities the list of required capabilities
//      optional_capabilities the list of optional capabilities
//
// @return:
//      bool false if plugin load fails or feature is not found in the mapping file

bool FeatureMapping::getCapabilities(WidgetFeatures &features, 
        QList<QString> &required_capabilities, 
        QList<QString> &optional_capabilities)
{
    bool result = true;

   // FeatureMapping mapping;
    QStringList featureList = features.keys();
    for (int i = 0; i < featureList.count(); i++) {
        QStringList caps;
        int err = capabilities(featureList[i], caps);
        //if err is -1 the feature is not found. probably a mistake in the config.xml
        if ((err < 0) && (features.value(featureList[i]).compare("true",
                Qt::CaseInsensitive) == 0)) {
            result = false;
            break;
        }
        // Differentiate between the required and optional capabilities based on
        // required attribute of feature
        if ((features.value(featureList[i]).compare("true",Qt::CaseInsensitive)) == 0) {
            for (int k = 0; k < caps.count(); k++) {
                if (!required_capabilities.contains(caps[k]))
                    required_capabilities.append(caps[k]);
            }
        } else {
            for (int k = 0; k < caps.count(); k++) {
                if (!optional_capabilities.contains(caps[k]))
                    optional_capabilities.append(caps[k]);
                }
        }
    }
    m_widgetType = getWidgetType();
    return result;
}

bool FeatureMapping::getServiceNames(WidgetFeatures& features,QList<QString>& serviceNamesList)
{
    bool result = true;
    QStringList featureList = features.keys();
    for (int i=0; i<featureList.count(); i++) {
        QStringList services;
        serviceNames(featureList[i],services);
        for (int j=0; j<services.count(); j++)
            if (!serviceNamesList.contains(services[j]))
                   serviceNamesList.append(services[j]);
    }
    return result;
}
/*
 returns the wrapper js file  corressponding to featureName
 */
bool FeatureMapping::getFeatureFile(const QString &featureUrl,
        QString &featureFile)
{
    // retrieves the wrapper file using featureName
    if (m_featureSetInfoList.isEmpty())
        {
        if (!parse())
            return false;
        }
    bool ret(false);

    int cnt = m_featureSetInfoList.count();
    for (int i = 0; i < cnt; i++)
        {
        if ((!m_featureSetInfoList[i].value(KName).isNull()))
            {
            QString fName = m_featureSetInfoList[i].value(KName);

            if(featureUrl.length() >= fName.length())
                {
                QString featureName = featureUrl.left(fName.length());
                if (fName.compare(featureName, Qt::CaseInsensitive) == 0)
                    {
                    featureFile = m_featureSetInfoList[i].value(KFeatureFile);
                    qDebug() << "featureFile is" << featureFile;
                    ret = true;
                    break;
                    }
                }
            }
        }
    return ret;
}
