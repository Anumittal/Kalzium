/**********************************************************************
  LabelEngine - Engine for displaying labels.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2007 Benoit Jacob

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

#include "labelengine.h"
#include <config.h>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool LabelEngine::render(GLWidget *gl)
{
  if(gl->labelsStyle() == 0) return true;
  gl->painter()->beginText();
  QList<Primitive *> list;

  list = queue().primitiveList(Primitive::AtomType);
  
  foreach( Primitive *p, list ) {
    Atom *atom = static_cast<const Atom *>(p);
    const Vector3d pos = atom->pos();

    double renderRadius = gl->renderingEngine()->radius(atom) + 0.05;

    double zDistance = gl->camera()->distance(pos);

    if(zDistance < 50.0) {
      QString str;
      if(gl->labelsStyle() == 1) {
        str = QString::number(atom->GetIdx());
      } else if(gl->labelsStyle() == 2) {
        str = QString::fromAscii(etab.GetSymbol(atom->GetAtomicNum()));
      } else {
        str = QString::fromAscii(etab.GetName(atom->GetAtomicNum()).data());
      }
      const MatrixP3d & m = gl->camera()->modelview();

      Vector3d zAxis = gl->camera()->backtransformedZAxis();

      Vector3d drawPos = pos + zAxis * renderRadius;

      glColor3f(1.0, 1.0, 1.0);
      gl->painter()->drawText(drawPos, str);
    }
  }
  gl->painter()->endText();
}

#include "labelengine.moc"

Q_EXPORT_PLUGIN2(labelengine, LabelEngineFactory)
