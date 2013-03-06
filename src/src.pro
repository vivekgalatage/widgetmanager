# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
#
# This file is part of Qt Web Runtime.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
#
#

#
#  widget utilities files
#
TEMPLATE = lib
TARGET = wacWidgetUtils

QT += xml sql
CONFIG += dll link_prl create_prl

ROOT_DIR = $$PWD/../../..
WRT_DIR = $$ROOT_DIR/wrt
include($$ROOT_DIR/wrt/wrt.pri)
include($$ROOT_DIR/wrt/cwrt.pri)
include($$ROOT_DIR/qtsfw.pri)
!contains ( what, tenone ) {
    include($$ROOT_DIR/wrt/translation.pri)
}
include($$ROOT_DIR/wrt/utilities/CertificateManager/CertificateManager.pri)
include($$ROOT_DIR/wrt/utilities/W3CWidgetVerifier/W3CWidgetVerifier.pri)

DEFINES *=BUILDING_WIDGETUTILS_LIB
!symbian:include (../../3rdparty/zlib/zlib.pri)

contains ( what, blockunsignedwidgets ) {
    DEFINES += BLOCK_UNSIGNED_WIDGETS
}

contains (what, enable-widget-deps): DEFINES += ENABLE_WIDGET_DEPS

DEFINES += OCSP_FEATURE_ENABLED

INCLUDEPATH += $$PWD \
               $$PWD/private \
               $$PWD/resource  \
               $$PWD/W3CXmlParser \
               $$PWD/../../.. \
               $$PWD/../../3rdparty/zlib \
               $$PWD/../../runtimecore \
               $$PWD/../../../src/common/securestorage \
               $$PWD/../../../src/common/securestorage/platform/qt \
               $$PWD/../../../src/common/securestorage/platform/qt/securestorageclient \
               $$PWD/../../../src/common/security \
               $$PWD/../../MiscServices/SchemeServices \
               $$PWD/../../settings/inc \
               $$PWD/../../utilities/W3CWidgetVerifier

INCLUDEPATH += $$OPENSSL_INC_DIR
INCLUDEPATH += $$LIBXML2_INC_DIR

!symbian {
    LIBS += -lWrtSecureStorage -lWrtSecurityManager -lWrtSettings
} else {
    LIBS += -lwacsecurestorage -lwacsecmgr -lwacSettings
}

symbian {
    SOURCES += $$PWD/platform/symbian/EmbedWidgetInstaller.cpp
    HEADERS += $$PWD/platform/symbian/EmbedWidgetInstaller.h
    contains ( what, orbit ) {
        LIBS += -lsifui -lsif -luissclient -lsifnotification
    }
}

macx {
    LIBS += -lxml2 -lssl -lcrypto
} else {

    LIBS += $$OPENSSL_LIB_DIR
    LIBS += $$LIBXML2_LIB_DIR
}

win32 {
    LIBS += -llibxml2 \
        ssleay32.lib libeay32.lib
}

HEADERS += \
           $$PWD/private/WidgetUtilsLogs.h \
           $$PWD/private/WidgetLinkResolver.h \
           $$PWD/private/WidgetPListParser.h \
           $$PWD/private/webappmanager_p.h \
           $$PWD/wacAttributeMap.h \
           $$PWD/wacPropertyNames.h \
           $$PWD/wacWidgetManager.h \
           $$PWD/wacWidgetProperties.h \
           $$PWD/wacWebAppRegistry.h \
           $$PWD/wacwebappinfo.h \
           $$PWD/webapplocalizer.h \
           $$PWD/webappuniquenesshelper.h \
           $$PWD/private/wacWidgetInfo.h \
           $$PWD/W3CXmlParser/w3ctags.h \
           $$PWD/W3CXmlParser/wacw3celement.h \
           $$PWD/W3CXmlParser/w3cxmlplugin.h \
           $$PWD/W3CXmlParser/wacw3csettingskeys.h \
           $$PWD/W3CXmlParser/configw3xmlparser.h \
           $$PWD/wacSuperWidget.h \
           $$PWD/WgzWidget.h \
           $$PWD/WgtWidget.h \
           $$PWD/desktoputils.h \
           $$PWD/AsyncInstall.h \
           $$PWD/InstallWorker.h \
           $$PWD/widgetinstaller.h \
           $$PWD/WidgetManager_p.h \
           $$PWD/featuremapping.h

SOURCES += \
           $$PWD/private/WidgetLinkResolver.cpp \
           $$PWD/private/WidgetPListParser.cpp \
           $$PWD/WidgetManager.cpp \
           $$PWD/WidgetManager_p.cpp \
           $$PWD/WebAppRegistry.cpp \
           $$PWD/webappinfo.cpp \
           $$PWD/webapplocalizer.cpp \
           $$PWD/webappuniquenesshelper.cpp \
           $$PWD/W3CXmlParser/w3celement.cpp \
           $$PWD/W3CXmlParser/w3cxmlplugin.cpp \
           $$PWD/W3CXmlParser/configw3xmlparser.cpp \
           $$PWD/SuperWidget.cpp \
           $$PWD/WgzWidget.cpp \
           $$PWD/WgtWidget.cpp \
           $$PWD/desktoputils.cpp \
           $$PWD/AsyncInstall.cpp \
           $$PWD/InstallWorker.cpp \
           $$PWD/widgetinstaller.cpp \
           $$PWD/featuremapping.cpp
   
#contains( what, optinstaller ) { 
symbian {   
            DEFINES += OPTIMIZE_INSTALLER
            DEFINES += ENABLE_LOG
            DEFINES += USE_NATIVE_DIALOG
            
            LIBS += -lSWInstCommonUI 
            
            LIBS+=  -lavkon \
            -leikcdlg \
            -leikctl \
            -laknnotify \
            -lbafl \
            -lcone \
            -leikcore \
            -lcommonengine \
            -leuser 
            
            include(localization.pri)
            
            INCLUDEPATH += $$MW_LAYER_DOMAININCLUDE
            
            HEADERS += \
                       $$PWD/lifecycle/widgetlog.h \
                       $$PWD/lifecycle/widgetutilities.h \
                       $$PWD/lifecycle/widgettypes.h \                       
                       $$PWD/lifecycle/widgetinformation.h \
                       $$PWD/lifecycle/widgetcontext.h \                       
                       $$PWD/lifecycle/widgetcontextcreator.h \
                       $$PWD/lifecycle/widgetstatemachine.h \
                       $$PWD/lifecycle/widgetinstallcontext.h \
                       $$PWD/lifecycle/widgetoperation.h \                       
                       $$PWD/lifecycle/widgetoperationmanager.h \
                       $$PWD/lifecycle/widgetcontentextraction.h \
                       $$PWD/lifecycle/widgetinformationinitializer.h \ 
                       $$PWD/lifecycle/widgetparsemanifest.h \
                       $$PWD/lifecycle/widgetdigitalsignvalidation.h \
                       $$PWD/lifecycle/widgetprepareforinstall.h \
                       $$PWD/lifecycle/platform/symbian/widgethomescreenintegration.h \
                       $$PWD/lifecycle/widgetdirectorybackup.h \
                       $$PWD/lifecycle/widgetprepareinstallationdirectories.h \
                       $$PWD/lifecycle/widgetfinalizeinstallation.h \
                       $$PWD/lifecycle/widgetsetupinstallationdirectory.h \
                       $$PWD/lifecycle/widgetregistration.h \
                       $$PWD/lifecycle/widgetsecuritysessionsetup.h \
                       $$PWD/lifecycle/widgetcleanup.h

            SOURCES +=  \
                        $$PWD/lifecycle/widgetutilities.cpp \
                        $$PWD/lifecycle/widgetinformation.cpp \
                        $$PWD/lifecycle/widgetcontext.cpp \
                        $$PWD/lifecycle/widgetcontextcreator.cpp \
                        $$PWD/lifecycle/widgetstatemachine.cpp \                       
                        $$PWD/lifecycle/widgetinstallcontext.cpp \
                        $$PWD/lifecycle/widgetoperation.cpp \
                        $$PWD/lifecycle/widgetoperationmanager.cpp \
                        $$PWD/lifecycle/widgetcontentextraction.cpp \    
                        $$PWD/lifecycle/widgetinformationinitializer.cpp \
                        $$PWD/lifecycle/widgetparsemanifest.cpp \
                        $$PWD/lifecycle/widgetdigitalsignvalidation.cpp \
                        $$PWD/lifecycle/widgetprepareforinstall.cpp \
                        $$PWD/lifecycle/platform/symbian/widgethomescreenintegration.cpp \
                        $$PWD/lifecycle/widgetdirectorybackup.cpp \
                        $$PWD/lifecycle/widgetprepareinstallationdirectories.cpp \
                        $$PWD/lifecycle/widgetfinalizeinstallation.cpp \
                        $$PWD/lifecycle/widgetsetupinstallationdirectory.cpp \
                        $$PWD/lifecycle/widgetregistration.cpp \
                        $$PWD/lifecycle/widgetsecuritysessionsetup.cpp \
                        $$PWD/lifecycle/widgetcleanup.cpp
            
            symbian {
                        INCLUDEPATH += $$PWD/lifecycle
                        HEADERS += $$PWD/lifecycle/platform/symbian/widgetconstants.h \
                                   $$PWD/lifecycle/platform/symbian/widgetcheckinstallationfromfolder.h \
                                   $$PWD/lifecycle/platform/symbian/widgetfileextractor.h \
                                   $$PWD/lifecycle/platform/symbian/symbianutils.h
                        SOURCES += $$PWD/lifecycle/platform/symbian/widgetinformationinitializer_symbian.cpp \
                                   $$PWD/lifecycle/platform/symbian/widgetcontentextraction_symbian.cpp \
                                   $$PWD/lifecycle/platform/symbian/widgetcheckinstallationfromfolder.cpp \
                                   $$PWD/lifecycle/platform/symbian/widgetfileextractor.cpp \
                                   $$PWD/lifecycle/platform/symbian/symbianutils.cpp
            }
}
#}           

RESOURCES += $$PWD/resources.qrc

!symbian {
    DEFINES += SHA256_SUPPORT
}

symbian: {
    contains( what, ninetwo ) {
        DEFINES += SHA256_SUPPORT
    }

    contains( what, tenone ) {
        !contains( DEFINES, SHA256_SUPPORT ) {
            DEFINES += SHA256_SUPPORT
        }
    }

    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = All -Tcb
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000 // Min 128kB, Max 32MB
    TARGET.VID = VID_DEFAULT
    TARGET.UID3 = 0x2003DE21
    DEFINES += NO_IOSTREAM
    DEFINES += _WCHAR_T_DECLARED
    QMAKE_CXXFLAGS.CW = -O1 -wchar_t on

    INCLUDEPATH += \
                   $$PWD/platform/symbian/ \
                   $$MW_LAYER_SYSTEMINCLUDE

    HEADERS += \
               $$PWD/platform/symbian/IconConverter.h \
               $$PWD/platform/symbian/WidgetRegistrationS60.h \
               $$PWD/platform/symbian/WidgetRegistrationS60Apparc.h \
               $$PWD/platform/symbian/WidgetUnzipUtilityS60.h

    contains ( what, usif ) {
        HEADERS += $$PWD/platform/symbian/WidgetRegistrationS60_SCR.h
    }

    SOURCES += \
               $$PWD/platform/symbian/IconConverter.cpp \
               $$PWD/platform/symbian/WidgetRegistrationS60.cpp \
               $$PWD/platform/symbian/WidgetRegistrationS60Apparc.cpp \
               $$PWD/platform/symbian/WidgetUnzipUtilityS60.cpp

    contains ( what, usif ) {
        SOURCES += $$PWD/platform/symbian/WidgetRegistrationS60_SCR.cpp
    }

    !contains ( what, tenone ) {
        LIBS += -lWidgetRegistryClient
    }

    LIBS += -llibcrypt \
            -llibz \
            -llibssl \
            -llibcrypto \
            -lfbscli \
            -lefsrv \
            -limageconversion \
            -lbitgdi \
            -lgdi \
            -lbitmaptransforms \
            -lapgrfx \
            -lws32 \
            -lapparc \
            -lestor \
            -lezip \
            -lSWInstCli \
            -lxmlengine \
            -lcone \
            -lsysutil

    contains ( what, usif ) {
        LIBS += \
            -lscrclient \
            -lsifutils
    }

    wacwidgetmgrlibs.sources = wacWidgetUtils.dll
    wacwidgetmgrlibs.path = /sys/bin
    DEPLOYMENT += wacwidgetmgrlibs

    wacwidgetcertificate.sources = ../../../do-not-opensource/certs/myWidgetCert.pem
    wacwidgetcertificate.path = /private/2003DE07/widgets_21D_4C7/widgetCertificates
    DEPLOYMENT += wacwidgetcertificate

    multiprocsettingsini.sources = ./resource/multiprocsettings.ini
    multiprocsettingsini.path = /private/2003DE0D/multiprocini
    DEPLOYMENT += multiprocsettingsini

    mapping.sources = ./resource/mapping.xml \
    				  ./resource/prompt_mapping.xml
    mapping.path = /private/2003DE0D/mapping
    DEPLOYMENT += mapping

    BLD_INF_RULES.prj_exports += \
        "../../../do-not-opensource/certs/cacerts.dat   /epoc32/winscw/c/private/101f72A6/cacerts.dat"

    BLD_INF_RULES.prj_exports += \
        "./resource/mapping.xml   /epoc32/winscw/c/private/2003DE0D/mapping/mapping.xml"

contains ( what, tenone ) {
    BLD_INF_RULES.prj_exports += \
        "./platform/symbian/SCRConstants.h   $$MW_LAYER_PUBLIC_EXPORT_PATH(cwrt/SCRConstants.h)"
}

} # end of symbian:

contains( what, no_sha256 ) {
    DEFINES -= SHA256_SUPPORT
}

#export cert files
CONFIG(release, debug|release): EXPORT_CERT_DESTDIR = $$PWD/../../../WrtBuild/Release/bin/
CONFIG(debug, debug|release): EXPORT_CERT_DESTDIR = $$PWD/../../../WrtBuild/Debug/bin/
EXPORT_CERT_SRCDIR = $$PWD/../../../do-not-opensource/certs/
win32: {
   EXPORT_CERT_DESTDIR = $$replace(EXPORT_CERT_DESTDIR,/,\)
   EXPORT_CERT_SRCDIR = $$replace(EXPORT_CERT_SRCDIR,/,\)
}
unix:!maemo {
    # message("linux/meego - do not export certs")
} else {
    symbian-sbsv2: {
        export_cert_files_pem_cmd.target = $$PWD/__wrtexportcert__
        LOCAL_COPY = cp
    } else {
        export_cert_files_pem_cmd.target = __wrtexportcert__
        LOCAL_COPY = $$QMAKE_COPY
    }
    export_cert_files_pem_cmd.commands = $$LOCAL_COPY $${EXPORT_CERT_SRCDIR}rd.cer $${EXPORT_CERT_DESTDIR}rd.pem \
                                      && $$LOCAL_COPY $${EXPORT_CERT_SRCDIR}3rdparty.cer $${EXPORT_CERT_DESTDIR}3rdparty.pem
    PRE_TARGETDEPS += $$export_cert_files_pem_cmd.target
    QMAKE_EXTRA_TARGETS += export_cert_files_pem_cmd
}
#end of export

win32: {
    isEmpty(WRT_OUTPUT_DIR): {
        CONFIG(release, debug|release): EXPORT_LIBXML_DESTDIR=$$PWD/../../../WrtBuild/Release
        CONFIG(debug, debug|release): EXPORT_LIBXML_DESTDIR=$$PWD/../../../WrtBuild/Debug
    } else {
        EXPORT_LIBXML_DESTDIR = $$WRT_OUTPUT_DIR
    }

    EXPORT_LIBXML_DESTDIR = $$EXPORT_LIBXML_DESTDIR/bin/
    EXPORT_LIBXML_DESTDIR = $$replace(EXPORT_LIBXML_DESTDIR,/,\)

    EXPORT_LIBXML_DLL = $$PWD/../../utilities/W3CWidgetVerifier/platform/win32/libxml2/lib/libxml2.dll \
                        $$PWD/../../wrt/3rdparty/platform/win32/iconv/bin/iconv.dll \
                        $$PWD/../../wrt/3rdparty/platform/win32/zlib/bin/zlib1.dll

    EXPORT_LIBXML_DIR = $$EXPORT_LIBXML_DESTDIR

    export_libxml_dll_cmd.input = EXPORT_LIBXML_DLL
    export_libxml_dll_cmd.output = $$EXPORT_LIBXML_DIR${QMAKE_FILE_BASE}.dll
    export_libxml_dll_cmd.commands = $(COPY) ${QMAKE_FILE_NAME} $$EXPORT_LIBXML_DIR${QMAKE_FILE_BASE}.dll
    export_libxml_dll_cmd.CONFIG = target_predeps no_link
    export_libxml_dll_cmd.clean = $$EXPORT_LIBXML_DIR${QMAKE_FILE_BASE}.dll
    QMAKE_EXTRA_COMPILERS += export_libxml_dll_cmd
}

unix:!symbian {
    CONFIG += link_pkgconfig
    PKGCONFIG += openssl libxml-2.0 libarchive

    maemo {
        SOURCES -= $$PWD/widgetinstaller.cpp \
                   $$PWD/WgzWidget.cpp

        HEADERS -= $$PWD/WgzWidget.h

        SOURCES += $$PWD/platform/maemo/widgetinstaller.cpp \
                   $$PWD/platform/maemo/widgetinstaller_p.cpp \
                   $$PWD/platform/maemo/desktopfilewriter.cpp \
                   $$PWD/platform/maemo/archiver.cpp \
                   $$PWD/platform/maemo/packageutils.cpp

        HEADERS += $$PWD/platform/maemo/widgetinstaller_p.h \
                   $$PWD/platform/maemo/desktopfilewriter.h \
                   $$PWD/platform/maemo/archiver.h \
                   $$PWD/platform/maemo/packageutils.h

        maemo {
            SOURCES += $$PWD/platform/maemo/debianutils.cpp
            HEADERS += $$PWD/platform/maemo/debianutils.h
        }

        meego {
            SOURCES += $$PWD/platform/maemo/rpmutils.cpp
            HEADERS += $$PWD/platform/maemo/rpmutils.h
        }

        maemo6 {
            INCLUDEPATH += /usr/include/package-manager-dbus-qt-0.0
            LIBS += -lpackage-manager-dbus-qt -larchive
        }
    }

    headers.files = \
                    $$PWD/wacAttributeMap.h \
                    $$PWD/wacPropertyNames.h \
                    $$PWD/wacWidgetManager.h \
                    $$PWD/wacWidgetProperties.h \
                    $$PWD/wacWebAppRegistry.h \
                    $$PWD/wacwebappinfo.h \
                    $$PWD/wacSuperWidget.h \
                    $$PWD/WgtWidget.h \
                    $$PWD/AsyncInstall.h \
                    $$PWD/InstallWorker.h \
                    $$PWD/widgetinstaller.h \
                    $$PWD/featuremapping.h \
                    $$PWD/wacwidgetmanagerconstants.h \
                    $$PWD/wacWidgetUtils.h
    headers.path = $$CWRT_INSTALL_INC_DIR/widgetmanager

    headers_platform_qt.files = $$PWD/platform/qt/WidgetRegistrationQt.h
    headers_platform_qt.path = $$CWRT_INSTALL_INC_DIR/widgetmanager/platform/qt

    headers_w3cxmlparser.files = \
                                 $$PWD/W3CXmlParser/w3ctags.h \
                                 $$PWD/W3CXmlParser/wacw3celement.h \
                                 $$PWD/W3CXmlParser/w3cxmlplugin.h \
                                 $$PWD/W3CXmlParser/wacw3csettingskeys.h \
                                 $$PWD/W3CXmlParser/configw3xmlparser.h
    headers_w3cxmlparser.path = $$CWRT_INSTALL_INC_DIR/widgetmanager/W3CXmlParser

    headers_private.files = \
                            $$PWD/private/WidgetUtilsLogs.h \
                            $$PWD/private/WidgetLinkResolver.h \
                            $$PWD/private/WidgetPListParser.h \
                            $$PWD/private/wacWidgetInfo.h
    headers_private.path = $$CWRT_INSTALL_INC_DIR/widgetmanager/private

    mapping.path = $$CWRT_POLICY_DIR
    mapping.files = $$PWD/resource/mapping.xml

    !maemo {
        # message("linux/meego - do not export certs")
        INSTALLS += mapping headers headers_platform_qt headers_w3cxmlparser headers_private
    } else {
        # M6 uses aegisfs to secure policy files. The files are moved from
        # CWRT_POLICY_AEGIS_DIR to CWRT_POLICY_DIR during image creation,
        # this overwrites files in CWRT_POLICY_DIR. See debian/wrt.postinst
        maemo6 {
            mappingAegis.path = $$CWRT_POLICY_AEGIS_DIR
            mappingAegis.files = $$PWD/resource/mapping.xml
            INSTALLS += mappingAegis
        }

        cert.path = $$CWRT_CERT_DIR
        cert.files = resource/codesigning/58bf92e1b3c615698c6c5ef59c25c42ddf1fd8c5.pem \
                     resource/codesigning/32fcf9bf.0

        certman.path = $$CWRT_CERTMAN_DIR
        certman.files = resource/certman.codesigning

        maemo5 {
            INSTALLS += headers headers_platform_qt headers_w3cxmlparser headers_private
            PKGCONFIG += dbus-1 libhildonmime glib-2.0
        } else {
            INSTALLS += cert mapping certman headers headers_platform_qt headers_w3cxmlparser headers_private
        }
    }
}

symbian {
    EXPORT_FILES = \
        wacWebAppRegistry.h \
        wacWidgetManager.h \
        wacWidgetUtils.h \
        wacwebappinfo.h \
        wacAttributeMap.h \
        wacSuperWidget.h \
        wacwidgetmanagerconstants.h \
        wacWidgetProperties.h \
        wacPropertyNames.h \
        wacWebAppRegistry_p.h \
        W3CXmlParser/wacw3csettingskeys.h
    include($$WRT_DIR/cwrt-export-files.pri)

    EXPORT_DIR = $$MW_LAYER_PUBLIC_EXPORT_PATH(wac/private/)
    EXPORT_FILES = private/wacWidgetInfo.h
    include($$WRT_DIR/cwrt-export-files.pri)
    
    EXPORT_DIR = $$MW_LAYER_PUBLIC_EXPORT_PATH(wac/W3CXmlParser/)
    EXPORT_FILES = W3CXmlParser/wacw3celement.h
}

include($$WRT_DIR/cwrt-export.pri)
