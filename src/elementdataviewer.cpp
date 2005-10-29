/***************************************************************************
    copyright            : (C) 2004, 2005  by Carsten Niehaus
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

#include "elementdataviewer.h"
#include "plotsetupwidget.h"
#include "plotwidget.h"
#include "kalziumdataobject.h"

#include <element.h>

#include <klocale.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kstdaction.h>
#include <ktoolinvocation.h>

//QT-Includes
#include <qevent.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVariant>

typedef QList<double> DoubleList;

/**
 * @short the values of the y-Axis
 * @author Carsten Niehaus
 */
class AxisData
{
	public:
		AxisData();
		
		/**
		 * @return the value of the selected dataset of element @p element
		 */
		double value( int element ) const;

		/**
		 * This represents the possible datasets.
		 */
		enum PAXISDATA
		{
			MASS = 0,
			MEANWEIGHT,
			DENSITY,
			EN,
			MELTINGPOINT,
			BOILINGPOINT,
			ATOMICRADIUS,
			COVALENTRADIUS
		};

		int numberOfElements() const;

		/**
		 * the dataList contains the values off all elements
		 * but only of the currently selected data type. This
		 * means that it eg contains all boiling points
		 */
		DoubleList dataList;

		int currentDataType;
};

AxisData::AxisData() : currentDataType(-1)
{
}

double AxisData::value( int element ) const
{
	if ( ( element < 0 ) || ( element >= dataList.count() ) )
		return 0.0;

	return dataList[ element-1 ];
}

int AxisData::numberOfElements() const
{
	return dataList.count();
}


ElementDataViewer::ElementDataViewer( QWidget *parent )
  : KDialogBase( Plain, i18n( "Plot Data" ), Help|User1|Close, User1, parent,
                 "plotdialog", true, false, KGuiItem( i18n( "&Plot" ) ) )
{
	d = KalziumDataObject::instance();

	yData = new AxisData();

	QHBoxLayout *layout = new QHBoxLayout(plainPage(), 0, KDialog::spacingHint() );

	m_pPlotSetupWidget = new PlotSetupWidget( plainPage(), "plotsetup" );
	m_pPlotSetupWidget->from->setMaxValue( d->numberOfElements() - 1 );
	m_pPlotSetupWidget->to->setMaxValue( d->numberOfElements() );
	m_pPlotWidget = new PlotWidget( 0.0, 12.0 ,0.0 ,22.0, plainPage() );
	m_pPlotWidget->setObjectName( "plotwidget" );
	m_pPlotWidget->setYAxisLabel(" ");
	m_pPlotWidget->setMinimumWidth( 200 );
	m_pPlotWidget->resize( 400, m_pPlotWidget->height() );

	layout->addWidget( m_pPlotSetupWidget );
	layout->addWidget( m_pPlotWidget );
	layout->setStretchFactor( m_pPlotSetupWidget, 0 );
	layout->setStretchFactor( m_pPlotWidget, 1 );

	// setup the list of names
	QList<Element*>::iterator it = d->ElementList.begin();
	const QList<Element*>::iterator itEnd = d->ElementList.end();
	for( ; it != itEnd ; ++it )
	{
		names << (*it)->dataAsString( ChemicalDataObject::name );
	}

	m_actionCollection = new KActionCollection (this );
	KStdAction::quit( this, SLOT( slotClose() ), m_actionCollection );

	connect( m_pPlotSetupWidget->KCB_y, SIGNAL( activated(int) ),
	         this, SLOT( drawPlot()) );
	connect( m_pPlotSetupWidget->connectPoints, SIGNAL( toggled( bool ) ),
	         this, SLOT( drawPlot()) );
	connect( m_pPlotSetupWidget->showNames, SIGNAL( toggled( bool ) ),
	         this, SLOT( drawPlot()) );

	// Draw the plot so that the user doesn't have to press the "Plot"
	// button to seee anything.
	drawPlot();

	resize( 500, 500 );
}

void ElementDataViewer::slotHelp()
{
	KToolInvocation::invokeHelp( "plot_data", "kalzium" );
}

// Reimplement slotUser1 from KDialogBase

void ElementDataViewer::slotUser1()
{
	kdDebug() << "slotUser1" << endl;

	drawPlot();
}


void ElementDataViewer::setLimits(int f, int t)
{
	kdDebug() << "setLimits()" << endl;
	
	double minY = yData->value(f);
	double maxY = yData->value(f);

	for ( int _currentVal = f; _currentVal < t; _currentVal++ )
	{
		double v = yData->value( _currentVal );
		
		if( minY > v )
			minY = v;
		if( maxY < v)
			maxY = v;
	}
	
	m_pPlotWidget->setLimits( (double)f, (double)t, minY, maxY );
}

void ElementDataViewer::paintEvent(QPaintEvent*)
{
	m_pPlotWidget->update();
}

void ElementDataViewer::keyPressEvent(QKeyEvent *e)
{
	switch ( e->key() )
	{
		case Qt::Key_Plus:
		case Qt::Key_Equal:
			slotZoomIn();
			break;
		case Qt::Key_Minus:
		case Qt::Key_Underscore:
			slotZoomOut();
			break;
		case Qt::Key_Escape:
			close();
			break;
	}
}

void ElementDataViewer::slotZoomIn(){}
void ElementDataViewer::slotZoomOut(){}

void ElementDataViewer::setupAxisData()
{
	DoubleList l;

	const int selectedData = m_pPlotSetupWidget->KCB_y->currentItem();

	//this should be somewhere else, eg in its own method
	yData->currentDataType = selectedData;

	QList<Element*>::iterator it = d->ElementList.begin();
	const QList<Element*>::iterator itEnd = d->ElementList.end();
	ChemicalDataObject::BlueObelisk kind;
	QString caption;
	switch(selectedData)
	{
		case AxisData::MASS:
		{
			kind = ChemicalDataObject::mass;
			caption = i18n( "Atomic Mass [u]" );
			break;
		}
//X 		case AxisData::MEANWEIGHT:
//X 		{
//X 			kind = ChemicalDataObject::foo;
//X 			caption = i18n( "Mean Mass [u]" );
//X 			break;
//X 		}
//X 		case AxisData::DENSITY:
//X 		{
//X 			kind = ChemicalDataObject::foo;
//X 			caption = i18n( "Density" );
//X 			break;
//X 		}
		case AxisData::EN:
		{
			kind = ChemicalDataObject::electronegativityPauling;
			caption = i18n( "Electronegativity" );
			break;
		}
		case AxisData::MELTINGPOINT:
		{
			kind = ChemicalDataObject::meltingpoint;
			caption = i18n( "Melting Point [K]" );
			break;
		}
		case AxisData::BOILINGPOINT:
		{
			kind = ChemicalDataObject::boilingpoint;
			caption = i18n( "Boiling Point [K]" );
			break;
		}
//X 		case AxisData::ATOMICRADIUS:
//X 		{
//X 			kind = ChemicalDataObject::foo;
//X 			caption = i18n( "Atomic Radius [pm]" );
//X 			break;
//X 		}
		case AxisData::COVALENTRADIUS:
		{
			kind = ChemicalDataObject::radiusCovalent;
			caption = i18n( "Covalent Radius [pm]" );
			break;
		}
		default:
		{
		}
	}
	for( ; it != itEnd ; ++it )
	{
		double value = (*it)->dataAsVariant( kind ).toDouble();
		l << ( value > 0.0 ? value : 0.0 );
	}

	yData->dataList.clear();
	yData->dataList << l;
}

void ElementDataViewer::drawPlot()
{
	kdDebug() << "drawPlot()" << endl;

	/*
	 * to be 100% safe delete the old list
	 */
	m_pPlotWidget->clearObjectList();

	/*
	 * spare the next step in case everything is already set and done
	 */
	if( yData->currentDataType != m_pPlotSetupWidget->KCB_y->currentItem() )
		initData();

	/*
	 * if the user selected the elements 20 to 30 the list-values are 19 to 29!!!
	 */
	const int from = m_pPlotSetupWidget->from->value();
	const int to = m_pPlotSetupWidget->to->value();
	
	/*
	 * The number of elements. #20 to 30 are 30-20+1=11 Elements
	 */
	const int num = to-from+1;
	
	setLimits(from,to);

	/*
	 * check if the users wants to see the elementnames or not
	 */
	bool showNames = m_pPlotSetupWidget->showNames->isChecked();
	bool connectPoints = m_pPlotSetupWidget->connectPoints->isChecked();

	if ( connectPoints )
		m_pPlotWidget->setConnection( true );
	else
		m_pPlotWidget->setConnection( false );
		
	/*
	 * reserve the memory for the KPlotObjects
	 */
	QVector<KPlotObject*> dataPoint(num);
	QVector<KPlotObject*> dataPointLabel(num);

	int number = 0;

	double max = 0.0, av = 0.0;
	double min = yData->value( 1 );

	/*
	 * iterate for example from element 20 to 30 and contruct
	 * the KPlotObjects
	 */
	for( int i = from; i < to+1 ; i++ )
	{
		double v = yData->value( i );

		if ( v >= 0.0 )
		{
			if ( v < min )
				min = v;
			if ( v > max )
				max = v;
			av += v;

			dataPoint[number] = new KPlotObject( "whocares", "Blue", KPlotObject::POINTS, 4, KPlotObject::CIRCLE );
			dataPoint[number]->addPoint( new DPoint( (double)i , v ) );
			m_pPlotWidget->addObject( dataPoint[ number ] );

			if (showNames)
			{
				dataPointLabel[number] = new KPlotObject( names[i], "Red", KPlotObject::LABEL );
				dataPointLabel[number]->addPoint( new DPoint( (double)i , yData->value( i ) ) );
				m_pPlotWidget->addObject( dataPointLabel[number] );
			}
		}
		number++;
	}

	//now set the values for the min, max and avarage value
	m_pPlotSetupWidget->aValue->setText( QString::number( av/number ) );
	m_pPlotSetupWidget->minValue->setText( QString::number( min ) );
	m_pPlotSetupWidget->maxValue->setText( QString::number( max ) );
}

void ElementDataViewer::initData()
{
	setupAxisData();
}

#include "elementdataviewer.moc"
