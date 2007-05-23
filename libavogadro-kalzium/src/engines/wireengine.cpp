/**********************************************************************
  WireEngine - Engine for wireframe display

  Copyright (C) 2006-2007 Geoffrey R. Hutchison <geoff@geoffhutchison.net>
  Copyright (C) 2006-2007 Benoit Jacob <jacob@math.jussieu.fr>

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

#include "wireengine.h"
#include <config.h>

#include <avogadro/primitive.h>
#include <avogadro/color.h>
#include <avogadro/glwidget.h>

#include <openbabel/obiter.h>
#include <eigen/regression.h>

#include <QtPlugin>
#include <QMessageBox>

using namespace std;
using namespace OpenBabel;
using namespace Avogadro;
using namespace Eigen;

bool WireEngine::render(GLWidget *gl)
{
  m_glwidget = gl;

  QList<Primitive *> list;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);

  list = queue().primitiveList(Primitive::AtomType);
  foreach( Primitive *p, list ) {
    render(static_cast<const Atom *>(p));
  }

  list = queue().primitiveList(Primitive::BondType);
  foreach( Primitive *p, list ) {
    render(static_cast<const Bond *>(p));
  }

  glPopAttrib();

  return true;
}

bool WireEngine::render(const Atom *a)
{
  Color map = colorMap();
  const float selectionColor[3] = {0.3, 0.6, 1.0};
  glPushName(Primitive::AtomType);
  glPushName(a->GetIdx());

  const Vector3d & v = a->pos();

  if (m_glwidget->selectedItem(a)) {
    glColor3fv(selectionColor);
    glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * 4.0);
  }
  else{
    map.set(a);
    map.apply();
    glPointSize(etab.GetVdwRad(a->GetAtomicNum()) * 3.0);
  }

  glBegin(GL_POINTS);
  glVertex3d(v.x(), v.y(), v.z());
  glEnd();

  glPopName();
  glPopName();

  return true;
}

bool WireEngine::render(const Bond *b)
{
  const Atom *atom1 = static_cast<const Atom *>( b->GetBeginAtom() );
  const Atom *atom2 = static_cast<const Atom *>( b->GetEndAtom() );
  const Vector3d & v1 = atom1->pos();
  const Vector3d & v2 = atom2->pos();
  Color map = colorMap();

  glLineWidth(1.0);
  glBegin(GL_LINES);

  // hard to separate atoms from bonds in this view
  // so we let the user always select atoms
  map.set(atom1);
  map.apply();
  glVertex3d(v1.x(), v1.y(), v1.z());

  map.set(atom2);
  map.apply();
  glVertex3d(v2.x(), v2.y(), v2.z());

  glEnd();

  return true;
}

bool WireEngine::render(const Molecule *m)
{
  // Disabled
  return false;
}

#include "wireengine.moc"

Q_EXPORT_PLUGIN2(bsengine, WireEngineFactory)
