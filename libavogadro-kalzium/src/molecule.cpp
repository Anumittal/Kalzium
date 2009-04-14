/**********************************************************************
  Molecule - Molecule class derived from the base Primitive class

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Marcus D. Hanwell

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

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

#include "molecule.h"

#include "atom.h"
#include "bond.h"
#include "cube.h"
#include "mesh.h"
#include "fragment.h"
#include "residue.h"
#include "zmatrix.h"

#include <Eigen/Geometry>
#include <Eigen/LeastSquares>

#include <vector>

#include <openbabel/mol.h>
#include <openbabel/math/vector3.h>
#include <openbabel/griddata.h>
#include <openbabel/grid.h>
#include <openbabel/generic.h>

#include <QDir>
#include <QDebug>
#include <QVariant>

using std::vector;
using Eigen::Vector3d;

namespace Avogadro{

  class MoleculePrivate {
    public:
      MoleculePrivate() : farthestAtom(0), invalidGeomInfo(true),
              invalidRings(true), obmol(0), obunitcell(0), obvibdata(0) {}
    // These are logically cached variables and thus are marked as mutable.
    // Const objects should be logically constant (and not mutable)
    // http://www.highprogrammer.com/alan/rants/mutable.html
      mutable Eigen::Vector3d       center;
      mutable Eigen::Vector3d       normalVector;
      mutable double                radius;
      mutable Atom *                farthestAtom;
      mutable bool                  invalidGeomInfo;
      mutable bool                  invalidRings;
      mutable std::vector<double>   energies;

      // std::vector used over QVector due to index issues, QVector uses ints
      std::vector<Cube *>           cubes;
      std::vector<Mesh *>           meshes;
      std::vector<Residue *>        residues;
      std::vector<Fragment *>       rings;
      std::vector<ZMatrix *>        zMatrix;

      // Used to store the index based list (not unique ids)
      QList<Cube *>                 cubeList;
      QList<Mesh *>                 meshList;
      QList<Residue *>              residueList;
      QList<Fragment *>             ringList;
      QList<ZMatrix *>              zMatrixList;

      // Our OpenBabel OBMol object
      OpenBabel::OBMol *            obmol;
      // Our OpenBabel OBUnitCell object (if any)
      OpenBabel::OBUnitCell *       obunitcell;
      // Our OpenBabel OBVibrationData object (if any)
      // TODO: Cache an OBMol, in which case the vib. data (and others)
      //       won't be necessary
      OpenBabel::OBVibrationData *  obvibdata;
  };

  Molecule::Molecule(QObject *parent) : Primitive(MoleculeType, parent),
                                        d_ptr(new MoleculePrivate),
                                        m_fileName(""),
                                        m_atomPos(0), m_dipoleMoment(0),
    m_invalidPartialCharges(true), m_invalidAromaticity(true)
  {
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::Molecule(const Molecule &other) :
    Primitive(MoleculeType, other.parent()), d_ptr(new MoleculePrivate),
    m_atomPos(0), m_dipoleMoment(0), m_invalidPartialCharges(true),
    m_invalidAromaticity(true)
  {
    *this = other;
    connect(this, SIGNAL(updated()), this, SLOT(updatePrimitive()));
  }

  Molecule::~Molecule()
  {
    // Need to iterate through all atoms/bonds and destroy them
    //Q_D(Molecule);
    disconnect(this, 0, 0, 0);
    clear();
    delete d_ptr;
  }

  void Molecule::setFileName(const QString& name)
  {
    QWriteLocker lock(m_lock);
    m_fileName = name;
  }

  QString Molecule::fileName() const
  {
    QReadLocker lock(m_lock);
    return m_fileName;
  }

  Atom *Molecule::addAtom()
  {
    // Add an atom with the next unique id
    return addAtom(m_atoms.size());
  }

    // do some fancy footwork when we add an atom previously created
  Atom *Molecule::addAtom(unsigned long id)
  {
    Atom *atom = new Atom(this);

    m_lock->lockForWrite();
    if (!m_atomPos) {
      m_atomConformers.resize(1);
      m_atomConformers[0] = new vector<Vector3d>;
      m_atomPos = m_atomConformers[0];
      m_atomPos->reserve(100);
    }

    if(id >= m_atoms.size()) {
      m_atoms.resize(id+1,0);
      m_atomPos->resize(id+1, Vector3d::Zero());
    }
    m_atoms[id] = atom;
    // Does this still want to have the same index as before somehow?
    m_atomList.push_back(atom);
    m_lock->unlock();

    atom->setId(id);
    atom->setIndex(m_atomList.size()-1);
    // now that the id is correct, emit the signal
    connect(atom, SIGNAL(updated()), this, SLOT(updateAtom()));
    emit atomAdded(atom);
    return atom;
  }

  void Molecule::setAtomPos(unsigned long id, const Eigen::Vector3d& vec)
  {
    if (id < m_atomPos->size()) {
      m_lock->lockForWrite();
      (*m_atomPos)[id] = vec;
      m_lock->unlock();
    }
  }

  void Molecule::setAtomPos(unsigned long id, const Eigen::Vector3d *vec)
  {
    if (vec) setAtomPos(id, *vec);
  }

  void Molecule::removeAtom(Atom *atom)
  {
    if(atom) {
      // When deleting an atom this also implicitly deletes any bonds to the atom
      foreach (unsigned long bond, atom->bonds()) {
        removeBond(bond);
      }

      m_lock->lockForWrite();
      m_atoms[atom->id()] = 0;
      // 1 based arrays stored/shown to user
      int index = atom->index();
      m_atomList.removeAt(index);
      for (int i = index; i < m_atomList.size(); ++i)
        m_atomList[i]->setIndex(i);
      atom->deleteLater();
      m_lock->unlock();

      disconnect(atom, SIGNAL(updated()), this, SLOT(updateAtom()));
      emit atomRemoved(atom);
    }
  }

  void Molecule::removeAtom(unsigned long id)
  {
    removeAtom(atomById(id));
  }

  Bond *Molecule::addBond()
  {
    return addBond(m_bonds.size());
  }

  Bond *Molecule::addBond(unsigned long id)
  {
    Q_D(Molecule);
    Bond *bond = new Bond(this);

    m_lock->lockForWrite();
    d->invalidRings = true;
    m_invalidPartialCharges = true;
    m_invalidAromaticity = true;
    if(id >= m_bonds.size())
      m_bonds.resize(id+1,0);
    m_bonds[id] = bond;
    m_bondList.push_back(bond);
    m_lock->unlock();

    bond->setId(id);
    bond->setIndex(m_bondList.size()-1);
    // now that the id is correct, emit the signal
    connect(bond, SIGNAL(updated()), this, SLOT(updateBond()));
    emit bondAdded(bond);
    return(bond);
  }

  void Molecule::removeBond(Bond *bond)
  {
    if(bond) {
      removeBond(bond->id());
    }
  }

  void Molecule::removeBond(unsigned long id)
  {
    if (id < m_bonds.size()) {
      Q_D(Molecule);
      if (m_bonds[id] == 0) {
        return;
      }

      m_lock->lockForWrite();
      d->invalidRings = true;
      m_invalidPartialCharges = true;
      m_invalidAromaticity = true;
      Bond *bond = m_bonds[id];
      m_bonds[id] = 0;
      // Delete the bond from the list and reorder the remaining bonds
      int index = bond->index();
      m_bondList.removeAt(index);
      for (int i = index; i < m_bondList.size(); ++i) {
        m_bondList[i]->setIndex(i);
      }
      m_lock->unlock();

      // Also delete the bond from the attached atoms
      if (m_atoms.size() > bond->beginAtomId()) {
        if (m_atoms[bond->beginAtomId()])
          m_atoms[bond->beginAtomId()]->removeBond(id);
      }
      if (m_atoms.size() > bond->endAtomId()) {
        if (m_atoms[bond->endAtomId()])
          m_atoms[bond->endAtomId()]->removeBond(id);
      }

      disconnect(bond, SIGNAL(updated()), this, SLOT(updateBond()));
      emit bondRemoved(bond);
      bond->deleteLater();
    }
  }

  Residue *Molecule::residue(int index)
  {
    Q_D(Molecule);
    QReadLocker lock(m_lock);
    if (index >= 0 && index < d->residueList.size()) {
      return d->residueList[index];
    }
    else {
      return 0;
    }
  }

  const Residue *Molecule::residue(int index) const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    if (index >= 0 && index < d->residueList.size()) {
      return d->residueList[index];
    }
    else {
      return 0;
    }
  }

  Residue *Molecule::residueById(unsigned long id) const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    if(id < d->residues.size()) {
      return d->residues[id];
    }
    else {
      return 0;
    }
  }

  Cube *Molecule::cube(int index) const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    if (index >= 0 && index < d->cubeList.size()) {
      return d->cubeList[index];
    }
    else {
      return 0;
    }
  }

  Cube *Molecule::cubeById(unsigned long id) const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    if(id < d->cubes.size()) {
      return d->cubes[id];
    }
    else {
      return 0;
    }
  }

  Cube *Molecule::addCube()
  {
    Q_D(Molecule);

    Cube *cube = new Cube(this);

    m_lock->lockForWrite();
    d->cubes.push_back(cube);
    d->cubeList.push_back(cube);
    m_lock->unlock();

    cube->setId(d->cubes.size()-1);
    cube->setIndex(d->cubeList.size()-1);

    // now that the id is correct, emit the signal
    connect(cube, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(cube);
    return(cube);
  }

  void Molecule::removeCube(Cube *cube)
  {
    Q_D(Molecule);
    if(cube) {
      m_lock->lockForWrite();
      d->cubes[cube->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = cube->index();
      d->cubeList.removeAt(index);
      for (int i = index; i < d->cubeList.size(); ++i) {
        d->cubeList[i]->setIndex(i);
      }
      m_lock->unlock();

      cube->deleteLater();
      disconnect(cube, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(cube);
    }
  }

  void Molecule::removeCube(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->cubes.size())
      removeCube(d->cubes[id]);
  }

  Mesh * Molecule::addMesh()
  {
    Q_D(Molecule);

    Mesh *mesh = new Mesh(this);

    m_lock->lockForWrite();
    d->meshes.push_back(mesh);
    d->meshList.push_back(mesh);
    m_lock->unlock();

    mesh->setId(d->meshes.size()-1);
    mesh->setIndex(d->meshList.size()-1);

    // now that the id is correct, emit the signal
    connect(mesh, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(mesh);
    return(mesh);
  }

  void Molecule::removeMesh(Mesh *mesh)
  {
    Q_D(Molecule);
    if(mesh) {
      m_lock->lockForWrite();
      d->meshes[mesh->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = mesh->index();
      d->meshList.removeAt(index);
      for (int i = index; i < d->meshList.size(); ++i) {
        d->meshList[i]->setIndex(i);
      }
      m_lock->unlock();

      mesh->deleteLater();
      disconnect(mesh, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(mesh);
    }
  }

  void Molecule::removeMesh(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->meshes.size())
      removeMesh(d->meshes[id]);
  }

  Mesh * Molecule::mesh(int index) const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    if (index >= 0 && index < d->meshList.size()) {
      return d->meshList[index];
    }
    else {
      return 0;
    }
  }

  Mesh * Molecule::meshById(unsigned long id) const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    if(id < d->meshes.size()) {
      return d->meshes[id];
    }
    else {
      return 0;
    }
  }

  Residue * Molecule::addResidue()
  {
    Q_D(Molecule);

    Residue *residue = new Residue(this);

    d->residues.push_back(residue);
    residue->setId(d->residues.size()-1);

    d->residueList.push_back(residue);
    residue->setIndex(d->residueList.size()-1);

    // now that the id is correct, emit the signal
    connect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
    emit primitiveAdded(residue);
    return(residue);
  }

  void Molecule::removeResidue(Residue *residue)
  {
    Q_D(Molecule);
    if(residue) {
      d->residues[residue->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = residue->index();
      d->residueList.removeAt(index);
      for (int i = index; i < d->residueList.size(); ++i) {
        d->residueList[i]->setIndex(i);
      }

      residue->deleteLater();
      disconnect(residue, SIGNAL(updated()), this, SLOT(updatePrimitive()));
      emit primitiveRemoved(residue);
    }
  }

  void Molecule::removeResidue(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->residues.size())
      removeResidue(d->residues[id]);
  }

  Fragment * Molecule::addRing()
  {
    Q_D(Molecule);

    Fragment *ring = new Fragment(this);

    d->rings.push_back(ring);
    ring->setId(d->rings.size()-1);

    d->ringList.push_back(ring);
    ring->setIndex(d->ringList.size()-1);

    // now that the id is correct, emit the signal
    connect(ring, SIGNAL(updated()), this, SLOT(updatePrimitive()));
//    emit primitiveAdded(ring);
    return(ring);
  }

  void Molecule::removeRing(Fragment *ring)
  {
    Q_D(Molecule);
    if(ring) {
      d->rings[ring->id()] = 0;
      // 0 based arrays stored/shown to user
      int index = ring->index();
      d->ringList.removeAt(index);
      for (int i = index; i < d->ringList.size(); ++i) {
        d->ringList[i]->setIndex(i);
      }

      ring->deleteLater();
      disconnect(ring, SIGNAL(updated()), this, SLOT(updatePrimitive()));
//      emit primitiveRemoved(ring);
    }
  }

  void Molecule::removeRing(unsigned long id)
  {
    Q_D(Molecule);
    if (id < d->rings.size())
      removeRing(d->rings[id]);
  }

  ZMatrix * Molecule::addZMatrix()
  {
    Q_D(Molecule);
    ZMatrix *zmatrix = new ZMatrix(this);
    d->zMatrixList.push_back(zmatrix);

    return zmatrix;
  }

  void Molecule::removeZMatrix(ZMatrix *zmatrix)
  {
    Q_D(Molecule);
    if (zmatrix) {
      d->zMatrixList.removeAll(zmatrix);
      delete zmatrix;
    }
  }

  ZMatrix * Molecule::zMatrix(int index) const
  {
    Q_D(const Molecule);
    if (index < d->zMatrixList.size())
      return d->zMatrixList.at(index);
    else
      return 0;
  }

  QList<ZMatrix *> Molecule::zMatrices() const
  {
    Q_D(const Molecule);
    return d->zMatrixList;
  }

  unsigned int Molecule::numZMatrices() const
  {
    Q_D(const Molecule);
    return d->zMatrixList.size();
  }

  void Molecule::addHydrogens(Atom *a,
                              const QList<unsigned long> &atomIds,
                              const QList<unsigned long> &bondIds)
  {
    if (atomIds.size() != bondIds.size()) {
      qDebug() << "Error, addHydrogens called with atom & bond id lists of different size!";
    }

    // Construct an OBMol, call AddHydrogens and translate the changes
    OpenBabel::OBMol obmol = OBMol();
    if (a)
      obmol.AddHydrogens(obmol.GetAtom(a->index()+1));
    else
      obmol.AddHydrogens();
    // All new atoms in the OBMol must be the additional hydrogens
    unsigned int numberAtoms = numAtoms();
    int j = 0;
    for (unsigned int i = numberAtoms+1; i <= obmol.NumAtoms(); ++i, ++j) {
      if (obmol.GetAtom(i)->IsHydrogen()) {
        OpenBabel::OBAtom *obatom = obmol.GetAtom(i);
        Atom *atom;
        if (atomIds.isEmpty())
          atom = addAtom();
        else if (j < atomIds.size())
          atom = addAtom(atomIds.at(j));
        else {
          qDebug() << "Error - not enough unique ids in addHydrogens.";
          break;
        }
        atom->setOBAtom(obatom);
        // Get the neighbor atom
        OpenBabel::OBBondIterator iter;
        OpenBabel::OBAtom *next = obatom->BeginNbrAtom(iter);
        Bond *bond;
        if (bondIds.isEmpty())
          bond = addBond();
        else // Already confirmed by atom ids
          bond = addBond(bondIds.at(j));
        bond->setEnd(Molecule::atom(atom->index()));
        bond->setBegin(Molecule::atom(next->GetIdx()-1));
      }
    }
    for (unsigned int i = 1; i <= numberAtoms; ++i) {
      // Warning -- OB atom index off-by-one here
      atom(i-1)->setPartialCharge(obmol.GetAtom(i)->GetPartialCharge());
    }
  }

  void Molecule::removeHydrogens(Atom *atom)
  {
    if (atom) {
      // Delete any connected hydrogen atoms
      QList<unsigned long> neighbors = atom->neighbors();

      foreach (unsigned long a, neighbors) {
        Atom *nbrAtom = atomById(a);
        // we need to check if the atom still exists
        if (nbrAtom) {
          if (nbrAtom->isHydrogen()) {
            removeAtom(a);
          }
        }
        else {
          qDebug() << "Error, atom advertising deleted neighbor" << atom->id()
                   << a;
        }
      }
    }
    // Delete all of the hydrogens
    else {
      foreach (Atom *atom, m_atomList) {
        if (atom->isHydrogen()) {
          removeAtom(atom);
        }
      }
    }
  }

  void Molecule::setDipoleMoment(const Eigen::Vector3d &moment)
  {
    // Don't leak memory
    if (m_dipoleMoment)
      delete m_dipoleMoment;

    m_dipoleMoment = new Vector3d(moment);
  }

  const Eigen::Vector3d * Molecule::dipoleMoment() const
  {
    if (m_dipoleMoment)
      return m_dipoleMoment;
    else {
      // Calculate an estimate
      m_dipoleMoment = new Vector3d(0.0, 0.0, 0.0);
      foreach (Atom *a, atoms()) {
        *m_dipoleMoment += *a->pos() * a->partialCharge();
      }
      return m_dipoleMoment;
    }
  }

  void Molecule::calculatePartialCharges() const
  {
    if (numAtoms() < 1 || !m_invalidPartialCharges) {
      return;
    }
    OpenBabel::OBMol obmol = OBMol();
    for (unsigned int i = 0; i < numAtoms(); ++i) {
      // Warning: OB off-by-one index
      atom(i)->setPartialCharge(obmol.GetAtom(i+1)->GetPartialCharge());
    }
    m_invalidPartialCharges = false;
  }

  void Molecule::calculateAromaticity() const
  {
    if (numBonds() < 1 || !m_invalidAromaticity)
      return;

    OpenBabel::OBMol obmol = OBMol();
    for (unsigned int i = 0; i < numBonds(); ++i) {
      bond(i)->setAromaticity(obmol.GetBond(i)->IsAromatic());
    }
    m_invalidAromaticity = false;
  }

  unsigned int Molecule::numAtoms() const
  {
    QReadLocker lock(m_lock);
    return m_atomList.size();
  }

  unsigned int Molecule::numBonds() const
  {
    QReadLocker lock(m_lock);
    return m_bondList.size();
  }

  unsigned int Molecule::numCubes() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->cubeList.size();
  }

  unsigned int Molecule::numMeshes() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->meshList.size();
  }

  unsigned int Molecule::numResidues() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->residueList.size();
  }

  unsigned int Molecule::numRings() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->ringList.size();
  }

  void Molecule::updatePrimitive()
  {
    Q_D(Molecule);
    Primitive *primitive = qobject_cast<Primitive *>(sender());
    d->invalidGeomInfo = true;
    emit primitiveUpdated(primitive);
  }

  void Molecule::updateAtom()
  {
    Q_D(Molecule);
    Atom *atom = qobject_cast<Atom *>(sender());
    d->invalidGeomInfo = true;
    emit atomUpdated(atom);
  }

  void Molecule::updateBond()
  {
    Q_D(Molecule);
    Bond *bond = qobject_cast<Bond *>(sender());
    d->invalidGeomInfo = true;
    emit bondUpdated(bond);
  }

  void Molecule::update()
  {
    Q_D(Molecule);
    d->invalidGeomInfo = true;
    emit updated();
  }

  Bond* Molecule::bond(unsigned long id1, unsigned long id2)
  {
    // Take two atom IDs and see if we have a bond between the two
    if (atomById(id1)) {
      QList<unsigned long> bonds = atomById(id1)->bonds();
      foreach (unsigned long id, bonds) {
        Bond *bond = bondById(id);
        if (bond) {
          if (bond->otherAtom(id1) == id2) {
            return bond;
          }
        }
      }
    }
    return 0;
  }

  Bond* Molecule::bond(const Atom *a1, const Atom *a2)
  {
    if (a1 && a2) {
      return bond(a1->id(), a2->id());
    }
    else {
      return 0;
    }
  }

  bool Molecule::addConformer(const std::vector<Eigen::Vector3d> &conformer,
                              unsigned int index)
  {
    if (conformer.size() != m_atomPos->size())
      return false;

    if (m_atomConformers.size() < index+1) {
      unsigned int size = m_atomConformers.size();
      for (unsigned int i = size; i <= index; ++i)
        m_atomConformers.push_back( new vector<Vector3d>(m_atomPos->size()) );
    }
    *m_atomConformers[index] = conformer;
    return true;
  }

  vector<Vector3d> * Molecule::addConformer(unsigned int index)
  {
    if (index < m_atomConformers.size())
      return m_atomConformers[index];
    else {
      unsigned int size = m_atomConformers.size();
      m_atomConformers.resize(index+1);
      for (unsigned int i = size; i <= index; ++i)
        m_atomConformers[i] = new vector<Vector3d>(m_atomPos->size());
      return m_atomConformers[index];
    }
  }

  vector<Vector3d> * Molecule::conformer(unsigned int index)
  {
    if (index && index < m_atomConformers.size())
      return m_atomConformers[index];
    else if (index == 0)
      return m_atomPos;
    else
      return NULL;
  }

  bool Molecule::setConformer(unsigned int index)
  {
    // If the index is higher than the size
    if (m_atomConformers.size() < index + 1)
      return false;
    else {
      unsigned int size = m_atomPos->size();
      m_atomPos = m_atomConformers[index];
      while (m_atomPos->size() < size)
        m_atomPos->push_back(Eigen::Vector3d::Zero());
      return true;
    }
  }

  void Molecule::setAllConformers(const std::vector< std::vector<Eigen::Vector3d>* > conformers)
  {
    //    clearConformers();
    m_atomConformers.resize(1);
    for (unsigned int i = 0; i < conformers.size(); ++ i) {
      m_atomConformers.push_back(conformers[i]);
    }
  }

  void Molecule::clearConformers()
  {
    for (unsigned int i = 1; i < m_atomConformers.size(); ++i)
      delete m_atomConformers[i];
    m_atomConformers.resize(1);
  }

  unsigned int Molecule::numConformers() const
  {
    return m_atomConformers.size();
  }

  const std::vector<double>& Molecule::energies() const
  {
    Q_D(const Molecule);
    while (d->energies.size() != numConformers())
      d->energies.push_back(0.0);
    return d->energies;
  }

  double Molecule::energy(unsigned int index) const
  {
    Q_D(const Molecule);
    if (index < d->energies.size())
      return d->energies[index];
    else
      return 0.0;
  }

  void Molecule::setEnergies(const std::vector<double>& energies)
  {
    Q_D(const Molecule);
    d->energies = energies;
  }


  QList<Atom *> Molecule::atoms() const
  {
    QReadLocker lock(m_lock);
    return m_atomList;
  }

  QList<Bond *> Molecule::bonds() const
  {
    QReadLocker lock(m_lock);
    return m_bondList;
  }

  QList<Cube *> Molecule::cubes() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->cubeList;
  }

  QList<Mesh *> Molecule::meshes() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->meshList;
  }

  QList<Residue *> Molecule::residues() const
  {
    Q_D(const Molecule);
    QReadLocker lock(m_lock);
    return d->residueList;
  }

  QList<Fragment *> Molecule::rings()
  {
    Q_D(Molecule);
    // Check is the rings need updating before returning the list
    if(d->invalidRings) {
      // Now update the rings
      foreach(Fragment *ring, d->ringList) {
        removeRing(ring);
      }
      OpenBabel::OBMol obmol = OBMol();
      std::vector<OpenBabel::OBRing *> rings;
      rings = obmol.GetSSSR();
      foreach(OpenBabel::OBRing *r, rings) {
        Fragment *ring = addRing();
        foreach(int index, r->_path) {
          ring->addAtom(atom(index-1)->id());
        }
      }
      d->invalidRings = false;
    }
    QReadLocker lock(m_lock);
    return d->ringList;
  }

  OpenBabel::OBMol Molecule::OBMol() const
  {
    Q_D(const Molecule);
    // Right now we make an OBMol each time
    OpenBabel::OBMol obmol;
    obmol.BeginModify();

    foreach (Atom *atom, m_atomList) {
      OpenBabel::OBAtom *a = obmol.NewAtom();
      OpenBabel::OBAtom obatom = atom->OBAtom();
      *a = obatom;
    }
    foreach (Bond *bond, m_bondList) {
      Atom *beginAtom = atomById(bond->beginAtomId());
      if (!beginAtom)
        continue;

      Atom *endAtom = atomById(bond->endAtomId());
      if (!endAtom)
        continue;

      obmol.AddBond(beginAtom->index() + 1,
                    endAtom->index() + 1, bond->order());
    }
    obmol.EndModify();

    // Copy unit cells
    if (d->obunitcell != NULL) {
      OpenBabel::OBUnitCell *obunitcell = new OpenBabel::OBUnitCell;
      *obunitcell = *d->obunitcell;
      obmol.SetData(obunitcell);
    }

    // Copy OBPairData, if needed
    OpenBabel::OBPairData *obproperty;
    foreach(const QByteArray &propertyName, dynamicPropertyNames()) {
      obproperty = new OpenBabel::OBPairData;
      obproperty->SetAttribute(propertyName.data());
      obproperty->SetValue(property(propertyName).toByteArray().data());
      obmol.SetData(obproperty);
    }

    // Copy vibrations, if needed
    if (d->obvibdata != NULL) {
      obmol.SetData(d->obvibdata->Clone(&obmol));
    }

    // TODO: Copy residue information, cubes, etc.

    return obmol;
  }

  bool Molecule::setOBMol(OpenBabel::OBMol *obmol)
  {
    // Take an OBMol, copy everything we need and store this object
    Q_D(Molecule);
    qDebug() << "setOBMol called.";
    clear();
    // Copy all the parts of the OBMol to our Molecule
    blockSignals(true);

    qDebug() << "Copying atoms...";
    // Begin by copying all of the atoms
    std::vector<OpenBabel::OBAtom*>::iterator i;

    for (OpenBabel::OBAtom *obatom = obmol->BeginAtom(i); obatom; obatom = obmol->NextAtom(i)) {
      Atom *atom = addAtom();
      atom->setOBAtom(obatom);
    }

    qDebug() << "Copying bonds...";
    // Now bonds, we use the indices of the atoms to get the bonding right
    std::vector<OpenBabel::OBBond*>::iterator j;
    for (OpenBabel::OBBond *obbond = obmol->BeginBond(j); obbond; obbond = obmol->NextBond(j)) {
      Bond *bond = addBond();
      // Get the begin and end atoms - we use a 0 based index, OB uses 1 based
      bond->setAtoms(obbond->GetBeginAtom()->GetIdx()-1,
                     obbond->GetEndAtom()->GetIdx()-1,
                     obbond->GetBondOrder());
    }

    qDebug() << "Copying cubes...";
    // Now for the volumetric data
    std::vector<OpenBabel::OBGenericData*> data = obmol->GetAllData(OpenBabel::OBGenericDataType::GridData);
    for (unsigned int i = 0; i < data.size(); ++i) {
      QString name = QString(data[i]->GetAttribute().c_str());
      OpenBabel::OBGridData *grid = static_cast<OpenBabel::OBGridData *>(data[i]);
      OpenBabel::vector3 obmin = grid->GetOriginVector();
      Eigen::Vector3d min(obmin.x(), obmin.y(), obmin.z());
      OpenBabel::vector3 obmax = grid->GetMaxVector();
      Eigen::Vector3d max(obmax.x(), obmax.y(), obmax.z());
      int x, y, z;
      grid->GetNumberOfPoints(x, y, z);
      Eigen::Vector3i points(x, y, z);
      Cube *cube = addCube();
      cube->setLimits(min, max, points);
      cube->setData(grid->GetValues());
      cube->setName(name);
//      qDebug() << "Cube" << i << "added.";
    }

    qDebug() << "Copying residues...";
    // Copy the residues across...
    std::vector<OpenBabel::OBResidue *> residues;
    OpenBabel::OBResidueIterator iResidue;
    short chainNumber = 0;
    QHash<char, short> chains;
    for (OpenBabel::OBResidue *obres = static_cast<OpenBabel::OBResidue *>(obmol->BeginResidue(iResidue));
          obres; obres = static_cast<OpenBabel::OBResidue *>(obmol->NextResidue(iResidue))) {
      /// Copy these residues!
      Residue *residue = addResidue();
      residue->setName(obres->GetName().c_str());
      residue->setNumber(obres->GetNumString().c_str());
      if (!chains.contains(obres->GetChain())) {
        chains[obres->GetChain()] = chainNumber;
        chainNumber++;
      }
      residue->setChainID(obres->GetChain());
      residue->setChainNumber(chains.value(obres->GetChain()));
      std::vector<OpenBabel::OBAtom*> obatoms = obres->GetAtoms();
      foreach (OpenBabel::OBAtom *obatom, obatoms) {
        unsigned long atomId = atom(obatom->GetIdx()-1)->id();
        residue->addAtom(atomId);
        residue->setAtomId(atomId, obres->GetAtomID(obatom).c_str());
      }
      std::vector<OpenBabel::OBBond*> obbonds = obres->GetBonds();
      foreach (OpenBabel::OBBond *obbond, obbonds) {
        residue->addBond(bond(obbond->GetIdx())->id());
      }
    }

    qDebug() << "Copying other data...";
    // Copy the dipole moment of the molecule
    OpenBabel::OBVectorData *vd = (OpenBabel::OBVectorData*)obmol->GetData("Dipole Moment");
    if (vd) {
      OpenBabel::vector3 moment = vd->GetData();
      m_dipoleMoment = new Vector3d(moment.x(), moment.y(), moment.z());
    }

    // If available, copy the unit cell
    OpenBabel::OBUnitCell *obunitcell = static_cast<OpenBabel::OBUnitCell *>(obmol->GetData(OpenBabel::OBGenericDataType::UnitCell));
    if (obunitcell) {
      d->obunitcell = new OpenBabel::OBUnitCell;
      *d->obunitcell = *obunitcell;
    }
    // (that could return NULL, but other methods know they could get NULL)

    // Copy forces, if present and valid
    if (obmol->HasData(OpenBabel::OBGenericDataType::ConformerData)) {
    OpenBabel::OBConformerData *cd = static_cast<OpenBabel::OBConformerData*>(obmol->GetData(OpenBabel::OBGenericDataType::ConformerData));
    if (cd) {
      std::vector< std::vector<OpenBabel::vector3> > allForces = cd->GetForces();

      // check for validity (i.e., we have some forces, one for each atom
      if (allForces.size() && allForces[0].size() == numAtoms()) {
        OpenBabel::vector3 force;
        foreach (Atom *atom, m_atomList) { // loop through each atom
            force = allForces[0][atom->index()];
            qDebug() << " copying force " << force.x() << force.y() << force.z();
            atom->setForceVector(Eigen::Vector3d(force.x(), force.y(), force.z()));
          } // end setting forces on each atom
        }
      }
    }  // end HasData(ConformerData)

    // Copy any vibration data if possible
    if (obmol->HasData(OpenBabel::OBGenericDataType::VibrationData)) {
      OpenBabel::OBVibrationData *vibData = static_cast<OpenBabel::OBVibrationData*>(obmol->GetData(OpenBabel::OBGenericDataType::VibrationData));
      d->obvibdata = vibData;
    }

    // Finally, sync OBPairData to dynamic properties
    OpenBabel::OBDataIterator dIter;
    OpenBabel::OBPairData *property;
    data = obmol->GetAllData(OpenBabel::OBGenericDataType::PairData);
    for (dIter = data.begin(); dIter != data.end(); ++dIter) {
      property = static_cast<OpenBabel::OBPairData *>(*dIter);
      setProperty(property->GetAttribute().c_str(), property->GetValue().c_str());
    }
    blockSignals(false);
    return true;
  }

  OpenBabel::OBUnitCell *Molecule::OBUnitCell() const
  {
    Q_D(const Molecule);
    return d->obunitcell;
  }

  bool Molecule::setOBUnitCell(OpenBabel::OBUnitCell *obunitcell)
  {
    Q_D(Molecule);
    d->obunitcell = obunitcell;
    if (obunitcell == NULL) {
      // delete it from our private obmol
      if (d->obmol)
        d->obmol->DeleteData(OpenBabel::OBGenericDataType::UnitCell);
    }
    return true;
  }

  const Eigen::Vector3d Molecule::center() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->center;
  }

  const Eigen::Vector3d Molecule::normalVector() const
  {
    Q_D(const Molecule);
    if( d->invalidGeomInfo ) computeGeomInfo();
    return d->normalVector;
  }

  double Molecule::radius() const
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

  void Molecule::translate(const Eigen::Vector3d& offset)
  {
    m_lock->lockForWrite();
    if (!m_atomPos)
      return; // nothing to do

    foreach (Atom *atom, m_atomList) {
      (*m_atomPos)[atom->id()] += offset;
      emit atomUpdated(atom);
    }
    m_lock->unlock();
  }

  void Molecule::clear()
  {
    Q_D(Molecule);
    m_lock->lockForWrite();
    m_atoms.resize(0);
    foreach (Atom *atom, m_atomList) {
      atom->deleteLater();
      emit primitiveRemoved(atom);
    }
    m_atomList.clear();
    clearConformers();
    delete m_atomPos;
    m_atomPos = 0;
    delete m_dipoleMoment;
    m_dipoleMoment = 0;
    delete d->obunitcell;
    d->obunitcell = 0;

    m_bonds.resize(0);
    foreach (Bond *bond, m_bondList) {
      bond->deleteLater();
      emit primitiveRemoved(bond);
    }
    m_bondList.clear();

    d->cubes.resize(0);
    foreach (Cube *cube, d->cubeList) {
      cube->deleteLater();
      emit primitiveRemoved(cube);
    }
    d->cubeList.clear();

    d->meshes.resize(0);
    foreach (Mesh *mesh, d->meshList) {
      mesh->deleteLater();
      emit primitiveRemoved(mesh);
    }
    d->meshList.clear();

    d->residues.resize(0);
    foreach (Residue *residue, d->residueList) {
      residue->deleteLater();
      emit primitiveRemoved(residue);
    }
    d->residueList.clear();

    d->rings.resize(0);
    foreach (Fragment *ring, d->ringList) {
      ring->deleteLater();
      emit primitiveRemoved(ring);
    }
    d->ringList.clear();
    m_lock->unlock();
  }

  Molecule &Molecule::operator=(const Molecule& other)
  {
    // FIXME: Copy all the other stuff in the molecule!
    //Q_D(Molecule);
    clear();
    //const MoleculePrivate *e = other.d_func();
    m_atoms.resize(other.m_atoms.size(), 0);
    if (other.m_atomPos) {
      m_atomConformers.resize(1);
      m_atomConformers[0] = new vector<Vector3d>;
      m_atomPos = m_atomConformers[0];
      m_atomPos->reserve(100);

      m_atomPos->clear();
      m_atomPos->resize(other.m_atomPos->size());
    }
    else
      qDebug() << "Other atom has a position list of size zero!";

    m_bonds.resize(other.m_bonds.size(), 0);

    // Copy the atoms and bonds over
    unsigned int size = other.m_atoms.size();
    for (unsigned int i = 0; i < size; ++i) {
      if (other.m_atoms.at(i) > 0) {
        Atom *atom = new Atom(this);
        atom->setId(other.m_atoms[i]->id());
        atom->setIndex(other.m_atoms[i]->index());
        m_atoms[i] = atom;
        m_atomList.push_back(atom);
        *atom = *(other.m_atoms[i]);
        emit primitiveAdded(atom);
      }
    }

    size = other.m_bonds.size();
    for (unsigned int i = 0; i < size; ++i) {
      if (other.m_bonds.at(i)) {
        Bond *bond = new Bond(this);
        *bond = *(other.m_bonds[i]);
        bond->setId(other.m_bonds[i]->id());
        bond->setIndex(other.m_bonds[i]->index());
        m_bonds[i] = bond;
        m_bondList.push_back(bond);
        // Add the bond to it's atoms
        bond->beginAtom()->addBond(bond);
        bond->endAtom()->addBond(bond);
        emit primitiveAdded(bond);
      }
    }

    return *this;
  }

  Molecule &Molecule::operator+=(const Molecule& other)
  {
    // FIXME: Copy all the other stuff in the molecule!
    //const MoleculePrivate *e = other.d_func();
    // Create a temporary map from the old indices to the new for bonding
    QList<int> map;
    foreach (Atom *a, other.m_atomList) {
      Atom *atom = addAtom();
      *atom = *a;
      map.push_back(atom->id());
      emit primitiveAdded(atom);
    }
    foreach (Bond *b, other.m_bondList) {
      Bond *bond = addBond();
      *bond = *b;
      bond->setBegin(atomById(map.at(other.atomById(b->beginAtomId())->index())));
      bond->setEnd(atomById(map.at(other.atomById(b->endAtomId())->index())));
      emit primitiveAdded(bond);
    }
    return *this;
  }

  void Molecule::computeGeomInfo() const
  {
    Q_D(const Molecule);
    d->invalidGeomInfo = true;
    d->farthestAtom = 0;
    d->center.setZero();
    d->normalVector.setZero();
    d->radius = 1.0;
    // In order to calculate many parameters we need at least two atoms
    if(numAtoms() > 1) {
      // compute center
      foreach (Atom *atom, m_atomList)
        d->center += *atom->pos();

      d->center /= numAtoms();

      // compute the normal vector to the molecule's best-fitting plane
      int i = 0;
      Vector3d ** atomPositions = new Vector3d*[numAtoms()];
      foreach (Atom *atom, m_atomList)
        atomPositions[i++] = &m_atomPos->at(atom->id());

      Eigen::Hyperplane<double, 3> planeCoeffs;
      Eigen::fitHyperplane(numAtoms(), atomPositions, &planeCoeffs);
      delete[] atomPositions;
      d->normalVector = planeCoeffs.normal();

      // compute radius and the farthest atom
      d->radius = -1.0; // so that ( squaredDistanceToCenter > d->radius ) is true for at least one atom.
      foreach (Atom *atom, m_atomList) {
        double distanceToCenter = (*atom->pos() - d->center).norm();
        if(distanceToCenter > d->radius) {
          d->radius = distanceToCenter;
          d->farthestAtom = atom;
        }
      }
    }
    d->invalidGeomInfo = false;
  }

} // End namespace Avogadro

#include "molecule.moc"
