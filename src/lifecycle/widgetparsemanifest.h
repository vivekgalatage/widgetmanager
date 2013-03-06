#ifndef WIDGETPARSEMANIFEST_H
#define WIDGETPARSEMANIFEST_H

#include "widgetoperationmanager.h"
#include "widgetoperation.h"

#include "widgettypes.h"
#include "widgetinformation.h"
#include "featuremapping.h"
#include "private/wacWidgetInfo.h"

#include <QDomElement>
#include <QDomNode>

//typedef QMap<QString, QVariant> WidgetFeatures;		//TODO::To change security interfaces to take this type

class WidgetInfo;

class WidgetParseManifest: public WidgetOperation
{
public:
    explicit WidgetParseManifest( WidgetInformation& widgetInfo);
    ~WidgetParseManifest();

public:
    void execute();
    
    void restore();

	void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );

private:
    WidgetErrorType parseManifest();

	WidgetProperties* widgetProperties();

    QString getCertificateAKI();
    
    QString getTrustDomain();

	WidgetErrorType findFeatures(WidgetFeatures& features);

	QString resourcePath(const QString& installPath);

	bool findIcons( QStringList& iconList);

	bool isDirectionValid(const QString& AttributeValue);

	bool findStartFile(QString& startFile, const QString& path);
    
private:

    QDomElement iWidgetNode;
    
    friend class WidgetXmlParserHandler;

	FeatureMapping m_mapping;

	WidgetInfo *m_manifest;
        
};

#endif //WIDGETPARSEMANIFEST_H
