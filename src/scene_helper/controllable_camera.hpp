#ifndef PX_CG_SCENE_HELPER_CONTROLLABLE_CAMERA
#define PX_CG_SCENE_HELPER_CONTROLLABLE_CAMERA

#include "camera.hpp"
#include "app.hpp"

namespace px { namespace scene_helper {
class ControllableCamera;
}}

class px::scene_helper::ControllableCamera : public px::Camera
{
public:
    static const float FORWARD_SPEED;
    static const float BACKWARD_SPEED;
    static const float SIDESTEP_SPEED;
    static const float RUNNING_ACCEL;
    static const float TURN_SPEED;
    static const float UP_SPEED;
    static const bool MOUSE_CONTROL;
    static const bool INVERT_X_AXIS;
    static const bool INVERT_Y_AXIS;
    static const float MOUSE_SENSITIVITY;

    static const App::Key FORWARD_KEY;
    static const App::Key BACKWARD_KEY;
    static const App::Key SIDESTEP_LEFT_KEY;
    static const App::Key SIDESTEP_RIGHT_KEY;
    static const App::Key TURN_LEFT_KEY;
    static const App::Key TURN_RIGHT_KEY;
    static const App::Key UP_KEY;
    static const App::Key RUN_MODIFIER_KEY;

public:
    ControllableCamera();
    ~ControllableCamera() override = default;

    virtual void init();
    virtual void update(float dt);

    void filmHeight(float height) override;
    void filmWidth(float width) override;
    void filmSize(float width, float height) override;

    virtual void resetParams();
    virtual void resetShortcuts();
    virtual void forwardKey(App::Key key);
    virtual void backwardKey(App::Key key);
    virtual void sidestepLeftKey(App::Key key);
    virtual void sidestepRightKey(App::Key key);
    virtual void turnLeftKey(App::Key key);
    virtual void turnRightKey(App::Key key);
    virtual void upKey(App::Key key);
    virtual void runModifierKey(App::Key key);
    virtual void forwardSpeed(float sp);
    virtual void backwardSpeed(float sp);
    virtual void sidestepSpeed(float sp);
    virtual void runningAccel(float acc);
    virtual void turnSpeed(float sp);
    virtual void upSpeed(float sp);
    virtual void mouseControl(bool enable);
    virtual void invertXAxis(bool enable);
    virtual void invertYAxis(bool enable);
    virtual void mouseSensitivity(float sen);

    inline App::Key const forwardKey() const noexcept { return forward_key_; }
    inline App::Key const backwardKey() const noexcept { return backward_key_; }
    inline App::Key const sidestepLeftKey() const noexcept { return sidestep_left_key_; }
    inline App::Key const sidestepRightKey() const noexcept { return sidestep_right_key_; }
    inline App::Key const turnLeftKey() const noexcept { return turn_left_key_; }
    inline App::Key const turnRightKey() const noexcept { return turn_right_key_; }
    inline App::Key const upKey() const noexcept { return up_key_; }
    inline App::Key const runModifierKey() const noexcept { return run_modifier_key_; }
    inline float const forwardSpeed() const noexcept { return forward_speed_; }
    inline float const backwardSpeed() const noexcept { return backward_speed_; }
    inline float const sidestepSpeed() const noexcept { return sidestep_speed_; }
    inline float const runningAccel() const noexcept { return running_accel_; }
    inline float const turnSpeed() const noexcept { return turn_speed_; }
    inline float const upSpeed() const noexcept { return up_speed_; }
    inline bool const mouseControl() const noexcept { return mouse_control_; }
    inline bool const invertXAxis() const noexcept { return invert_x_axis_; }
    inline bool const invertYAxis() const noexcept { return invert_y_axis_; }
    inline float const mouseSensitivity() const noexcept { return mouse_sensitivity_; }

protected:
    bool mouse_detected;

private:
    App::Key forward_key_;
    App::Key backward_key_;
    App::Key sidestep_left_key_;
    App::Key sidestep_right_key_;
    App::Key turn_left_key_;
    App::Key turn_right_key_;
    App::Key up_key_;
    App::Key run_modifier_key_;
    float forward_speed_;
    float backward_speed_;
    float sidestep_speed_;
    float running_accel_;
    float turn_speed_;
    float up_speed_;
    bool mouse_control_;
    bool invert_x_axis_;
    bool invert_y_axis_;
    float mouse_sensitivity_;
};
#endif
