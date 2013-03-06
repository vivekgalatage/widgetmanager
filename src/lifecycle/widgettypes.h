#ifndef WIDGETTYPES_H
#define WIDGETTYPES_H

#include "widgetlog.h"

#include <QString>
#include <QMap>

enum WidgetContextType
{
    EContextInvalid = -1,
    EContextInstall,
    EContextUpdate,
    EContextUninstall,
    EContextBackup,
    EContextRestore
} ;

enum WidgetOperationType
{
    EOperationInvalid = -1,
    EOperationPreparing,
    EOperationAny,
    
    EOperationInitialize,
    
    EOperationLegacyContentExtraction,
    EOperationLegacyValidateDigitalSignature,
    EOperationLegacyParseManifest,
    EOperationLegacyPrepareForInstall,
    EOperationLegacySetupSecuritySession,
    EOperationLegacyFinalizeInstallation
} ;

enum WidgetPropertyType
{
    EPropertyInvalid = -1,

    EPropertyErrorCode,
    EPropertyErrorMessage,

    EPropertyFilePath,
    EPropertyWidgetId,
    EPropertyInteractiveMode,
    EPropertyUpdate,
    EPropertyInstalledVersion,
    EPropertyContinueInstallation,

    EPropertyWidgetSize,

    EPropertyWidgetName,
    EPropertyWidgetDescription,


//  EOperationInitialize properties
    EPropertyDestinationDrive,
    EPropertyInstallDirectory,
    EPropertyContentDirectory,
    EPropertyBackupDirectory,
    EPropertySettingsDirectory,
    EPropertyMaxReadBytes,

//  EOperationValidateDigitalSignature
    EPropertyCertificateAKIs,	
    EPropertyAllowUntrustedWidget,
    EPropertyAuthor,
    EPropertyDeveloperSigned,    

//  EOperationValidateProcessCapabilities
    EPropertyProcessCapabilityValid,

//  EOperationInitialize properties
    EPropertyInitialisationCompleted,

//  EOperationParseManifest
    EPropertyWidgetType,
    EPropertyUniqueId,
    EPropertyWidgetProperties,
    EPropertyWidgetVersion,
    EPropertyLicenseInfo,
	
    EPropertyWidgetFeatures,
	EPropertyAllowFeatureAccess,

	EPropertyAllowOverwrite,
    EPropertyAlreadyExists,
    EPropertyProcessUid
} ;


enum WidgetErrorType
{
    EErrorInvalid                       = -1,

	EErrorNone                          = 0,
	
	EErrorUserCancel					= 1000,

    EErrorGeneral                       = 2000,
    EErrorOperationNotRegistered,
    EErrorNoEnoughSpace,
    EErrorPermissionDenied,
    EErrorFileSystemPermissionDenied,
    EErrorInvalidPath,
    EErrorInvalidWidget,
    
    EErrorInitializerGeneral            = 3000,

    EErrorContentExtractionGeneral      = 4000,
    EErrorContentReadError,
    EErrorContentWriteError,

    EErrorParseManifestGeneral          = 5000,
    EErrorConfigFileNotFound,
    EErrorStartFileNotFound,

    EErrorDigSigValidationGeneral       = 6000,
    EErrorDigitalSignatureValidationFailed,
    EErrorCertificateRevoked,
	
	EErrorUpdateGeneral                 = 7000,  
	
	EErrorSetupSecuritySessionGeneral	= 8000,
	EErrorFeatureNotAllowed,    

    EErrorRegistrationGeneral           = 9000
    
} ;

#ifdef ENABLE_CONFIG_PROPERTIES
    #define BEGIN_CONFIG_PROPERTIES \
        static QMap<QString, WidgetPropertyType> configProperties; \
        bool insert( const QString& key, WidgetPropertyType value ) \
        { \
            configProperties.insert( key, value ); \
            return true; \
        }
    #define CONFIG_PROPERTY( property ) \
        static bool flag##property = insert( #property, EProperty##property );
    #define END_CONFIG_PROPERTIES
#else // !ENABLE_CONFIG_PROPERTIES
    #define BEGIN_CONFIG_PROPERTIES
    #define CONFIG_PROPERTY( property )
    #define END_CONFIG_PROPERTIES
#endif // ENABLE_CONFIG_PROPERTIES

BEGIN_CONFIG_PROPERTIES
    CONFIG_PROPERTY( WidgetId )
    CONFIG_PROPERTY( InstallDirectory )
    CONFIG_PROPERTY( ContentDirectory )
    CONFIG_PROPERTY( InteractiveMode )
    CONFIG_PROPERTY( AllowOverwrite )
    CONFIG_PROPERTY( AllowUntrustedWidget )
    CONFIG_PROPERTY( AllowFeatureAccess )
END_CONFIG_PROPERTIES

#endif // WIDGETTYPES_H
