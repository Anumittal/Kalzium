/**********************************************************************
  Primitive - Wrapper class around the OpenBabel classes

  Copyright (C) 2007 Donald Ephraim Curtis <donald-curtis@uiowa.edu>

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

#include <config.h>

#include <avogadro/primitive.h>
#include <QDebug>
#include <eigen/regression.h>
#include <openbabel/obiter.h>

using namespace OpenBabel;

namespace Avogadro {

  class PrimitivePrivate {
    public:
      PrimitivePrivate() : type(Primitive::OtherType), selected(false) {};

      enum Primitive::Type type;
      bool selected;
  };

  Primitive::Primitive(QObject *parent) : d_ptr(new PrimitivePrivate), QObject(parent) {}

  Primitive::Primitive(enum Type type, QObject *parent) : d_ptr(new PrimitivePrivate), QObject(parent)
  {
    Q_D(Primitive);
    d->type = type;
  }

  Primitive::Primitive(PrimitivePrivate &dd, QObject *parent) : d_ptr(&dd), QObject(parent) {}

  Primitive::Primitive(PrimitivePrivate &dd, enum Type type, QObject *parent) : d_ptr(&dd), QObject(parent)
  {
    Q_D(Primitive);
    d->type = type;
  }

  Primitive::~Primitive()
  {
    delete d_ptr;
  }

  bool Primitive::isSelected() const
  {
    Q_D(const Primitive);
    return d->selected;
  }

  void Primitive::setSelected( bool s ) 
  {
    Q_D(Primitive);
    d->selected = s;
  }

  void Primitive::toggleSelected()
  {
    Q_D(Primitive);
    d->selected = !d->selected;
  }

  enum Primitive::Type Primitive::type() const
  {
    Q_D(const Primitive);
    return d->type;
  }

  void Primitive::update()
  {
    emit updated();
  }

  class MoleculePrivate : public PrimitivePrivate {
    public:
      MoleculePrivate() : PrimitivePrivate(), farthestAtom(0), invalidGeomInfo(true) {}
      mutable Eigen::Vector3d       center;
      mutable Eigen::Vector3d       normalVector;
      mutable double                radius;
      mutable Atom *                farthestAtom;
      mutable bool                  invalidGeomInfo;
  };

  Molecule::Molecule(QObject *parent) : OpenBabel::OBMol(), Primitive(*new MoleculePrivate, MoleculeType, parent) 
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::Molecule(const Molecule &other) : OpenBabel::OBMol(other), Primitive(*new MoleculePrivate, MoleculeType, other.parent())
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
  }

  Atom * Molecule::CreateAtom()
  {
    Atom *atom = new Atom(this);
    connect(atom, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(atom);
    return(atom);
  }

  Bond * Molecule::CreateBond()
  {
    Bond *bond = new Bond(this);
    connect(bond, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(bond);
    return(bond);
  }

  Residue * Molecule::CreateResidue()
  {
    Residue *residue = new Residue(this);
    connect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(residue);
    return(residue);
  }

  void Molecule::DestroyAtom(OpenBabel::OBAtom *obatom)
  {
    Atom *atom = static_cast<Atom *>(obatom);
    if(atom) {
      emit primitiveRemoved(atom);
      atom->deleteLater();
    }
  }

  void Molecule::DestroyBond(OpenBabel::OBBond *obbond)
  {
    Bond *bond = static_cast<Bond *>(obbond);
    if(bond) {
      emit primitiveRemoved(bond);
      bond->deleteLater();
    }
  }

  void Molecule::DestroyResidue(OpenBabel::OBResidue *obresidue)
  {
    Residue *residue = static_cast<Residue *>(obresidue);
    if(residue) {
      emit primitiveRemoved(residue);
      residue->deleteLater();
    }
  }

  void Molecule::updatePrimitive()
  {
    Q_D(Molecule);
    Primitive *primitive = qobject_cast<Primitive *>(sender());
    d->invalidGeomInfo = true;
    emit primitiveUpdated(primitive);
  }

  void Molecule::update()
  {
    Q_D(Molecule);
    d->invalidGeomInfo = true;
    emit updated();
  }

  const Eigen::Vector3d & Molecule::center() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->center;
  }
    
  const Eigen::Vector3d & Molecule::normalVector() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->normalVector;
  }
    
  const double & Molecule::radius() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->radius;
  }
    
  const Atom * Molecule::farthestAtom() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->farthestAtom;
  }

  Molecule &Molecule::operator=(const Molecule& other)
  {
    OpenBabel::OBMol::operator=(other);
  }

  Molecule &Molecule::operator+=(const Molecule& other)
  {
    unsigned int initialAtoms = NumAtoms();
    OpenBabel::OBMol::operator+=(other);

    // select atoms we just added
    FOR_ATOMS_OF_MOL(a, this) {
      if (a->GetIdx() <= initialAtoms) // potential OB atom index issue FIXME
        continue;
      dynamic_cast<Atom *>(&*a)->setSelected(true);
    }
  }

  void Molecule::computeGeomInfo() const
  {
//     MoleculePrivate *d = reinterpret_cast<MoleculePrivate *>(d_ptr);
    Q_D(const Molecule);
    d->invalidGeomInfo = true;
    d->farthestAtom = 0;
    d->center.loadZero();
    d->normalVector.loadZero();
    d->radius = 0.0;
    if( NumAtoms() != 0 )
    {
      // compute center
      std::vector< OpenBabel::OBAtom * >::iterator atom_iterator;
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        d->center += atom->pos();
      }
      d->center /= NumAtoms();
  
      // compute the normal vector to the molecule's best-fitting plane
      Eigen::Vector3d * atomPositions = new Eigen::Vector3d[NumAtoms()];
      int i = 0;
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        atomPositions[i++] = atom->pos();
      }
      Eigen::Vector4d planeCoeffs;
      Eigen::computeFittingHyperplane( NumAtoms(), atomPositions, &planeCoeffs );
      delete[] atomPositions;
      d->normalVector = Eigen::Vector3d( planeCoeffs.x(), planeCoeffs.y(), planeCoeffs.z() );
      d->normalVector.normalize();
  
      // compute radius and the farthest atom
      d->radius = -1.0; // so that ( squaredDistanceToCenter > d->radius ) is true for at least one atom.
      for( Atom* atom = (Atom*) const_cast<Molecule*>(this)->BeginAtom(atom_iterator); atom; atom = (Atom *) const_cast<Molecule*>(this)->NextAtom(atom_iterator) )
      {
        double distanceToCenter = (atom->pos() - d->center).norm();
        if( distanceToCenter > d->radius )
        {
          d->radius = distanceToCenter;
          d->farthestAtom = atom;
        }
      }
    }
    d->invalidGeomInfo = false;
  }

  class PrimitiveQueuePrivate {
    public:
      PrimitiveQueuePrivate() {};

      QVector< QList<Primitive *> > queue;
  };

  PrimitiveQueue::PrimitiveQueue() : d(new PrimitiveQueuePrivate) { 
    d->queue.resize(Primitive::LastType);
//     for( int type=0; type<Primitive::LastType; type++ ) { 
//       d->queue.append(QList<Primitive *>); 
//     } 
  }

  PrimitiveQueue::~PrimitiveQueue() { 
//     for( int i = 0; i<d->queue.size(); i++ ) { 
//       delete d->queue[i];
//     } 
    delete d;
  }

  const QList<Primitive *>& PrimitiveQueue::primitiveList(enum Primitive::Type type) const { 
    return(d->queue[type]); 
  }

  void PrimitiveQueue::addPrimitive(Primitive *p) { 
    d->queue[p->type()].append(p); 
  }

  void PrimitiveQueue::removePrimitive(Primitive *p) {
    d->queue[p->type()].removeAll(p);
  }

  int PrimitiveQueue::size() const {
    int sum = 0;
    for( int i=0; i<d->queue.size(); i++ ) {
      sum += d->queue[i].size();
    }
    return sum;
  }

  void PrimitiveQueue::clear() {
    for( int i=0; i<d->queue.size(); i++ ) {
      d->queue[i].clear();
    }
  }
}

#include "primitive.moc"
