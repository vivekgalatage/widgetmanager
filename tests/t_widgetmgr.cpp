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

#include <QTest>
#include <QDebug>
#include <QVariant>
#include <QMetaType>

#include "wacWidgetManager.h"
#include "wrtwidgetcontainer.h"

#include "wacWebAppRegistry.h"
#include "t_widgetmgr.h"
#include "wacw3celement.h"
#include "wacAttributeMap.h"
#include "wacw3csettingskeys.h"
#include "proprietarysettingskeys.h"
#include "wacsettings.h"
#include "apitestconstants.h"
#include "testhelpers.h"
#include "webapplocalizer.h"

using namespace WAC;

Q_DECLARE_METATYPE(const W3CElement *)

//  C interface to enable explicit linking at runtime
extern "C" WrtWidgetManagerTest* createWidgetManagerTests() {
    return new WrtWidgetManagerTest();
}

/*****************************
 * Default constructor
 * ***************************/
WrtWidgetManagerTest::WrtWidgetManagerTest(QWidget *parent )
    : QObject(parent)
{
    qRegisterMetaType <const W3CElement *>("const W3CElement *");
}

////////////////////////////////
//Default destructor
///////////////////////////////
 WrtWidgetManagerTest::~WrtWidgetManagerTest()
{
    // do nothing
}

bool WrtWidgetManagerTest::WaitForSignal(QSignalSpy &spy_signal , int expected)
{
    int ccount = spy_signal.count();

    qDebug() << "Count: " << ccount << "Expected: " << expected;

    if (ccount >= expected ) {
        qDebug() << "Count already as expected (" << expected << ") or greater";
        return true;
    }

    for (int iter = 0; (spy_signal.count() != ccount + 1) && iter < 10; ++iter)
        QTest::qWait(250);

    qDebug() << "Count Return: " << spy_signal.count();

    return (ccount < spy_signal.count());
}

/**************************************************************
 * Test function : Funtion to initialize the test widgetmanager
 *                 test suite
 *
 **************************************************************/
void WrtWidgetManagerTest::initTestCase()
{
    __countTestsHelper__(this);
    ///////////////////////////
    // Cleanup the localization
    //////////////////////////
    WrtSettings::createWrtSettings()->setValue("UserAgentLanguage" , QString(""));
}

/******************************************************************
 * Test function: Cleanup function
 *
 ****************************************************************/
void WrtWidgetManagerTest::cleanupTestCase()
{
    /////////////////////////////
    // Cleanup the userAgent
    ///////////////////////////
    WrtSettings::createWrtSettings()->setValue("UserAgentLanguage" , QString(""));
}

/********************************************************************
 * Description: Test function to install a valid test widget
 *
 * Test case 1 : install a simple test widget.
 * Test case 2 : istall the same widget once again
 *               (Second time the widget should get installed)
 * *****************************************************************/
void WrtWidgetManagerTest::InstallWidgetTest()
{
    QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");
    WidgetManager mgr(NULL);

    ////////////////////////////////////
    //Test case 1
    ////////////////////////////////////
qDebug()<<"*************"<<QFile(PkgPath).exists();
    QCOMPARE(twidgetInstall(PkgPath), true);

    /////////////////////////////////////
    //Test case 2
    ////////////////////////////////////
    QCOMPARE(twidgetInstall(PkgPath), true);
}

/********************************************************************************
 * Description: Test function to install a valid test widget on a drive
 *              passed in lower case
 *
 * Test case 1 : install a simple test widget on a drive passed in lower case.
 * Test case 2 : istall  the same widget once again on same drive
 *               (Second time the widget should get installed)
 * Test case 3 : install  the same widget once again on different drive
 *               (This time also the widget should get installed)
 * **************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_LowerCase()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");

 ////////////////////////////////////
 //Test case 1
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "c:\\"));

 /////////////////////////////////////
 //Test case 2
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "c:\\"));

 /////////////////////////////////////
 //Test case 3
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "e:\\"));
}
#endif

/********************************************************************************
 * Description: Test function to install a valid test widget on a drive
 *              passed in upper case
 *
 * Test case  : install a simple test widget on a drive passed in upper case.
 * **************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_UpperCase()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");

 ////////////////////////////////////
 //Test case 1
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "C:\\"));

}
#endif

/********************************************************************
 * Description : Install a signed widget on multiple drive
 *
 * Test case 1 : Install a signed wgt widget on a drive
 * Test case 2 : Install a signed wgt widget on same drive again
 * Test case 3 : Install the same signed wgt widget again on a different drive
 * *****************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_SignedWgt()
{

  QString pkgpath =  APITEST_WIDGETBASE + QString("widgets/SmokeTestSigned.wgt");

 ////////////////////////////////////
 //Test case 1
 ////////////////////////////////////

  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "C:\\"));

 ////////////////////////////////////
 //Test case 2
 ////////////////////////////////////

  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "C:\\"));
 ////////////////////////////////////
 //Test case 3
 ////////////////////////////////////

  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "E:\\"));
}
#endif

/********************************************************************
 * Description : Install a test widget on mass storage
 *
 * Test case 1 : Install a wgt widget on mass storage
 * Test case 2 : Install the same wgt widget on mass storage again
 * *****************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_Massstorage()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");

 ////////////////////////////////////
 //Test case 1
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "E:\\"));

 ////////////////////////////////////
 //Test case 2
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "E:\\"));
}
#endif

 
/********************************************************************************
 * Description: Test function to install a valid test widget on a drive
 *              (passed only the drive letter in lower case)
 *
 * **************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) 
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_DriveLetterLowerCase()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");
 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "c"));
}
#endif

/********************************************************************************
 * Description: Test function to install a valid test widget on a drive
 *              (passed only the drive letter in upper case)
 *
 * **************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_DriveLetterUpperCase()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");
 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "C"));
}
#endif

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) && !defined(QTWRT_API_LINUX)
/********************************************************************************
 * Description: Test function to install a valid test widget on a invalid drive
 * Test Case 1: Number passed
 * Test Case 2: Invalid drive letter passed
 * Test Case 3: Special char passed
 * **************************************************************************/
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_InvalideDrive()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");

 ////////////////////////////////////
 //Test case 1
 ////////////////////////////////////

 QCOMPARE(false , twidgetInstallMultiDrive(PkgPath, "0"));

 ////////////////////////////////////
 //Test case 2
 ////////////////////////////////////

 QCOMPARE(false , twidgetInstallMultiDrive(PkgPath, "P"));

 ////////////////////////////////////
 //Test case 3
 ////////////////////////////////////

 QCOMPARE(false , twidgetInstallMultiDrive(PkgPath, "@"));

}
#endif

/********************************************************************************
 * Description: Test function to install a valid test widget on a "Empty/Null" drive
 * Test Case 1: "" is passed as drive (Here the widget will be installed on default 'C' drive)
 * Test Case 2: Uninitialised string is passed
 * **************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_Empty()
{

 QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");

 ////////////////////////////////////
 //Test case 1
 ////////////////////////////////////

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, ""));

 ////////////////////////////////////
 //Test case 2
 ////////////////////////////////////

 QString drive;
// QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, drive));

}
#endif

/********************************************************************************
 * Description: Test function to install a valid test widget on a wrong path
 * **************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
 void WrtWidgetManagerTest::MultiDriveInstallWidgetTest_WrongPath()
{

  QString PkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");

 QCOMPARE(true , twidgetInstallMultiDrive(PkgPath, "C:\\ABC"));

}
#endif


/***********************************************************************
 * Description: Test function to install an valid wgz widget with wgt
 *               extension.
 *
 * Test case 1 : Install a invalid wgz widget with wgt extension
 * Test case 2 : Install a invalid wgt widget
 * Test case 3 : Try to install a widget by giving an invalid pkg path
 * Test case 4 : Try installing wgt widget without config.xml contents
 * *********************************************************************/
void WrtWidgetManagerTest::InvalidWidgetInstall()
{
    QString path = APITEST_WIDGETBASE + QString("widgets/invalid.wgt");
    //////////////////////////////////////////
    //Test case 1
    /////////////////////////////////////////
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
    QCOMPARE(twidgetInstall(path), false);
#endif

    /////////////////////////////////////////
    //Test case 2
    ////////////////////////////////////////
    path = APITEST_WIDGETBASE + QString("widgets/corrupt.wgt");
    QCOMPARE(twidgetInstall(path), false);

    /////////////////////////////////////////
    //Test case 3
    ////////////////////////////////////////
    path = APITEST_WIDGETBASE + QString("asdf.wgt");
    QCOMPARE(twidgetInstall(path), false);

    ///////////////////////////////////////////
    //Test case 4
    //////////////////////////////////////////
    path = APITEST_WIDGETBASE + QString("widgets/noconfxml.wgt");
    QCOMPARE(twidgetInstall(path), false);
}

/*****************************************************
 * Description: Invalid widget installation path
 *
 * Test case: try to install a widget with invalid path
 * **************************************************/
void WrtWidgetManagerTest::InstallWidgetInvalidPath()
{
    ///////////////
    //Test case
    ///////////////
    QString path;
    QCOMPARE(twidgetInstall(path), false);
}


/************************************************************************
 * Description : Test function to install an wgt widgets with invalid
 *               config.xml contents.
 *
 * Test case 1 : Install a widget with invalid namespace
 *
 *
 ************************************************************************/
void WrtWidgetManagerTest::InvalidConfXmlContents()
{
    ///////////////////////////
    //Test case 1
    ///////////////////////////
    QString path = APITEST_WIDGETBASE + QString("widgets/invalidns.wgt");
    QCOMPARE(twidgetInstall(path), false);
}

/*******************************************************************
 * Description : Test function to install a test widget having
 *               Two name elements in different namespace,
 *               mismatched tags.
 *
 *
 * Test case 1 : Install a widget name as part of different
 *               namespace and with mismatched tags.  Install
 *               must fail due to XML parse error--incorrect usage
 *               of xmlns attribute, malformed xml
 ****************************************************************/
void WrtWidgetManagerTest::MultiNameElementInDiffNS()
{
    /////////////////////////////
    //Test case  1
    ////////////////////////////
    QString path = APITEST_WIDGETBASE + QString("widgets/mutiNamesInDiffNS.wgt");
    QCOMPARE(twidgetInstall(path), false);
}

/************************************************************************
 * Description: Test function to install a test widget, with name element
 *              in different name space
 *
 *  Test case :  Install a widget with name element in different namespace
 *
 * **********************************************************************/
void WrtWidgetManagerTest::WidgetNameInDiffNs()
{
    /////////////////////////////
    //Test case  1
    ////////////////////////////
    QString path = APITEST_WIDGETBASE + QString("widgets/nameinDiffNs.wgt");
    QCOMPARE(twidgetInstall(path), false);
}

/***********************************************************************
 * Description: Test function which installs a widget containing all the
 *              configxml elements.
 *
 * Test case : Install a widget which contains all configxml contents
 * *******************************************************************/
void WrtWidgetManagerTest::WidgetInstallSmokeTest()
{
    //////////////////////////
    //Test case
    ////////////////////////
    QString path = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QCOMPARE(twidgetInstall(path), true);
}

/***************************************************************************
 * Description: Test function which installs a localeized widget
 *
 * Test case 1 : Install widget which is folder localized
 * Test case 2 : Install widgets with locales mixed.
 * Test case 3:  Install Widgets with locales folder empty
 * ************************************************************************/
void WrtWidgetManagerTest::InstallLocaleizedWidget()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/localesInRoot.wgt");
    ///////////////////////////////////
    //Test case 1
    //////////////////////////////////
    QCOMPARE(twidgetInstall(pkgpath), true);

    /////////////////////////////////
    //Test case 2
    ////////////////////////////////
    pkgpath = APITEST_WIDGETBASE + QString("widgets/localesMixedCase.wgt");
    QCOMPARE(twidgetInstall(pkgpath), true);

    //////////////////////////////
    //Test case 3
    /////////////////////////////
    pkgpath = APITEST_WIDGETBASE + QString("widgets/localesEmptyFolder.wgt");
    QCOMPARE(twidgetInstall(pkgpath), true);
}

/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale en-us
 *
 **************************************************************************/
void WrtWidgetManagerTest::InstallLocalizedIconEnUSWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("locales/en-us/flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en-us"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath));
}


/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale fi
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedIconFiWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("locales/fi/flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("fi"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath));
}

/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale fr
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedIconFrWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("locales/fr/flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("fr"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath ));
}

/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale en-gb
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedIconEn_GbWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("locales/en-gb/flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en-gb"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath ));
}

/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale es
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedIconESWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("locales/es/flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("es"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath ));
}


/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale zh
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedIconZhWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("locales/zh/flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("zh"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath ));
}


/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale En
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedIconEnWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/local.wgt");
   QString iconPath = QDir::toNativeSeparators("flag.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("28479858ea4365fad1b9c64b419ad52f84e44feb") , iconPath ));
}

#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5) //Icon is not localizable via xml:lang
/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Element  Based Localization 

                  According to the latest spec choosing the icon follows the following priority:
               1. Check for the localised icon in Locales folder.
               2. If the localized icon in locales folder is not found then we use the first custon icon that is parsed.
               3. If no localized icon or custom icon exists, then we use the default icon which shoulkd be present in the root dir of the widget package and  have the name "icon.png"
 *
 *  Test case  : Install Widget containing Element  Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale en
 *
 **************************************************************************/




 void WrtWidgetManagerTest::InstallIconEnWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/icon.wgt");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   // As en flag is not available,
   // test should pick up default icon.png
   //////////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("d90a45a8d5b23bc4c82a3597093e6df93a5bd676") , QString("images/france.png") ));
}




/**************************************************************************
 * Description : Installes a widget with ICON Elements localized
 *               Element  Based Localization
 *
 *   According to the latest spec choosing the icon follows the following priority:
 *   1. Check for the localised icon in Locales folder.
 *   2. If the localized icon in locales folder is not found then we
 *      use the first custom icon that is parsed.
 *   3. If no localized icon or custom icon exists, then we use the
 *      default icon which should be present in the root dir of the widget
 *      package and  have the name "icon.png"
 *
 *  Test case  : Install Widget containing Element  Based ICON Localized
 *               element.
 *                Elemented tested WIDGET_ICON  for Locale zh
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallIconZhWidget()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/icon.wgt");
   QString iconPath = QDir::toNativeSeparators("images/france.png");

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("zh"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   // Check the icon path
   // As en flag is not available,
   // test should pick up default icon.png
   //////////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_ICON , QString("d90a45a8d5b23bc4c82a3597093e6df93a5bd676") , iconPath ));
}




#endif
/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale en
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentEnWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/en/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}


/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale en-gb
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentEn_GbWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/en-gb/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en-gb"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path));
}


/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale en-us
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentEn_UsWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/en-us/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en-us"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}


/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale es
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentESWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/es/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("es"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}


/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale fi(finish)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentFiWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/fi/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("fi"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}


/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale fr(france)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentFrWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/fr/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("fr"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}



/**************************************************************************
 * Description : Installes a widget with Content Elements localized
 *               Folder Based Localization
 *
 *  Test case  : Install Widget containing Folder Based CONTENT Localized
 *               element.
 *                Elemented tested WIDGET_CONTENT  for Locale zh-hans-ch(chinese simplified)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentZh_hans_cnWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/zh-hans-cn/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // First set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("zh-hans-cn"));

   ////////////////////////////
   // Install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //Test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}


/**************************************************************************
 * description : installes a widget with content elements localized
 *               folder based localization
 *
 *  test case  : install widget containing folder based content localized
 *               element.
 *                elemented tested widget_content  for locale zh(chinese)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentZhWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/zh/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("zh"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}

/**************************************************************************
 * description : installes a widget with content elements localized
 *               folder based localization
 *
 *  test case  : install widget containing element based  localized name
 *               element.
 *                elemented tested widget_name for locale  zh-hans(Chinese)
 *
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedContentZh_hansWgt()
{
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent.wgt");
   QString path = QDir::toNativeSeparators("locales/zh-hans/index.html");
#else
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/loccontent2.wgt");
   QString path = QDir::toNativeSeparators("start.html");
#endif

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("zh-hans"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_CONTENT , QString("d58b3e5bd84fcc4b6379937c2b8f6c5a05ec9a3e") , path ));
}

/**************************************************************************
 * description : installes a widget with  elements localized
 *               Element based localization
 *
 *  test case  : install widget containing folder based content localized
 *               element.
 *                elemented tested widget_name  for locale en(english)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameEnWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("EnglishName") ));
}

/**************************************************************************
 * description : installes a widget with  elements localized
 *               Element based localization
 *
 *  test case  : install widget containing element based  localized
 *                name  element.
 *                elemented tested widget_name  for locale en-us(english-us)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameEn_UsWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en-us"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("USName") ));
}

/**************************************************************************
 * description : installes a widget with  elements localized
 *               Element based localization
 *
 *  test case  : install widget containing folder based content localized
 *               element.
 *                elemented tested widget_content  for locale en-gb(english-gb)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameEn_GbWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");
   pkgpath = QDir::toNativeSeparators(pkgpath);

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("en-gb"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("EnglishName") ));
}

/**************************************************************************
 * description : installes a widget with  elements localized
 *               name
 *
 *  test case  : install widget containing element based  localized
 *                name  element.
 *                elemented tested widget_name  for locale es(spainish)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameEsWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");
   pkgpath = QDir::toNativeSeparators(pkgpath);

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("es"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("SpanishName") ));
}



/**************************************************************************
 * description : test localized name elements in widget
 *
 *  test case  : install widget containing localized name elements, including
 *               duplicate name element with language tag for "Finnish".
 *               The language tag must be handled in case insensitive mode.
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameFiWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");
   pkgpath = QDir::toNativeSeparators(pkgpath);

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("fi"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   //  The config.xml in lname.wgt contains two name elements with Finnish lang tag as follows:
   //  <name xml:lang="Fi"> and  <name xml:lang="fi">, in this order.
   //  According to IETF "Best Current Practices" sec 2.1.1, the lang tag should be handled in
   //  case-insensitive mode:
   //
   //  http://www.rfc-editor.org/rfc/bcp/bcp47.txt
   //
   //  Thus, since <name xml:lang="Fi"> occurs first in the config.xml, we match this and return
   //  the text from this element.
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("Fi widget") ));
}

/**************************************************************************
 * description : installes a widget with  elements localized
 *               name
 *
 *  test case  : install widget containing element based  localized
 *                name  element.
 *                elemented tested widget_name  for locale Fr(france)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameFrWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");
   pkgpath = QDir::toNativeSeparators(pkgpath);

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("fr"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("FranceName") ));
}


/**************************************************************************
 * description : installes a widget with  elements localized
 *               name
 *
 *  test case  : install widget containing element based  localized
 *                name  element.
 *                elemented tested widget_name  for locale Zh(chinese)
 *
 **************************************************************************/

 void WrtWidgetManagerTest::InstallLocalizedNameZhWgt()
{
   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/lname.wgt");
   pkgpath = QDir::toNativeSeparators(pkgpath);

   ////////////////////////////
   // first set the user agent
   ///////////////////////////
   QCOMPARE(true , tSetLanguage("zh"));

   ////////////////////////////
   // install localized widget
   ///////////////////////////
   QCOMPARE(true , twidgetInstall(pkgpath));

   ////////////////////////////
   //test case
   ///////////////////////////
   QCOMPARE (true , tCheckLocalizedPath(W3CSettingsKey::WIDGET_NAME , QString("78ec3052a373ea15b9fcc1a61fb0b71cc4ecc35b") , QString("ChineseName") ));
}

/************************************************************************
 * Description : Function to test widget with keys in different namespace
 *
 * Test case 1 : install an widget with (name, and content in diff
 *               namespace).
 *
 *************************************************************************/
void WrtWidgetManagerTest::WidgetInstallInvalidKeys()
{
    QString path = APITEST_WIDGETBASE + QString("widgets/keysindiffns.wgt");
    QCOMPARE(twidgetInstall(path), false);
}

/************************************************************************
 * Description : Function to insall widget which has keys in 2 different
 *               Name space.
 *               name:Standandard (in namespace http://www.w3.org/ns/widgets)
 *               name: Services (in namespace services)
 *
 *Test case 1 : Install widget with keys in diff ns, and check the
 *               installed widget's name(Shall be Standandard)
 ***********************************************************************/
void  WrtWidgetManagerTest::WidgetKeysIn2NS()
{
    QString path = APITEST_WIDGETBASE + QString("widgets/WgtKeysInStdNCustomNS.wgt");
    QCOMPARE(twidgetInstall(path), true);
    WebAppInfo widgetInfo;
    
    QCOMPARE(WebAppRegistry::instance()->isRegistered(QString("06837c4735eea1d2bb545b12841752f6398f3be0") , widgetInfo), true);
    //QCOMPARE(WebAppRegistry::instance()->isRegistered(QString("http://www.keysin2diffns.com") , widgetInfo), true);

    ////////////////////////
    //Test case
    ////////////////////////
    QCOMPARE(widgetInfo.appTitle(), QString("Services"));
}

/********************************************************************
 * Description : Install a signed widget
 * Test case   : Install a signed wgt widget
 * *****************************************************************/
void WrtWidgetManagerTest::SignedSmokeWgtWidgetInstall()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/SmokeTestSigned.wgt");
    QCOMPARE(twidgetInstall(pkgpath), true);
}

/********************************************************************
 * Description : Install a  signed widget (With curropt signature)
 * Test case   : Install a signed wgt widget (With curropt signature)
 * *****************************************************************/
void WrtWidgetManagerTest::SignedCurrouptWidgetInstall()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/SmokeSignedCurropt.wgt");
    //QCOMPARE(true , twidgetInstall(pkgpath)); //CRASH to be fixed.
}




/********************************************************************
 * Description: Test function to install a valid shared library widget
 *
 * Test case 1 : install a shared library widget
 * *****************************************************************/
void WrtWidgetManagerTest::SharedLibraryTest()
{
    QString PkgPath = APITEST_WIDGETBASE + QString("widgets/S_WShLib1.wgt");

    ////////////////////////////////////
    //Test case 1
    ////////////////////////////////////
    QCOMPARE(twidgetInstall(PkgPath), true);
}


/*************************************************************************
 * Description : Function to test widget uninstallation api
 *
 * Test case 1 : uninstall an installed widget
 * Test case 2 : uninstall an invalid widget
 * Test case 3 : uninstall an  uninstalled widget
 * **********************************************************************/
void WrtWidgetManagerTest::UninstallWidgetTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");
    QCOMPARE(twidgetInstall(pkgpath), true);

    ////////////////////////////
    //Test case 1
    ///////////////////////////
    QCOMPARE(twidgetUnInstall(id), true);

    //////////////////////////
    //Test case 3
    /////////////////////////
    QCOMPARE(twidgetUnInstall(id), false);

    ///////////////////////
    //Test case 2
    //////////////////////
    QString invalidId;
    QCOMPARE(twidgetUnInstall(invalidId), false);

}

/*********************************************************************************
 * Description : Function to test widget uninstallation api for multidrive feature
 *
 * Test case 1 : uninstall an installed widget from C drive
 * Test case 2 : uninstall an invalid widget
 * Test case 3 : uninstall an  uninstalled widget
 * Test Case 4 : uninstall an widget from mass storage
 * Test Case 5 : Install again the same uninstalled widget
 * ******************************************************************************/
#if !defined(Q_OS_MAEMO6) && !defined(Q_OS_MAEMO5)
void WrtWidgetManagerTest::MultiDriveUninstallWidgetTest()
{

  QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
  QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");

  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "C"));

  ////////////////////////////
  //Test case 1
  ///////////////////////////
  QCOMPARE(true  , twidgetUnInstall(id));

  //////////////////////////
  //Test case 3
  /////////////////////////
  QCOMPARE(false , twidgetUnInstall(id));

  ///////////////////////
  //Test case 2
  //////////////////////
  QString invalidId ;
  QCOMPARE(false , twidgetUnInstall(invalidId));

  ///////////////////////
  //Test case 4
  //////////////////////

  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "E"));
  QCOMPARE(true  , twidgetUnInstall(id));
  QCOMPARE(false , twidgetUnInstall(id));

  ///////////////////////
  //Test case 5
  //////////////////////

  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "E"));
  QCOMPARE(true  , twidgetUnInstall(id));
  QCOMPARE(true , twidgetInstallMultiDrive(pkgpath, "C"));
  QCOMPARE(true  , twidgetUnInstall(id));
}
#endif



 /*************************************************************************
 * Description : Helper function for WidgetPropertiesWgt, WidgetPropertiesWgz
 *               tests below...
 *
 * **********************************************************************/
static bool wPropsTestHelper(WidgetProperties *WgtProperties, QString &id,
        QString &titleString, bool skipSize)
    {
    bool status = true;

    ///////////////////
    //Test case 1
    ///////////////////
    if (WgtProperties == NULL)
        {
        qDebug() << "WidgetProperties returned null";
        return false;
        }

    qDebug() << "WidgetProperties" << WgtProperties;

    ////////////////////
    //Test case 2
    ///////////////////
    if (WgtProperties->id() != id)
        {
        qDebug() << "Compared id and actual id differ " << id
                << WgtProperties->id();
        status = false;
        }
    ///////////////////
    //Test case 3
    /////////////////
    if (WgtProperties->title() != titleString)
        {
        qDebug() << "Compared title" << titleString << "actual"
                << WgtProperties->title();
        status = false;
        }
    ///////////////
    //Test case 4
    /////////////
    if (WgtProperties->source().isEmpty())
        {
        qDebug() << "Source returned null";
        status = false;
        }
    ///////////////
    //Test case 5
    //////////////
    if (WgtProperties->installPath().isEmpty())
        {
        qDebug() << "Install path returned null";
        status = false;
        }
    //////////////
    //Test case 6
    //////////////
    if (WgtProperties->iconPath().isEmpty())
        {
        qDebug() << "Icon path returned null";
        status = false;
        }
    ///////////////
    //Test case 7
    //////////////
    if ((!skipSize) && (WgtProperties->size() == 0))
        {
        qDebug() << "Size returned zero";
        status = false;
        }
    return status;
    }

/***********************************************************************
 * Description : Test function for WidgetProperties
 *
 * Run twice for widgetMgr.getProperties(pkgpath) and
 * widgetMgr.getProperties()
 *
 * Test case 1 : Wgt Widget Properties
 * Test case 2 : Test for widget id
 * Test case 3 : Widget  Title
 * Test case 4 : Widget  Src
 * Test case 5 : Widget  installpath
 * Test case 6:  Widget  icon path
 * Test case 7:  Widget  size
 * *********************************************************************/
 void WrtWidgetManagerTest::WidgetPropertiesWgt()
{
   bool status = true ;
   QString widgetId;
   QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");
   QString title("This element would be used.");

   WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
   WebAppInfo widgetInfo;
   WidgetManager widgetMgr(NULL);
   WidgetProperties *WgtProperties1 = NULL;
   WidgetProperties *WgtProperties2 = NULL;


   QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
   QString pkgpathNative = QDir::toNativeSeparators(pkgpath);

   //////////////////////////////////////////////////////////////////
   /// Test for widgetMgr.getProperties()
   //////////////////////////////////////////////////////////////////
   if (widgetReg->isRegistered(id , widgetInfo))   {
       QCOMPARE(true  , twidgetUnInstall(id));
   }
   QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));
   if (widgetReg->isRegistered(id , widgetInfo)) { // need widgetInfo filled after install..
     WgtProperties1 = widgetMgr.getProperties();
   }
   //Do Test cases 1 - 7
   //status = ::widgetPropertiesTestHelper(WgtProperties1, id, title, false);
   status = wPropsTestHelper(WgtProperties1, id, title, false);

   QVERIFY(status);

   //////////////////////////////////////////////////////////////////
   /// Test for widgetMgr.getProperties(installPath)
   //////////////////////////////////////////////////////////////////
   WgtProperties2 = widgetMgr.getProperties(widgetInfo.m_appPath);
   //Do Test cases 1 - 7
   status = wPropsTestHelper(WgtProperties2, id, title, true); // skip size check

   QVERIFY(status);
}


/***********************************************************************
 * Description : Test function to ConfigXmlName Element
 * Test Case   : Get Name Element from Attribute Map
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlNameElementTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString expected("This element would be used.");
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    AttributeMap attrMap = wgtProperties->plist();
    QString elem("default/widget/name");
    QVariant varelm = attrMap.value(elem);
    QCOMPARE(varelm.toString(), expected);

}

/***********************************************************************
 * Description : Test function to ConfigXmlDescription Element
 * Test Case   : Get Description Element from Attribute Map
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlDescriptionElemTest()
{
    QString expected("HelloWorld Example Widget");
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    AttributeMap attrMap = wgtProperties->plist();
    QString elem("default/widget/description");
    QVariant varelm = attrMap.value(elem);
    QCOMPARE(varelm.toString(), expected);
}

/***********************************************************************
 * Description : Test function to ConfigXml Icon Element
 * Test Case   : Verify icon count returned
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlIconTest()
{
    QString src("src");
    QString elem("widget/icon");
    int count=0;
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    AttributeMap attrMap = wgtProperties->plist();
    AttributeMap::Iterator iter= attrMap.begin();
    for(;iter!=attrMap.end();++iter){
        if(iter.key().startsWith(elem)&&(iter.key().contains(src))){
            ++count;
          }
      }
    QCOMPARE(count, 3);

}

/***********************************************************************
 * Description : Test function to ConfigXml Author Element
 * Test Case 1 : Test for author Name
 * Test Case 2 : Test for author Email
 * Test case 3 : Test for author href
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlAuthorElemTest()
{
    QString elem(W3CSettingsKey::WIDGET_AUTHOR);
    QVariant value;
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    ///////////////////////
    //Test case 1
    //////////////////////

    QString expected="Bhimsen G Kulkarni";
    AttributeMap attrMap = wgtProperties->plist();
    AttributeMap::Iterator iter= attrMap.begin();
    QVariant varelm = attrMap.value(elem);
    QCOMPARE(varelm.toString(), expected);

    ///////////////////////
    //Test case 2
    /////////////////////

    QString email("xyz@xyz.com");
    QString aAttr("email");
    for(; iter!=attrMap.end(); ++iter){
      if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            value=(*iter);
          }
      }

    QCOMPARE(value.toString(), email);
 
    ///////////////////////
    //Test case 3
    /////////////////////

    iter= attrMap.begin();
    QString href("asdf");
    aAttr="href";
    for(; iter!=attrMap.end(); ++iter) {
        if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            value=(*iter);
          }
      }
    QCOMPARE(value.toString(),href );
}


/***********************************************************************
 * Description : Test function to ConfigXml Access Element
 * Test Case   : Verify Access  count returned
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlAccessElementTest()
{
  ///////////////////////
  //Test case
  //////////////////////
  QVERIFY(CheckW3CElem(W3CSettingsKey::WIDGET_ACCESS , 3));
}

/***********************************************************************
 * Description : Test function to ConfigXml Feature Element
 * Test Case   : Verify Feature  count returned
 *********************************************************************/

void WrtWidgetManagerTest::ConfigXmlFeatureElementTest()
{

    QString aAttr("name");
    QString elem("widget/feature");
    int count=0;
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    AttributeMap attrMap = wgtProperties->plist();
    AttributeMap::Iterator iter= attrMap.begin();
    for(; iter!=attrMap.end(); ++iter)
      {
        if(iter.key().startsWith(elem) && (iter.key().contains(aAttr)))
          {
            ++count;
          }
      }
    QCOMPARE(count, 3);
}

/***********************************************************************
 * Description : Test function to ConfigXml License Element
 * Test Case 1 : Test for License Name
 * Test case 2 : Test for License href
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlLicenseElementTest()
{

    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    QVariant value;
    QString elem("en/widget/license");
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    ///////////////////////
    //Test case 1
    //////////////////////

    QString expected("Creative Commons Attribution License");
    AttributeMap attrMap = wgtProperties->plist();
    QVariant varelm = attrMap.value(elem);
    QCOMPARE(varelm.toString(), expected);
 
    ///////////////////////
    //Test case 2
    //////////////////////

    AttributeMap::Iterator iter= attrMap.begin();
    expected="http://creativecommons.org/licenses/by/3.0";
    QString aAttr("href");
    for(; iter!=attrMap.end(); ++iter){
        if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            value=(*iter);
          }
      }
    QCOMPARE(value.toString(), expected);

}

/***********************************************************************
 * Description : Test function to ConfigXml View Mode, Timer and Hidden elements
 * Test Case   : Verify widget installs, preferences as expected
 * ********************************************************************/

void WrtWidgetManagerTest::ConfigXmlPreferenceElementTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    QString elem("widget/preference");
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

     ////////////////////////////////////
    //Test case 1
    ////////////////////////////////////

    QString aAttr("name");
    int count=0;
    AttributeMap attrMap = wgtProperties->plist();
    AttributeMap::Iterator iter= attrMap.begin();
    for(; iter!=attrMap.end(); ++iter){
        if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            ++count;
          }
      }
    QCOMPARE(count, 3);
}


void WrtWidgetManagerTest::ConfigXmlViewModeHiddenTimerTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/configxmlparser.wgt");
    QString pkgpathNative = QDir::toNativeSeparators(pkgpath);
    QString widgetId;
    QVariant value;
    QString elem("widget/NOKIA");
    WebAppInfo widgetInfo;
    WidgetManager widgetMgr(NULL);
    WidgetInstallError errCode = WidgetInstallSuccess;

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    QCOMPARE(true,(WidgetInstallSuccess == widgetMgr.install(pkgpathNative, widgetId, true, false)));

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *wgtProperties = widgetMgr.getProperties(widgetInfo.m_appPath);
    QVERIFY(wgtProperties);

    ////////////////////////////////////
    //Test case 1
    ////////////////////////////////////

    QString TimerExpected("neversuspend");
    QString expected("Creative Commons Attribution License");
    AttributeMap attrMap = wgtProperties->plist();
    AttributeMap::Iterator iter= attrMap.begin();
    QString aAttr("timer");
    for(; iter!=attrMap.end(); ++iter){
      if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            value=(*iter);
          }
      }
    QCOMPARE(value.toString(),TimerExpected);

    ////////////////////////////////////
    //Test case 2
    ////////////////////////////////////

    iter= attrMap.begin();
    QString MinimizedExpected("false");
    aAttr="minimized";
    for(; iter!=attrMap.end(); ++iter){
        if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            value=(*iter);
          }
      }
    QCOMPARE(value.toString(), MinimizedExpected);

    ////////////////////////////////////
    //Test case 3
    ////////////////////////////////////

    iter= attrMap.begin();
    QString HiddenExpected("false");
    aAttr="hidden";
    for(; iter!=attrMap.end(); ++iter){
      if(iter.key().startsWith(elem) && (iter.key().contains(aAttr))){
            value=(*iter);
          }
      }
    QCOMPARE(value.toString(), HiddenExpected);
}

void WrtWidgetManagerTest::ConfigXmlSharedLibraryIconWidgetTestInvalid()
{
    QString WidgetExpected("True");
    QString FolderExpected("SWShLib1");

    WidgetManager widgetInstaller(NULL);
    QString pkgpath = QDir::toNativeSeparators(APITEST_WIDGETBASE + QString("widgets/SharedLib_icon_true_widget_true1.wgt"));
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetInstallError errCode = WidgetInstallSuccess;

    ////////////////////////////////////
    //Test case 1
    ////////////////////////////////////

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    errCode = widgetInstaller.install(pkgpath, widgetId, true, false);

    QVERIFY(WidgetInstallSuccess == errCode);

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *WgtProperties = widgetInstaller.getProperties(widgetInfo.m_appPath);
    QVERIFY(WgtProperties);

    AttributeMap attrMap = WgtProperties->plist();

    QVariant varelm = attrMap.value(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_WIDGET);
    QCOMPARE(varelm.toString(), WidgetExpected);

    varelm = attrMap.value(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_FOLDER);
    QCOMPARE(varelm.toString(), FolderExpected);

    //cleanup
    widgetInstaller.uninstall(widgetId, true);
}

void WrtWidgetManagerTest::ConfigXmlSharedLibraryIconWidgetTestValid()
{
    QString WidgetExpected("True");
    QString FolderExpected("SWShLib2");

    WidgetManager widgetInstaller(NULL);
    QString pkgpath = QDir::toNativeSeparators(APITEST_WIDGETBASE + QString("widgets/SharedLib_icon_true_widget_true2.wgt"));
    QString widgetId;
    WebAppInfo widgetInfo;
    WidgetInstallError errCode = WidgetInstallSuccess;

    ////////////////////////////////////
    //Test case 1
    ////////////////////////////////////

    QVERIFY(QFile::exists(pkgpath));
    // Successful install returns zero, positive value otherwise
    errCode = widgetInstaller.install(pkgpath, widgetId, true, false);

    QVERIFY(WidgetInstallSuccess == errCode);

    WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
    QVERIFY(widgetReg->isRegistered(widgetId, widgetInfo));

    WidgetProperties *WgtProperties = widgetInstaller.getProperties(widgetInfo.m_appPath);
    QVERIFY(WgtProperties);

    AttributeMap attrMap = WgtProperties->plist();

    QVariant varelm = attrMap.value(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_WIDGET);
    QCOMPARE(varelm.toString(), WidgetExpected);

    varelm = attrMap.value(ProprietarySettingsKey::WIDGET_SHARED_LIBRARY_FOLDER);
    QCOMPARE(varelm.toString(), FolderExpected);


    //cleanup
    widgetInstaller.uninstall(widgetId, true);
}

#if defined(Q_OS_MAEMO6) || defined(Q_OS_MAEMO5)
void WrtWidgetManagerTest::BackupFilesTest()
{
    QString pkgPath = APITEST_WIDGETBASE + QString("widgets/simple.wgt");
    WidgetManager mgr(NULL);

    QVERIFY(twidgetInstall(pkgPath));

    QString id("35d020bf737c47b6a37b57c370431e27b2647714");

    // Check installation of backup and restore scripts
    QString backupScriptDir = QString("/usr/share/wrt/backup/%1").arg(id);

    QFileInfo backupScriptFile(backupScriptDir + QString("/%1-backup").arg(id));
    QVERIFY(backupScriptFile.isExecutable());

    QFileInfo restoreScriptFile(backupScriptDir + QString("/%1-restore").arg(id));
    QVERIFY(restoreScriptFile.isExecutable());

    // Check installation of widget config file
    QString backupConfigDir("/usr/share/backup-framework/applications/");
    QFile backupConfigFile(backupConfigDir + QString("/widget_%1.conf").arg(id));
    QVERIFY(backupConfigFile.exists());
}
#endif

/*************************************************************************
 * Description: Test function  for isRegisteredApi
 * Test case 1 : isRegistered() on an registered widget
 * Test case 2 : isRegistered() on an unregistered widget
 * Test case 3 : isRegistered() with invalid params
 * ***********************************************************************/
void WrtWidgetManagerTest::isRegisteredTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");
    WebAppInfo widgetInfo;
    WebAppRegistry *widgetReg = WebAppRegistry::instance();
    if (!widgetReg->isRegistered(id , widgetInfo))
      QCOMPARE(twidgetInstall(pkgpath), true);

    ////////////////////
    //Test case 1
    //////////////////
    QCOMPARE(widgetReg->isRegistered(id, widgetInfo), true);
    
    /////////////////////
    //Test case 2
    ////////////////////
    QCOMPARE(widgetReg->isRegistered(QString("asdf"), widgetInfo), false);
}

/***************************************************************************
 * Description: Test function to verify present() in WebAppRegistry
 *
 * Test case 1:  Install a test widget
 * Test case 2:  Confirm that list returned by present() contains
 *               the widget
 * Test case 3:  Mark installed widget not present
 * Test case 4:  Confirm that list returned by present() does not
 *               contain the widget
 * *************************************************************************/
void WrtWidgetManagerTest::presenceTest()
{
        QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");
    bool present;
    
    // Test case 1
    //////////////////
    QCOMPARE(true, twidgetInstall(pkgpath));

    //////////////////
    // Test case 2
    //////////////////
    present = false;
    QList<WebAppInfo>* presentList = WebAppRegistry::instance()->present(true);
    for (int i = 0 ; i < presentList->count() ; ++i) {
         if ((*presentList)[i].appId() == id)
      {present = true;
      }    }
    QCOMPARE(true, present);

    //////////////////
    // Test case 3
    //////////////////
    
    QCOMPARE(true, WebAppRegistry::instance()->setIsPresent(id, false));

    //////////////////
    // Test case 4
    //////////////////
    
    present = false;
    presentList = WebAppRegistry::instance()->present(true);
    for (int i = 0 ; i < presentList->count() ; ++i) {
      if ((*presentList)[i].appId() == id) {
           present = true;
      }
    }
   QCOMPARE(false, present);
   QCOMPARE(true, WebAppRegistry::instance()->setIsPresent(id, true));
   
}

/***************************************************************************
 * Description: Test function to verify present() in WebAppRegistry
 *
 * Test case 1:  Install a test widget
 * Test case 2:  Use WebAppInfo::getElement to get W3CElements
 * *************************************************************************/
void WrtWidgetManagerTest::getElementTest()
{
    QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
    QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");

    //////////////////
    // Test case 1
    //////////////////
    QCOMPARE(true, twidgetInstall(pkgpath));

    WebAppInfo webAppInfo;

    QCOMPARE(WebAppRegistry::instance()->isRegistered(id, webAppInfo), true);

    QList<W3CElement*> elements = webAppInfo.getElement(W3CSettingsKey::WIDGET_NAME,
                                                        NULL, QString());
    QCOMPARE(1, elements.count());
}



/***************************************************************************
 * Description: function to test WebApp Registery :: register Api
 *
 * Test case 1: Register a simple widget
 * Test case 2: Register a invalid widget
 * Test case 3: Register an already registered widget
 * **************************************************************************/
void WrtWidgetManagerTest::RegisterWidgetTest()
{
    WidgetManager wgtmgr(NULL);
    QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");    
    WebAppRegistry *widgetReg = WebAppRegistry::instance();
    WebAppInfo widgetInfo;
    WidgetProperties *props = NULL;

    if (!widgetReg->isRegistered(id , widgetInfo)) {
        QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
        QCOMPARE(twidgetInstall(pkgpath), true);
    }

    if (widgetReg->isRegistered(id , widgetInfo)) {
        props = wgtmgr.getProperties(widgetInfo.m_appPath);
        QCOMPARE((props != NULL), true);
        QCOMPARE(widgetReg->unregister(id), true);
    }


    //////////////////////////////
    //Test case 1
    /////////////////////////////
    const QString mainHtml = wgtmgr.launcherPath(widgetInfo.m_appPath);
    qDebug() << "Start file" << mainHtml;
    if (props)
        qDebug() << props->size();

    QCOMPARE(widgetReg->registerApp(id, widgetInfo.appTitle(), widgetInfo.appPath(),
                                    widgetInfo.iconPath(), widgetInfo.attributes(),
                                    props->type(), props->size(), mainHtml),
             true);


    ////////////////////////////////////
    // Test case 3
    //////////////////////////////////
#if defined(Q_OS_MAEMO5)
    // NOTE: code only returns false if there is an error....
    // does not return false if id previously registered
    QCOMPARE(widgetReg->registerApp(id, widgetInfo.appTitle(), widgetInfo.appPath(),
                                    widgetInfo.iconPath(), widgetInfo.attributes(),
                                    props->type(), props->size(), mainHtml),
             true);
#else
    // code returns false on error for Linux, unsure for Symbian
    QCOMPARE(widgetReg->registerApp(id, widgetInfo.appTitle(), widgetInfo.appPath(),
                                    widgetInfo.iconPath(), widgetInfo.attributes(),
                                    props->type(), props->size(), mainHtml),
             false);
#endif

    /////////////////////////////
    //Test case 2
    ///////////////////////////
    QCOMPARE(widgetReg->registerApp(QString(""), QString(""), QString(""), QString(""),
                                    widgetInfo.attributes(), QString(""), 0, QString("")),
             false);

}


////////////////////////////////////////////////////////////////////////
//Internal utility function to install the widgets
///////////////////////////////////////////////////////////////////////
bool WrtWidgetManagerTest::twidgetInstall(QString &pkgpath)
{
    WidgetManager widgetInstaller(NULL);
    QString widgetId;
    pkgpath = QDir::toNativeSeparators(pkgpath);
    WidgetInstallError errCode = WidgetInstallSuccess;
    // Successful install returns zero, positive value otherwise
    errCode = widgetInstaller.install(pkgpath, widgetId, true, false);
    // print the return code to log
    switch (errCode) {
    case WidgetInstallSuccess:
        QWARN("WidgetInstallSuccess");
        return true;
    case WidgetValidSignature:
        QWARN("WidgetValidSignature");
        return false;
    case WidgetUnZipBundleFailed:
        QWARN("WidgetUnZipBundleFailed");
        return false;
    case WidgetFindSignatureFailed:
        QWARN("WidgetFindSignatureFailed");
        return false;
    case WidgetSignatureParsingFailed:
        QWARN("WidgetSignatureParsingFailed");
        return false;
    case WidgetSignatureOrSignedInfoMissing:
        QWARN("WidgetSignatureOrSignedInfoMissing");
        return false;
    case WidgetSignatureRefExistFailed:
        QWARN("WidgetSignatureRefExistFailed");
        return false;
    case WidgetSignatureRefValidationFailed:
        QWARN("WidgetSignatureRefValidationFailed");
        return false;
    case WidgetCertValidationFailed:
        QWARN("WidgetCertValidationFailed");
        return false;
    case WidgetSignatureValidationFailed:
        QWARN("WidgetSignatureValidationFailed");
        return false;
    case WidgetParseManifestFailed:
        QWARN("WidgetParseManifestFailed");
        return false;
    case WidgetRegistrationFailed:
        QWARN("WidgetRegistrationFailed");
        return false;
    case WidgetReplaceFailed:
        QWARN("WidgetReplaceFailed");
        return false;
    case WidgetRmDirFailed:
        QWARN("WidgetRmDirFailed");
        return false;
    case WidgetCapabilityNotAllowed:
        QWARN("WidgetCapabilityNotAllowed");
        return false;
    case WidgetPlatformSpecificInstallFailed:
        QWARN("WidgetPlatformSpecificInstallFailed");
        return false;
    case WidgetCorrupted:
        QWARN("WidgetCorrupted");
        return false;
    case WidgetSharedLibraryNotSigned:
        QWARN("WidgetSharedLibraryNotSigned");
        return false;
    case WidgetDriveLetterValidationFailed:
        QWARN("WidgetDriveLetterValidationFailed");
        return false;
    case WidgetTypeValidationFailed:
        QWARN("WidgetTypeValidationFailed");
        return false;
    case WidgetSystemError:
        QWARN("WidgetSystemError");
        return false;
    case WidgetInstallPermissionFailed:
        QWARN("WidgetInstallPermissionFailed");
        return false;
    case WidgetUpdateFailed:
        QWARN("WidgetUpdateFailed");
        return false;
    case WidgetUpdateVersionCheckFailed:
        QWARN("WidgetUpdateVersionCheckFailed");
        return false;
    case WidgetUserConfirmFailed:
        QWARN("WidgetUserConfirmFailed");
        return false;
    case WidgetInstallFailed:
        QWARN("WidgetInstallFailed");
        return false;
    case WidgetStartFileNotFound:
        QWARN("WidgetStartFileNotFound");
        return false;
    default:
        QWARN("Unknown Error");
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////
//Internal utility function to install the widgets on user defined drives
/////////////////////////////////////////////////////////////////////////

 bool WrtWidgetManagerTest::twidgetInstallMultiDrive(QString &pkgpath, QString drive)
{
  WidgetManager widgetInstaller(NULL) ;
  QString widgetId;
  pkgpath = QDir::toNativeSeparators(pkgpath);
  WidgetInstallError errCode = WidgetInstallSuccess;
  // Successful install returns zero, positive value otherwise
  errCode = widgetInstaller.install(pkgpath, widgetId, true, false, drive);
  // print the return code to log
  switch (errCode)
  {
      case WidgetInstallSuccess:
          QWARN("WidgetInstallSuccess");
          return true;
      case WidgetValidSignature:
          QWARN("WidgetValidSignature");
          return false;
      case WidgetUnZipBundleFailed:
          QWARN("WidgetUnZipBundleFailed");
          return false;
      case WidgetFindSignatureFailed:
          QWARN("WidgetFindSignatureFailed");
          return false;
      case WidgetSignatureParsingFailed:
          QWARN("WidgetSignatureParsingFailed");
          return false;
      case WidgetSignatureOrSignedInfoMissing:
          QWARN("WidgetSignatureOrSignedInfoMissing");
          return false;
      case WidgetSignatureRefExistFailed:
          QWARN("WidgetSignatureRefExistFailed");
          return false;
      case WidgetSignatureRefValidationFailed:
          QWARN("WidgetSignatureRefValidationFailed");
          return false;
      case WidgetCertValidationFailed:
          QWARN("WidgetCertValidationFailed");
          return false;
      case WidgetSignatureValidationFailed:
          QWARN("WidgetSignatureValidationFailed");
          return false;
      case WidgetParseManifestFailed:
          QWARN("WidgetParseManifestFailed");
          return false;
      case WidgetRegistrationFailed:
          QWARN("WidgetRegistrationFailed");
          return false;
      case WidgetReplaceFailed:
          QWARN("WidgetReplaceFailed");
          return false;
      case WidgetRmDirFailed:
          QWARN("WidgetRmDirFailed");
          return false;
      case WidgetCapabilityNotAllowed:
          QWARN("WidgetCapabilityNotAllowed");
          return false;
      case WidgetPlatformSpecificInstallFailed:
          QWARN("WidgetPlatformSpecificInstallFailed");
          return false;
      case WidgetCorrupted:
          QWARN("WidgetCorrupted");
          return false;
      case WidgetSharedLibraryNotSigned:
          QWARN("WidgetSharedLibraryNotSigned");
          return false;
      case WidgetDriveLetterValidationFailed:
          QWARN("WidgetDriveLetterValidationFailed");
          return false;
      case WidgetTypeValidationFailed:
          QWARN("WidgetTypeValidationFailed");
          return false;
      case WidgetSystemError:
          QWARN("WidgetSystemError");
          return false;
      case WidgetInstallPermissionFailed:
          QWARN("WidgetInstallPermissionFailed");
          return false;
      case WidgetUpdateFailed:
          QWARN("WidgetUpdateFailed");
          return false;
      case WidgetUpdateVersionCheckFailed:
          QWARN("WidgetUpdateVersionCheckFailed");
          return false;
      case WidgetUserConfirmFailed:
          QWARN("WidgetUserConfirmFailed");
          return false;
      case WidgetInstallFailed:
          QWARN("WidgetInstallFailed");
          return false;
      default:
          QWARN("Unknown Error");
          return false;
  }
}



////////////////////////////////////////////////////////////////////////
//Internal  function to uninstall the widgets
///////////////////////////////////////////////////////////////////////

 bool WrtWidgetManagerTest::twidgetUnInstall(QString &id)
{
  WidgetManager widgetInstaller(NULL) ;
  if (widgetInstaller.uninstall(id, true) == WidgetUninstallSuccess)
       return true;
   else
       return false;
}

///////////////////////////////////////////////////////////
//Inter function to check the w3c elements
////////////////////////////////////////////////////////////
 bool  WrtWidgetManagerTest::CheckW3CElem(const QString &elem , QString &expected , QString aAttr/*=NULL*/)
{
  return true ;
  ///////////////////////////////////////
  //Below lines needs to be enabled once
  //The latest patch to widget manager is
  //available
  //////////////////////////////////////
  WidgetManager wgtMgr(NULL);

  QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
  QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");

  WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
  WebAppInfo widgetInfo;

  if (!widgetReg->isRegistered(id , widgetInfo ))  {
     QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");

     if (!twidgetInstall(pkgpath) )   {
       qDebug() << "failed to install file" << pkgpath;
       return false ;
     }
  }

  WidgetProperties *WgtProperties = wgtMgr.getProperties(widgetInfo.m_appPath) ;

  if (!WgtProperties)   {
    qDebug() << "Failed to find the widget properties" ;
    return false ;
  }
  AttributeMap attrMap  = WgtProperties->plist();

  QVariant varelm = attrMap.value(elem);
  QString  actualValue;

  if (!varelm.isNull()  )   {
     const W3CElement *w3elem =  qVariantValue<const W3CElement *> (varelm);

     if (!aAttr.isEmpty())   {
        actualValue = w3elem->attributeValue(aAttr);
     }
     else   {
        actualValue = w3elem->readElementText();
     }
  }
  else   {
    return false ;
  }
  if (actualValue != expected)   {
    qDebug() << "Actual and compared values are different";
    return false ;
  }
  return true ;
}

///////////////////////////////////////////////////////////
//Inter function to check the w3c elements
////////////////////////////////////////////////////////////
 bool  WrtWidgetManagerTest::CheckW3CElem(const QString &elem , int Expectedcount ,  QString aAttr/*=NULL*/)
{
  return true ;
  ///////////////////////////////////////
  //Below lines needs to be enabled once
  //The latest patch to widget manager is
  //available
  //////////////////////////////////////
  WidgetManager wgtMgr(NULL);

  QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
  QString id("46518f8352111086ac66a8f18d68a6fab8ae76e9");

  WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
  WebAppInfo widgetInfo;

  if (!widgetReg->isRegistered(id , widgetInfo ))  {
     QString pkgpath = APITEST_WIDGETBASE + QString("widgets/completewidget.wgt");
     if (!twidgetInstall(pkgpath) )   {
       qDebug() << "failed to install file" << pkgpath;
       return false ;
     }
  }

  WidgetProperties *WgtProperties = wgtMgr.getProperties(widgetInfo.m_appPath) ;

  if (!WgtProperties)   {
    qDebug() << "Failed to find the widget properties" ;
    return false ;
  }
  AttributeMap attrMap  = WgtProperties->plist();
  AttributeMap::iterator iter = attrMap.begin();
  int count = 0 ;

  for ( ; iter != attrMap.end() ; ++iter) {
     if (iter.key().startsWith(elem))   {
        ++count;
     }
  }

  return count == Expectedcount;
}


////////////////////////////////////////////////////////////////////
//Inetnal function for language settings
////////////////////////////////////////////////////////////////////

 bool WrtWidgetManagerTest::tSetLanguage(const QString &aLang)
{
  WrtSettings* settings;
  settings = WrtSettings::createWrtSettings();
  settings->setValue("UserAgentLanguage" , aLang) ;
  QString setValue = settings->valueAsString("UserAgentLanguage");
  return  setValue == aLang ;
}

////////////////////////////////////////////////////////////////////
//Inetnal function for checking localization path  path
////////////////////////////////////////////////////////////////////

 bool WrtWidgetManagerTest::tCheckLocalizedPath(const QString &elemKey , const QString &WidgetId , const QString &aPath )
{
  WidgetManager wgtMgr(NULL);
  WebAppRegistry *widgetReg = WebAppRegistry::instance() ;
  WebAppInfo widgetInfo;

  if (!widgetReg->isRegistered(WidgetId , widgetInfo ))  {
     qDebug() << "Widget Not registered" ;
     return false ;
  }

  WidgetProperties *WgtProperties = wgtMgr.getProperties(widgetInfo.m_appPath) ;

  if (!WgtProperties)   {
    qDebug() << "Failed to find the widget properties" ;
    return false ;
  }

  bool retval = false ;

  if (elemKey == W3CSettingsKey::WIDGET_ICON)   {
    qDebug() << "Icon Path" << WgtProperties->iconPath();
    QString iconPath = WgtProperties->iconPath();

    ///////////////////////////
    // Check the icon path with
    // the passed value
    //////////////////////////
    
    if (!iconPath.contains( QDir::toNativeSeparators(aPath) ))   {
      qDebug() << "Path mismatch" << "Actual" << iconPath << "compared" << aPath ;
      return false ;
    }
    ///////////////////////////////
    //Verify if the file is present
    //////////////////////////////
    
    QFile iconFile(iconPath);
    if (!iconFile.exists())   {
      qDebug() << "Icon file dosen't exist";
      return false ;
    }
    return true ;
  }  // End of WIDGET_ICON

  else if (elemKey == W3CSettingsKey::WIDGET_CONTENT)   {
    QRegExp part(WIDGET_FOLDER);
    QString stFile = wgtMgr.launcherPath(WgtProperties->installPath());
    if (stFile.isEmpty())   {
    qDebug() << "empty StartFile" ;
    return false ;
    }
    
    // Folder-based localization should compare with current page
    QString path = stFile;
    QStringList fileParts;
    
#if defined (Q_OS_SYMBIAN)
    QString part2 = path.section(part,1,1);
    QString fileSeparator("\\");            
    fileParts.append(path.section(part,0,0)+WIDGET_FOLDER+fileSeparator+part2.section(fileSeparator,0,0,QString::SectionSkipEmpty));
    fileParts.append(part2.section(fileSeparator,1,-1,QString::SectionSkipEmpty));
 
#else 
    if (!SuperWidget::getWidgetPathParts(fileParts,path))return false;
#endif  
    QString root = fileParts.at(0);
   
#if !defined(Q_OS_LINUX) && !defined(Q_OS_MEEGO) 
      if (root.startsWith("/"))
      root = root.mid(1,-1);
#endif
    if (SuperWidget::getWidgetType(root)!=WidgetTypeW3c) return false;
    
#if defined(Q_OS_MAEMO5) || defined(Q_OS_MAEMO6) || defined(Q_OS_LINUX) || defined(Q_OS_MEEGO)
    QString locFile = WebAppLocalizer::getLocalizedFile(path, true, root);
#else   // symbian || win32
    QString locFile = WebAppLocalizer::getLocalizedFile(path, true, root);
#endif
    locFile = QDir::toNativeSeparators(locFile);
    QString sep = QDir::separator();
    if(locFile.contains(sep+sep))
    {   
      locFile.replace(sep+sep,sep);
    }
       stFile = locFile;
       
    ///////////////////////////////
    //Verify launch path contains
    //locale folder
    //////////////////////////////
       
       if (!stFile.contains(QDir::toNativeSeparators(aPath)))   {
       qDebug() << "Launch path contains no locale component" ;
       qDebug() << aPath << stFile ;
       return false ;
    }

#if defined(Q_OS_MAEMO6) || defined(Q_OS_MAEMO5)
    WrtSettings* settings;
    settings = WrtSettings::createWrtSettings();
    QString setValue = settings->valueAsString("UserAgentLanguage");
    stFile = WgtProperties->installPath()+QDir::separator()+"locales"+QDir::separator()+setValue+QDir::separator()+aPath;
#endif


    ///////////////////////////////
    //Verify if the file is present
    //////////////////////////////
    QFile contentFile(stFile);
    if (!contentFile.exists())   {
    qDebug() << "start file dosen't exist";
    return false ;
    }
        
  } //End of WIDGET_CONTENT
  else if (elemKey ==  W3CSettingsKey::WIDGET_NAME)   {
     QString title = WgtProperties->title();

      if (title == aPath)   {
         return true ;
      }
      qDebug() << "Name elements are different" << aPath << title ;
      return false ;
  }
  return true;
}

 #ifdef QTWRT_USE_DEBIAN_PACKAGING
 /********************************************************************
  * Description: Test function to check Author field in INFO of deb package
  *
  * Test case : create deb package.
  *             check Author field in INFO file
  * *****************************************************************/
 void WrtWidgetManagerTest::WidgetDebInfoAuthorTest()
 {
     QProcess* runner = new QProcess(this);
     QStringList params;

     params << APITEST_WIDGETBASE + "widgets/HelloWorldWithoutEmail.wgt" << "--no-install" << "--path"<< "/var/tmp/" << "--test";
     int ret = QProcess::execute("widgetinstaller", params);
     params.clear();
     runner->start("dpkg --info /var/tmp/adaa43ff1b48658120cd38b17518c9621e3c04cb-wrt-widget.deb");
     QVERIFY(runner->waitForFinished());
     QByteArray info = runner->readAllStandardOutput();
     QString str(info);
     QVERIFY(str.contains("Maintainer:") &&
             str.contains("HelloWorld", Qt::CaseInsensitive));
 }

 /********************************************************************
  * Description: Test function to check author email attribute in INFO
  *              file of deb package
  * Test case : create deb package.
  *             check author email attribute in INFO file
  * *****************************************************************/
 void WrtWidgetManagerTest::WidgetDebInfoAuthorMailTest()
 {
     QProcess* runner = new QProcess(this);
     QStringList params;

     params << APITEST_WIDGETBASE + "widgets/HelloWorld.wgt" << "--no-install" << "--path"<< "/var/tmp/" << "--test";
     int ret = QProcess::execute("widgetinstaller", params);
     params.clear();
     runner->start("dpkg --info /var/tmp/fa201cafca0fcb107a4efe5789b77768cd05f100-wrt-widget.deb");

     QVERIFY(runner->waitForFinished());
     QByteArray info = runner->readAllStandardOutput();
     QString str(info);
     QVERIFY(str.contains("Maintainer:") &&
             str.contains("HelloWorld", Qt::CaseInsensitive) &&
             str.contains("HelloWorld@HelloWorld.com", Qt::CaseInsensitive));
 }
 #endif
