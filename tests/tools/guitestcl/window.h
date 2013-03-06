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

#ifndef WINDOW_H
#define WINDOW_H

#include <QFrame>
#include <QDir>

#include <e32std.h>
#include <e32cons.h>
#include <s32mem.h>
#include <bacline.h>
#include <apgcli.h>
#include <apacmdln.h>
#include <APGTASK.H>
#include <wacWebAppRegistry.h>
#include <wacw3csettingskeys.h>
#include "messagesender.h"
#ifdef QTWRT_USE_USIF

#include <usif/scr/scr.h>
#include <usif/scr/screntries.h>
#include <usif/scr/appreginfo.h>
#include <usif/usiferror.h>
#include <usif/sif/sifutils.h>
#include "../../../src/platform/symbian/SCRConstants.h"
using namespace Usif;

#else

#include <WidgetRegistryClient.h> 

#endif  // QTWRT_USE_USIF TENONE

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QLineEdit;
QT_END_NAMESPACE


class Window : public QFrame
{
  Q_OBJECT

 public:
  Window(QWidget* parent = 0);
  

 private slots:
  void send();

 private:
  QPushButton *createButton(const QString &text, const char *member);
  QLabel* widgetIDLabel;
  QLabel* messageLabel;
  QLineEdit* widgetIDLineEdit;
  QLineEdit* messageLineEdit;
  QPushButton* sendButton;
  QPushButton* installButton;
  QPushButton* exitButton;
  MessageSender *sender;

  
};

#endif
