#include "widgetsecuritysessionsetup.h"
#include "DigSigService.h"
#include "widgetconstants.h"
#include "wacsettings.h"
#include "wactrustsession.h"
#include "wacsecmgrdefs.h"
#include "wacsecmgr.h"

#include <QDir>
#include <QVariant>
#include <QMap>
#include <QList>
#include <QStringList>

#define EXTERNAL_ACCESS "widget/access/1"



REGISTER_OPERATION( EOperationLegacySetupSecuritySession , WidgetSetupSecuritySession )

const QString FEATURE_ATTR_NAME("name");
const QString FEATURE_ATTR_REQD("required");

WidgetSetupSecuritySession::WidgetSetupSecuritySession( WidgetInformation& widgetInfo )
    : WidgetOperation( widgetInfo ),
      m_hasFeatures( false )
{
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
    m_storage = WAC::Storage::createInstance(WIDGET_STORAGE);
#endif
    QVariant v = widgetInfo[EPropertyWidgetProperties];
    m_widgetProps = (WidgetProperties *)v.value<void *>();
}

WidgetSetupSecuritySession::~WidgetSetupSecuritySession()
{
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
    delete m_storage;
#endif

}

void WidgetSetupSecuritySession::execute()
{
    WidgetErrorType err = setupSecuritySession();
    if (err == EErrorNone)
    {
        emit completed();
        return;
    }
    emit aborted( err );
    return;
}

WidgetErrorType WidgetSetupSecuritySession::setupSecuritySession()
{
	QString trustDomain;
    WAC::SecSession* secureSession;
    QList<QString> required_capabilities;
    QList<QString> optional_capabilities;

    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();

    // get policy files from WrtSettings
    QString trustPath =     m_settings->valueAsString("SecurityTrustPolicyFile");

	qDebug()<<"OPTINSTALLER:: WidgetSetupSecuritySession::createSecuritySession() trustPath -"<< trustPath;		

    // create a trust session
    WAC::TrustSession trustSession(trustPath);

    // get trust domain
    if(m_WidgetInfo[EPropertyCertificateAKIs].isValid()) {
        QString certAKI = (m_WidgetInfo[EPropertyCertificateAKIs].toStringList()).at(0);
        trustDomain = trustSession.domainFor("certificate",certAKI);
    } else {
        // if we end up with the default domain, make sure it's UntrustedWidgets
        // redundant - change default domain in browser_access_policy.xml
		trustDomain = "UntrustedWidgets";
    }

	if( (m_WidgetInfo[EPropertyDeveloperSigned ] == "true") && (trustDomain == "TrustedWidgets" ))
		trustDomain = "Developer";
	
	qDebug()<<"OPTINSTALLER:: WidgetSetupSecuritySession::createSecuritySession() trustDomain -"<< trustDomain;	

	if(m_WidgetInfo[EPropertyWidgetFeatures].isValid()) {
		WidgetFeatures features = getFeaturesFromFeatureList();
		if(features.count() > 0){
			if (!m_mapping.getCapabilities(features,required_capabilities,optional_capabilities))
				return EErrorFeatureNotAllowed;
		}
	}

	bool hasAccessTag = false;
	if(m_widgetProps){
 		hasAccessTag = ( m_widgetProps->plistValue( EXTERNAL_ACCESS) ).isValid();
		qDebug()<<"OPTINSTALLER:: WidgetSetupSecuritySession::createSecuritySession(): hasAccessTag"<< hasAccessTag;
	}else{
		qDebug()<<"OPTINSTALLER:: WidgetSetupSecuritySession::createSecuritySession() m_widgetProps is NULL";	
	}


	WAC::SecMgr secMgr;
	if(	!secMgr.createSecsession(&secureSession, trustDomain, required_capabilities, optional_capabilities, hasAccessTag))
		return EErrorFeatureNotAllowed;

	setSecuritySessionString(secureSession);
	delete secureSession;
	return EErrorNone;

}

//Needed to convert from stored WidgetInformation to WidgetFeatures type for passing to FeatureMapping APIs. To be removed if Featuremapping API gets corrected
WidgetFeatures WidgetSetupSecuritySession::getFeaturesFromFeatureList()
{
    WidgetFeatures features;
    QList<QVariant> featureList;
    featureList = (m_WidgetInfo[EPropertyWidgetFeatures].toList());
    for(int i=0; i<featureList.count();i++)
    {
        QMap<QString,QVariant> feature = featureList.at(i).toMap();
        QString featureName = feature.value(FEATURE_ATTR_NAME).toString();
        QString featureReqd = feature.value(FEATURE_ATTR_REQD).toString();
        if (!featureName.isEmpty()) {
            features.insert(featureName ,featureReqd);
        }
    }
    return features;
}


//sets securitySessionString to widget properties
void WidgetSetupSecuritySession::setSecuritySessionString(WAC::SecSession* secureSession)
{
    if (m_widgetProps) {
        secureSession->setClientInfo(WAC::KWIDGETPATH, m_widgetProps->installPath());
        QByteArray secureSessionByteArr;
        secureSession->persist(secureSessionByteArr);
        QString secureSessionString(secureSessionByteArr);
        m_widgetProps->setSecureSessionString(secureSessionString);
    }
}

bool WidgetSetupSecuritySession::createSecuritySessionFile()
{
    if (m_widgetProps) {
        QString resourcesDir = m_widgetProps->resourcePath();
        if (!resourcesDir.isNull() && !resourcesDir.isEmpty()) {
            QDir dir;
            if(dir.exists(resourcesDir)) {
                QString secSessionFileName(QDir::toNativeSeparators(resourcesDir));
                secSessionFileName += QDir::separator();
                secSessionFileName += SecsessionFileName;
                QFile secSessionFile(secSessionFileName);
                if (secSessionFile.open(QIODevice::WriteOnly)) {
                    QTextStream stream(&secSessionFile);
                    stream << m_widgetProps->secureSessionString(); //codescanner::leave
                    secSessionFile.close();
                }
                if(!secSessionFile.exists())
                    return false;
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
                m_storage->add(secSessionFile);
#endif
            }
            else
                return false;
       }
    }
    return true;
}

void WidgetSetupSecuritySession::setProcessUidInProps()
{
    QString procUid = m_WidgetInfo[ EPropertyProcessUid ].toString();
    const QString Process(PROCESS_UID);
    AttributeMap map = m_widgetProps->plist();
    map.insert(Process,procUid);
    m_widgetProps->setInfoPList(map);
}

void WidgetSetupSecuritySession::restore()
{
    if( isFinalized() )
    {
        // Remove SecSession
        QString resourcesDir = m_widgetProps->resourcePath();
        // Leave widget data, remove only secsession
        QString secSessionFileName(QDir::toNativeSeparators(resourcesDir));
        secSessionFileName += QDir::separator();
        secSessionFileName += SecsessionFileName;
        QFile::remove(secSessionFileName);
    
#ifdef CWRT_WIDGET_FILES_IN_SECURE_STORAGE
        m_storage->remove(secSessionFileName);
#endif
    }
    m_widgetProps->setSecureSessionString(NULL);
    m_widgetProps->setSecureSessionPath(NULL);
    
}

void WidgetSetupSecuritySession::interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties )
{
    if( m_hasFeatures )
        properties[ EPropertyAllowFeatureAccess] = m_WidgetInfo[ EPropertyAllowFeatureAccess ];
}


WidgetErrorType WidgetSetupSecuritySession::finalize()
{
    if ( m_hasFeatures && (m_WidgetInfo[ EPropertyAllowFeatureAccess ].toMap().value("allow")).toBool() != true )
        return EErrorUserCancel;    
    
    if (m_mapping.getWidgetType() == WidgetTypeJIL)
        m_widgetProps->setType(WIDGET_PACKAGE_FORMAT_JIL);
    
    if(createSecuritySessionFile()) {
        m_widgetProps->setSecureSessionPath(m_widgetProps->resourcePath() + SecsessionFileName);
        setProcessUidInProps();         //To be moved to parser operation
    }
    
    return EErrorNone;
}
