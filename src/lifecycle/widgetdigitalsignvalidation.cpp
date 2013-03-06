#include "widgetdigitalsignvalidation.h"
#include "DigSigService.h"
#include "widgetconstants.h"

#include "wacsettings.h"
#include "wactrustsession.h"

#include <QDir>

REGISTER_OPERATION( EOperationLegacyValidateDigitalSignature , WidgetDigSignValidation )

WidgetDigSignValidation::WidgetDigSignValidation( WidgetInformation& widgetInfo )
    : WidgetOperation( widgetInfo )
{
}

WidgetDigSignValidation::~WidgetDigSignValidation()
{
}

void WidgetDigSignValidation::execute()
{
// TODO:Signature certificate validation and set CertificateAKI
    WidgetErrorType error = EErrorNone;
    if( ( error = validateDigitalSignature() ) == EErrorNone )
    {
        if( (error = validateTrustDomain() ) == EErrorNone )
            emit completed();
        else
            emit aborted( error );
    }
    else
        emit aborted( error );
}

WidgetErrorType WidgetDigSignValidation::validateDigitalSignature()
{
    WidgetValidationError validationError( ValidateOK );
    WidgetErrorType m_DigSigError = EErrorNone;
    QString commaSeparatedAKIs;
    QDir resourcePath(m_WidgetInfo[ EPropertyContentDirectory ].toString());

   // Widget which has digital signatures must have valid dig sigs.
    DigSigService* dsService = new DigSigService();
    validationError = dsService->validateWidget(resourcePath.absolutePath());

    if (validationError == ValidateOK) {
        QStringList certificateAKIs = dsService->getCertificateAKIs();
        if(!(certificateAKIs.isEmpty())) {
            m_WidgetInfo[ EPropertyCertificateAKIs ] = certificateAKIs;
        }          
    }
    else 
    {
           //Check if untrusted widget installation is allowed
            if((m_WidgetInfo[ EPropertyAllowUntrustedWidget].toBool()) == true ) 
            {
                switch(validationError)
                {
                    case CertificateRevoked:
                        //Certificate Revoked. Abort installation
                        m_DigSigError = EErrorCertificateRevoked;
                        break; 
                    default:
                       {
                        //emit and get the user response for proceeding with signature invalid widget installation
                        if (m_Mode == EModeInteractive) {
                            QMap< WidgetPropertyType, QVariant > propertyMap;
                            interactiveProperties( propertyMap );
                        }
                       }
                }
            }
            else {
             // Untrusted Widget Installation NOT ALLOWED
             return EErrorDigitalSignatureValidationFailed;
            }
    }

	if(dsService->isTestSigned())
        m_WidgetInfo[EPropertyDeveloperSigned ] = "true";
    else
        m_WidgetInfo[EPropertyDeveloperSigned ] = "false";		
    
    // Author Info from DigSigService
    AuthorInfo author;
    dsService->getAuthorInfo(author);
    
    if(author.isRecognized)
        m_WidgetInfo[EPropertyAuthor] = author.authorName;
    else
        m_WidgetInfo[EPropertyAuthor] = "unrecognized";
    
    delete dsService;
    return m_DigSigError;
}

QString WidgetDigSignValidation::getRootDirectory( const QString& rootDirectory, const QString& processUid )
{
    QString result = NULL;
    QChar driveLetter(NULL);
    
    if ( rootDirectory.length() >= 1 ) 
    {
        driveLetter = rootDirectory[0];
        if (!driveLetter.isLetter())
            return QString();

        // Handle edge case when only drive letter is available from rootDirectory (and no subdirectories) 
        else if (rootDirectory.length() <= strlen("A:\\")) 
                result = QDir::toNativeSeparators( QString(driveLetter) + QDir::separator() + PrivateDirectory + QDir::separator() + processUid );
                
        // Use the passed driveletter and root dir
        else
        {
            QString rootDrive = rootDirectory;
            if (!(rootDrive.startsWith("\\") || rootDrive.startsWith("/")))
                rootDrive.remove(0, 2);
            QString rootPath = "";
            rootPath.append(driveLetter);
            rootPath = rootPath+':'+rootDrive;
            rootPath = QDir::toNativeSeparators(rootPath);
            result = rootPath;
        }
    } 
    else 
    {
        if (driveLetter.isNull()) {
            result = DefaultDrive + QDir::separator() + PrivateDirectory + QDir::separator() + processUid; //DefaultRootDirectory;
        }
    }
    
    QString installPath("");
    installPath = QDir::toNativeSeparators(result);
    
    if (installPath.startsWith("Z"))
        installPath.replace(0, 1, "C");
    
    QString privatePath = installPath[0]+":"+QDir::separator()+"private";
    if ( !createDir(privatePath) ) 
    {
        return QString();
    }
    
    if (! createDir(installPath)) {
        return QString();
    }
    installPath = installPath + QDir::separator() + WidgetFolder;
    
    if (! createDir(installPath)) {
        return QString();
    }
    return installPath;   
}

bool WidgetDigSignValidation::createDir(const QString& path)
{
  QDir dir(path);
  if (!dir.exists())
  {
    if (!dir.mkpath(dir.absolutePath())) 
    {
      return false;
    }
  }
  return true;
}

WidgetErrorType WidgetDigSignValidation::updateInstallationPaths( const QString& processUid )
{
    if( processUid.isEmpty() )
        return EErrorDigSigValidationGeneral;

    QString rootDirectory = m_WidgetInfo[ EPropertyInstallDirectory ].toString();
    QString rootDir = getRootDirectory( rootDirectory, processUid );
    if(rootDir.isEmpty() || rootDir.isNull() )
        return EErrorFileSystemPermissionDenied;
    
    m_WidgetInfo[ EPropertyInstallDirectory ] = rootDir;
    m_WidgetInfo[ EPropertySettingsDirectory ]  = DefaultDrive + QDir::separator() + PrivateDirectory + QDir::separator() + processUid + QDir::separator() + DataFolderName; //DataPath;

    return EErrorNone;
}

QString WidgetDigSignValidation::getProcessUid(QString& domain) const
{
    QSettings settings(MultiProcessIniPath,QSettings::IniFormat);    
    QString uid = settings.value(domain +"/processuid").toString();
    if(uid.isEmpty())
    {
        domain = DefaultDomain;
        uid = settings.value(DefaultDomain+"/processuid").toString();
    }
    return uid;
}

WidgetErrorType WidgetDigSignValidation::allowInstallation( const QString& domain )
{
    QSettings settings(MultiProcessIniPath,QSettings::IniFormat);
    bool isallowed = settings.value(domain +"/allowinstall").toBool();
    if( !isallowed )
    {
        QStringList alloweduid3 = settings.value(domain +"/alloweduid3").toStringList();
        QString widgetFilePath = m_WidgetInfo[ EPropertyFilePath ].toString();
        bool isAllowed = false;
        for( int i=0; i< alloweduid3.count(); i++ )
        {
            if( !(alloweduid3.at(i).isEmpty()) )
            {
                QString path = PrivatePath + QDir::separator() + alloweduid3.at(i);
                if(widgetFilePath.contains(path,Qt::CaseInsensitive))
                {
                    isAllowed = true;
                    break;
                }
            }
        }
        if( !isAllowed )
            return EErrorPermissionDenied;
    }
    return EErrorNone;
}   

WidgetErrorType WidgetDigSignValidation::validateTrustDomain()
{
    WAC::WrtSettings* m_settings = WAC::WrtSettings::createWrtSettings();
    QString trustPath =     m_settings->valueAsString("SecurityTrustPolicyFile"); 
    bool allow = m_settings->value("SideLoading").toBool();
    WAC::TrustSession trustSession(trustPath);
    QString trustDomain = "";
    QString processUid = "";
    
    if(m_WidgetInfo[EPropertyCertificateAKIs].isValid()) {
        QString certAKI = (m_WidgetInfo[EPropertyCertificateAKIs].toStringList()).at(0);
        trustDomain = trustSession.domainFor("certificate",certAKI);
    } 
    else 
        trustDomain = UntrustedWidgets;

    if( m_WidgetInfo[EPropertyDeveloperSigned ] == "true" )
        trustDomain = DeveloperSigned;
    
    processUid = getProcessUid( trustDomain );
    m_WidgetInfo[ EPropertyProcessUid ] = processUid;
    
    if( !allow )
    {
        WidgetErrorType error = allowInstallation(trustDomain);
        if( error != EErrorNone )
            return error;
    }

    return updateInstallationPaths(processUid);
}

void WidgetDigSignValidation::restore()
{
    m_WidgetInfo[ EPropertyCertificateAKIs ] = NULL;
    m_WidgetInfo[ EPropertyAllowUntrustedWidget] =NULL;
}

void WidgetDigSignValidation::interactiveProperties( QMap<WidgetPropertyType, QVariant> &properties )
{
     properties[ EPropertyAllowUntrustedWidget ] = m_WidgetInfo[ EPropertyAllowUntrustedWidget ];
    if(m_WidgetInfo[EPropertyAuthor].isValid())
        properties[EPropertyAuthor] = m_WidgetInfo[EPropertyAuthor];
}

WidgetErrorType WidgetDigSignValidation::finalize()
{
    if ((m_WidgetInfo[ EPropertyAllowUntrustedWidget ].toBool())==false)
        return EErrorUserCancel;
    return EErrorNone;
}

