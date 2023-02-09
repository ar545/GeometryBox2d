//
//  Star.h
//
#ifndef __STAR_H__
#define __STAR_H__

#include <cugl/physics2/CUPolygonObstacle.h>
#include <cugl/math/CUPoly2.h>
#include <cugl/math/CUVec2.h>

  /**
  star model
  */
  class Star : public cugl::physics2::PolygonObstacle {
  public:
      bool real;
  
  /**
 * Returns a (not necessarily convex) polygon
 *
 * The given polygon defines an implicit coordinate space. The body (and hence
 * the rotational center) will be placed at the given origin position.
 *
 * @param poly   The polygon vertices
 * @param origin The rotational center with respect to the vertices
 *
 * @return a (not necessarily convex) polygon
 */
      static std::shared_ptr<Star> alloc(const cugl::Poly2& poly, const cugl::Vec2 origin) {
          std::shared_ptr<Star> result = std::make_shared<Star>();
          return (result->init(poly, origin) ? result : nullptr);
      }

      void updatePhysics(float dt, int timecount, bool real);
     
  };

#endif /* __STAR_H__ */

