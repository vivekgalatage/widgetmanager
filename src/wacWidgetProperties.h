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


#ifndef _WAC_WIDGET_PROPERTIES_H_
#define _WAC_WIDGET_PROPERTIES_H_

#include <QMap>
#include <QString>
#include <QVariant>
#include <QSize>

#include "wacWebAppRegistry.h"

enum CapabilityKey
{
    Internet,
    SOMEMORE,
    Location,
};

/*!
 \class WidgetProperties
 \brief Information container class for widget files only with .wgz extention. Whether this has to be plugin class or not TBD
*/
class WidgetProperties
{
  public:

    // default constructor
    WidgetProperties()
        {
          m_capabilities = 0;
          m_size = 0;
          m_sharedLibrary = false;
          m_sharedLibraryWidget = false;
          m_allowBackgroundTimers = false;
          m_hideIcon = false;
          m_installPath = "";
#ifdef OPTIMIZE_INSTALLER
		  m_startFile = "";
#endif //OPTIMIZE_INSTALLER
        }
    // copy constructor
    WidgetProperties(const WidgetProperties& props)
    {
        m_id = props.id();
        m_title = props.title();
        m_source = props.source();
        m_capabilities = props.capabilities();
        m_installPath = props.installPath();
        m_iconPath = props.iconPath();
        m_plist = props.plist();
        m_size = props.size();
        m_secSessionStr = props.secureSessionString();
        m_secSessionPath = props.secureSessionPath();
        m_type = props.type();
        m_sharedLibrary = props.isSharedLibrary();
        m_sharedLibraryWidget = props.isSharedLibraryWidget();
        m_hidden = props.isHiddenWidget();
        m_resourcePath = props.resourcePath();
        m_titleDir = props.titleDir();
        m_descriptionDir = props.descriptionDir();
        m_authorDir = props.authorDir();
        m_licenseDir = props.licenseDir();
        m_certificateAki = props.certificateAki();
        m_allowBackgroundTimers = props.allowBackgroundTimers();
        m_minimizedSize = props.minimizedSize();
        m_hideIcon = props.hideIcon();
#ifdef OPTIMIZE_INSTALLER
		m_langs = props.languages();
		m_startFile = props.startFile();
#endif //OPTIMIZE_INSTALLER
    }

    // getter
    QString id() const { return m_id; }
    QString title() const { return m_title; }
    QString installPath() const { return m_installPath; }
    QString source() const { return m_source; }
    int capabilities() const { return m_capabilities; }
    QVariant plistValue(const QString& key) const { return m_plist.value(key); } const
    AttributeMap plist() const { return m_plist; } const
    QString iconPath() const { return m_iconPath; }
    bool capability(CapabilityKey capability) const { return m_capabilities & capability; }
    unsigned long size() const { return m_size; }
    QString secureSessionString() const { return m_secSessionStr; }
    QString secureSessionPath() const { return m_secSessionPath; }
    QString type() const { return m_type; }
    bool isSharedLibrary() const { return m_sharedLibrary; }
    bool isSharedLibraryWidget() const { return m_sharedLibraryWidget; }
    bool isHiddenWidget() const { return m_hidden; }
    bool isPreInstallWidget() const { return m_preinstall; }
    QString resourcePath() const { return m_resourcePath; }
    QString titleDir() const { return m_titleDir; }
    QString descriptionDir() const { return m_descriptionDir; }
    QString authorDir() const { return m_authorDir; }
    QString licenseDir() const { return m_licenseDir; }
    QString certificateAki() const { return m_certificateAki; }
    bool allowBackgroundTimers() const { return m_allowBackgroundTimers; }
    QSize minimizedSize() const { return m_minimizedSize; }

    bool hideIcon() const { return m_hideIcon; }
#ifdef OPTIMIZE_INSTALLER
	QSet<QString> languages() const {return m_langs;}
	QString startFile() const {return m_startFile;}
#endif //OPTIMIZE_INSTALLER
    //setter
    void setInfoPList(const AttributeMap& plist) { m_plist = plist; }
    void setCapabilites(int capability) { m_capabilities = capability; }
    void setId(const QString& id) { m_id = id; }
    void setTitle(const QString& title) { m_title = title; }
    void setInstallPath(const QString& installedpath) { m_installPath = installedpath; }
    void setSource(const QString& source) { m_source = source; }
    void setIconPath(const QString& iconPath) { m_iconPath = iconPath; }
    void setSize(unsigned long size) { m_size = size; }
    void setSecureSessionString(const QString& secureSessionString) { 
        m_secSessionStr = secureSessionString; 
    }
    void setSecureSessionPath(const QString& secureSessionPath) { 
        m_secSessionPath = secureSessionPath; 
    }
    void setType(const QString& type) { m_type = type; }
    void setSharedLibrary(bool isSharedLib) { m_sharedLibrary = isSharedLib; }
    void setSharedLibraryWidget(bool isSharedLibWidget) { 
        m_sharedLibraryWidget = isSharedLibWidget; 
    }
    void setHiddenWidget(bool hidden) { m_hidden = hidden; }
    void setPreInstallWidget(bool preinstall) { m_preinstall = preinstall; }
    void setResourcePath(const QString& resourcepath) { m_resourcePath = resourcepath; }
    void setTitleDir(const QString& titleDir) { m_titleDir = titleDir; }
    void setDescriptionDir(const QString& descriptionDir) { m_descriptionDir = descriptionDir; }
    void setAuthorDir(const QString& authorDir) { m_authorDir = authorDir; }
    void setLicenseDir(const QString& licenseDir) { m_licenseDir = licenseDir; }
    void setCertificateAki(const QString& certificateAki) { m_certificateAki = certificateAki; }
    void setAllowBackgroundTimers(bool allowBackgroundTimers) { 
        m_allowBackgroundTimers = allowBackgroundTimers; }
    void setMinimizedSize(QSize minimizedSize) { m_minimizedSize = minimizedSize; }

    void setHideIcon(const bool hideIcon) { m_hideIcon = hideIcon; }
#ifdef OPTIMIZE_INSTALLER
	void setLanguages(const QSet<QString>& lang) {m_langs = lang;}
	void setStartFile(const QString& file){m_startFile = file;}
#endif //OPTIMIZE_INSTALLER
private:
    QString m_id;
    QString m_title;
    QString m_source;
    int m_capabilities;
    QString m_installPath;
    QString m_iconPath;
    AttributeMap m_plist;
    unsigned long m_size;
    QString m_secSessionStr;
    QString m_secSessionPath;
    QString m_type;
    bool m_sharedLibrary;
    bool m_sharedLibraryWidget;
    bool m_hidden;
    bool m_preinstall;
    QString m_resourcePath;
    QString m_titleDir;
    QString m_descriptionDir;
    QString m_authorDir;
    QString m_licenseDir;
    QString m_certificateAki;
    bool m_allowBackgroundTimers;
    QSize m_minimizedSize;
    bool m_hideIcon;
#ifdef OPTIMIZE_INSTALLER
	QSet<QString> m_langs;
	QString m_startFile;
#endif //OPTIMIZE_INSTALLER
    // NOTE anything added here must be added to copy constructor!
};

#endif //_WAC_WIDGET_PROPERTIES_H_
