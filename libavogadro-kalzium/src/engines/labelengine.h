/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2007 Donald Ephraim Curtis <donald-curtis@uiowa.edu>
  Copyright (C) 2007 Benoit Jacob <jacob@math.jussieu.fr>

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef __LABELENGINE_H
#define __LABELENGINE_H

#include <avogadro/global.h>
#include <avogadro/engine.h>

#include <openbabel/mol.h>

#include <QGLWidget>
#include <QObject>
#include <QStringList>
#include <QImage>

namespace Avogadro {

  //! Label Engine class.
  class LabelEngine : public Engine
  {
    Q_OBJECT

    public:
      //! Constructor
      LabelEngine(QObject *parent=0) : Engine(parent) {}
      //! Deconstructor
      ~LabelEngine() {}

      //! \name Description methods
      //@{
      //! @return engine name
      QString name() { return(QString(tr("Label"))); }
      //! @return engine description
      QString description() { return(QString(tr("Renders primitive labels"))); }
      //@}

      //! \name Render Methods
      //@{
      bool render(GLWidget *gl);
      //@}

  };

  //! Generates instances of our LabelEngine class
  class LabelEngineFactory : public QObject, public EngineFactory
  {
    Q_OBJECT
    Q_INTERFACES(Avogadro::EngineFactory)

    public:
      Engine *createInstance(QObject *parent = 0) { return new LabelEngine(parent); }
  };

} // end namespace Avogadro

#endif
