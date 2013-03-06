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

/****************************************************************
 * **
 * ** Qt tutorial 1
 * **
 * ****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <QXmlStreamReader>
#include <QFile>
#include <QXmlStreamAttributes>
#include <QVariant>
#include <QMap>
#include "w3ctags.h"

#include "wrtsettings.h"
#include "wacw3csettingskeys.h"
#include "configw3xmlparser.h"
#include "wacw3celement.h"


using namespace WAC ;

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    ConfigXmlParser xmlparser("/tmp/config.xml");

    if (xmlparser.parseFile() == false)   {
        LOG("Parse failed");
    }
    LOG(<< "version " << xmlparser.version());
    LOG("namespace" << xmlparser.namespaceUri());
    LOG("id" << xmlparser.id());
    LOG("lang" << xmlparser.language());
    LOG("height" << xmlparser.height());
    LOG("width" << xmlparser.width());
    QString lang("kan");
    const QString fi("fi");

    WAC::WrtSettings* settings;
    settings = WAC::WrtSettings::createWrtSettings();

    QString userAgentLang = settings->valueAsString("UserAgentLanguage");

    if (userAgentLang.isEmpty()) {
       QLocale language;
       userAgentLang = language.name();
       userAgentLang.replace(QString("_"),QString("-"));
    }

    const W3CElement *name = xmlparser.getElement(W3CSettingsKey::WIDGET_NAME , userAgentLang);

    if (name) {
       LOG(name->readElementText());
    }
    return 0;
}

