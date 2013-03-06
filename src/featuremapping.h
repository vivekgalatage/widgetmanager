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

#ifndef FEATURE_MAPPING_PLUGIN_H
#define FEATURE_MAPPING_PLUGIN_H

#include <QObject>
#include <QMap>

#include "wacsettings.h"
#include "wacSuperWidget.h"

typedef QList<QMap<QString,QString> > SuperFeatureInfoList;
typedef QList<QMap<QString,QString> > FeatureSetInfoList;
typedef QList<QMap<QString,QString> > FeatureInfoList;

#define MAPPING_TAG "mapping"
#define MAPPING_SUPER_FEATURE_TAG "super-feature"
#define MAPPING_FEATURESETS_TAG "feature-sets"
#define MAPPING_FEATURESET_TAG "feature-set"
#define MAPPING_FEATURE_TAG "feature"
#define MAPPING_FEATURE_NAME_TAG "featurename"
#define MAPPING_FEATURESET_NAME_TAG "featuresetname"
#define MAPPING_SERVICE_TAG "service"
#define MAPPING_CAPABILITIES_TAG "capabilities"
#define MAPPING_FEATUREFILE_TAG "featureFile"

const QString KFeatureSets("featureSets");
const QString KFeatures("feature");
const QString KCapabilities("capabilities");
const QString KName("name");
const QString KService("service");
const QString KMappingListSeparator(",");
const QString KWidgetType ("type");
const QString KFeatureFile ("featureFile");

class QXmlStreamReader;

class FeatureMapping
{
public:
    FeatureMapping();
    bool getCapabilities(WidgetFeatures& features,QList<QString>& required_capabilities,
            QList<QString>& optional_capabilities);
    bool getServiceNames(WidgetFeatures& features,QList<QString>& serviceNamesList);
    bool getServiceNamesFromCapabilities(QList<QString>& capabilities, 
            QList<QString>& serviceNames);    
    bool getFeatureFile (const QString &featureUrl,QString &featureFile);
    WidgetType getWidgetType();
    ~FeatureMapping();
private:
    bool parse();
    bool processMappingElement(QXmlStreamReader& aXmlReader);
    bool processSuperFeatureElement(QXmlStreamReader& aXmlReader);
    bool processFeatureSetElement(QXmlStreamReader& aXmlReader);
    bool processFeatureElement(QXmlStreamReader& aXmlReader,QMap<QString,
            QString>& aFeatureSetInfo);
    int getCapsFromSuperFeature(const QString& aFeature,QList<QString>& capList);
    int getCapsFromFeatureSet(const QString& aFeature,QList<QString>& capList);
    int getCapsFromFeature(const QString& aFeature,QList<QString>& capList);
    int getServiceNameFromSuperFeature(const QString& aFeature,QList<QString>& serviceNameList);
    int getServiceNameFromFeatureSet(const QString& aFeature,QList<QString>& serviceNameList);
    int capabilities(const QString& aFeature,QList<QString>& capList);
    int serviceNames(const QString& aFeature,QList<QString>& serviceNameList);
private:
    WAC::WrtSettings* m_settings;
    QString m_xmlFilePath;
    SuperFeatureInfoList m_superFeatureInfoList;
    FeatureSetInfoList m_featureSetInfoList;
    FeatureInfoList m_featureInfoList;
    WidgetType m_widgetType;
};

#endif //FEATURE_MAPPING_PLUGIN_H
