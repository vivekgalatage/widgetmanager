#include "widgetparsemanifest.h"

#include "W3CXmlParser/w3cxmlplugin.h"
#include "W3CXmlParser/wacw3csettingskeys.h"
#include "W3CXmlParser/proprietarysettingskeys.h"
#include "webappuniquenesshelper.h"
#include "wacWebAppRegistry.h"
#include "widgetconstants.h"
#include "webapplocalizer.h"

#include "wacsettings.h"
#include "wactrustsession.h"
#include "wacwebappinfo.h"

#include <QUuid>
#include <QVariant>

#include <QDomDocument>
#include <QDomNodeList>

#include <QXmlSimpleReader>
#include <QXmlParseException>

REGISTER_OPERATION( EOperationLegacyParseManifest, WidgetParseManifest)

WidgetParseManifest::WidgetParseManifest( WidgetInformation& widgetInfo )
    : WidgetOperation( widgetInfo )
{
}

WidgetParseManifest::~WidgetParseManifest()
{
}

void WidgetParseManifest::execute()
{
    WidgetErrorType error = parseManifest();
    
    if( error == EErrorNone )
        emit completed();
    else
        emit aborted( error );
    
    return;

}

WidgetErrorType WidgetParseManifest::parseManifest()
{
    WidgetErrorType errorCode = EErrorNone;
    bool status = true;
    
    QString widgetContentPath = m_WidgetInfo[EPropertyContentDirectory].toString();
    QString configFilePath = widgetContentPath + QDir::separator() + QString("config.xml").toLower();
    if( !QFile::exists( configFilePath ) )
        return EErrorConfigFileNotFound;

    m_manifest = new W3cXmlPlugin;
    m_manifest->setdir( widgetContentPath );

    status = m_manifest->process();
    if (status) {
        WidgetProperties* widgetProps =  widgetProperties();
        if(widgetProps) {
            //Check for start File
            QString startFile;
            if (!findStartFile(startFile,m_WidgetInfo[EPropertyContentDirectory].toString())) {
                errorCode = EErrorStartFileNotFound;
            }
            else {
                widgetProps->setStartFile(startFile);
                //Setting WidgetProperties pointer to widgetInformation QVariant
                QVariant v = qVariantFromValue((void *) widgetProps);
                m_WidgetInfo[EPropertyWidgetProperties]=v;
            }
        }
        else 
            errorCode = EErrorParseManifestGeneral;
    } 
    else 
        errorCode = EErrorParseManifestGeneral;

    if(m_manifest) {
        delete m_manifest;
        m_manifest = NULL;
    }
    
    return errorCode;
}

WidgetProperties* WidgetParseManifest::widgetProperties()
{
    WidgetProperties *props = new WidgetProperties;

    //props->setId(value(W3CSettingsKey::WIDGET_ID));
    QString widgetid = m_manifest->value(W3CSettingsKey::WIDGET_ID );
    QString widgetType = m_manifest->value(ProprietarySettingsKey::WIDGET_TYPE);

    /**
     *  1) Try to: set id for widget from config.xml id field
     *  2) Set QUid
     *
     *  This avoid widgets installation failure in following case:
     *  - Install a widget which does not have id in field config.xml (id will be "", this hashed to zero)
     *  - Install a second widget that does not have id field in config.xml (install of second widget failed)
     *
     */
     // TODO: Check if this is required or need to emit error
    if (widgetid.isEmpty()) {
      widgetid = QUuid::createUuid().toString();
    }
    QString certId = getCertificateAKI();
    QString uniqueId("");
    // If widget is already installed, don't generate uniqueId
    QString rootDir = m_WidgetInfo[EPropertyInstallDirectory].toString();
    if (WebAppUniquenessHelper::getUniqueNameFromPath(rootDir, uniqueId)) {
        WebAppInfo appInfo;
        if (WebAppRegistry::instance()->isRegistered(uniqueId, appInfo)) {
            // grab it from registry
            uniqueId = appInfo.appId();
        }
        else
             uniqueId.clear();
    }
    else 
        uniqueId.clear();   //Restore uniqueId to calculate for first time

    WebAppUniquenessHelper *uniquenessHelper = new WebAppUniquenessHelper(rootDir);
    if ( ( uniqueId.isEmpty() || uniqueId.isNull() ) && uniquenessHelper ) {
        // if it is not already installed, generate uniqueId
        uniqueId = uniquenessHelper->generateUniqueWebAppId(widgetid, certId, getTrustDomain());
        delete uniquenessHelper;
    }
        
    props->setId(uniqueId);
    m_WidgetInfo[ EPropertyWidgetId ] = uniqueId;

    if (!widgetType.isEmpty() && !widgetType.compare(WIDGET_PACKAGE_FORMAT_JIL))
        props->setType(WIDGET_PACKAGE_FORMAT_JIL);
    else
        props->setType(WIDGET_PACKAGE_FORMAT_WGT);

    WidgetFeatures features;
    WidgetErrorType error= findFeatures(features);
    if(error != EErrorNone)
        return NULL;
    else {
        if(features.count()) {
            QList<QString> required_capabilities;
            QList<QString> optional_capabilities;
            if (m_mapping.getCapabilities(features,required_capabilities,optional_capabilities)) {
                if (required_capabilities.contains(CAPABILITY_OVI))
                    props->setType(WIDGET_PACKAGE_FORMAT_OVIAPP);
            }
        }
    }

    QString id = props->id();
    props->setInstallPath(rootDir+ QDir::separator() + id + QDir::separator());
    props->setHiddenWidget(m_manifest->value(ProprietarySettingsKey::WIDGET_HIDDEN).startsWith('T',Qt::CaseInsensitive));
    props->setSource(m_WidgetInfo[EPropertyFilePath].toString());
    props->setInfoPList(m_manifest->getDictionary());
    props->setSize(m_WidgetInfo[EPropertyWidgetSize].toULongLong());
    props->setResourcePath(resourcePath(props->installPath()));
    props->setAllowBackgroundTimers(m_manifest->value(
              ProprietarySettingsKey::WIDGET_BACKGROUND_TIMERS).startsWith("neversuspend",Qt::CaseInsensitive));

    props->setMinimizedSize(QSize( m_manifest->value(ProprietarySettingsKey::WIDGET_VIEWMODE_MINIMIZED_SETTINGS,"width").toInt(),
                                   m_manifest->value(ProprietarySettingsKey::WIDGET_VIEWMODE_MINIMIZED_SETTINGS,"height").toInt()) );
    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();
    QString myLang = m_settings->valueAsString("UserAgentLanguage");
    if (myLang.isEmpty() || myLang.compare(" ") == 0) {
        QLocale language = QLocale::system();
        myLang = language.name().toLower();
        myLang.replace(QString("_"),QString("-"));
    }

    QString myTitle = m_manifest->value(W3CSettingsKey::WIDGET_NAME, QString(""), myLang);
    if (!myLang.isEmpty()) {
        if (!myTitle.isEmpty()) {
            props->setTitle(myTitle);
            m_WidgetInfo[EPropertyWidgetName] = myTitle;
        }
    }

    if (myLang.isEmpty() || myTitle.isEmpty()) {
        QString wName = m_manifest->value(W3CSettingsKey::WIDGET_NAME);
        if (wName.isEmpty()) {
            wName = "NoName";
        }
        props->setTitle(wName);
        m_WidgetInfo[EPropertyWidgetName] = wName;
    }

    QString myTitleDir = m_manifest->value(W3CSettingsKey::WIDGET_NAME, QString("its:dir"));
    if (!myTitle.isEmpty() && !myTitleDir.isEmpty() && isDirectionValid(myTitleDir)) {
        props->setTitleDir(myTitleDir);
    }

    QStringList icons;
    if (findIcons(icons)) {
        QString iconPath = props->installPath() + icons.at(0);
        if (!icons.at(0).startsWith(QDir::separator()) && !props->installPath().endsWith(QDir::separator())) {
            iconPath = props->installPath()+QDir::separator()+icons.at(0);
        }
        if (icons.at(0).startsWith(QDir::separator()) && props->installPath().endsWith(QDir::separator())) {
            iconPath = props->installPath();
            iconPath.chop(1);
            iconPath.append(icons.at(0));
        }
        props->setIconPath(iconPath);
    } else {
        props->setIconPath(":/resource/default_widget_icon.png");
    }

    QString myDesc    = m_manifest->value(W3CSettingsKey::WIDGET_DESCRIPTION, QString(""), myLang);
    QString myDescDir = m_manifest->value(W3CSettingsKey::WIDGET_DESCRIPTION, QString("its:dir"));
    if (!myDesc.isEmpty() && !myDescDir.isEmpty() && isDirectionValid(myDescDir)) {
        props->setDescriptionDir(myDescDir);
    }

    QString myAuthor;
    if (!myLang.isEmpty()) {
        myAuthor    = m_manifest->value(W3CSettingsKey::WIDGET_AUTHOR, QString(""), myLang);
    } else {
       myAuthor = m_manifest->value(W3CSettingsKey::WIDGET_AUTHOR);
    }

    QString myAuthorDir = m_manifest->value(W3CSettingsKey::WIDGET_AUTHOR, QString("its:dir"));
    if (!myAuthor.isEmpty() && !myAuthorDir.isEmpty() && isDirectionValid(myAuthorDir)) {
        props->setAuthorDir(myAuthorDir);
    }

    QString myLic;
    if (!myLang.isEmpty()) {
        myLic = m_manifest->value(W3CSettingsKey::WIDGET_LICENSE, QString(""), myLang);
    } else {
       myLic = m_manifest->value(W3CSettingsKey::WIDGET_LICENSE);
    }
    m_WidgetInfo[EPropertyLicenseInfo]=myLic;


    QString myLicDir = m_manifest->value(W3CSettingsKey::WIDGET_LICENSE, QString("its:dir"));
    if (!myLic.isEmpty() && !myLicDir.isEmpty() && isDirectionValid(myLicDir)) {
        props->setLicenseDir(myLicDir);
    }

    props->setLanguages(m_manifest->languages());  //Adding language set to widget properties

    QString version = (props->plistValue(W3CSettingsKey::WIDGET_VERSION)).toString();
    m_WidgetInfo[EPropertyWidgetVersion]=version;

    return props;
}

QString WidgetParseManifest::getCertificateAKI() 
{
    QString uniqueID;
    bool uniquenameexists = WebAppUniquenessHelper::getUniqueNameFromPath( m_WidgetInfo[ EPropertyInstallDirectory].toString() , uniqueID);

    if(uniquenameexists)
    {
        WebAppInfo appInfo;
        if (WebAppRegistry::instance()->isRegistered(uniqueID, appInfo)) {
            QString commaSeparatedAKIs = appInfo.certificateAki();
            QStringList certificateAKIs = commaSeparatedAKIs.split(",");
            if (certificateAKIs.length() > 0) {
                return certificateAKIs.at(0); // the first is the EE cert
                }
        }
    }
    else {
        //Unique Name not in WebAppRegistry but set in EPropertyCertificateAKIs as part of previous DigSign Validation operation 
        QStringList certificateAKIs  = m_WidgetInfo[EPropertyCertificateAKIs].toStringList();
        if(!(certificateAKIs.isEmpty())) {
            if (certificateAKIs.length() > 0){
                return certificateAKIs.at(0); // the first is the EE cert
                }
            }
        }
    return QString();
}


QString WidgetParseManifest::getTrustDomain() 
{
    WAC::WrtSettings* settings = WAC::WrtSettings::createWrtSettings();
    // get policy files from WrtSettings
    QString trustPath = settings->valueAsString("SecurityTrustPolicyFile");

    // create a trust session
    WAC::TrustSession trustSession(trustPath);

    // if we are installing, we put certificate AKI list in widget properties
    // if we are already installed (launch time) we get the AKI list from the registry
    QString uniqueID;
    QStringList certificateAKIs;
    if(WebAppUniquenessHelper::getUniqueNameFromPath(m_WidgetInfo[ EPropertyContentDirectory ].toString(), uniqueID)){
        WebAppInfo appInfo;
        if (WebAppRegistry::instance()->isRegistered(uniqueID, appInfo)) {
            QString commaSeparatedAKIs = appInfo.certificateAki();
            certificateAKIs = commaSeparatedAKIs.split(",");
        } 
    }else {
        certificateAKIs = m_WidgetInfo[ EPropertyCertificateAKIs].toStringList();
    }
    
    QString domain;
    // End Entity AKI is first in the list, then any SubCAs
    for (int i = 0; i < certificateAKIs.length(); i++) {
        QString aki = certificateAKIs.at(i);
        domain = trustSession.domainFor("certificate", aki);
        // compare against UntrustedWidgets
        if (domain != trustSession.domainFor("certificate", "")) {
            break;
        }
    }
    return domain;
}

WidgetErrorType WidgetParseManifest::findFeatures(WidgetFeatures& features)
{
    WidgetErrorType error = EErrorNone;
    QList< QVariant > featureList;
    int featureCount = m_manifest->count(W3CSettingsKey::WIDGET_FEATURE);

    for (int iter = 0 ; iter < featureCount ; ++iter) {
        QString featureName = m_manifest->value(W3CSettingsKey::WIDGET_FEATURE , iter + 1 , QString("name"));
        QString required_attr = m_manifest->value(W3CSettingsKey::WIDGET_FEATURE , iter + 1 , QString("required"));
        if (required_attr.isEmpty())
            required_attr = "true";

        if (!featureName.isEmpty()) {
            //test feature support for feature elements - W3C P&C spec [ http://dev.w3.org/2006/waf/widgets/test-suite/]
            if(featureName.compare("feature:a9bb79c1") == 0)
                continue;
            if( !(features.isEmpty()) && (features.contains(featureName)) ) {
                error = EErrorParseManifestGeneral;         //Error since same feature name defined more than once
            break;
            }

            features.insert(featureName ,required_attr);

            QMap<QString,QVariant> featureMap;                  //attributes hardcorded! TBR
            featureMap.insert("name",featureName);
            featureMap.insert("required",required_attr);
            featureList.append(static_cast<QVariant>(featureMap));
        }
    }

    if(error == EErrorNone && (featureList.count() > 0))
        m_WidgetInfo[EPropertyWidgetFeatures]= static_cast<QVariant>(featureList);

    return error;
}

bool  WidgetParseManifest::isDirectionValid(const QString& AttributeValue)
{
    if (((AttributeValue.compare("ltr",Qt::CaseInsensitive)) == 0) ||
       ((AttributeValue.compare("rtl",Qt::CaseInsensitive)) == 0) ||
       ((AttributeValue.compare("lro",Qt::CaseInsensitive)) == 0) ||
       ((AttributeValue.compare("rlo",Qt::CaseInsensitive)) == 0))   {
        return true;
    }
    return false;
}

QString WidgetParseManifest::resourcePath(const QString& installPath) 
{
    QDir dir(installPath);
    QString resourcePath;
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    resourcePath = DATA_PATH;
#else
    resourcePath = installPath.left(installPath.indexOf(WIDGET_FOLDER, Qt::CaseInsensitive))+ QString(DataFolderName) + QDir::separator();
#ifdef Q_OS_SYMBIAN
    // Ensure that the resource path is on C drive
    if (resourcePath[1] == ':') {
        resourcePath.replace(0, 1, 'C');
    }
#endif
#endif
    QString resourcesDir = resourcePath + dir.dirName();
    resourcesDir = QDir::toNativeSeparators(resourcesDir);
    resourcesDir += QDir::separator();
    return resourcesDir;
}

bool WidgetParseManifest::findIcons( QStringList& icons )
{
    QString widgetPath=m_WidgetInfo[EPropertyContentDirectory].toString();

    QString absPath(widgetPath);
        if (!absPath.endsWith(QDir::separator())) {
            absPath = absPath+QDir::separator();
        }
    
    QStringList iconFiles;
    int count = m_manifest->count(W3CSettingsKey::WIDGET_ICON);

    for (int iter = 1; iter <= count; ++iter) {
        QString icon = m_manifest->value(W3CSettingsKey::WIDGET_ICON, iter, QString("src"));
        if (!icon.isEmpty()) {
                iconFiles.append(icon);
        }
    }
    
        iconFiles.append("icon.png");
#if !defined(Q_OS_MAEMO5) && !defined(Q_OS_MAEMO6) && !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO)
        iconFiles.append("icon.gif");
        iconFiles.append("icon.ico");
        iconFiles.append("icon.svg");
        iconFiles.append("icon.jpeg");
#endif
    
        icons.clear();
    
        foreach (const QString &fileName, iconFiles) {
            QString locFile = WebAppLocalizer::getLocalizedFile(fileName, false, absPath);
            if (!locFile.isEmpty() && QFile::exists(widgetPath + QDir::separator() + locFile)) {
                icons.append(QDir::toNativeSeparators(locFile));
            }
        }
        return icons.count();
}

// function to find widget start file
// parameters:
//    startFile    returns a string that specifies startFile of widget
//    path         path to widget files
// return:
//    bool         true if start file found
//                 false if start file is not found
//
bool WidgetParseManifest::findStartFile(QString& startFile, const QString& path)
{
    QString contentSrc = m_manifest->value(W3CSettingsKey::WIDGET_CONTENT, QString("src"));
  /*  if(contentSrc.size() == 0) {
        return false;
    }*/
    startFile = WebAppLocalizer::findStartFile(contentSrc, path);
    if (!startFile.isEmpty()) {
        return true;
    }
    return false;
}

void WidgetParseManifest::restore()
{
    QVariant v = m_WidgetInfo[EPropertyWidgetProperties];
    if(v.isValid()) {
        WidgetProperties *props = (WidgetProperties *)v.value<void *>();
        delete props;
    }
    m_WidgetInfo[EPropertyWidgetProperties] = NULL;
}

void WidgetParseManifest::interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties )
{
    if(m_WidgetInfo[EPropertyLicenseInfo].isValid())
        properties[ EPropertyLicenseInfo] = m_WidgetInfo[ EPropertyLicenseInfo ];
    if(m_WidgetInfo[EPropertyWidgetVersion].isValid())
        properties[ EPropertyWidgetVersion] = m_WidgetInfo[ EPropertyWidgetVersion ];
    if(m_WidgetInfo[EPropertyWidgetName].isValid())
        properties[ EPropertyWidgetName] = m_WidgetInfo[ EPropertyWidgetName ];
}


