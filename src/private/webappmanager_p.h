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


#ifndef _WIDGET_MANAGER_P_H_
#define _WIDGET_MANAGER_P_H_


/* Widget managers own widget installation and launching:
 *  Manage widget storage
 *  Own widget preferences
 *  Use widget factory at install and launch
 */

/* Uses the widget engine and package management components */
class WidgetManager_p {

public:
    explicit WidgetManager_p(QWidget *parent);
    ~WidgetManager_p();

    QWidget                *m_parent;
    QMap <QString, WidgetProperties*> m_widgetProps;
};

#endif //_WIDGET_MANAGER_P_H_
