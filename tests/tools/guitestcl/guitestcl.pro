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

TEMPLATE = app
TARGET = guitestcl

INCLUDEPATH +=  $$PWD $$MW_LAYER_SYSTEMINCLUDE $$APP_LAYER_SYSTEMINCLUDE

HEADERS += $$PWD/window.h $$PWD/messagesender.h
           

SOURCES += $$PWD/main.cpp $$PWD/window.cpp  $$PWD/messagesender.cpp

TARGET.UID2 = 0
TARGET.UID3 = 0x200267f2

TARGET.CAPABILITY = All -Tcb
TARGET.VID = 0x101FB657

LIBS += \
    -lestor \
    -lapgrfx \
    -lbafl \
    -lapparc \
    -lws32 \
    -lwacWidgetUtils
    
contains ( what, ninetwo ){
     LIBS += -lwidgetregistryclient
}
    
contains ( what, tenone ) {
     DEFINES += QTWRT_USE_USIF
     LIBS +=   -lscrclient
}    
    
INCLUDEPATH +=  $$PWD/../../../src $$PWD/../../../src/W3CXmlParser   