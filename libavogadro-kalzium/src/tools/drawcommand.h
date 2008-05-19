/**********************************************************************
  DrawCommand - Set of command classes for drawing.

  Copyright (C) 2007 Donald Ephraim Curtis
  Copyright (C) 2008 Tim Vandermeersch

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

#ifndef DRAWCOMMAND_H
#define DRAWCOMMAND_H

#include <QUndoCommand>
#include <eigen/vector.h>

namespace Avogadro {

  class Molecule;
  class Atom;
  class Bond;

  class AddAtomDrawCommandPrivate;
  class AddAtomDrawCommand : public QUndoCommand
  {
  public:
    AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos,
                       unsigned int element, int adjustValence);
    AddAtomDrawCommand(Molecule *molecule, Atom *atom, int adjustValence);
    ~AddAtomDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    AddAtomDrawCommandPrivate * const d;
  };

  class DeleteAtomDrawCommandPrivate;
  class DeleteAtomDrawCommand : public QUndoCommand
  {
  public:
    DeleteAtomDrawCommand(Molecule *molecule, int index, int adjustValence);
    ~DeleteAtomDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    DeleteAtomDrawCommandPrivate * const d;
  };

  class AddBondDrawCommandPrivate;
  class AddBondDrawCommand : public QUndoCommand
  {
  public:
    AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order, int adjustValence);
    AddBondDrawCommand(Molecule *molecule, Bond *bond, int adjustValence);
    ~AddBondDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    AddBondDrawCommandPrivate * const d;
  };

  class DeleteBondDrawCommandPrivate;
  class DeleteBondDrawCommand : public QUndoCommand
  {
  public:
    DeleteBondDrawCommand(Molecule *molecule, int index, int adjustValence);
    ~DeleteBondDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    DeleteBondDrawCommandPrivate * const d;
  };

  class ChangeElementDrawCommandPrivate;
  class ChangeElementDrawCommand : public QUndoCommand
  {
  public:
    ChangeElementDrawCommand(Molecule *molecule, Atom *atom, 
                             unsigned int element, int adjustValence);
    ~ChangeElementDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    ChangeElementDrawCommandPrivate * const d;
  };

  class ChangeBondOrderDrawCommandPrivate;
  class ChangeBondOrderDrawCommand : public QUndoCommand
  {
  public:
    ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond,
                               unsigned int bondOrder, int adjustValence);
    ~ChangeBondOrderDrawCommand();

    virtual void undo();
    virtual void redo();

  private:
    ChangeBondOrderDrawCommandPrivate * const d;
  };

  class InsertFragmentCommandPrivate;
  class InsertFragmentCommand : public QUndoCommand
  {
  public:
    InsertFragmentCommand(Molecule *molecule, Molecule &generatedMolecule);
    ~InsertFragmentCommand();

    virtual void undo();
    virtual void redo();

  private:
    InsertFragmentCommandPrivate * const d;
  };


} // end namespace Avogadro

#endif
