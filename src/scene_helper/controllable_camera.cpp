#include "controllable_camera.hpp"
#include "app.hpp"


using namespace px;

const float scene_helper::ControllableCamera::FORWARD_SPEED = 5.f;
const float scene_helper::ControllableCamera::BACKWARD_SPEED = 2.5f;
const float scene_helper::ControllableCamera::SIDESTEP_SPEED = 3.f;
const float scene_helper::ControllableCamera::RUNNING_ACCEL = 2.f;
const float scene_helper::ControllableCamera::TURN_SPEED = 60.f;
const float scene_helper::ControllableCamera::MOUSE_SENSITIVITY = 1.f;
const float scene_helper::ControllableCamera::UP_SPEED = 3.f;

const App::Key scene_helper::ControllableCamera::FORWARD_KEY = App::Key::W;
const App::Key scene_helper::ControllableCamera::BACKWARD_KEY = App::Key::S;
const App::Key scene_helper::ControllableCamera::SIDESTEP_LEFT_KEY = App::Key::A;
const App::Key scene_helper::ControllableCamera::SIDESTEP_RIGHT_KEY = App::Key::D;
const App::Key scene_helper::ControllableCamera::TURN_LEFT_KEY = App::Key::Q;
const App::Key scene_helper::ControllableCamera::TURN_RIGHT_KEY = App::Key::E;
const App::Key scene_helper::ControllableCamera::UP_KEY = App::Key::Space;
const App::Key scene_helper::ControllableCamera::RUN_MODIFIER_KEY = App::Key::Shift;
const bool scene_helper::ControllableCamera::MOUSE_CONTROL = true;
const bool scene_helper::ControllableCamera::INVERT_X_AXIS = false;
const bool scene_helper::ControllableCamera::INVERT_Y_AXIS = true;

scene_helper::ControllableCamera::ControllableCamera()
    : Camera()
{}

void scene_helper::ControllableCamera::init()
{
    resetParams();
    resetShortcuts();
    mouse_detected = false;
}

void scene_helper::ControllableCamera::filmHeight(float height)
{
    mouse_detected = false;
    Camera::filmHeight(height);
}

void scene_helper::ControllableCamera::filmWidth(float width)
{
    mouse_detected = false;
    Camera::filmWidth(width);
}

void scene_helper::ControllableCamera::filmSize(float width, float height)
{
    mouse_detected = false;
    Camera::filmSize(width, height);
}


void scene_helper::ControllableCamera::update(float dt)
{
    auto app = App::instance();
    if (dt > 0)
    {
        // run
        auto acc_coef = app->keyTriggered(runModifierKey()) ? runningAccel() : 1.f;
        // for/backward
        if (app->keyPressed(forwardKey()) && !app->keyPressed(backwardKey()))
            translate(forward() * (forwardSpeed() * acc_coef * dt));
        else if (app->keyPressed(backwardKey()) && !app->keyPressed(forwardKey()))
            translate(forward() * (-backwardSpeed() * acc_coef * dt));
        // left/right sidestep
        if (app->keyPressed(sidestepLeftKey()) && !app->keyPressed(sidestepRightKey()))
            translate(strafe() * (sidestepSpeed()* acc_coef * dt));
        else if (app->keyPressed(sidestepRightKey()) && !app->keyPressed(sidestepLeftKey()))
            translate(strafe() * (-sidestepSpeed() * acc_coef * dt));
        // left/right turn
        if (app->keyPressed(turnLeftKey()) && !app->keyPressed(turnRightKey()))
            yaw(yaw() - turnSpeed()*dt);
        else if (app->keyPressed(turnRightKey()) && !app->keyPressed(turnLeftKey()))
            yaw(yaw() + turnSpeed()*dt);
        // up
        if (app->keyPressed(upKey()) || app->keyPressed(upKey()))
            translate(up() * (upSpeed()*dt));
    }

    if (mouseControl())
    {
        auto center_y = app->windowHeight() * .5f;
        auto center_x = app->windowWidth() * .5f;
        if (dt > 0)
        {
            if (mouse_detected)
            {
                auto x_offset = app->cursorPosition().x - center_x;
                auto y_offset = app->cursorPosition().y - center_y;
                if (invertXAxis())
                    yaw(yaw() - x_offset * mouseSensitivity() * dt);
                else
                    yaw(yaw() + x_offset * mouseSensitivity() * dt);
                if (invertYAxis())
                    pitch(pitch() + y_offset * mouseSensitivity() * dt);
                else
                    pitch(pitch() - y_offset * mouseSensitivity() * dt);
            }
            else
                mouse_detected = true;
            zoom(app->scrollOffset().y);
        }
        else 
             mouse_detected = false;
        app->setCursorPosition(center_x, center_y);
    }   
}

void scene_helper::ControllableCamera::resetParams()
{
    forwardSpeed(FORWARD_SPEED);
    backwardSpeed(BACKWARD_SPEED);
    sidestepSpeed(SIDESTEP_SPEED);
    runningAccel(RUNNING_ACCEL);
    turnSpeed(TURN_SPEED);
    upSpeed(UP_SPEED);
    invertXAxis(INVERT_X_AXIS);
    invertYAxis(INVERT_Y_AXIS);
    mouseSensitivity(MOUSE_SENSITIVITY);
}

void scene_helper::ControllableCamera::resetShortcuts()
{
    forwardKey(FORWARD_KEY);
    backwardKey(BACKWARD_KEY);
    sidestepLeftKey(SIDESTEP_LEFT_KEY);
    sidestepRightKey(SIDESTEP_RIGHT_KEY);
    turnLeftKey(TURN_LEFT_KEY);
    turnRightKey(TURN_RIGHT_KEY);
    upKey(UP_KEY);
    runModifierKey(RUN_MODIFIER_KEY);
    mouseControl(MOUSE_CONTROL);
}

void scene_helper::ControllableCamera::forwardKey(App::Key key)
{
    forward_key_ = key;
}

void scene_helper::ControllableCamera:: backwardKey(App::Key key)
{
    backward_key_ = key;
}
void scene_helper::ControllableCamera:: sidestepLeftKey(App::Key key)
{
    sidestep_left_key_ = key;
}
void scene_helper::ControllableCamera:: sidestepRightKey(App::Key key)
{
    sidestep_right_key_ = key;
}
void scene_helper::ControllableCamera:: turnLeftKey(App::Key key)
{
    turn_left_key_ = key;
}
void scene_helper::ControllableCamera:: turnRightKey(App::Key key)
{
    turn_right_key_ = key;
}
void scene_helper::ControllableCamera:: upKey(App::Key key)
{
    up_key_ = key;
}
void scene_helper::ControllableCamera:: runModifierKey(App::Key key)
{
    run_modifier_key_ = key;
}

void scene_helper::ControllableCamera::forwardSpeed(float sp)
{
    forward_speed_ = sp;
}

void scene_helper::ControllableCamera::backwardSpeed(float sp)
{
    backward_speed_ = sp;
}

void scene_helper::ControllableCamera::sidestepSpeed(float sp)
{
    sidestep_speed_ = sp;
}

void scene_helper::ControllableCamera::runningAccel(float acc)
{
    running_accel_ = acc;
}

void scene_helper::ControllableCamera::turnSpeed(float sp)
{
    turn_speed_ = sp;
}

void scene_helper::ControllableCamera::upSpeed(float sp)
{
    up_speed_ = sp;
}

void scene_helper::ControllableCamera::mouseControl(bool enable)
{
    mouse_control_ = enable;
}

void scene_helper::ControllableCamera::invertYAxis(bool enable)
{
    invert_y_axis_ = enable;
}

void scene_helper::ControllableCamera::invertXAxis(bool enable)
{
    invert_x_axis_ = enable;
}

void scene_helper::ControllableCamera::mouseSensitivity(float sen)
{
    mouse_sensitivity_ = sen;
}
