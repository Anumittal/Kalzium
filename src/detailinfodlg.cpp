/***************************************************************************
    begin                : Tue Apr 8 2003
    copyright            : (C) 2003, 2004, 2005, 2006 by Carsten Niehaus
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

#include "detailinfodlg.h"
#include "isotope.h"
#include "kalziumdataobject.h"

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <khtml_part.h>
#include <dom/html_base.h>
#include <dom/html_document.h>
#include <khtmlview.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kcombobox.h>
#include <kstandardaction.h>
#include <kpagewidgetmodel.h>
#include <ktoolinvocation.h>

#include <QFile>
#include <QLabel>
#include <QImage>
#include <QLayout>
#include <QPushButton>
#include <QStackedWidget>

#include "element.h"
#include "orbitswidget.h"
#include "detailedgraphicaloverview.h"
//FIXME Readd for KDE 4.1
//X #include "spectrumviewimpl.h"
#include "kalziumutils.h"
#include "kalziumtabletype.h"

DetailedInfoDlg::DetailedInfoDlg( int el , QWidget *parent )
    : KPageDialog( parent ), m_ktt( 0 )
{
	setFaceType( List );
	setButtons( Help | User1 | User2 | Close );
	setDefaultButton( Close );
	setButtonGuiItem( User1, KGuiItem( i18nc( "Next element", "Next" ), "arrow-right", i18n( "Goes to the next element" ) ) );
	setButtonGuiItem( User2, KGuiItem( i18nc( "Previous element", "Previous" ), "arrow-left", i18n( "Goes to the previous element" ) ) );

	m_baseHtml = KGlobal::dirs()->findResourceDir( "appdata", "data/" ) + "data/htmlview/";
	m_baseHtml2 = KGlobal::dirs()->findResourceDir( "appdata", "data/" ) + "data/hazardsymbols/";

//X 	m_picsdir = KGlobal::dirs()->findResourceDir( "appdata", "elempics/" ) + "elempics/";

	//actionButton( Close )->setFocus();

	// creating the tabs but not the contents, as that will be done when
	// setting the element
	createContent();

	m_actionCollection = new KActionCollection(this);	
	KStandardAction::quit(this, SLOT(close()), m_actionCollection);

	connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotUser1() ) );
	connect( this, SIGNAL( user2Clicked() ), this, SLOT( slotUser2() ) );
	connect( this, SIGNAL( helpClicked() ), this, SLOT( slotHelp() ) );

	// setting the element and updating the whole dialog
	setElement( el );
}

void DetailedInfoDlg::setElement( int el )
{
	Element *element = KalziumDataObject::instance()->element( el );
	if ( !element ) return;
     
	m_element = element;
	m_elementNumber = el;

        emit elementChanged( m_elementNumber );
	
	reloadContent();

	enableButton( User1, true );
	enableButton( User2, true );
	if ( m_elementNumber == 1 )
		enableButton( User2, false );
	else if ( m_elementNumber == KalziumDataObject::instance()->numberOfElements() )
		enableButton( User1, false );
}

void DetailedInfoDlg::setOverviewBackgroundColor( const QColor &bgColor )
{
	dTab->setBackgroundColor( bgColor );
}

void DetailedInfoDlg::setTableType( KalziumTableType* ktt )
{
	m_ktt = ktt;
}

KHTMLPart* DetailedInfoDlg::addHTMLTab( const QString& title, const QString& icontext, const QString& iconname )
{
	QWidget* frame = new QWidget();
	KPageWidgetItem *item = addPage( frame, title );
	item->setHeader( icontext );
	item->setIcon( KIcon( iconname ) );
	QVBoxLayout *layout = new QVBoxLayout( frame );
	layout->setMargin( 0 );
	KHTMLPart *w = new KHTMLPart( frame, frame );
    layout->addWidget( w->view() );

	return w;
}

void DetailedInfoDlg::fillHTMLTab( KHTMLPart* htmlpart, const QString& htmlcode )
{
	if ( !htmlpart ) return;

	htmlpart->begin();
	htmlpart->write( htmlcode );

    // set the background color of the document to match that of the dialog
    DOM::HTMLElement element = htmlpart->htmlDocument().body();
    if ( element.tagName() == "body" )
    {
        const QColor backgroundColor = palette().background().color();
	    ((DOM::HTMLBodyElement)element).setBgColor( backgroundColor.name() );
    }

    htmlpart->end();
}

QString DetailedInfoDlg::getHtml( DATATYPE type )
{

    QString html =
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"
        "<html><head><title>Chemical data</title>"
        "<link rel=\"stylesheet\" type=\"text/css\" href=\"" + m_baseHtml + "style.css\" />"
        "<base href=\"" + m_baseHtml + "\"/></head><body>"
        "<div class=\"chemdata\"><div><table summary=\"header\">"
        "<tr><td>" + m_element->dataAsString( ChemicalDataObject::symbol ) + "<td><td>"
        + i18n( "Block: %1", m_element->dataAsString( ChemicalDataObject::periodTableBlock ) ) +
        "</td></tr></table></div>"
        "<table summary=\"characteristics\" class=\"characterstics\">";

    //        kDebug() << beautifyOrbitalString(m_element->dataAsString( ChemicalDataObject::electronicConfiguration ) );
    QString newOrbits = beautifyOrbitalString(m_element->dataAsString( ChemicalDataObject::electronicConfiguration ) );

    kDebug() << "in getHtml: " << newOrbits;

    switch ( type )
    {
        kDebug() << "im switch";
        case CHEMICAL:
        {
            kDebug() << "im chemical switch";
            //Electronic configuration
            html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Eyylectronic configuration: %1", newOrbits ) );
            html.append( "</td></tr>" );

            //Density
            html.append( "<tr><td><img src=\"density.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Density: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::density ) ) );
            html.append( "</td></tr>" );
            // covalent radius
            html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Covalent Radius: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::radiusCovalent ) ) );
            html.append( "</td></tr>" );
            // van der Waals radius
            html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "van der Waals Radius: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::radiusVDW ) ) );
            html.append( "</td></tr>" );
            html.append( "<tr><td stype=\"text-align:center\"><img src=\"mass.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Mass: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::mass ) ) );
            html.append( "</td></tr>" );
            break;
        }
        case MISC:
        {
            // discovery date and discoverers
            html.append( "<tr><td><img src=\"discovery.png\" alt=\"icon\"/></td><td>" );
            html += KalziumUtils::prettyUnit( m_element, ChemicalDataObject::date );
            QString discoverers = m_element->dataAsString( ChemicalDataObject::discoverers );
            if ( !discoverers.isEmpty() )
            {
                discoverers = discoverers.replace( ";", ", " );
                html += "<br />" + i18n( "It was discovered by %1.", discoverers );
            }
            html.append( "</td></tr>" );
            html.append( "<tr><td><img src=\"mass.png\" alt=\"icon\"/></td><td>" );
            double mass = m_element->dataAsVariant( ChemicalDataObject::mass ).toDouble();
            double neutrons = m_element->dataAsVariant( ChemicalDataObject::atomicNumber ).toInt();
            html.append( i18n( "Mean mass: %1 u", mass/neutrons) );
            html.append( "</td></tr>" );
            // origin of the name
            QString nameorigin = m_element->dataAsString( ChemicalDataObject::nameOrigin );
            if ( !nameorigin.isEmpty() )
            {
                html.append( "<tr><td><img src=\"book.png\" alt=\"icon\"/></td><td>" );
                html.append( i18n( "Origin of the name:<br/>%1", nameorigin ) );
                html.append( "</td></tr>" );
            }
            //X 			if ( m_element->artificial() || m_element->radioactive() )
            //X 			{
            //X 				html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
            //X 				if ( !m_element->radioactive() )
            //X 					html.append( i18n( "This element is artificial" ));
            //X 				else if ( !m_element->artificial() )
            //X 					html.append( i18n( "This element is radioactive" ));
            //X 				else
            //X 					html.append( i18n( "This element is radioactive and artificial" ));
            //X 				html.append( "</td></tr>" );
            //X 			}
            break;
        }
        case ENERGY:
        {
            // melting point
            html.append( "<tr><td><img src=\"meltingpoint.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Melting Point: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::meltingpoint ) ) );
            html.append( "</td></tr>" );
            // boiling point
            html.append( "<tr><td><img src=\"boilingpoint.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Boiling Point: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::boilingpoint ) ) );
            html.append( "</td></tr>" );
            // electro negativity
            html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Electronegativity: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::electronegativityPauling ) ) );
            html.append( "</td></tr>" );
            // electro affinity
            html.append( "<tr><td><img src=\"electronaffinity.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Electron Affinity: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::electronAffinity ) ) );
            html.append( "</td></tr>" );
            // 1st ionization energy
            html.append( "<tr><td><img src=\"ionisation.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "First Ionization energy: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::ionization ) ) );
            html.append( "</td></tr>" );
            break;
        }
        case ISOTOPES:
        {
            html.append( "<tr><td>" );
            html.append( isotopeTable() );
            html.append( "</td></tr>" );
            break;
        }
        case DATA:
        {
            html.append( i18n( "<tr><th colspan=\"2\" align=\"left\">Compound properties</th></tr>" ) );
            //Compound Data
            // melting point
            html.append( "<tr><td><img src=\"meltingpoint.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Melting Point: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::meltingpoint ) ) );
            html.append( "</td></tr>" );

            // boiling point
            html.append( "<tr><td><img src=\"boilingpoint.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Boiling Point: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::boilingpoint ) ) );
            html.append( "</td></tr>" );

            html.append( "</table>" );
            html.append( "<table summary=\"characteristics\" class=\"characterstics\">" );

            //Atomic Data
            html.append( i18n( "<tr><th colspan=\"2\" align=\"left\">Atomic properties</th></tr>" ) );
            // electro affinity
            html.append( "<tr><td><img src=\"electronaffinity.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Electron Affinity: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::electronAffinity ) ) );
            html.append( "</td></tr>" );

            //Density
            html.append( "<tr><td><img src=\"density.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Density: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::density ) ) );
            html.append( "</td></tr>" );

            //Electronic configuration
            html.append( "<tr><td><img src=\"structure.png\" alt=\"icon\"/></td><td>" );
            //Probably beautify here...
            QString config = beautifyOrbitalString(m_element->dataAsString( ChemicalDataObject::electronicConfiguration ) );
            html.append( i18n( "Electronic configuration: %1", config ) );
            html.append( "</td></tr>" );

            // covalent radius
            html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Covalent Radius: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::radiusCovalent ) ) );
            html.append( "</td></tr>" );

            // van der Waals radius
            html.append( "<tr><td><img src=\"radius.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "van der Waals Radius: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::radiusVDW ) ) );
            html.append( "</td></tr>" );
            html.append( "<tr><td stype=\"text-align:center\"><img src=\"mass.png\" alt=\"icon\"/></td><td>" );
            html.append( i18n( "Mass: %1", KalziumUtils::prettyUnit( m_element, ChemicalDataObject::mass ) ) );
            html.append( "</td></tr>" );
        }
    }

    html += "</table></div></body></html>";

    return html;
}

QString DetailedInfoDlg::isotopeTable() const
{
    QList<Isotope*> list = KalziumDataObject::instance()->isotopes( m_elementNumber );

    QString html;

    html = "<table class=\"isotopes\" cellspacing=\"0\"><tr><td colspan=\"7\">";
    html += i18n( "Isotope-Table" );
    html += "</tr></td><tr><td><b>";
    html += i18n( "Mass" );
    html += "</b></td><td><b>";
    html += i18n( "Neutrons" );
    html += "</b></td><td><b>";
    html += i18n( "Percentage" );
    html += "</b></td><td><b>";
    html += i18n( "Half-life period" );
    html += "</b></td><td><b>";
    html += i18n( "Energy and Mode of Decay" );
    html += "</b></td><td><b>";
    html += i18n( "Spin and Parity" );
    html += "</b></td><td><b>";
    html += i18n( "Magnetic Moment" );
    html += "</b></td></tr>";

    foreach( Isotope * isotope , list )
    {
        html.append( "<tr><td align=\"right\">" ); 
        if ( isotope->mass() > 0.0 )
            html.append( i18n( "%1 u", isotope->mass() ) );
        html.append( "</td><td>" );
        html.append( QString::number( (( isotope )->nucleons()-( isotope )->parentElementNumber()) ) );
        html.append( "</td><td>" );
        //		if ( ( isotope )->abundance() > 0.0 )
        if ( !( isotope )->abundance().isEmpty() )
            html.append( i18nc( "this can for example be '24%'", "%1%", ( isotope )->abundance() ) );
        html.append( "</td><td>" );
        // 		if ( ( isotope )->halflife() > 0.0 )
        //FIXME: There's something (an object) missing...
        //			html.append( i18n( "%1 %2", ( isotope )->halflife(), ChemicalDataObject::unitAsString( )) );
        html.append( "</td><td>" );
        if ( ( isotope )->alphalikeliness() > 0.0){
            if ( ( isotope )->alphadecay() > 0.0 )
                html.append( i18n( "%1 MeV", ( isotope )->alphadecay() ));
            html.append( i18n( " %1",  QChar( 945 ) ));
            if ( ( isotope )->alphalikeliness() < 100.0)
                html.append( i18n( "(%1%)", ( isotope )->alphalikeliness()));
            if ( ( isotope )->betaminuslikeliness() > 0.0 || ( isotope )->betapluslikeliness() > 0.0 || ( isotope )->eclikeliness() > 0.0)
                html.append( i18n( ", " ) );
        }
        if ( ( isotope )->betaminuslikeliness() > 0.0){
            if ( ( isotope )->betaminusdecay() > 0.0 )
                html.append( i18n( "%1 MeV", ( isotope )->betaminusdecay() ));
            html.append( i18n( " %1<sup>-</sup>", QChar( 946 )  ));
            if ( ( isotope )->betaminuslikeliness() < 100.0)
                html.append( i18n( "(%1%)", ( isotope )->betaminuslikeliness() ));

            if ( ( isotope )->betapluslikeliness() > 0.0 || ( isotope )->eclikeliness() > 0.0 )
                html.append( i18n( ", " ) );
        }
        if ( ( isotope )->betapluslikeliness() > 0.0)	{
            if ( ( isotope )->betaplusdecay() > 0.0 )
                html.append( i18n( "%1 MeV", ( isotope )->betaplusdecay() ));
            html.append( i18n( " %1<sup>+</sup>", QChar( 946 ) ));
            if ( ( isotope )->betapluslikeliness() == ( isotope )->eclikeliness() ) {
                if ( ( isotope )->ecdecay() > 0.0 ) {
                    html.append( i18n( "%1 MeV", ( isotope )->ecdecay() )); }
                html.append( i18nc( "Acronym of Electron Capture"," EC" ) ); 
            }
            if ( ( isotope )->betapluslikeliness() < 100.0)	
                html.append( i18n( "(%1%)", ( isotope )->betapluslikeliness() )); 
            html += ' ';
        }
        if ( ( isotope )->eclikeliness() > 0.0 ){
            if ( ( isotope )->ecdecay() > 0.0 )
                html.append( i18n( "%1 MeV", ( isotope )->ecdecay()  ));
            html.append( i18nc( "Acronym of Electron Capture"," EC" ) );
            if ( ( isotope )->eclikeliness() < 100.0 )
                html.append( i18n( "(%1%)", ( isotope )->eclikeliness() ));
        }
        html.append( "</td><td>" );
        html.append( ( isotope )->spin() );
        html.append( "</td><td>" );
        if ( !( isotope )->magmoment().isEmpty() )
            html.append( i18n( "%1 %2<sub>n</sub>", ( isotope )->magmoment(), QChar( 956 ) ) );
        html.append( "</td></tr>" );

    }

    html += "</table>";

    return html;
}

void DetailedInfoDlg::createContent()
{
	KPageWidgetItem *item = 0;

	// overview tab
	QWidget *m_pOverviewTab = new QWidget();
	item = addPage( m_pOverviewTab, i18n( "Overview" ) );
	item->setHeader( i18n( "Overview" ) );
	item->setIcon( KIcon( "overview" ) );
	QVBoxLayout *overviewLayout = new QVBoxLayout( m_pOverviewTab );
	overviewLayout->setMargin( 0 );
	dTab = new DetailedGraphicalOverview( m_pOverviewTab );
	dTab->setObjectName( "DetailedGraphicalOverview" );
	overviewLayout->addWidget( dTab );

//X 	// picture tab
//X 	QWidget *m_pPictureTab = new QWidget();
//X 	item = addPage( m_pPictureTab, i18n( "Picture" ) );
//X 	item->setHeader( i18n( "What does this element look like?" ) );
//X 	item->setIcon( KIcon( "elempic" ) );
//X 	QVBoxLayout *mainLayout = new QVBoxLayout( m_pPictureTab );
//X 	mainLayout->setMargin( 0 );
//X 	piclabel = new QLabel( m_pPictureTab );
//X 	piclabel->setMinimumSize( 400, 350 );
//X 	mainLayout->addWidget( piclabel );

	// atomic model tab
	QWidget *m_pModelTab = new QWidget();
	item = addPage( m_pModelTab, i18n( "Atom Model" ) );
	item->setHeader( i18n( "Atom Model" ) );
	item->setIcon( KIcon( "orbits" ) );
	QVBoxLayout *modelLayout = new QVBoxLayout( m_pModelTab );
	modelLayout->setMargin( 0 );
	wOrbits = new OrbitsWidget( m_pModelTab );
	modelLayout->addWidget( wOrbits );

	// html tabs
//X	m_htmlpages["chemical"] = addHTMLTab( i18n( "Chemical Data" ), i18n( "Chemical Data" ), "chemical" );
//X	m_htmlpages["energies"] = addHTMLTab( i18n( "Energies" ), i18n( "Energy Information" ), "energies" );
	m_htmlpages["misc"] = addHTMLTab( i18n( "Miscellaneous" ), i18n( "Miscellaneous" ), "misc" );
	m_htmlpages["isotopes"] = addHTMLTab( i18n( "Isotopes" ), i18n( "Isotopes" ), "isotopes" );
//X	m_htmlpages["warnings"] = addHTMLTab( i18n( "Warnings" ), i18n( "Warnings" ), "warnings" );
	m_htmlpages["new"] = addHTMLTab( i18n( "Data Overview" ), i18n( "Data Overview" ), "data" );

	// spectrum widget tab
//X 	QWidget *m_pSpectrumTab = new QWidget();
//X 	item = addPage( m_pSpectrumTab, i18n( "Spectrum" ) );
//X 	item->setHeader( i18n( "Spectrum" ) );
//X 	item->setIcon( KIcon( "spectrum" ) );
//X 	QVBoxLayout *spectrumLayout = new QVBoxLayout( m_pSpectrumTab );
//X 	spectrumLayout->setMargin( 0 );
//X 	m_spectrumStack = new QStackedWidget( m_pSpectrumTab );
//X 	spectrumLayout->addWidget( m_spectrumStack );
//X 	m_spectrumview = new SpectrumViewImpl( m_spectrumStack );
//X 	m_spectrumview->setObjectName( "spectrumwidget" );
//X 	m_spectrumStack->addWidget( m_spectrumview );
//X 	m_spectrumLabel = new QLabel( m_spectrumStack );
//X 	m_spectrumStack->addWidget( m_spectrumLabel );
}

void DetailedInfoDlg::reloadContent()
{
    // reading the most common data
    const QString element_name = m_element->dataAsString( ChemicalDataObject::name );
    const QString element_symbol = m_element->dataAsString( ChemicalDataObject::symbol );

    // updating caption
    setCaption( i18nc( "For example Carbon (6)" , "%1 (%2)", element_name, m_elementNumber ) );

    // updating overview tab
    dTab->setElement( m_elementNumber );

    //X 	// updating picture tab
    //X 	QString picpath = m_picsdir + element_symbol + ".jpg";
    //X 	if ( QFile::exists( picpath ) )
    //X 	{
    //X 		QImage img( picpath, "JPEG" );
    //X 		img = img.scaled( 400, 400, Qt::KeepAspectRatio );
    //X 		piclabel->setPixmap( QPixmap::fromImage( img ) );
    //X 	}
    //X 	else 
    //X 		piclabel->setText( i18n( "No picture of %1 found.", element_name ) );

    // updating atomic model tab
    wOrbits->setElementNumber( m_elementNumber );
    /*
       wOrbits->setWhatsThis(
       i18n( "Here you can see the atomic hull of %1. %2 has the configuration %3." )
       .arg( m_element->dataAsString( ChemicalDataObject::name ) )
       .arg( m_element->dataAsString( ChemicalDataObject::name ) )
       .arg( "" ));//m_element->parsedOrbits() ) );
       */

    // updating html tabs
    //	fillHTMLTab( m_htmlpages["chemical"], getHtml( CHEMICAL ) );
    //	fillHTMLTab( m_htmlpages["energies"], getHtml( ENERGY ) );
    fillHTMLTab( m_htmlpages["new"], getHtml( DATA ) );
    fillHTMLTab( m_htmlpages["misc"], getHtml( MISC ) );
    fillHTMLTab( m_htmlpages["isotopes"], getHtml( ISOTOPES ) );

//X     Spectrum*s = new Spectrum();
//X     s->addPeak( new Spectrum::peak(400,300) );
//X     s->addPeak( new Spectrum::peak(450,500) );
//X     s->addPeak( new Spectrum::peak(455,500) );
//X     s->addPeak( new Spectrum::peak(470,100) );
//X     s->addPeak( new Spectrum::peak(490,50) );
//X     s->addPeak( new Spectrum::peak(500,600) );
//X     s->addPeak( new Spectrum::peak(550,700) );
//X     s->adjustIntensities();
//X 
//X     m_element->setSpectrum( s );
//X 
//X     // updating spectrum widget
//X     if ( m_element->spectrum() )
//X     {
//X         m_spectrumview->setSpectrum( m_element->spectrum() );
//X     }
//X     else
//X     {
//X         m_spectrumLabel->setText( i18n( "No spectrum of %1 found.", element_name ) );
//X         m_spectrumStack->setCurrentWidget( m_spectrumLabel );
//X     }
}

void DetailedInfoDlg::slotHelp()
{
    //TODO fix this stuff...
#if 0
	QString chapter = "infodialog_overview";
	switch ( activePageIndex() )
	{
		case 0:
			chapter = "infodialog_overview";
			break;
		case 1:
			 chapter = "infodialog_orbits";
			break;
		case 2:
			 chapter = "infodialog_chemical";
			break;
		case 3:
			 chapter = "infodialog_energies";
			break;
		case 4:
			 chapter = "infodialog_misc";
			break;
		case 5:
			 chapter = "infodialog_spectrum";
			break;
		case 6:
			 chapter = "infodialog_warnings";
			break;
	}
#endif
	KToolInvocation::invokeHelp( "infodialog_spectrum", QLatin1String( "kalzium" ) );
}

void DetailedInfoDlg::slotUser1()
{
    setElement( m_ktt->nextOf( m_elementNumber ) );
}

QString DetailedInfoDlg::beautifyOrbitalString(const QString& orbits)
{
    kDebug() << "beautifyOrbitalString";
    QString newOrbit = orbits;
    kDebug() << "foo     was:" << newOrbit;

    QRegExp reg( "(.*)(f|s|d|f)(\\d+)(.*)" );

    bool superindexesLeft = newOrbit.contains( reg );
    while (superindexesLeft) {
        if (superindexesLeft) 
            kDebug() << "there are indexes left";

        newOrbit = newOrbit.replace( reg, "\\1\\2<sup>\\3</sup>\\4" );
        kDebug() << "newOrbit is: " << newOrbit;
    
        superindexesLeft = newOrbit.contains( reg );
    }

    return newOrbit;
}

void DetailedInfoDlg::slotUser2()
{
    setElement( m_ktt->previousOf( m_elementNumber ) );
}

#include "detailinfodlg.moc"
