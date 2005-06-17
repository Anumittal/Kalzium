/***************************************************************************
    copyright            : (C) 2004 by Carsten Niehaus
    email                : cniehaus@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "plotwidget.h"

//KDE-Includes
#include <kdebug.h>

//QT-Includes
#include <qpainter.h>

PlotWidget::PlotWidget( double x1,
		double x2,
		double y1,
		double y2,
		QWidget *parent,
		const char* name)
		: KPlotWidget( x1, x2, y1, y2, parent, name )
{
	m_connectPoints = false;
}

void PlotWidget::drawObjects( QPainter *p )
{
	int p1x, p1y, //the first point
	    p2x = 0, p2y = 0; //the second point
	bool first = true;
	for ( KPlotObject *po = ObjectList.first(); po; po = ObjectList.next() ) 
	{
		if ( po->points()->count() ) 
		{
			//draw the plot object
			p->setPen( QColor( po->color() ) );

			switch ( po->type() ) 
			{
				case KPlotObject::POINTS :
					{
						p->setBrush( QColor( po->color() ) );

						for ( DPoint *dp = po->points()->first(); dp; dp = po->points()->next() ) 
						{
							QPoint q = dp->qpoint( PixRect, DataRect );
//							q += QPoint( rightPadding(), topPadding() );
							int x1 = q.x() - po->size()/2;
							int y1 = q.y() - po->size()/2;

							p->drawEllipse( x1, y1, po->size(), po->size() ); 

							if ( m_connectPoints )
							{
								p1x = q.x();
								p1y = q.y();

								if ( first )
									first = false;
								else
									p->drawLine(p1x,p1y,p2x,p2y);

								p2x = p1x;
								p2y = p1y;
							}
						}
						p->setBrush( Qt::NoBrush );
						break;
					}
				case KPlotObject::LABEL : //draw label centered at point in x, and slightly below point in y.
					{
					QPoint q = po->points()->first()->qpoint( PixRect, DataRect );
					p->drawText( q.x()-20, q.y()+6, 40, 10, Qt::AlignCenter | Qt::DontClip, po->name() );
					break;
					}
				default: ;
			}
		}
	}
}

#include "plotwidget.moc"
