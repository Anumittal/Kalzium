/***************************************************************************
                         searchdlg.cpp  -  description
                             -------------------
    begin                : June 2003
	copyright            : (C) 2003 by Carsten Niehaus                     
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

//KDE-Includes
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kfiledialog.h>

//QT-Includes
#include <qmessagebox.h>
#include <qtable.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include "searchdlg.h"
#include "csvdialogimpl.h"
#include "datafilterdialogimpl.h"

SearchDlg::SearchDlg (QWidget *parent, const char *name )  
    : SearchDialogUI (parent,name)
{
	fillDataStringLists();
	fillCheckBoxList();

	StringLists.append( nameList);
	StringLists.append(	symbolList);
	StringLists.append(	WeightList);
	StringLists.append(	ENList);
	StringLists.append(	DensityList);
	StringLists.append(	orbitList);
	StringLists.append(	blockList);
	StringLists.append(	acidList);
	StringLists.append(	groupList);
	StringLists.append(	MPList);
	StringLists.append(	BPList);
	StringLists.append(	IEList);
	StringLists.append(	ARList);
	StringLists.append(	OxidationList);
}

void SearchDlg::fillCheckBoxList()
{
	checkBoxes.append( checkBox1 );
	checkBoxes.append( checkBox2 );
	checkBoxes.append( checkBox3 );
	checkBoxes.append( checkBox4 );
	checkBoxes.append( checkBox5 );
	checkBoxes.append( checkBox6 );
	checkBoxes.append( checkBox7 );
	checkBoxes.append( checkBox8 );
	checkBoxes.append( checkBox9 );
	checkBoxes.append( checkBox10 );
	checkBoxes.append( checkBox11 );
	checkBoxes.append( checkBox12 );
	checkBoxes.append( checkBox13 );
	checkBoxes.append( checkBox14 );
}

void SearchDlg::fillDataStringLists()
{
    KSimpleConfig config (locate("data", "kalzium/kalziumrc"));
	for( int i = 0  ; i < 109 ;  ++i )
	{
		config.setGroup(QString::number(i+1));
		nameList.append( config.readEntry("Name", "Unknown") );
		symbolList.append( config.readEntry( "Symbol", "Unknown" ) );
		blockList.append( config.readEntry( "Block", "Unknown" ) );
		acidList.append( config.readEntry( "acidbeh", "Unknown" ) );
		groupList.append( config.readEntry( "Group", "Unknown" ) );
		orbitList.append( config.readEntry( "Orbits", "Unknown" ) );
		OxidationList.append( config.readEntry( "Ox", "Unknown" ) );
		ARList.append( config.readEntry( "AR", "Unknown" ) );
		ENList.append( config.readEntry( "EN", "Unknown" ) );
		BPList.append( config.readEntry( "BP", "Unknown" ) );
		MPList.append( config.readEntry( "MP", "Unknown" ) );
		IEList.append( config.readEntry( "IE", "Unknown" ) );
		WeightList.append( config.readEntry( "Weight", "Unknown" ) );
		DensityList.append( config.readEntry( "Density", "Unknown" ) );
	}
}


QString SearchDlg::beautifyOrbits( QString orbits ) const
{
	QRegExp rxs("([a-z])([0-9]+)");
	QRegExp rxb("([a-z]{2}) ",false);
	orbits.replace(rxs,"\\1<sup>\\2</sup>"); //superscript around electron number
	orbits.replace(rxb,"<b>\\1</b> "); //bold around element symbols
	return orbits;
}

void SearchDlg::slotApplyChanges()
{
	int col = 0;
	for ( checkBoxesList::Iterator it = checkBoxes.begin() ; it != checkBoxes.end() ; ++it )
	{
		//show or hide cols
		if ( (**it).isChecked() )
			DataTable->showColumn( col );
		else
			DataTable->hideColumn( col );
		col++;
	}

	int i = 0;
	
	for ( StrList::Iterator StrIt = StringLists.begin() ; StrIt != StringLists.end() ; ++StrIt )
	{
		int e = 0;
		for ( QStringList::Iterator listsIt = (*StrIt).begin() ; listsIt != (*StrIt).end() ; ++listsIt )
		{
			DataTable->setText( e , i , (*listsIt) );
			e++;
		}
		i++;
	}

	if ( !RangeLE->text().isEmpty() ) //only parse if the user entered numbers
		slotFilterData();
}

void SearchDlg::slotFilter()
{
	Datafilterdialogimpl *dfdlg = new Datafilterdialogimpl( this );
	dfdlg->show();
}

void SearchDlg::slotFilterData()
{
	bool show;
	if ( ShowHideCB->currentItem() == 0 )
		show = true;
	else show = false;

	for ( int i = 0 ; i < 110 ; ++i )
	{
		if ( show )
			DataTable->hideRow( i );
		else
			DataTable->showRow( i );
	}
	
	IntValueList l = parseRange( RangeLE->text() );
	for ( IntValueList::Iterator it = l.begin(); it != l.end() ; ++it )
	{
		if ( show )
			DataTable->showRow( *it-1 );
		else
			DataTable->hideRow( *it-1 );
	}
}

IntValueList SearchDlg::parseDashes( QString dashedString )
{
	IntValueList l;
	QRegExp rx( "-" );
	QString left = dashedString.left( dashedString.find( rx ) );
	QString right = dashedString.right( dashedString.length() - dashedString.find( rx ) - 1 );
	if ( left.toInt() > right.toInt() )
	{
		QMessageBox::information(  this, i18n( "dashedStringproblem" ),
				i18n( "The left value has to be smaller than the right value." ) );
		return l;
	}
	else //the dashedString is syntactically ok
	{
		for ( int i = left.toInt() ; i < right.toInt()+1 ; i++ )
		{
			l.append( i );
		}
	}
	return l;
}

IntValueList SearchDlg::parseRange(QString range)
{
	IntValueList l;
	
	if ( range.contains( ";" ) ) //at least to ranges are given
	{
		QRegExp semcol_rx( ";" );
		QRegExp dash_rx( "-" );

		bool hasStillSemicolons = true;
		QString part, rest, left, right;

		int bar = 1;
		while ( hasStillSemicolons )
		{
			if ( range.contains(";" ) )
			{
				part = range.left( range.find( semcol_rx ) );
				rest = range.right( range.length() - range.find( semcol_rx ) - 1 );
			}
			else
			{
				part = range;
				rest = "";
			}

			IntValueList e = parseDashes( part );
			for( IntValueList::Iterator it = e.begin() ; it != e.end() ; ++it )
			{
				l.append( *it );
			}

			range = rest;

			bar++;

			//check if we still need to parse
			if ( !range.isEmpty() )
			{
				if ( !range.contains( ";" ) )//only one element is left
				{
					l.append( range.toInt() );
					hasStillSemicolons = false;
				}
				else
					hasStillSemicolons = true;
			}
			else
			{
				hasStillSemicolons = false;
			}
		}
	}
	else
	{
		if ( range.contains( "-" ) ) //one range is given
		{
			l = parseDashes( range );
		}
		else //only one element is chosen
		{
			l.append( range.toInt() );
		}
	}

	
	return l;
}

void SearchDlg::slotExportData()
{
/*
 * Robert: Ich bekomme folgendes nicht hin:
 * Ich habe ein paar Werte ( QChar, QString und noch ein paar ).
 * Ich will diese in einem Dialog ( csvdialogimpl hei�t die Klasse )
 * manipulieren k�nnen. Meine Idee war, diese Variablen an den Konstruktor 
 * per Referenz ( also QChar& und so weiter ) zu �bergeben. Aber irgendwie
 * klappt das nicht so wie es soll.
 *
 * Frage: Wie kann ich in einem Dialog Variable der Mutterklasse ver�ndern?
 * Eigentlich geht das doch mit Referenzen aber das klappt hier nicht.
 */
	
	//int rows = DataTable->numRows();
	//int cols = DataTable->numCols();
	

//X 	for ( int i = 0 ; i < rows ; i++ )
//X 	{
//X 		if ( DataTable->rowHeight(i) == 0 )
//X 				continue;
//X 		for ( int e = 0 ; e < cols ; e++ )
//X 		{
//X 			if ( DataTable->columnWidth(e) == 0 )
//X 				continue;
//X 			str.append( DataTable->text( i , e ) );
//X 			str.append( csvDelimiter );
//X 		}
//X 		str.append( "\n" );
//X 	}
//X 
//X 	QCString cstr( str.local8Bit() );
//X 
//X 	QFile out( KFileDialog::getSaveFileName()  );
//X 	if (  !out.open( IO_WriteOnly ) )
//X 	{
//X 		out.close();
//X 		return;
//X 	}
//X 	out.writeBlock( cstr.data(), cstr.length() );
//X 
//X 	out.close();
}

#include "searchdlg.moc"
