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

#include <QApplication>
#include <QDebug>


#include "messagesender.h"
#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    

    if (argc > 1) {
    	MessageSender sender;
        if ((argc < 3) || (argc > 4)) {
            qDebug() << "Usage: " << argv[0] << " [appID] [open or close] \"arguments\" ";
            qDebug() << "Please enclose arguments in \" \"";
            return(0);
        }
        QString arg1;
        QString arg2;
        QString appId(argv[1]);
        
        if (argc == 3) {
            arg1 = argv[2];
            arg2 = "";
        } else {  
            arg1 = argv[2];
            arg2 = argv[3]; //Command line arguements.
        }
      sender.sendMessage(appId, arg1, arg2);      
      return 0;
    }

    window.showFullScreen();
    return app.exec();
}
