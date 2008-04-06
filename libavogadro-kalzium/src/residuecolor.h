/**********************************************************************
  ResidueColor - Class for coloring based on residues (if available)

  Copyright (C) 2007 Geoffrey R. Hutchison

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

#ifndef __RESIDUECOLOR_H
#define __RESIDUECOLOR_H

#include <avogadro/global.h>
#include <avogadro/color.h>

namespace Avogadro {

  /**
   * This class maps colors based on residues
   * http://jmol.sourceforge.net/jscolors/
   *
   * @author Geoff Hutchison
   */
  class A_EXPORT ResidueColor: public Color
  {
  public:
    ResidueColor();
    virtual ~ResidueColor();

    /**
     * Set the color based on the supplied Primitive
     * If NULL is passed, do nothing */
    virtual void set(const Primitive *);

    virtual QString type() const { return "Color by Residue"; }
  };

}

#endif
