/***************************************************************************
 *   Copyright (C) 2003, 2004, 2005 by Carsten Niehaus                     *
 *   cniehaus@kde.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "element.h"
#include <kdebug.h>
#include <kunitconversion/converter.h>


Element::Element()
{
}

QVariant Element::dataAsVariant( ChemicalDataObject::BlueObelisk type ) const
{
	foreach( const ChemicalDataObject &o, dataList ) {
		if ( o.type() == type )
			return o.value();
	}
	return QVariant();
}

QVariant Element::dataAsVariant(ChemicalDataObject::BlueObelisk type, int unit) const
{
	foreach( const ChemicalDataObject &o, dataList ) {
		if ( o.type() == type ) {
			KUnitConversion::Value data( o.value().toDouble(), o.unit() );
			return QVariant( data.convertTo(unit).number() );
		}
	}
	return QString();
}

QString Element::dataAsString(ChemicalDataObject::BlueObelisk type) const
{
        return dataAsVariant(type).toString();
}

QString Element::dataAsString(ChemicalDataObject::BlueObelisk type, int unit) const
{
        return dataAsVariant(type, unit).toString();
}

QString Element::dataAsStringWithUnit(ChemicalDataObject::BlueObelisk type, int unit) const
{
       QString valueAndUnit( QString::number( dataAsVariant( type, unit).toDouble(), 'g', 4 ) );

       if(valueAndUnit.isEmpty()) 
           return QString();

       valueAndUnit.append(" ");
       valueAndUnit.append( KUnitConversion::Converter().unit( unit ).data()->symbol() );
       return valueAndUnit;
}

Element::~Element()
{
}

void Element::addData( const ChemicalDataObject& o )
{
	dataList.append( o );
}

void Element::addData( const QVariant& value, ChemicalDataObject::BlueObelisk type )
{
	ChemicalDataObject tmp( value, type );
	dataList.append( tmp );
}
