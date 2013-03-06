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

TARGET = wacRegistry

TEMPLATE = app
# prevent .rss and _reg.rss file creation
CONFIG += NO_ICON

TARGET.UID3 = 0x2003DE23
TARGET.VID = 0x101FB657

# CAPABILITY      CAP_SERVER AllFiles
TARGET.CAPABILITY = All -Tcb
TARGET.EPOCSTACKSIZE = 0x5000
TARGET.EPOCHEAPSIZE = 0x5000 0x1000000
RSS_RULES += "hidden = KAppIsHidden;"

INCLUDEPATH += \
    $$PWD/../common \
    $$PWD/../../../.. \
    /epoc32/include/libc \
    $$APP_LAYER_SYSTEMINCLUDE

LIBS += \
    -leuser \
    -lefsrv \
    -lestor \
    -lestlib \
    -lapgrfx \
    -lws32 \
    -lapparc \
    -lbafl \
    -lXmlEngineUtils \
    -lXMLEngine \
    -lXmlEngineDOM \
    -lcharconv \
    -lwacWidgetUtils

contains( what, ninetwo ) {
    LIBS += -lflogger
}

contains( what, tenone ) {
    LIBS += -lflogger
}

CONFIG(debug, debug|release): LIBS += -lflogger

HEADERS += \
    $$PWD/CwrtRegistrySession.h \
    $$PWD/CwrtRegistryServer.h \
    $$PWD/../common/CwrtRegistryData.h

SOURCES += \
    $$PWD/CwrtRegistrySession.cpp \
    $$PWD/CwrtRegistryServer.cpp \
    $$PWD/../common/CwrtRegistryData.cpp
