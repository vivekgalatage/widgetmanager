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
#include <QtGui>
#include "messagesender.h"
#include "window.h"
#include <QString>
#include <QCoreApplication>



Window::Window(QWidget *parent)
    : QFrame(parent)
{
    // labels
    widgetIDLabel = new QLabel(tr("Wgt id (ex: com.nokia.widget.myWgt)"));
    messageLabel = new QLabel(tr("Message"));
    // buttons
    sendButton = new QPushButton(tr("&Send"));
    connect(sendButton, SIGNAL(clicked()), this, SLOT(send()));    
    exitButton = new QPushButton(tr("&Exit"));
    connect(exitButton, SIGNAL(clicked()), QCoreApplication::instance(), SLOT(quit()));
    // text edits
    widgetIDLineEdit = new QLineEdit();
    messageLineEdit = new QLineEdit();
    sender=new MessageSender();
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(widgetIDLabel, 0, 0);
    mainLayout->addWidget(widgetIDLineEdit, 1, 0);
    mainLayout->setRowMinimumHeight(2, 10);
    mainLayout->addWidget(messageLabel, 3, 0);
    mainLayout->addWidget(messageLineEdit, 4, 0);
    mainLayout->setRowMinimumHeight(5, 10);
    mainLayout->addWidget(sendButton, 6, 0);
    mainLayout->addWidget(exitButton, 7, 0);
    
    setLayout(mainLayout);
    setWindowTitle(tr("Test1 FG w/ Param"));
  
    widgetIDLabel->setFocus();  
}

void Window::send()
{
	   sender->sendMessage(widgetIDLineEdit->text(), "open", messageLineEdit->text());
}





