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

#ifndef CWRTREGISTRYDATA_H_
#define CWRTREGISTRYDATA_H_

#include <QString>
#include <QVariant>
#include "wacAttributeMap.h"

// Forward declaration
class RWriteStream;

/**
* Write a string to aStream
*/
void SerializeStringL( RWriteStream& aStream, const QString& aString );

/**
* Write a map to aStream
*/
void SerializeMapL( RWriteStream& aStream, const AttributeMap& aMap );

/**
* Write an integer to aStream
*/
void SerializeIntL( RWriteStream& aStream, const TInt& aInt );

/**
* Write a boolean to aStream
*/
void SerializeBoolL( RWriteStream& aStream, const TBool& aBool );

/*
* Read string from aStream
*/
QString DeserializeStringL( RReadStream& aStream );

/*
* Read string (without the "str" prefix) from aStream
*/
QString DeserializeStringWithoutPrefixL( RReadStream& aStream );

/*
* Read map from aStream
*/
void DeserializeMapL( RReadStream& aStream, AttributeMap& aMap );

/*
* Read integer from aStream
*/
int DeserializeIntL( RReadStream& aStream );

#endif /* CWRTREGISTRYDATA_H_ */
