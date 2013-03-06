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

contains( what, tenone ) {
            DEFINES *= CWRT_BUILDING_TENONE
}

contains( what, ninetwo ) {
            DEFINES *= CWRT_BUILDING_NINETWO
}

!contains(DEFINES , BLD_WIDGET_MANAGER_TESTSUITE) : DEFINES += BLD_WIDGET_MANAGER_TESTSUITE

HEADERS += $$PWD/t_widgetmgr.h
SOURCES += $$PWD/t_widgetmgr.cpp

contains(DEFINES, QTWRT_ENABLE_DESKTOPFILE=1) {
    HEADERS += $$PWD/t_desktopfile.h
    SOURCES += $$PWD/t_desktopfile.cpp
}

INCLUDEPATH += $$PWD \
               $$PWD/../../ \
               $$PWD/../../widgetmanager/src \
               $$PWD/../../widgetmanager/src/W3CXmlParser \
               $$PWD/../../widgetcore \
               $$PWD/../../runtimecore \
               $$PWD/../../settings/inc \
               $$PWD/../../tests/api \



