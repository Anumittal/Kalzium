/**********************************************************************
  Painter - drawing spheres, cylinders and text in a GLWidget

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

#include <config.h>

#include <avogadro/painter.h>
#include <avogadro/glwidget.h>
#include <avogadro/camera.h>
#include "sphere.h"
#include "cylinder.h"
#include "textrenderer.h"

#include <cassert>

using namespace Eigen;
using namespace std;
using namespace Avogadro;

namespace Avogadro
{

  const int      PAINTER_MAX_DETAIL_LEVEL = PAINTER_DETAIL_LEVELS - 1;
  const double   PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL
    = sqrt(PAINTER_SPHERES_LIMIT_MIN_LEVEL);
  const double   PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL
    = sqrt(PAINTER_SPHERES_LIMIT_MAX_LEVEL);
  const double   PAINTER_SPHERES_DETAIL_COEFF
    = static_cast<double>(PAINTER_MAX_DETAIL_LEVEL - 1)
    / (PAINTER_SPHERES_SQRT_LIMIT_MAX_LEVEL - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL);
  const double   PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL
    = sqrt(PAINTER_CYLINDERS_LIMIT_MIN_LEVEL);
  const double   PAINTER_CYLINDERS_SQRT_LIMIT_MAX_LEVEL
    = sqrt(PAINTER_CYLINDERS_LIMIT_MAX_LEVEL);
  const double   PAINTER_CYLINDERS_DETAIL_COEFF
    = static_cast<double>(PAINTER_MAX_DETAIL_LEVEL - 1)
    / (PAINTER_CYLINDERS_SQRT_LIMIT_MAX_LEVEL - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL);

  class PainterPrivate
  {
    public:
      PainterPrivate() : widget(0), globalQualitySetting(0), spheres(0), cylinders(0),
      textRenderer(new TextRenderer), initialized(false) {};
      ~PainterPrivate()
      {
        deleteObjects();
        delete textRenderer;
      }

      GLWidget *widget;

      int globalQualitySetting;

      /** array of pointers to Spheres. You might ask, why not have
        * a plain array of Spheres. The idea is that more than one global detail level
        * may use a given sphere detail level. It is therefore interesting to be able
        * to share that sphere, instead of having redundant spheres in memory.
        */
      Sphere **spheres;
      /** array of pointers to Cylinders. You might ask, why not have
        * a plain array of Cylinders. The idea is that more than one global detail level
        * may use a given cylinder detail level. It is therefore interesting to be able
        * to share that cylinder, instead of having redundant cylinder in memory.
        */
      Cylinder **cylinders;
      
      TextRenderer *textRenderer;

      bool initialized;

      void deleteObjects();
      void createObjects();
  };

  void PainterPrivate::deleteObjects()
  {
    int level, lastLevel, n;
    // delete the spheres. One has to be wary that more than one sphere
    // pointer may have the same value. One wants to avoid deleting twice the same sphere.
    if(spheres) {
      lastLevel = -1;
      for(n = 0; n < PAINTER_DETAIL_LEVELS; n++)
      {
        level = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][n];
        if( level != lastLevel ) {
          lastLevel = level;
          if( spheres[n] ) {
            delete spheres[n];
            spheres[n] = 0;
          }
        }
      }
      delete[] spheres;
      spheres = 0;
    }
    
    // delete the cylinders. One has to be wary that more than one cylinder
    // pointer may have the same value. One wants to avoid deleting twice the same cylinder.
    if(cylinders) {
      lastLevel = -1;
      for(n = 0; n < PAINTER_DETAIL_LEVELS; n++)
      {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[globalQualitySetting][n];
        if( level != lastLevel ) {
          lastLevel = level;
          if( cylinders[n] ) {
            delete cylinders[n];
            cylinders[n] = 0;
          }
        }
      }
      delete[] cylinders;
      cylinders = 0;
    }
  }

  void PainterPrivate::createObjects()
  {
    int level, lastLevel, n;
    // create the spheres. More than one sphere detail level may have the same value.
    // in that case we want to reuse the corresponding sphere by just copying the pointer,
    // instead of creating redundant spheres.
    if(spheres == 0)
    {
      spheres = new Sphere*[PAINTER_DETAIL_LEVELS];
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][0];
      spheres[0] = new Sphere( lastLevel );
      for(n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][n];
        if( level == lastLevel ) {
          spheres[n] = spheres[n-1];
        }
        else {
          lastLevel = level;
          spheres[n] = new Sphere( level );
        }
      }
    }
    
    // create the cylinders. More than one cylinder detail level may have the same value.
    // in that case we want to reuse the corresponding cylinder by just copying the pointer,
    // instead of creating redundant cylinders.
    if(cylinders == 0)
    {
      cylinders = new Cylinder*[PAINTER_DETAIL_LEVELS];
      int level, lastLevel;
      lastLevel = PAINTER_SPHERES_LEVELS_ARRAY[globalQualitySetting][0];
      cylinders[0] = new Cylinder( lastLevel );
      for(int n = 1; n < PAINTER_DETAIL_LEVELS; n++ )
      {
        level = PAINTER_CYLINDERS_LEVELS_ARRAY[globalQualitySetting][n];
        if( level == lastLevel ) {
          cylinders[n] = cylinders[n-1];
        }
        else {
          lastLevel = level;
          cylinders[n] = new Cylinder( level );
        }
      }
    }
  }

  Painter::Painter() : d(new PainterPrivate)
  {
  }

  Painter::~Painter()
  {
    delete d;
  }

  void Painter::setGLWidget( GLWidget * widget )
  {
    d->widget = widget;
    d->textRenderer->setGLWidget(d->widget);
  }

  void Painter::setGlobalQualitySetting( int globalQualitySetting )
  {
    assert( d->initialized );
    assert( globalQualitySetting >= 0 && globalQualitySetting < PAINTER_GLOBAL_QUALITY_SETTINGS );
    d->deleteObjects();
    d->globalQualitySetting = globalQualitySetting;
    d->createObjects();
  }

  int Painter::globalQualitySetting() const
  {
    return d->globalQualitySetting;
  }

  void Painter::initialize( GLWidget * widget, int globalQualitySetting )
  {
    assert( globalQualitySetting >= 0 && globalQualitySetting < PAINTER_GLOBAL_QUALITY_SETTINGS );
    d->initialized = true;
    setGLWidget(widget);
    setGlobalQualitySetting(globalQualitySetting);
  }

  void Painter::drawSphere( const Eigen::Vector3d & center, double radius, int detailLevel ) const
  {
    assert( d->initialized );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->spheres[detailLevel]->draw(center, radius);
  }

  void Painter::drawSphere( const Eigen::Vector3d & center, double radius ) const
  {
    assert( d->initialized );
    Eigen::Vector3d transformedCenter = d->widget->camera()->modelview() * center;
    double distance = transformedCenter.norm();
    
    // perform a rough form of frustum culling
    double dot = transformedCenter.z() / distance;
    if(dot > -0.8) return;
    
    double apparentRadius = radius / distance;
    int detailLevel = 1 + static_cast<int>( floor(
          PAINTER_SPHERES_DETAIL_COEFF * (sqrt(apparentRadius) - PAINTER_SPHERES_SQRT_LIMIT_MIN_LEVEL)
          ) );
    if( detailLevel < 0 ) {
      detailLevel = 0;
    }
    if( detailLevel > PAINTER_MAX_DETAIL_LEVEL ) {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }
    d->spheres[detailLevel]->draw(center, radius);
  }

  void Painter::drawCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius, int detailLevel ) const
  {
    assert( d->initialized );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->cylinders[detailLevel]->draw(end1, end2, radius);
  }

  void Painter::drawCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius) const
  {
    assert( d->initialized );
    
    Eigen::Vector3d transformedEnd1 = d->widget->camera()->modelview() * end1;
    double distance = transformedEnd1.norm();
    
    // perform a rough form of frustum culling
    double dot = transformedEnd1.z() / distance;
    if(dot > -0.8) return;
    
    double apparentRadius = radius / distance;
    int detailLevel = 1 + static_cast<int>( floor(
          PAINTER_CYLINDERS_DETAIL_COEFF
          * (sqrt(apparentRadius) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL)
          ) );
    if( detailLevel < 0 ) {
      detailLevel = 0;
    }
    if( detailLevel > PAINTER_MAX_DETAIL_LEVEL ) {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }
    d->cylinders[detailLevel]->draw(end1, end2, radius);
  }

  void Painter::drawMultiCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius, int order, double shift, int detailLevel ) const
  {
    assert( d->initialized );
    assert( detailLevel >= 0 && detailLevel <= PAINTER_MAX_DETAIL_LEVEL );
    d->cylinders[detailLevel]->drawMulti(end1, end2, radius, order,
        shift, d->widget->normalVector() );
  }

  void Painter::drawMultiCylinder( const Eigen::Vector3d &end1, const Eigen::Vector3d &end2,
      double radius, int order, double shift ) const
  {
    assert( d->initialized );
    Eigen::Vector3d transformedEnd1 = d->widget->camera()->modelview() * end1;
    double distance = transformedEnd1.norm();
    
    // perform a rough form of frustum culling
    double dot = transformedEnd1.z() / distance;
    if(dot > -0.8) return;
    
    double apparentRadius = radius / distance;
    int detailLevel = 1 + static_cast<int>( floor(
          PAINTER_CYLINDERS_DETAIL_COEFF
          * (sqrt(apparentRadius) - PAINTER_CYLINDERS_SQRT_LIMIT_MIN_LEVEL)
          ) );
    if( detailLevel < 0 ) {
      detailLevel = 0;
    }
    if( detailLevel > PAINTER_MAX_DETAIL_LEVEL ) {
      detailLevel = PAINTER_MAX_DETAIL_LEVEL;
    }
    d->cylinders[detailLevel]->drawMulti(end1, end2, radius, order,
        shift, d->widget->normalVector());
  }

  int Painter::drawText( int x, int y, const QString &string ) const
  {
    return d->textRenderer->draw(x, y, string);
  }

  int Painter::drawText( const QPoint& pos, const QString &string ) const
  {
    return d->textRenderer->draw(pos.x(), pos.y(), string);
  }

  int Painter::drawText( const Eigen::Vector3d &pos, const QString &string ) const
  {
    Eigen::Vector3d transformedPos = d->widget->camera()->modelview() * pos;
    
    // perform a rough form of frustum culling
    double dot = transformedPos.z() / transformedPos.norm();
    if(dot > -0.8) return 0;
    
    return d->textRenderer->draw(pos, string);
  }

  void Painter::beginText() const
  {
    d->textRenderer->begin();
  }

  void Painter::endText() const
  {
    d->textRenderer->end();
  }

} // end namespace Avogadro
