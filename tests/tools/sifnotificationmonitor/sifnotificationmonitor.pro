TEMPLATE = app
TARGET = sifnotificationmonitor

INCLUDEPATH += $$PWD $$MW_LAYER_SYSTEMINCLUDE $$APP_LAYER_SYSTEMINCLUDE

LIBS += -lsif -lsifnotification

SOURCES += sifnotificationmonitor.cpp main.cpp

TARGET.CAPABILITY = All -Tcb
