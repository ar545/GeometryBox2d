//
//  CUCapsuleObstacle.cpp
//  Cornell Extensions to Cocos2D
//
//  This class implements a capsule physics object. A capsule is a box with
//  semicircular ends along the major axis.  They are a popular physics objects,
//  particularly for character avatars.  The rounded ends means they are less
//  likely to snag, and they naturally fall off platforms when they go too far.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White
//  Version: 11/6/16
//
#include <cugl/physics2/CUCapsuleObstacle.h>
#include <cugl/math/polygon/CUPathFactory.h>

using namespace cugl::physics2;

/** How many line segments to use to draw a circle */
#define BODY_DEBUG_SEGS 12

/** Epsilon factor to prevent issues with the fixture seams */
#define DEFAULT_EPSILON 0.01


#pragma mark -
#pragma mark Constructors
/**
 * Initializes a new capsule object of the given dimensions.
 *
 * The orientation of the capsule is determined by the major axis. A `HALF`
 * capsule is rounded on the left for horizontal orientation and on the
 * bottom for vertical orientation.  A `HALF_REVERSE` capsule is the reverse.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param  pos      Initial position in world coordinates
 * @param  size     The capsule size (width and height)
 * @param  shape    The capsule shape/orientation
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool CapsuleObstacle::init(const Vec2 pos, const Size size, poly2::Capsule shape) {
    Obstacle::init(pos);
    _realcore = nullptr;
    _realcap1 = nullptr;
    _realcap2 = nullptr;
    _orient = shape;
    _seamEpsilon = (float)DEFAULT_EPSILON;
    resize(size);
    return true;
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Resets the polygon vertices in the shape to match the dimension.
 *
 * @param  size The new dimension (width and height)
 */
void CapsuleObstacle::resize(const Size size) {
    _dimension = size;

    // Get an AABB for the core
    float iw = size.width;
    float ih = size.height;
    if (size.width > size.height) {
        ih -= iw;
    }
    else {
        iw -= ih;
    }
    _center.upperBound.x = iw / 2.0f;
    _center.upperBound.y = ih / 2.0f;
    _center.lowerBound.x = -iw / 2.0f;
    _center.lowerBound.y = -ih / 2.0f;

    // Now adjust the core
    float r = 0;
    switch (_orient) {
    case poly2::Capsule::FULL:
        if (size.width > size.height) {
            r = size.height / 2.0f;
            _center.upperBound.x -= r;
            _center.lowerBound.x += r;
            _center.lowerBound.y += _seamEpsilon;
            _center.upperBound.y -= _seamEpsilon;
        }
        else {
            r = size.width / 2.0f;
            _center.upperBound.y -= r;
            _center.lowerBound.y += r;
            _center.lowerBound.x += _seamEpsilon;
            _center.upperBound.x -= _seamEpsilon;
        }
        break;
    case poly2::Capsule::HALF:
        if (size.width > size.height) {
            r = size.height / 2.0f;
            _center.upperBound.x -= r;
            _center.lowerBound.y += _seamEpsilon;
            _center.upperBound.y -= _seamEpsilon;
        }
        else {
            r = size.width / 2.0f;
            _center.lowerBound.y += r;
            _center.lowerBound.x += _seamEpsilon;
            _center.upperBound.x -= _seamEpsilon;
        }
        break;
    case poly2::Capsule::HALF_REVERSE:
        if (size.width > size.height) {
            r = size.height / 2.0f;
            _center.lowerBound.x += r;
            _center.lowerBound.y += _seamEpsilon;
            _center.upperBound.y -= _seamEpsilon;
        }
        else {
            r = size.width / 2.0f;
            _center.upperBound.y -= r;
            _center.lowerBound.x += _seamEpsilon;
            _center.upperBound.x -= _seamEpsilon;
        }
    case poly2::Capsule::DEGENERATE:
        r = size.width / 2.0f;
        _center.upperBound.x = 0.0f;
        _center.upperBound.y = 0.0f;
        _center.lowerBound.x = 0.0f;
        _center.lowerBound.y = 0.0f;
        break;
    }

    // Handle degenerate polys
    if (_center.lowerBound.x == _center.upperBound.x) {
        _center.lowerBound.x -= _seamEpsilon;
        _center.upperBound.x += _seamEpsilon;
    }
    if (_center.lowerBound.y == _center.upperBound.y) {
        _center.lowerBound.y -= _seamEpsilon;
        _center.upperBound.y += _seamEpsilon;
    }

    // Make the box for the core
    b2Vec2 corners[4];
    corners[0].x = _center.lowerBound.x;
    corners[0].y = _center.lowerBound.y;
    corners[1].x = _center.lowerBound.x;
    corners[1].y = _center.upperBound.y;
    corners[2].x = _center.upperBound.x;
    corners[2].y = _center.upperBound.y;
    corners[3].x = _center.upperBound.x;
    corners[3].y = _center.lowerBound.y;
    _shape.Set(corners, 4);

    _ends.m_radius = r;
    if (_debug != nullptr) {
        resetDebug();
    }

    markDirty(true);
}

/**
 * Sets the shape/orientation of this capsule.
 *
 * @param value  the shape/orientation of this capsule
 */
void CapsuleObstacle::setShape(poly2::Capsule value) {
    if (value != _orient) {
        _orient = value;
        resize(_dimension);
    }
}

/**
 * Redraws the outline of the physics fixtures to the debug node
 *
 * The debug node is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 *
 * Unfortunately, the current implementation is very inefficient.  Cocos2d
 * does not batch drawnode commands like it does Sprites or PolygonSprites.
 * Therefore, every distinct DrawNode is a distinct OpenGL call.  This can
 * really hurt framerate when debugging mode is on.  Ideally, we would refactor
 * this so that we only draw to a single, master draw node.  However, this
 * means that we would have to handle our own vertex transformations, instead
 * of relying on the transforms in the scene graph.
 */
void CapsuleObstacle::resetDebug() {
    PathFactory factory;
    Path2 path = factory.makeCapsule(_orient, Vec2::ZERO, _dimension);

    if (_debug == nullptr) {
        _debug = scene2::WireNode::allocWithPath(path);
        _debug->setColor(_dcolor);
        if (_scene != nullptr) {
            _scene->addChild(_debug);
        }
    }
    else {
        _debug->setPath(path);
    }

    _debug->setAnchor(Vec2::ANCHOR_CENTER);
    _debug->setPosition(getPosition());
}


#pragma mark -
#pragma mark Physics Methods
/**
 * Sets the density of this body
 *
 * The density is typically measured in usually in kg/m^2. The density can be zero or
 * positive. You should generally use similar densities for all your fixtures. This
 * will improve stacking stability.
 *
 * @param value  the density of this body
 */
void CapsuleObstacle::setDensity(float value) {
    _fixture.density = value;
    if (_realcore != nullptr && _drawcore != nullptr) {
        _realcore->SetDensity(value);
        _drawcore->SetDensity(value);
    }
    if (_realcap1 != nullptr && _drawcap1 != nullptr) {
        _realcap1->SetDensity(value / 2.0f);
        _drawcap1->SetDensity(value / 2.0f);
    }
    if (_realcap2 != nullptr && _drawcap2 != nullptr) {
        _realcap2->SetDensity(value / 2.0f);
        _drawcap2->SetDensity(value / 2.0f);
    }
    if (_realbody != nullptr && _drawbody != nullptr && !_masseffect) {
        _realbody->ResetMassData();
        _drawbody->ResetMassData();
    }
}

/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void CapsuleObstacle::createFixtures() {
    if (_realbody == nullptr || _drawbody == nullptr) {
        return;
    }

    releaseFixtures();
    poly2::Capsule trueOrient = (_dimension.width == _dimension.height) ? poly2::Capsule::DEGENERATE : _orient;

    // Create the core fixture
    if (trueOrient == poly2::Capsule::DEGENERATE) {
        _realcore = nullptr;
        _drawcore = nullptr;
    }
    else {
        _fixture.shape = &_shape;
        _realcore = _realbody->CreateFixture(&_fixture);
        _drawcore = _drawbody->CreateFixture(&_fixture);
    }

    _ends.m_p.Set(0, 0);
    switch (trueOrient) {
    case poly2::Capsule::FULL:
        _fixture.density = _fixture.density / 2.0f;
        if (_dimension.width > _dimension.height) {
            _ends.m_p.x = _center.lowerBound.x;
            _fixture.shape = &_ends;
            _realcap1 = _realbody->CreateFixture(&_fixture);
            _drawcap1 = _drawbody->CreateFixture(&_fixture);
            _ends.m_p.x = _center.upperBound.x;
            _fixture.shape = &_ends;
            _realcap2 = _realbody->CreateFixture(&_fixture);
            _drawcap2 = _drawbody->CreateFixture(&_fixture);
        }
        else {
            _ends.m_p.y = _center.upperBound.y;
            _fixture.shape = &_ends;
            _realcap1 = _realbody->CreateFixture(&_fixture);
            _drawcap1 = _drawbody->CreateFixture(&_fixture);
            _ends.m_p.y = _center.lowerBound.y;
            _fixture.shape = &_ends;
            _realcap2 = _realbody->CreateFixture(&_fixture);
            _drawcap2 = _drawbody->CreateFixture(&_fixture);
        }
        _fixture.density = _fixture.density * 2.0f;
        break;
    case poly2::Capsule::HALF:
        _fixture.density = _fixture.density / 2.0f;
        if (_dimension.width > _dimension.height) {
            _ends.m_p.x = _center.lowerBound.x;
            _fixture.shape = &_ends;
            _realcap1 = _realbody->CreateFixture(&_fixture);
            _drawcap1 = _drawbody->CreateFixture(&_fixture);
            _realcap2 = nullptr;
            _drawcap2 = nullptr;
        }
        else {
            _ends.m_p.y = _center.lowerBound.y;
            _fixture.shape = &_ends;
            _realcap1 = _realbody->CreateFixture(&_fixture);
            _drawcap1 = _drawbody->CreateFixture(&_fixture);
            _realcap2 = nullptr;
            _drawcap2 = nullptr;
        }
        _fixture.density = _fixture.density * 2.0f;
        break;
    case poly2::Capsule::HALF_REVERSE:
        _fixture.density = _fixture.density / 2.0f;
        if (_dimension.width > _dimension.height) {
            _ends.m_p.x = _center.upperBound.x;
            _fixture.shape = &_ends;
            _realcap1 = _realbody->CreateFixture(&_fixture);
            _drawcap1 = _drawbody->CreateFixture(&_fixture);
            _realcap2 = nullptr;
            _drawcap2 = nullptr;
        }
        else {
            _ends.m_p.y = _center.upperBound.y;
            _fixture.shape = &_ends;
            _realcap1 = _realbody->CreateFixture(&_fixture);
            _drawcap1 = _drawbody->CreateFixture(&_fixture);
            _realcap2 = nullptr;
            _drawcap2 = nullptr;
        }
        _fixture.density = _fixture.density * 2.0f;
        break;
    case poly2::Capsule::DEGENERATE:
        _fixture.shape = &_ends;
        _realcap1 = _realbody->CreateFixture(&_fixture);
        _drawcap1 = _drawbody->CreateFixture(&_fixture);
        _realcap2 = nullptr;
        _drawcap2 = nullptr;
        break;
    }

    markDirty(false);
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects
 */
void CapsuleObstacle::releaseFixtures() {
    if (_realcore != nullptr && _drawcore != nullptr) {
        _realbody->DestroyFixture(_realcore);
        _drawbody->DestroyFixture(_drawcore);
        _realcore = nullptr;
        _drawcore = nullptr;
    }
    if (_realcap1 != nullptr && _drawcap1 != nullptr) {
        _realbody->DestroyFixture(_realcap1);
        _drawbody->DestroyFixture(_drawcap1);
        _realcap1 = nullptr;
        _drawcap1 = nullptr;
    }
    if (_realcap2 != nullptr && _drawcap2 != nullptr) {
        _realbody->DestroyFixture(_realcap2);
        _drawbody->DestroyFixture(_drawcap2);
        _realcap2 = nullptr;
        _drawcap2 = nullptr;
    }
}

/**
 * Sets the seam offset of the core rectangle
 *
 * If the center rectangle is exactly the same size as the circle radius,
 * you may get catching at the seems.  To prevent this, you should make
 * the center rectangle epsilon narrower so that everything rolls off the
 * round shape. This parameter is that epsilon value
 *
 * @parm  value the seam offset of the core rectangle
 */
void CapsuleObstacle::setSeamOffset(float value) {
    CUAssertLog(value > 0, "The seam offset must be positive");
    _seamEpsilon = value; markDirty(true);
}
