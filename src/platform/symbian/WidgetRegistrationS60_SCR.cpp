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

#include "WidgetRegistrationS60_SCR.h"
#include "SCRConstants.h"
#include "wacwebappinfo.h"
#include "wacWebAppRegistry.h"
#include "wacSuperWidget.h"
#include <qstring.h>
#include <QUrl>
#include <QTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QDir>
#include <apaid.h>
#include <s32mem.h>
#include <usif/usiferror.h>
#include <usif/sif/sifutils.h>

#include "configw3xmlparser.h"
#include "wacw3csettingskeys.h"

// Symbian file path separator.
_LIT(KPathSeperator, '\\');

HBufC* qt_QString2HBufCNewL(const QString& aString)
{
    HBufC *buffer;
#ifdef QT_NO_UNICODE
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString.toLocal8Bit().constData()));
    buffer = HBufC8::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#else
    TPtrC16 ptr(reinterpret_cast<const TUint16*>(aString.utf16()));
    buffer = HBufC16::NewL(ptr.Length());
    buffer->Des().Copy(ptr);
#endif
    return buffer;
}

WidgetRegistrationS60_SCR::WidgetRegistrationS60_SCR()
{
  languageMap["en"]      = ELangInternationalEnglish;
  languageMap["en-gb"]   = ELangEnglish;
  languageMap["fr"]      = ELangFrench;
  languageMap["de"]      = ELangGerman;
  languageMap["es"]      = ELangSpanish;
  languageMap["it"]      = ELangItalian;
  languageMap["sv"]      = ELangSwedish;
  languageMap["da"]      = ELangDanish;
  languageMap["no"]      = ELangNorwegian;
  languageMap["fi"]      = ELangFinnish;
  languageMap["en-us"]   = ELangAmerican;
  languageMap["fr-ch"]   = ELangSwissFrench;
  languageMap["ge-ch"]   = ELangSwissGerman;
  languageMap["pt"]      = ELangPortuguese;
  languageMap["tr"]      = ELangTurkish;
  languageMap["is"]      = ELangIcelandic;
  languageMap["ru"]      = ELangRussian;
  languageMap["hu"]      = ELangHungarian;
  languageMap["nl"]      = ELangDutch;
  languageMap["en-au"]   = ELangAustralian;
  languageMap["cs"]      = ELangCzech;
  languageMap["sk"]      = ELangSlovak;
  languageMap["pl"]      = ELangPolish;
  languageMap["sl"]      = ELangSlovenian;
  languageMap["zh"]      = ELangPrcChinese;
  languageMap["zh-hant"] = ELangHongKongChinese;
  languageMap["zh-hans"] = ELangPrcChinese;
  languageMap["ja"]      = ELangJapanese;
  languageMap["th"]      = ELangThai;
  languageMap["ar"]      = ELangArabic;
  languageMap["tl"]      = ELangTagalog;
  languageMap["bg"]      = ELangBulgarian;
  languageMap["ca"]      = ELangCatalan;
  languageMap["hr"]      = ELangCroatian;
  languageMap["en-ca"]   = ELangCanadianEnglish;
  languageMap["et"]      = ELangEstonian;
  languageMap["fa"]      = ELangFarsi;
  languageMap["fr-ca"]   = ELangCanadianFrench;
  languageMap["el"]      = ELangGreek;
  languageMap["he"]      = ELangHebrew;
  languageMap["hi"]      = ELangHindi;
  languageMap["id"]      = ELangIndonesian;
  languageMap["ko"]      = ELangKorean;
  languageMap["lv"]      = ELangLatvian;
  languageMap["lt"]      = ELangLithuanian;
  languageMap["ms"]      = ELangMalay;
  languageMap["ml"]      = ELangMalayalam;
  languageMap["pt-br"]   = ELangBrazilianPortuguese;
  languageMap["ro"]      = ELangRomanian;
  languageMap["sr"]      = ELangSerbian;
  languageMap["es"]      = ELangInternationalSpanish;
  languageMap["es-419"]  = ELangLatinAmericanSpanish;
  languageMap["ur"]      = ELangUrdu;
  languageMap["vi"]      = ELangVietnamese;
  languageMap["eu"]      = ELangBasque;
  languageMap["gl"]      = ELangGalician;
}


bool WidgetRegistrationS60_SCR::registerApp(
                 const QString& appId,
                 const QString& appTitle,
                 const QString& appPath,
                 const QString& dataPath,
                 const QString& startPath,
                 const QString& iconPath,
                 int size,
                 int widgetUid,
                 const QSet<QString>& languages,
                 const QString& version,
                 const QString& author,
                 const QString& widgetType,
                 const AttributeMap& attributes,
                 bool hideIcon)
{
    bool ret = false;
    TRAP_IGNORE(ret = registerAppL(appId, appTitle, appPath, dataPath, startPath, iconPath, size,
                                  widgetUid, languages, version, author, widgetType,
                                   attributes, hideIcon));
    return ret;
}

bool WidgetRegistrationS60_SCR::registerAppL(
                 const QString& appId,
                 const QString& appTitle,
                 const QString& appPath,
                 const QString& dataPath,
                 const QString& startPath,
                 const QString& iconPath,
                 int size,
                 int widgetUid,
                 const QSet<QString>& languages,
                 const QString& version,
                 const QString& author,
                 const QString& widgetType,
                 const AttributeMap& attributes,
                 bool hideIcon)
{
    bool ret(false);
    TComponentId compID = 0;

    HBufC *startFileVal = qt_QString2HBufCNewL(startPath);
    CleanupStack::PushL(startFileVal);

    HBufC *compUidName= qt_QString2HBufCNewL(SCR_PROP_COMPUID);
    CleanupStack::PushL(compUidName);

    HBufC *uidName= qt_QString2HBufCNewL(SCR_PROP_UID);
    CleanupStack::PushL(uidName);

    HBufC *appIdName= qt_QString2HBufCNewL(SCR_PROP_APPID);
    CleanupStack::PushL(appIdName);
    HBufC *appIdVal = qt_QString2HBufCNewL(appId);
    CleanupStack::PushL(appIdVal);

    HBufC *appPathName = qt_QString2HBufCNewL(SCR_PROP_APPPATH);
    CleanupStack::PushL(appPathName);
    HBufC *appPathVal = qt_QString2HBufCNewL(appPath);
    CleanupStack::PushL(appPathVal);

    HBufC *dataPathName = qt_QString2HBufCNewL(SCR_PROP_DATAPATH);
    CleanupStack::PushL(dataPathName);
    HBufC *dataPathVal = qt_QString2HBufCNewL(dataPath);
    CleanupStack::PushL(dataPathVal);

    HBufC *procUidName = qt_QString2HBufCNewL(SCR_PROP_PROCUID);
    CleanupStack::PushL(procUidName);

    HBufC *allowNetworkName = qt_QString2HBufCNewL(SCR_PROP_ALLOWNETWORKACCESS);
    CleanupStack::PushL(allowNetworkName);

    HBufC *miniviewSupportName = qt_QString2HBufCNewL(SCR_PROP_MINIVIEWSUPPORT);
    CleanupStack::PushL(miniviewSupportName);

    HBufC *nokiaWidgetName = qt_QString2HBufCNewL(SCR_PROP_NOKIAWIDGET);
    CleanupStack::PushL(nokiaWidgetName);

    // Calculate a few attribute values
    QVariant procUidProp = attributes["processUid"];
    QString procUidStr = procUidProp.toString();
    bool ok;
    int processUid = procUidStr.toInt(&ok, 16);

    // Conversion failed, invalid uid was probably added
    // to the plist.  Just set the processUid to zero.
    if (!ok){
        processUid = 0;
    }

    int isMiniviewSupported = 0;
    int nokiaWidget = 0;

    if (attributes.contains(W3CSettingsKey::WIDGET_VIEWMODES)) {
        QStringList viewModeList =
             attributes.value(W3CSettingsKey::WIDGET_VIEWMODES).toString().split(" ");

        foreach (const QString &str, viewModeList) {

            if (str.contains("minimized", Qt::CaseInsensitive)) {

                isMiniviewSupported = 1;
                break;
            }
        }
    }
    if (widgetType == WIDGET_PACKAGE_FORMAT_WGT ||
        widgetType == WIDGET_PACKAGE_FORMAT_JIL) {
        nokiaWidget = 2;
    }
    else if (widgetType == WIDGET_PACKAGE_FORMAT_SHARED_LIBRARY) {
        nokiaWidget = 3;
    }

    TInt err;
    // Opens SCR connection and initialize transaction.
    if (widgetUid)
    {
        TRAP_IGNORE(SCROpenL(4, 500));

        TRAP_IGNORE(compID = RegisterComponentL(author, version,
                                         appTitle, appId, (TInt64)size,
                                         TBool(false)));
    }

    if (compID)
    {
        ret = false;

        TRAP_IGNORE(m_SCRClient.RegisterComponentFileL(compID, *startFileVal));

        TRAP(err,
             SetComponentPropertyL(compID, *uidName, widgetUid);
             SetComponentPropertyL(compID, *compUidName, widgetUid);
             SetComponentPropertyL(compID, *appIdName, *appIdVal);
             SetComponentPropertyL(compID, *appPathName, *appPathVal);
             SetComponentPropertyL(compID, *dataPathName, *dataPathVal);
             SetComponentPropertyL(compID, *procUidName, processUid);
             SetComponentPropertyL(compID, *allowNetworkName, true);
             SetComponentPropertyL(compID, *miniviewSupportName, isMiniviewSupported);
             SetComponentPropertyL(compID, *nokiaWidgetName, nokiaWidget);
             ret = RegisterApplicationL(compID, appId, widgetUid, iconPath,
                                        widgetType, languages, hideIcon));

    } else {
        ret = false;
    }
    if (ret) {
        // Commit transaction and close SCR connection.
        TRAP(err, SCRCloseAndCommitL());
        if (err != KErrNone)
            ret = false;
    } else {
        TRAP_IGNORE(m_SCRClient.RollbackTransactionL());
        m_SCRClient.Close();
    }

    CleanupStack::PopAndDestroy(nokiaWidgetName);
    CleanupStack::PopAndDestroy(miniviewSupportName);
    CleanupStack::PopAndDestroy(allowNetworkName);
    CleanupStack::PopAndDestroy(procUidName);
    CleanupStack::PopAndDestroy(dataPathVal);
    CleanupStack::PopAndDestroy(dataPathName);
    CleanupStack::PopAndDestroy(appPathVal);
    CleanupStack::PopAndDestroy(appPathName);
    CleanupStack::PopAndDestroy(appIdVal);
    CleanupStack::PopAndDestroy(appIdName);
    CleanupStack::PopAndDestroy(uidName);
    CleanupStack::PopAndDestroy(compUidName);
    CleanupStack::PopAndDestroy(startFileVal);

    return ret;
}

bool WidgetRegistrationS60_SCR::unregister(const QString& appID)
{
  bool ret;

  TRAPD(err, ret = unregisterL(appID));
  if (err != KErrNone)
    return false;

  return ret;
}

bool WidgetRegistrationS60_SCR::unregisterL(const QString& appID)
{
    TComponentId compID = 0;
    bool ret(false);
    HBufC *id;
    TRAPD(err, id  = qt_QString2HBufCNewL(appID));

    if (err != KErrNone)
    {
        return false;
    }
    else
    {
        err = KErrNone;
    }
    CleanupStack::PushL(id);

    // Opens SCR connection and initialize transaction.
    TRAP_IGNORE(SCROpenL(4, 500));

    TRAP(err, compID = m_SCRClient.GetComponentIdL(*id, Usif::KSoftwareTypeWidget));
    // If component does not exist in SCR, no need to return failure.
    if (err == KErrNotFound)
    {
        ret = true;
    }
    if (compID)
    {
        TRAP_IGNORE(m_SCRClient.DeleteComponentL(compID));
        ret = true;
    }

    CleanupStack::PopAndDestroy(id);

    // Commit transaction and close SCR connection.
    TRAP_IGNORE(SCRCloseAndCommitL());

    return ret;
}

bool WidgetRegistrationS60_SCR::RegisterApplicationL(const TComponentId& aComponentId,
                                                     const QString& aAppId,
                                                     int aAppUid,
                                                     const QString& iconPath,
                                                     const QString& widgetType,
                                                     const QSet<QString>& languages,
                                                     bool hideIcon)
{
    Q_UNUSED(widgetType);
    WebAppInfo info;
    RPointerArray<HBufC> ownedFileArray;
    RPointerArray<Usif::CServiceInfo> serviceArray;
    RPointerArray<Usif::CPropertyEntry> appPropertiesArray;
    RPointerArray<Usif::CAppViewData> viewDataList;
    RPointerArray<Usif::CLocalizableAppInfo> localizableAppInfoList;
    RPointerArray<Usif::COpaqueData> opaqueDataArray;

    TUid appUid = TUid::Uid(aAppUid);
    WebAppRegistry* webAppReg = WebAppRegistry::instance();
    TInt numIcons = 0;
    bool ret = false;
    
    if (webAppReg->isRegistered(aAppId, info))
    {
        TFileName appFileName;
        appFileName.Copy(KPathSeperator);
        appFileName.AppendNum(appUid.iUid);

        QSetIterator<QString> lang(languages);
        QString currentLang;
        QList<W3CElement*> tmpElemList;
        W3CElement* tmpElem;
        HBufC* tmpCaption = NULL;
        HBufC* tmpShortCaption = NULL;
        HBufC* iconFileName = NULL;
        bool isPrcChineseLocalized = false;

        while (lang.hasNext()){
            currentLang = lang.next();
            TLanguage tmpLanguage = languageMap[currentLang];
            // IANA lang tags "zh" and "zh-hans" must both map to PRC Chinese.
            // Check that appInfo is not registered more than once for PRC Chinese.
            // Also check if TLanguage val is ELangTest.  If so, language is not
            // supported and the app info must not be registered for this language.
            if ((((currentLang == "zh") || (currentLang == "zh-hans")) &&
                isPrcChineseLocalized) || (tmpLanguage == ELangTest))
            {
                continue;
            }
            else
            {
                tmpElemList = info.getElement(W3CSettingsKey::WIDGET_NAME, 0, currentLang);
                if (!tmpElemList.isEmpty())
                {
                    tmpElem = tmpElemList[0];
                    tmpCaption = qt_QString2HBufCNewL(tmpElem->readElementText());
                    CleanupStack::PushL(tmpCaption);
                    QString shortVal = tmpElem->attributeValue("short");
                    if (!shortVal.isEmpty())
                    {
                        tmpShortCaption = qt_QString2HBufCNewL(shortVal);
                        CleanupStack::PushL(tmpShortCaption);
                    } else {
                        tmpShortCaption = tmpCaption->Alloc();
                        CleanupStack::PushL(tmpShortCaption);
                    }
                } else {
                    // if there is no widget name in this locale, register the default name
                    tmpCaption = qt_QString2HBufCNewL(info.appTitle());
                    CleanupStack::PushL(tmpCaption);
                    tmpShortCaption = qt_QString2HBufCNewL(info.appTitle());
                    CleanupStack::PushL(tmpShortCaption);
                }

                iconFileName = qt_QString2HBufCNewL(iconPath);
                CleanupStack::PushL(iconFileName);

                CCaptionAndIconInfo *tmpCaptionAndIconInfo =
                    CCaptionAndIconInfo::NewLC(*tmpCaption, *iconFileName, 0);

                CLocalizableAppInfo *tmpLocAppInfo =
                  CLocalizableAppInfo::NewLC(*tmpShortCaption, tmpLanguage, KNullDesC,
                                               tmpCaptionAndIconInfo, viewDataList);

                localizableAppInfoList.AppendL(tmpLocAppInfo);

                if ((currentLang == "zh") || (currentLang == "zh-hans"))
                {
                    isPrcChineseLocalized = true;
                }

                CleanupStack::Pop(tmpLocAppInfo);
                CleanupStack::Pop(tmpCaptionAndIconInfo);

                CleanupStack::PopAndDestroy(iconFileName);
                iconFileName = NULL;
                CleanupStack::Pop(tmpShortCaption);
                tmpShortCaption = NULL;
                CleanupStack::Pop(tmpCaption);
                tmpCaption = NULL;
            }
        } //  end of 'while (lang.hasNext())'

        TApplicationCharacteristics appCharacteristics;
        appCharacteristics.iAttributes = TApaAppCapability::ENonNative;

        if (hideIcon) {
            appCharacteristics.iAppIsHidden = ETrue;
        }

        TBuf8<16> opaqueData;
        RDesWriteStream writeStream( opaqueData );

        writeStream.WriteUint32L( appUid.iUid );
        writeStream.WriteUint32L( 0 );
        writeStream.CommitL();

        COpaqueData* cod = COpaqueData::NewLC(opaqueData, (TLanguage)0);
        opaqueDataArray.Append(cod);

        CApplicationRegistrationData *appRegData =
            CApplicationRegistrationData::NewLC(ownedFileArray,
                                                serviceArray,
                                                localizableAppInfoList,
                                                appPropertiesArray,
                                                opaqueDataArray,
                                                appUid, appFileName,
                                                appCharacteristics, 0);

        TRAPD(err, m_SCRClient.AddApplicationEntryL(aComponentId, *appRegData));
        CleanupStack::Pop(appRegData);
        CleanupStack::Pop(cod);
        if (err == KErrNone)
        {
            ret = true;
        }
    } else {
          ret = false;
    }
    return ret;
}

TComponentId WidgetRegistrationS60_SCR::RegisterComponentL(const QString &aVendor,
                                                           const QString &aVersion,
                                                           const QString &aName,
                                                           const QString &aGlobalId,
                                                           const TInt64& aComponentSize,
                                                           const TBool& aIsDrmProtected)

{
    HBufC *name = qt_QString2HBufCNewL(aName);
    CleanupStack::PushL(name);
    HBufC *vendor = qt_QString2HBufCNewL(aVendor);
    CleanupStack::PushL(vendor);
    HBufC *version = qt_QString2HBufCNewL(aVersion);
    CleanupStack::PushL(version);
    HBufC *globalId = qt_QString2HBufCNewL(aGlobalId);
    CleanupStack::PushL(globalId);

    TComponentId componentId = 0;
    bool isUnregistered = false;
    // Handle the KErrAlreadyExists error thrown by AddComponentL.
    // Sometimes, SCR still contains a component entry for a widget that was uninstalled.
    // This can be due to botched install, crash during deregistration in web app registry,
    // or other similar reason.  Thus it's important to check registry for the component
    // before registering.  If it already exists in SCR, it must first be removed and then
    // registered.
    TRAPD(err, componentId = m_SCRClient.AddComponentL(*name, *vendor,
                                                       Usif::KSoftwareTypeWidget, globalId,
                                                       EScrCompInstall));

    if (err == KErrAlreadyExists)
    {
        isUnregistered = unregister(aGlobalId);
        if (isUnregistered)
        {
            componentId = m_SCRClient.GetComponentIdL(*globalId, Usif::KSoftwareTypeWidget);
            m_SCRClient.DeleteComponentL(componentId);
            componentId = m_SCRClient.AddComponentL(*name, *vendor, Usif::KSoftwareTypeWidget,
                                                globalId, EScrCompInstall);
        }
    }

    if (componentId) {
      m_SCRClient.SetVendorNameL(componentId, *vendor);
      m_SCRClient.SetComponentVersionL(componentId, *version);
      m_SCRClient.SetComponentSizeL(componentId, aComponentSize);
      m_SCRClient.SetIsComponentDrmProtectedL(componentId, aIsDrmProtected);
    }

      CleanupStack::PopAndDestroy(globalId);
      CleanupStack::PopAndDestroy(version);
      CleanupStack::PopAndDestroy(vendor);
      CleanupStack::PopAndDestroy(name);


    return componentId;
}

void WidgetRegistrationS60_SCR::SetComponentPropertyL(const TComponentId& aComponentId, const TDesC& aName, const TInt64& value)
{
  m_SCRClient.SetComponentPropertyL(aComponentId, aName, value);
}

void WidgetRegistrationS60_SCR::SetComponentPropertyL(const TComponentId& aComponentId, const TDesC& aName, const TDesC& value)
{
  m_SCRClient.SetComponentPropertyL(aComponentId, aName, value);
}

/*
 * Establishes connection to SCR and starts a transaction, retrying per parameters
 * if transaction fails due to existing transaction
 * @param retries number of retry attempts to make
 * @param retryInterval milliseconds to wait between retry attempts
 */
void WidgetRegistrationS60_SCR::SCROpenL(int retries, unsigned long retryInterval)
{
    TInt err;
    TInt attempts = 0;

    if (retries < 1)
        retries = 1;

    User::LeaveIfError(m_SCRClient.Connect());
    while (attempts++ < retries) {
        TRAP(err, m_SCRClient.CreateTransactionL());
        if ((err == KErrScrWriteOperationInProgress) ||
            (err == KErrScrReadOperationInProgress))
        {
            if (attempts == retries)
                User::Leave(err);

            // Wait to retry
            QEventLoop loop;
            QTimer::singleShot(retryInterval, &loop, SLOT(quit()));
            loop.exec();
        } else if (err != KErrNone) {
            User::Leave(err);
        } else {
            // Success
            return;
        }
    }
}

void WidgetRegistrationS60_SCR::SCRCloseAndCommitL(void)
{
    TRAP_IGNORE(m_SCRClient.CommitTransactionL());
    m_SCRClient.Close();
}

void WidgetRegistrationS60_SCR::SetIsPresentL(const QString& appId, bool isPresent)
{
    HBufC *globalId = qt_QString2HBufCNewL(appId);
    CleanupStack::PushL(globalId);

    // Opens SCR connection and initialize transaction.
    TRAP_IGNORE(SCROpenL(4, 500));

    TComponentId compId = m_SCRClient.GetComponentIdL(*globalId, Usif::KSoftwareTypeWidget);

    TInt err;
    TRAP(err, m_SCRClient.SetIsComponentPresentL(compId, isPresent));

    // Commit transaction and close SCR connection.
    TRAP_IGNORE(SCRCloseAndCommitL());

#ifdef QTWRT_USE_USIF
    WebAppInfo info;
    if (WebAppRegistry::instance()->isRegistered(appId, info)) {
        if (isPresent) {
            TRAP_IGNORE(SuperWidget::NotifyAppArcOfInstallL(info.uid()));
        } else {
            TRAP_IGNORE(SuperWidget::NotifyAppArcOfUninstallL(info.uid()));
        }
    }
#endif

    CleanupStack::PopAndDestroy(globalId);
}

void WidgetRegistrationS60_SCR::SetPropertyL(const QString& appId, const QString& attr,
                                             bool value)
{
    HBufC *globalId = qt_QString2HBufCNewL(appId);
    CleanupStack::PushL(globalId);

    HBufC *property = qt_QString2HBufCNewL(attr);
    CleanupStack::PushL(property);

    // Opens SCR connection and initialize transaction.
    TRAP_IGNORE(SCROpenL(4, 500));

    TComponentId compId = m_SCRClient.GetComponentIdL(*globalId, Usif::KSoftwareTypeWidget);

    TInt err;
    TRAP(err, SetComponentPropertyL(compId, *property, value));

    // Commit transaction and close SCR connection.
    TRAP_IGNORE(SCRCloseAndCommitL());

    CleanupStack::PopAndDestroy(property);
    CleanupStack::PopAndDestroy(globalId);
}

// Origin verification happens after SCR component and application registration of
// the widget.  WgtWidget stores the certificateAki in WebAppRegistry only after
// the widget has gone through registration process.  Thus, WebAppRegistry must
// call SetIsComponentOriginVerifiedL() when certificateAki is set.
void WidgetRegistrationS60_SCR::SetIsComponentOriginVerifiedL(const QString& appId,
                                                              bool aIsOriginVerified)
{
    HBufC *id = qt_QString2HBufCNewL(appId);
    CleanupStack::PushL(id);

    // Opens SCR connection and initialize transaction.
    TRAP_IGNORE(SCROpenL(500, 4));

    TComponentId compId = m_SCRClient.GetComponentIdL(*id, Usif::KSoftwareTypeWidget) ;

    TRAPD(err, m_SCRClient.SetIsComponentOriginVerifiedL(compId, aIsOriginVerified));

    // Commit transaction and close SCR connection.
    TRAP_IGNORE(SCRCloseAndCommitL());

    CleanupStack::PopAndDestroy(id);
}
