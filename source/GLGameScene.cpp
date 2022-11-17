//
//  SLGameScene.cpp
//  Ship Lab
//
//  This is the primary class file for running the game.  You should study this file
//  for ideas on how to structure your own root class. This class is a reimagining of
//  the first game lab from 3152 in CUGL.
//
//  Author: Walker White
//  Based on original GameX Ship Demo by Rama C. Hoetzlein, 2002
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include <iostream>
#include <sstream>

#include "GLGameScene.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout

// Lock the screen size to fixed height regardless of aspect ratio
#define SCENE_HEIGHT 720

/** How big the spline extrusion should be */
#define LINE_WIDTH 50

/** How big the handle width should be */
#define HANDLE_WIDTH 3

/** How big the handle should be */
#define KNOB_RADIUS 15

/** The ratio between the physics world and the screen. */
#define PHYSICS_SCALE 50

/** The ratio between the physics world and the screen. */
#define GRAVITY 9.8

/** The initial control points for the spline. */
float CIRCLE[] = {    0,  200,  120,  200,
        200,  120,  200,    0,  200, -120,
        120, -200,    0, -200, -120, -200,
       -200, -120, -200,    0, -200,  120,
       -120,  200,    0,  200};


/** The (CLOCKWISE) polygon for the star */
float STAR[] = {     0,    50,  10.75,    17,   47,     17,
                 17.88, -4.88,   29.5, -40.5,    0, -18.33,
                 -29.5, -40.5, -17.88, -4.88,  -47,     17,
                -10.75,    17};

#pragma mark -
#pragma mark Constructors
/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GameScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Initialize the scene to a locked width
    Size dimen = Application::get()->getDisplaySize();
    dimen *= SCENE_HEIGHT/dimen.height;
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::init(dimen)) {
        return false;
    }

    Vec2* vec = reinterpret_cast<Vec2*>(CIRCLE);
    int num_points = ((sizeof CIRCLE) / (2 * sizeof(*CIRCLE)));
    // CULog("%d", num_points);
    _spline.set(vec, num_points);
    _spline.setClosed(true);
    

    buildGeometry();
    sel = -1;
    previous_timestep = 0.009f;

    _world = physics2::ObstacleWorld::alloc(Rect(Vec2(0, 0), (getSize() / PHYSICS_SCALE)), Vec2(0, -GRAVITY));
    addObstacles();

    _duplicate_world = physics2::ObstacleWorld::alloc(Rect(Vec2(0, 0), (getSize() / PHYSICS_SCALE)), Vec2(0, -GRAVITY));
    addDuplicateObstacles();

    // Start up the input handler
    _input.init();
    

    return true;
}

/** Add obstacles to the world */
void GameScene::addObstacles() {
    _world->clear();

    Poly2 spline_copy = Poly2(_spline_poly) / PHYSICS_SCALE;
    _center = cugl::physics2::PolygonObstacle::alloc(spline_copy, Vec2::ZERO);

    Poly2 star_copy = Poly2(_star_poly) / PHYSICS_SCALE;
    _star = (Star::alloc(star_copy, Vec2::ZERO));

    _center->setBodyType(b2_staticBody);
    _star->setBodyType(b2_dynamicBody);
    _star->setDensity(1);
    _center->setPosition(getSize() / (2 * PHYSICS_SCALE));
    _star->setPosition(getSize() / (2 * PHYSICS_SCALE));
    _star->setName("star");
    _world->addObstacle(_center);
    _world->addObstacle(_star);
}

/** Add obstacles to the world */
void GameScene::addDuplicateObstacles() {
    _duplicate_world->clear();

    Poly2 spline_copy = Poly2(_spline_poly) / PHYSICS_SCALE;
    _duplicate_center = cugl::physics2::PolygonObstacle::alloc(spline_copy, Vec2::ZERO);

    Poly2 star_copy = Poly2(_star_poly) / PHYSICS_SCALE;
    //_duplicate_star = cugl::physics2::PolygonObstacle::alloc(star_copy, Vec2::ZERO);
    _duplicate_star = Star::alloc(star_copy, Vec2::ZERO);

    _duplicate_center->setBodyType(b2_staticBody);
    _duplicate_star->setBodyType(b2_dynamicBody);
    _duplicate_star->setDensity(1);
    _duplicate_center->setPosition(getSize() / (2 * PHYSICS_SCALE));
    _duplicate_star->setPosition(getSize() / (2 * PHYSICS_SCALE));
    _duplicate_star->setName("star");
    _duplicate_world->addObstacle(_duplicate_center);
    _duplicate_world->addObstacle(_duplicate_star);
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    // NOTHING TO DO THIS TIME
}


#pragma mark -
#pragma mark Gameplay Handling
/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep) {
    // We always need to call this to update the state of input variables
    // This SYNCHRONIZES the call back functions with the animation frame.
    _input.update();

    //CULog("%f", timestep);

    Vec3 mouse_pos_vec3 = screenToWorldCoords(_input.getPosition());
    Vec2 mouse_pos_vec2 = Vec2(mouse_pos_vec3.x, mouse_pos_vec3.y).subtract(getSize() / 2);

    if (sel != -1) {
        Vec3 mouse_pre_vec3 = screenToWorldCoords(_input.getPrevious());
        Vec2 mouse_pre_vec2 = Vec2(mouse_pre_vec3.x, mouse_pre_vec3.y).subtract(getSize() / 2);
        _spline.setTangent(sel, Vec2().add(_spline.getTangent(sel)).add(mouse_pos_vec2).subtract(mouse_pre_vec2), true);
    }
    else {
        _world->update(timestep);
        _duplicate_world->update(previous_timestep);
        previous_timestep = timestep;
    }

    if (_input.didPress()) {
        for (int i = 0; i < 8; i++) {
            Vec2 v_control = _spline.getTangent(i);
            if (Vec2().add(v_control).subtract(mouse_pos_vec2).length() < KNOB_RADIUS) {
                sel = i;
                break;
            }
        }
    }

    _handles.clear();
    _knobs.clear();
    buildGeometry();

    if (_input.didRelease()) {
        sel = -1;
        _world->clear();
        addObstacles();
        _duplicate_world->clear();
        addDuplicateObstacles();
        _world->resetTime();
        _duplicate_world->resetTime();
    }

    //if (_world->getTime() % 100 == 0) {
    //    CULog("leftworld, %d, %f", _world->getTime(), _star->getPosition().x);
    //}
    //if (_duplicate_world->getTime() % 100 == 0) {
    //    CULog("rightworld, %d, %f", _duplicate_world->getTime(), _duplicate_star->getPosition().x);
    //}
    if (_world->getTime() == _duplicate_world->getTime()) {
        if (_star->getPosition().x != _duplicate_star->getPosition().x) {
            CULog("xpos, %f, %f", _star->getPosition().x, _duplicate_star->getPosition().x);
        }
        else if (_star->getPosition().y != _duplicate_star->getPosition().y) {
            CULog("ypos, %f, %f", _star->getPosition().y, _duplicate_star->getPosition().y);
        }
        else if (_star->getAngle() != _duplicate_star->getAngle()) {
            CULog("angle, %f, %f", _star->getAngle(), _duplicate_star->getAngle());
        }
        /*else {
            CULog("deterministic at %d", _world->getTime());
        }*/
    }

}

/**
 * Draws all this scene to the given SpriteBatch.
 *
 * The default implementation of this method simply draws the scene graph
 * to the sprite batch.  By overriding it, you can do custom drawing
 * in its place.
 *
 * @param batch     The SpriteBatch to draw with.
 */
void GameScene::render(const std::shared_ptr<cugl::SpriteBatch>& batch) {
   
    // DO NOT DO THIS IN YOUR FINAL GAME
    batch->begin(getCamera()->getCombined());
    batch->setColor(Color4::GRAY);
    batch->outline(_spline_path, getSize() / 2);
    batch->setColor(Color4::BLACK);
    batch->fill(_spline_poly, getSize() / 2);
    batch->setColor(Color4::WHITE);
    for (const Poly2& elem : _handles) {
        /* ... process elem ... */
        batch->fill(elem, getSize() / 2);
    }
    batch->setColor(Color4::RED);
    for (auto itr = _knobs.begin(); itr != _knobs.end(); ++itr) {
        /* ... process *itr ... */
        batch->fill(*itr, getSize() / 2);
    }
    batch->setColor(Color4::BLUE);
    // getAngle and translated by getPosition()*PHYSICS_SCALE
    // TODO: use draw angle and draw position instead of real angle and real position
    Affine2 transform = Affine2();
    transform.rotate(_star->getAngle()).translate((_star->getPosition())* PHYSICS_SCALE);
    batch->fill(_star_poly, Vec2::ZERO, transform); // set origin not to be 'getSize() / 2'
    if (_world->getTime() == _duplicate_world->getTime() && (_star->getAngle() != _duplicate_star->getAngle() || _star->getPosition() != _duplicate_star->getPosition())) {
        batch->setColor(Color4::GREEN);
        // TODO: use draw angle and draw position instead of real angle and real position
        Affine2 duplicate_transform = Affine2();
        duplicate_transform.rotate(_duplicate_star->getAngle()).translate((_duplicate_star->getPosition()) * PHYSICS_SCALE);
        batch->fill(_star_poly, Vec2::ZERO, duplicate_transform); // set origin not to be 'getSize() / 2'
    }
    // batch->drawText(to_string(previous_timestep), Font::Font(), getSize() / 2);
    batch->end();
}

/**
 * Rebuilds the geometry.
 *
 * This method should recreate all the polygons for the spline, the handles
 * and the falling star. It should also recreate all physics objects.
 *
 * However, to cut down on performance overhead, this method should NOT add
 * those physics objects to the world inside this method (as this method is
 * called repeatedly while the user moves a handle). Instead, those objects
 * should not be activated until the state is "stable".
 */
void GameScene::buildGeometry() {

    SplinePather sp = SplinePather();
    sp.set(&_spline);
    sp.calculate();
    _spline_path = sp.getPath();
    //_spline_path->set

    SimpleExtruder se = SimpleExtruder();
    se.set(_spline_path);
    // se.set(vec, num_points, true);
    se.calculate(LINE_WIDTH);
    _spline_poly = se.getPolygon();

    _handles = vector<Poly2>();
    _knobs = vector<Poly2>();

    for (int i = 0; i < 4; i++) {
        Vec2 v_left = _spline.getTangent(2 * i);
        Vec2 v_right = _spline.getTangent(i == 0 ? 7 : (2 * i - 1));
        Path2 path = Path2();
        path.add(0, v_left);
        path.add(1, v_right);

        // SimpleExtruder se = SimpleExtruder();
        se.set(path);
        se.calculate(HANDLE_WIDTH);
        Poly2 poly_handle = se.getPolygon();
        _handles.push_back(poly_handle);

        PolyFactory pf = PolyFactory();
        Poly2 poly_knob_left = pf.makeCircle(v_left, KNOB_RADIUS);
        Poly2 poly_knob_right = pf.makeCircle(v_right, KNOB_RADIUS);
        _knobs.push_back(poly_knob_left);
        _knobs.push_back(poly_knob_right);
    }

    Vec2* vec_star = reinterpret_cast<Vec2*>(STAR);
    int num_points_star = ((sizeof STAR) / (2 * sizeof(*STAR)));
    Path2 path_star = Path2(vec_star, num_points_star);
    path_star.reverse();
    EarclipTriangulator et = EarclipTriangulator();
    et.set(path_star);
    et.calculate();
    _star_poly = et.getPolygon();
}
