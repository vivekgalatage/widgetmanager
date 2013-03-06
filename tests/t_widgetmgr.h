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

#ifndef T_WRT_WIDGET_MANAGER_H
#define T_WRT_WIDGET_MANAGER_H

#include <QtTest/QtTest>

class Q_DECL_EXPORT WrtWidgetManagerTest: public QObject
{
Q_OBJECT

public:
    WrtWidgetManagerTest(QWidget *parent = NULL);
    ~WrtWidgetManagerTest();

private Q_SLOTS:

    void initTestCase();
/////////////////////////////////////////////////
//Widget installation and uninstallation tests
/////////////////////////////////////////////////

    void InstallWidgetTest();
    void InvalidWidgetInstall();
    void InvalidConfXmlContents();
    void MultiNameElementInDiffNS();
    void WidgetNameInDiffNs();
    void WidgetInstallSmokeTest();
    void InstallWidgetInvalidPath();
    void WidgetInstallInvalidKeys();
    void WidgetKeysIn2NS();
#ifdef QTWRT_USE_DEBIAN_PACKAGING
    void WidgetDebInfoAuthorTest();
    void WidgetDebInfoAuthorMailTest();
#endif

    ///////////////////////////////////////////////////////////
    //Widget installation and uninstallation Multi-Drive tests
    ///////////////////////////////////////////////////////////
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
    void MultiDriveInstallWidgetTest_LowerCase();
    void MultiDriveInstallWidgetTest_UpperCase();
    void MultiDriveInstallWidgetTest_SignedWgt();
    void MultiDriveInstallWidgetTest_Massstorage();
    void MultiDriveInstallWidgetTest_DriveLetterLowerCase();
    void MultiDriveInstallWidgetTest_DriveLetterUpperCase();
#if !defined(QTWRT_API_LINUX)
    void MultiDriveInstallWidgetTest_InvalideDrive();
#endif
    void MultiDriveInstallWidgetTest_Empty();
    void MultiDriveInstallWidgetTest_WrongPath();
#endif
    
    ///////////////////////////////////////////////////
    //Signature parser tests, for signed widgets
    //////////////////////////////////////////////////
    void SignedSmokeWgtWidgetInstall();
    void SignedCurrouptWidgetInstall();
    void UninstallWidgetTest();

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
    void MultiDriveUninstallWidgetTest();
#endif

    ////////////////////////////////////////////////////
    //WidgetRegistery test cases
    ////////////////////////////////////////////////////
    void SharedLibraryTest();
    void RegisterWidgetTest();
    void isRegisteredTest();
    void presenceTest();
    void getElementTest();

    ////////////////////////////////////////////////////////
    //Widget localization tests
    //Here we try to install widgets Containing localization
    // folder, and run the widgets to see if correct
    //Localization folders are pickedup
    ////////////////////////////////////////////////////////
    void InstallLocaleizedWidget();

    //////////////////////////////////////////////////////
    // Folder based localiation tests
    //Icon localization tests
    // Bug ID : 1927
    ////////////////////////////////////////////////////
    void InstallLocalizedIconEnUSWidget();
    void InstallLocalizedIconFiWidget();
    void InstallLocalizedIconFrWidget();
    void InstallLocalizedIconZhWidget();
    void InstallLocalizedIconEn_GbWidget();
    void InstallLocalizedIconESWidget();
    void InstallLocalizedIconEnWidget();
    
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
    /////////////////////////////////////////////////////
    // Element based localization tests for icon
    // Bug ID: 1957
    ////////////////////////////////////////////////////
    void InstallIconEnWidget();    
    void InstallIconZhWidget();
#endif

    //////////////////////////////////////////////////////
    //Installing widgets having localized content types
    ////////////////////////////////////////////////////
    void InstallLocalizedContentEnWgt();
    void InstallLocalizedContentEn_UsWgt();
    void InstallLocalizedContentEn_GbWgt();
    void InstallLocalizedContentESWgt();
    void InstallLocalizedContentFiWgt();
    void InstallLocalizedContentFrWgt();
    void InstallLocalizedContentZh_hans_cnWgt();
    void InstallLocalizedContentZhWgt();
    void InstallLocalizedContentZh_hansWgt();
    
    /////////////////////////////////////////////////////
    //Installing widgets with localized name elements.
    //Bug ID:1987
    ////////////////////////////////////////////////////
    void InstallLocalizedNameEnWgt();
    void InstallLocalizedNameEsWgt();
    void InstallLocalizedNameEn_GbWgt();
    void InstallLocalizedNameEn_UsWgt();
    void InstallLocalizedNameFiWgt();
    void InstallLocalizedNameFrWgt();
    void InstallLocalizedNameZhWgt();
    
    ///////////////////////////////////////////////////
    //Element grouping tests  as per w3c configxml
    //localization or as per UserAgent Localization
    /////////////////////////////////////////////////
    //void ElementGroupingName();
    //void ElementGroupingIcon();

    /////////////////////////////////////////////////////
    //Widget Properties test
    ///////////////////////////////////////////////////

    void WidgetPropertiesWgt();
    
    ///////////////////////////////////////////////////
    //Config Xml Parsing tests
   //////////////////////////////////////////////////
   
    void ConfigXmlNameElementTest();
    void ConfigXmlDescriptionElemTest();
    void ConfigXmlIconTest();
    void ConfigXmlAuthorElemTest();
    void ConfigXmlAccessElementTest();
    void ConfigXmlFeatureElementTest();
    void ConfigXmlLicenseElementTest();
    void ConfigXmlPreferenceElementTest();
    void ConfigXmlViewModeHiddenTimerTest();    
    void ConfigXmlSharedLibraryIconWidgetTestInvalid();
    void ConfigXmlSharedLibraryIconWidgetTestValid();
       
    // Backup files tests
#if defined(Q_OS_MAEMO6) || defined(Q_OS_MAEMO5)
    void BackupFilesTest();
#endif
    
    void cleanupTestCase();

private:
    bool WaitForSignal(QSignalSpy & spy_signal, int expected);
    //////////////////////////////////
    //Internal utility function to
    //Install widgets
    /////////////////////////////////
    bool twidgetInstall(QString &aPkgPath);
    bool twidgetInstallMultiDrive(QString &aPkgPath, QString );
    bool twidgetUnInstall(QString &id) ;
    bool CheckW3CElem(const QString &elem , QString &expected , QString aAttr = QString(""));
    bool CheckW3CElem(const QString &elem , int expectedCount ,  QString Attr = QString(""));
    bool CheckPlistElem(QString &elem);
    bool tCheckLocalizedPath(const QString &elemKey ,  const QString &widgetId , const QString &IconPath);
    bool tSetLanguage(const QString &lang);
    
    };

#endif
