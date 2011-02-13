/***************************************************************************
 *   Copyright (C) 2005-2006 by Pino Toscano, toscano.pino@tiscali.it      *
 *   Copyright (C) 2003-2006 by Carsten Niehaus, cniehaus@kde.org          *
 *   Copyright (C) 2007-2009 by Marcus D. Hanwell                          *
 *   Copyright (C) 2010 by Etienne Rebetez, etienne.rebetez@oberwallis.ch  *
 *                                                                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef PERIODSYSTEMBASE_H
#define PERIODSYSTEMBASE_H

#include <QGraphicsView>
#include <QtCore/QStateMachine>
#include <QtCore/QParallelAnimationGroup>

#include "kalziumdataobject.h"
#include "kalziumelementproperty.h"
#include "kalziumnumerationtype.h"

#include "elementitem.h"
#include "numerationitem.h"
#include "periodictablescene_p.h"


/**
 * @class periodSystem
 * In this class the periodic table of elements is created.
 * All elements, states, numeration item and animations are defined.
 * It provides slots to change the tables and accessing the element propertys
 * @short Base class and creation for the pse System
 * @author Carsten Niehaus
 * @author Marcus D. Hanwell
 * @author Etienne Rebetez
 *
*/

class periodSystem : public QGraphicsView
{
    Q_OBJECT
public:
    periodSystem(KalziumElementProperty *elProperty, QWidget *parent = 0);
    ~periodSystem();

    /**
     * Returns the qgraphicsscene
     */
    PeriodicTableScene *pseScene() const;

    /**
     * Returns the current id of the pse-table.
     * The id is the same as the menü and the pse list from the pseTables class.
     */
    int table() const;

    /**
     * Genarates and saves the pse as svg in the given filename.
     * @param filename filename of the destination.
     */
    void generateSvg(const QString& filename);


Q_SIGNALS:
    /**
     * Is emited when the pse table is changed.
     */
    void tableChanged(int tableTyp);
    /**
     * Is emited when the numeration of the pse table is changed.
     */
    void numerationChange(int num);

public slots:
    /**
     * Change the pse table to the given id.
     * @param newtable id of the pse table.
     */
    void slotChangeTable(int newtable);
    /**
     * fits the pse in the qGraphicsView.
     */
    void fitPseInView();
    /**
     * Elements can be selected with this function.
     * The selection is only a graphical feedback for the user.
     * @param element number of the element.
     */
    void slotSelectElement(int element);
    /**
     * Unselects all elements
     */
    void slotUnSelectElements();

private:
    void setupStatesAndAnimation();
    void setBiggerSceneRect();

    void createNumerationItems();
    void createElementItems();

    /// Adding position of Nummeration for each tabletyp
    void setNumerationItemPositions(int tableIndex);
    void hideAllNumerationItems(int tableIndex);
    int maxNumerationItemXCoordinate(int tableIndex);

    void addElementAnimation(QGraphicsObject *object, int factor = 1);

    void setElementItemPositions(int tableIndex);


    int m_width, m_height;

    int m_currentTableInex;

    QPoint m_hiddenPoint;

    QStateMachine m_states;
    QParallelAnimationGroup *m_group;

    QList<NumerationItem *> m_numerationItemList;

    QList<QState *> m_tableStatesList;

    PeriodicTableScene *m_tableScene;

protected:
    /**
     * Generic event handler, currently defaults to calling parent class
     * (included for future compatibility)
     */
    bool event(QEvent *e);

    /**
     * is caled every time the view is resized.
     */
    void resizeEvent ( QResizeEvent * event );
};
#endif // PERIODSYSTEMBASE_H
