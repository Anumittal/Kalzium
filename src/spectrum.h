#ifndef SPECTRUM_H
#define SPECTRUM_H
/***************************************************************************
 *   Copyright (C) 2005 by Carsten Niehaus                                 *
 *   cniehaus@kde.org                                                      *
 *   
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <qwidget.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qimage.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qspinbox.h>

#include <kimageeffect.h>
#include <kdebug.h>
#include <kpixmapeffect.h>
#include <kcombobox.h>

#define MAXCOLOR = 750
#define MINCOLOR = 450

class SpectrumWidget : public QWidget
{
	Q_OBJECT

	public:
		SpectrumWidget( QWidget *parent, const char* name = 0 );
		~SpectrumWidget();

		void setSpectra( QValueList<double> l ){
			m_spectra = l;
			update();
		}

		/**
		 * This limits the width of the spectrum in terms of
		 * wavelength. For example you can set it to only
		 * show the area between 500 and 550 nm
		 *
		 * @param left the left border
		 * @param right the right border
		 */
		void setBorders( double left, double right ){
			startValue = left;
			endValue = right;
		}
		
		/**
		 * find the nearest band. The returnvalue is the number
		 * of pixel the next band is away
		 */
		int findNearestBand( QValueList<double>::iterator it );

		/**
		 * there are several possible types.
		 */
		enum SpectrumType
		{
			EmissionSpectrum = 0,
			AbsorptionSpectrum
		};

		/**
		 * sets the type of the spectrum to @p t
		 * @param t the type of the spectrum
		 */
		void setType( SpectrumType t ){
			m_type = t;
		}
	
	private:
		QValueList<double> m_spectra;

		SpectrumType m_type;

		/**
		 * @return the adjusted value of the @p color. The
		 * correction depends on @p factor which has been
		 * figured out emperically
		 */
		int Adjust( double color, double factor );

		/**
		 * @param the position on a 0 to 1-scale in the widget. 0.5 mean 
		 * that you want the wavelength in the middle of the widget,
		 * 0.25 mean at one quarter of the width of the widget
		 *
		 * @param position the position on a 0 to 1 scale.
		 * @return the Wavelength on @p position
		 */
		int Wavelength( double position );

		/**
		 * This method changes the three values @p r @p g and @p b to the 
		 * correct values
		 * param wavelength the wavelength for which the color is searched
		 * param r red
		 * param g green 
		 * param b blue
		 */
		void wavelengthToRGB( double wavelength, int& r, int& g, int& b );

		double Gamma;
		int IntensityMax;

		/**
		 * draws the spectra-lines
		 */
		void drawAbsorptionSpectrum( QPainter *p );
		
		/**
		 * draws the spectra-lines
		 */
		void drawEmmissionSpectrum( QPainter *p );

		/**
		 * Draw the scale
		 */
		void drawTickmarks( QPainter *p );

		inline int xPos( double value );

		/**
		 * @returns the color of a line
		 * @param spectrum the value of the spectrum
		 */
		QColor linecolor( double spectrum );
	
		double startValue;
		double endValue;

		int m_realWidth;
		int m_realHeight;

		double m_stretch;

	public slots:
		/**
		 * set the the minimumvalue to @p value
		 */
		void setRightBorder( int value ){
			endValue = value;
			if ( endValue < startValue )
				startValue = endValue-1;
			update();
		}
		
		/**
		 * set the the maximumvalue to @p value
		 */
		void setLeftBorder( int value ){
			startValue = value;
			if ( startValue > endValue )
				endValue = startValue+1;
			update();
		}

		void slotActivateSpectrum( int spectrumtype ){
			m_type = ( SpectrumType )spectrumtype;
			update();
		}
	
	protected:
		virtual void paintEvent( QPaintEvent *e );
};

class SpectrumView : public QWidget
{
	Q_OBJECT

	public: 
		SpectrumView( QWidget* parent, const char* name );

		void setSpectra( QValueList<double> l ){
			m_spectrum->setSpectra( l );
			update();
		}

	private:
		SpectrumWidget *m_spectrum;

		QSpinBox *m_spinbox_left, *m_spinbox_right;

		KComboBox *m_spectrumbox;
};

#endif // SPECTRUM_H

