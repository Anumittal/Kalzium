/**********************************************************************
  Color - Base class for handling color changes in OpenGL

  Copyright (C) 2006 Benoit Jacob
  Copyright (C) 2007 Geoffrey R. Hutchison

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

#ifndef COLOR_H
#define COLOR_H

#include <avogadro/global.h>
#include <avogadro/plugin.h>

#ifdef ENABLE_GLSL
  #include <GL/glew.h>
#endif

#include <QGLWidget> // for GLfloat
#include <QColor> // for returning QColor

#define AVOGADRO_COLOR(i, t, d)                 \
  public: \
    static QString staticIdentifier() { return i; }          \
    QString identifier() const { return i; }                 \
    static QString staticName() { return t; }                \
    QString name() const { if (m_name.isEmpty()) return t; else return m_name; } \
    static QString staticDescription() { return d; }         \
    QString description() const { return d; }

#define AVOGADRO_COLOR_FACTORY(c)         \
  public: \
    Plugin *createInstance(QObject *parent = 0) { Q_UNUSED(parent); return new c(); } \
    Plugin::Type type() const { return Plugin::ColorType; }; \
    QString identifier() const { return c::staticIdentifier(); } \
    QString name() const { return c::staticName(); }         \
    QString description() const { return c::staticDescription(); }

namespace Avogadro {

  class Primitive;
  class ColorPrivate; // for future expansion

  /**
   * @class Color color.h <avogadro/color.h>
   * @brief Map a primitive (atom, bond, etc.) to a OpenGL red, green, blue, alpha color
   * @author Benoit Jacob
   * @author Geoff Hutchison
   *
   * Color plugins are used to provide custom coloring for engines -- mapping atoms
   * and residues. Provide new plugins based on atomic or residue properties
   */
  class A_EXPORT Color : public Plugin
  {
    Q_OBJECT
  public:
    Color();
    virtual ~Color();

    /**
     * This constructor sets the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means totally
     * transparent and 1.0 (the default) means totally opaque.
     *
     * @param red The red component of the color
     * @param green The green component
     * @param blue The blue component
     * @param alpha The opacity of the color
     * \sa set()
     */
    Color(const GLfloat red, const GLfloat green, 
          const GLfloat blue, const GLfloat alpha = 1.0 );

    /**
     * Set the color based on the supplied Primitive.
     * If NULL is passed do nothing.
     * @param p the color is derived from this primitive.
     */
    Color( const Primitive *p);

    /**
     * Set the four components of the color
     * individually. Each one ranges from 0.0 (lowest intensity) to
     * 1.0 (highest intensity). For the alpha component, 0.0 means totally
     * transparent and 1.0 (the default) means totally opaque.
     *
     * @param red The red component of the color
     * @param green The green component
     * @param blue The blue component
     * @param alpha The opacity of the color
     */
    virtual void set(const GLfloat red, const GLfloat green, 
                     const GLfloat blue, const GLfloat alpha = 1.0 );

    /**
     * Set the color to the selection color
     * By default, the selection color is (0.3, 0.6, 1.0, 0.7)
     * which is a light transparent blue */
    virtual void setToSelectionColor();

    /**
     * Set the color based on the supplied Primitive.
     * If NULL is passed do nothing.
     * @param p the color is derived from this primitive.
     */
    virtual void set(const Primitive *p);

    /**
     * Set the color based on the supplied index (e.g., in an indexed color table)
     * @param index the color is derived from this value
     */
    virtual void setIndex(const unsigned int index);
    
    /**
     * Set the color based on the supplied floating point value (e.g., a gradient)
     * @param value the floating point number to consider
     * @param lo the low point of the range
     * @param mid the middle point of the range (e.g., 0.0)
     * @param hi the high point of the range
     */
    virtual void setGradient(const double value, const double lo, 
                             const double mid, const double hi);

    virtual void set(const QColor &color);

    /**
     * Set the alpha component of the color, 0.0 means totally transparent and
     * 1.0 means totally opaque.
     *
     * @param alpha The opacity of the color
     */
    virtual void setAlpha(double alpha);

    /**
     * Sets this color to be the one used by OpenGL for rendering
     * when lighting is disabled.
     */
    inline virtual void apply()
    {
      glColor4fv(m_channels);
    }

    /**
     * Applies nice OpenGL materials using this color as the
     * diffuse color while using different shades for the ambient and
     * specular colors. This is only useful if lighting is enabled.
     */
    virtual void applyAsMaterials();

    /**
     * Applies an OpenGL material more appropriate for flat surfaces.
     */
    virtual void applyAsFlatMaterials();

    /**
     * @return the color as a QColor.
     */
    inline QColor color() const
    { QColor returnColor;
      returnColor.setRgbF(m_channels[0],
                          m_channels[1],
                          m_channels[2],
                          m_channels[3]);
        return returnColor; }

    /**
     * @return the red component of the color.
     */
    inline float red() const { return m_channels[0]; }
    /**
     * @return the green component of the color.
     */
    inline float green() const { return m_channels[1]; }
    /**
     * @return the blue component of the color.
     */
    inline float blue() const { return m_channels[2]; }
    /**
     * @return the alpha component of the color.
     */
    inline float alpha() const { return m_channels[3]; }

    /**
     * Set the name of this instance of the class.
     */
    virtual void setName(const QString& name);
    /**
     * @return the translated name of this instance of the class.
     */
    virtual QString name() const;
    /**
     * @return the untranslated identifier of this color plugin
     */
    virtual QString identifier() const;
    /**
     * @return the type of the Color class.
     */
    virtual Plugin::Type type() const { return Plugin::ColorType; }

  Q_SIGNALS:
      /**
       * Signals that something has been changed and the engine needs to render
       * after these changes were made.
       */
      virtual void changed();

  protected:
    /**
     * \var GLfloat m_channels
     * The components of the color ranging from 0 to 1.
     */
    GLfloat m_channels[4];

    /**
     * The name of the class instance.
     */
    QString m_name;

    /**
     * The d-pointer used to preserve binary compatibility.
     */
    ColorPrivate *d;
  };

}

#endif
