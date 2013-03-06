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

#ifndef _WIDGET_UTILS_LOGS_H
#define _WIDGET_UTILS_LOGS_H

// enable for debug logs
#define WIDGET_UTILS_DEBUG 0


#if WIDGET_UTILS_DEBUG
#include <QDebug>
#define LOG(x) qDebug() << x
#else
#define LOG(x)
#endif

#endif // _WIDGET_UTILS_LOGS_H
