#ifndef WIDGETDIGSIGNVALIDATION_H
#define WIDGETDIGSIGNVALIDATION_H

#include "widgetoperation.h"
#include "widgetoperationmanager.h"
#include <QDir>

const QString DefaultDomain = "others";
const QString UntrustedWidgets = "UntrustedWidgets";
const QString DeveloperSigned = "Developer";
const QString PrivatePath = QString(":") + QDir::separator() + QString("private");

class WidgetDigSignValidation: public WidgetOperation
{
public:
    explicit WidgetDigSignValidation( WidgetInformation& widgetInfo);
    ~WidgetDigSignValidation();

public:
    void execute();

	WidgetErrorType finalize();
    
    void restore();

	void interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties );

private:
    WidgetErrorType validateDigitalSignature();
    
    WidgetErrorType validateTrustDomain();
    
    WidgetErrorType updateInstallationPaths( const QString& processUid );
    
    bool createDir(const QString& path);
    
    QString getRootDirectory( const QString& rootDirectory, const QString& processUid );
    
    QString getProcessUid(QString& domain) const;
    
    WidgetErrorType allowInstallation( const QString& domain );
    
};

#endif //WIDGETDIGSIGNVALIDATION_H
