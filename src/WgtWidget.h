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

#ifndef _WGT_WIDGET_H_
#define _WGT_WIDGET_H_

#include "wacwidgetmanagerconstants.h"
#include "wacSuperWidget.h"
#include "featuremapping.h"
#include "wacsecsession.h"

#if !defined (Q_OS_SYMBIAN)
namespace WAC{
   namespace Maemo{
       class WebWidget;
   }
}
#endif

class WebAppUniquenessHelper;
class WebAppLocalizer;

/*****************************************************************************
 * Wgt specific widget
 *
 * *************************************************************************/
class WgtWidget : public SuperWidget
{
public:
    explicit WgtWidget(QString& rootDirectory);
    virtual ~WgtWidget();

    // TODO : Code of these two are almost equal. Should be fixed.
    WidgetInstallError install(const bool update = false);
    WidgetInstallError install(const QString& sigId, const bool update = false);
    QString launcherPath(const QString &pkgPath);

    void writeManifest(const QString& path="");

    QString value(const QString& key, const QString & attribute = QString(""));

    /**
     * Returns true if the key with the attribute exists in w3c widget config; otherwise false.
     * @param key the config.xml element name
     * @attribute the attribute is an optional argument. With attribute you can test whether a value
     * of given attribute within specified key exists in config.xml element.
     */
    bool contains( const QString & key, const QString & attribute = QString(""));
    bool unZipBundle(const QString& path){return SuperWidget::unZipBundle(path);};
    bool parseManifest(const QString& path="", const bool minimal = false);
    bool allowRemovableInstallation();
    void disableBackupRestoreValidation(bool disableUnsignWidgetSignCheck);
#if ENABLE(AEGIS_LOCALSIG)
    bool isFromTrustedDomain();
#endif

protected :
    void initialize(QString& rootDirectory) {SuperWidget::initialize(rootDirectory);};
    WidgetProperties* widgetProperties(bool forceUpdate = false, bool minimal = false);
    WidgetProperties* minimalWidgetProperties(bool forceUpdate = false);
    bool findStartFile(QString& startFile, const QString& widgetPath="");
    bool findIcons(QStringList& icons, const QString& widgetPath="");
    bool findFeatures(WidgetFeatures& features, const QString& widgetPath="");
    bool isFeatureAllowed(WidgetFeatures& features, WAC::SecSession* secureSession, 
            bool runtime = false);
    bool createSecuritySession(WAC::SecSession** secureSession, QString& trustDomain,
            const QString& sigId="", bool runtime = false);
    bool getCertificateAKI(QString& aki);
    QString getTrustDomain();
    void setSecuritySessionString(WAC::SecSession* secureSession);

private:
    void saveStartFile(QString& startFile, const QString& widgetPath="");
    bool getStartFile(QString& startFile, const QString& widgetPath="");
    bool isDirectionValid(const QString& AttributeValue);

    WidgetInstallError setupSecuritySession();

#ifdef Q_OS_SYMBIAN
    QString setProcessUidInProps(WidgetProperties *props);
    void updatePaths(const QString& domainNameUid);
    QString getProcUid(const QString& domain) const;
    void restartNotificationL();


    HBufC* qt_QString2HBufCNewL(const QString& aString);


#endif
private:
    bool m_disableUnsignWidgetSignCheck;
    QString m_validCertificateAKI;
    FeatureMapping m_mapping;
    WebAppUniquenessHelper* m_uniquenessHelper;
    WebAppLocalizer* m_appLocalizer;
    static QHash<QString, QString> s_widgetStartFileCache;
    //friend class declaration starts here
    friend class WgtWidgetTest;

#if !defined(Q_OS_SYMBIAN)
   friend class WAC::Maemo::WebWidget;
#endif

};
#endif
