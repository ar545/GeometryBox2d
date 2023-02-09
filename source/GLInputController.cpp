//
//  GLInputController.cpp
//  Geometry Lab
//
//  This class buffers in input from the devices and converts it into its
//  semantic meaning. If your game had an option that allows the player to
//  remap the control keys, you would store this information in this class.
//  That way, the main game scene does not have to keep track of the current
//  key mapping.
//
//  Author: Walker M. White
//  Version: 1/20/22
//
#include <cugl/cugl.h>
#include "GLInputController.h"

using namespace cugl;

#pragma mark Input Control
/**
 * Creates a new input controller.
 *
 * This constructor DOES NOT attach any listeners, as we are not
 * ready to do so until the scene is created. You should call
 * the {@link #init} method to initialize the scene.
 */
InputController::InputController() :
_active(false),
_currDown(false),
_prevDown(false),
_mouseDown(false),
_mouseKey(0) {
}


/**
 * Initializes the control to support mouse or touch.
 *
 * This method attaches all of the listeners. It tests which
 * platform we are on (mobile or desktop) to pick the right
 * listeners.
 *
 * This method will fail (return false) if the listeners cannot
 * be registered or if there is a second attempt to initialize
 * this controller
 *
 * @return true if the initialization was successful
 */
bool InputController::init() {
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        mouse->setPointerAwareness(Mouse::PointerAwareness::DRAG);
        _mouseKey = mouse->acquireKey();
        mouse->addPressListener(_mouseKey,[=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonDownCB(event,clicks,focus);
        });
        mouse->addReleaseListener(_mouseKey,[=](const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
            this->buttonUpCB(event,clicks,focus);
        });
        mouse->addDragListener(_mouseKey,[=](const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
            this->motionCB(event,previous,focus);
        });
        _active = true;
    }
    Touchscreen* touch = Input::get<Touchscreen>();
    if (touch) {
        //touch->setPointerAwareness(Mouse::PointerAwareness::DRAG);
        _touchKey = touch->acquireKey();
        touch->addBeginListener(_touchKey, [=](const cugl::TouchEvent& event, bool focus) {
            this->touchDownCB(event, focus);
            });
        touch->addEndListener(_touchKey, [=](const cugl::TouchEvent& event, bool focus) {
            this->touchUpCB(event, focus);
            });
        touch->addMotionListener(_touchKey, [=](const cugl::TouchEvent& event, const Vec2 previous, bool focus) {
            this->touchmotionCB(event, previous, focus);
            });
        _active = true;
    }
    return _active;
}

void InputController::dispose() {
    if (_active) {
        Mouse* mouse = Input::get<Mouse>();
        if (mouse) {
            mouse->removePressListener(_mouseKey);
            mouse->removeReleaseListener(_mouseKey);
            mouse->removeDragListener(_mouseKey);
            mouse->setPointerAwareness(Mouse::PointerAwareness::BUTTON);
        }
        Touchscreen* touch = Input::get<Touchscreen>();
        if (touch) {
            touch->removeBeginListener(_touchKey);
            touch->removeEndListener(_touchKey);
            touch->removeMotionListener(_touchKey);
            //touch->setPointerAwareness();
        }
        _active = false;
    }
}

/**
 * Updates the input controller for the latest frame.
 *
 * It might seem weird to have this method given that everything
 * is processed with call back functions.  But we need some way
 * to synchronize the input with the animation frame.  Otherwise,
 * how can we know what was the touch location *last frame*?
 * Maybe there has been no callback function executed since the
 * last frame. This method guarantees that everything is properly
 * synchronized.
 */
void InputController::update() {
    _prevDown = _currDown;
    _prevPos = _currPos;
    Mouse* mouse = Input::get<Mouse>();
    if (mouse) {
        _currDown = _mouseDown;
        _currPos = _mousePos;
    }
    else {
        _currDown = _touchDown;
        _currPos = _touchPos;
    }
    
}

#pragma mark -
#pragma mark Mouse Callbacks
/**
 * Call back to execute when a mouse button is first pressed.
 *
 * This function will record a press only if the left button is pressed.
 *
 * @param event     The event with the mouse information
 * @param clicks    The number of clicks (for double clicking)
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::buttonDownCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    // Only recognize the left mouse button
    if (!_mouseDown && event.buttons.hasLeft()) {
        _mouseDown = true;
        _mousePos = event.position;
    }
}

/**
 * Call back to execute when a mouse button is first released.
 *
 * This function will record a release for the left mouse button.
 *
 * @param event     The event with the mouse information
 * @param clicks    The number of clicks (for double clicking)
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::buttonUpCB(const cugl::MouseEvent& event, Uint8 clicks, bool focus) {
    // Only recognize the left mouse button
    if (_mouseDown && event.buttons.hasLeft()) {
        _mouseDown = false;
    }
}

/**
 * Call back to execute when the mouse moves.
 *
 * This input controller sets the pointer awareness only to monitor a mouse
 * when it is dragged (moved with button down), not when it is moved. This
 * cuts down on spurious inputs. In addition, this method only pays attention
 * to drags initiated with the left mouse button.
 *
 * @param event     The event with the mouse information
 * @param previous  The previously reported mouse location
 * @param focus     Whether this device has focus (UNUSED)
 */
void InputController::motionCB(const cugl::MouseEvent& event, const Vec2 previous, bool focus) {
    if (_mouseDown) {
        _mousePos = event.position;
    }
}

void InputController::touchDownCB(const cugl::TouchEvent& event, bool focus)
{
    if (!_touchDown) {
        _touchDown = true;
        _touchPos = event.position;
    }
}

void InputController::touchUpCB(const cugl::TouchEvent& event, bool focus)
{
    // Only recognize the first finger
    if (_touchDown /* &&  TODO */) {
        _touchDown = false;
    }
}

void InputController::touchmotionCB(const cugl::TouchEvent& event, const cugl::Vec2 previous, bool focus)
{
    if (_touchDown) {
        _touchPos = event.position;
    }
}



