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

######################################################################
# Automatically generated by qmake (2.01a) Thu Oct 15 13:10:03 2009
######################################################################
TEMPLATE = lib

symbian: {
CONFIG += stdbinary
} else {
CONFIG += dll
}

QT += testlib xml network

ROOT_DIR = $$PWD/../../..
WRT_DIR = $$ROOT_DIR/wrt

include($$WRT_DIR/cwrt-webkit.pri)
include($$PWD/t_widgetmanager.pri)

linux | meego: {
    DEFINES += QTWRT_API_LINUX=1
}
contains( what, tenone ) {
            DEFINES *= CWRT_BUILDING_TENONE
}

contains( what, ninetwo ) {
            DEFINES *= CWRT_BUILDING_NINETWO
}

!maemo {
isEmpty(WRT_OUTPUT_DIR): {
    CONFIG(release, debug|release):WIDGET_TESTS_OUTPUT_DIR=$$ROOT_DIR/WrtBuild/Release
    CONFIG(debug, debug|release):WIDGET_TESTS_OUTPUT_DIR=$$ROOT_DIR/WrtBuild/Debug
} else {
    WIDGET_TESTS_OUTPUT_DIR = $$WRT_OUTPUT_DIR
}

DESTDIR = $$WIDGET_TESTS_OUTPUT_DIR/bin


QMAKE_RPATHDIR += $$WIDGET_TESTS_OUTPUT_DIR/bin
QMAKE_LIBDIR   += $$WIDGET_TESTS_OUTPUT_DIR/bin
}

# believe that the following lines are useless on all platforms
# as there are no libraries in $$WRT_OUTPUT_DIR
!s40:!isEmpty(WRT_OUTPUT_DIR): {
    LIBS += -L$$WRT_OUTPUT_DIR
}


symbian: {
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x20000 0x2000000 // Min 128kB, Max 32MB
    TARGET.CAPABILITY= All -Tcb

    symbian:TARGET.UID3 = 0xEE00013

    wrtwidgetmanagertest.sources = wacWidgetManagerTest.dll
    wrtwidgetmanagertest.path = /sys/bin
    DEPLOYMENT += wrtwidgetmanagertest

    !contains(what,tenone):!contains(what,ninetwo) {
        MMP_RULES += NOSTDCPP
    }
    MMP_RULES += EXPORTUNFROZEN
}

!symbian {
    LIBS += -lWrtWidgetUtils -lWrtSettings
    TARGET = WrtWidgetManagerTest
} else {
    LIBS += -lwacWidgetUtils -lwacSettings
    TARGET = wacWidgetManagerTest
}
TARGET = wacWidgetManagerTest
!symbian:INCLUDEPATH += $$PWD/../platform/qt

INCLUDEPATH += $$CWRT_HOME/tests/helpers
