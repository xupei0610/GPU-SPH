#ifndef PX_CG_CAMERA_HPP
#define PX_CG_CAMERA_HPP

#include "glm.hpp"

namespace px
{
class Camera;
}

class px::Camera
{
public:
    const static float DEFAULT_FILM_HEIGHT;
    const static float DEFAULT_FILM_WIDTH;
    const static float DEFAULT_NEAR_CLIP;
    const static float DEFAULT_FAR_CLIP;
    const static float DEFAULT_FOV;

public:
    Camera();
    virtual ~Camera() = default;

    virtual void nearClip(float near);
    virtual void farClip(float far);
    virtual void clip(float near, float far);
    virtual void filmHeight(float height);
    virtual void filmWidth(float width);
    virtual void filmSize(float width, float height);
    virtual void fov(float fov);
    virtual void pitch(float pitch);
    virtual void yaw(float yaw);
    virtual void roll(float roll);
    virtual void position(glm::vec3 const &pos);
    inline void zoom(float dfov) { fov(fov() + dfov); }
    void rotate(float dpitch, float dyaw, float droll)
    { roll(roll() + droll); yaw(yaw() + dyaw); pitch(pitch() + dpitch);}
    void translate(glm::vec3 const &dpos) { position(position() + dpos); }

    virtual void updateProjection();
    virtual void updateView();

    inline float const filmHeight() const noexcept { return film_height_; }
    inline float const filmWidth() const noexcept { return film_width_; }
    inline float const nearClip() const noexcept { return near_clip_; }
    inline float const farClip() const noexcept { return far_clip_; }
    inline float const fov() const noexcept { return fov_; }
    inline float const pitch() const noexcept { return pitch_; }
    inline float const yaw() const noexcept { return yaw_; }
    inline float const roll() const noexcept { return roll_; }
    inline glm::vec3 const &position() const noexcept { return position_; }
    inline glm::vec3 const &forward()
    {
        if (force_update_view) { updateView(); force_update_view = false; }
        return forward_;
    }
    inline glm::vec3 const &strafe()
    {
        if (force_update_view) { updateView(); force_update_view = false; }
        return strafe_;
    }
    inline glm::vec3 const &up()
    {
        if (force_update_view) { updateView(); force_update_view = false; }
        return up_;
    }
    inline glm::mat4 const &view()
    {
        if (force_update_view) { updateView(); force_update_view = false; }
        return view_;
    }
    inline glm::mat4 const &projection()
    {
        if (force_update_projection) { updateProjection(); force_update_projection = false; }
        return projection_;
    }

protected:
    bool force_update_projection;
    bool force_update_view;

private:
    float film_width_;
    float film_height_;
    float near_clip_;
    float far_clip_;
    float fov_;
    float pitch_;
    float yaw_;
    float roll_;
    glm::vec3 position_;
    glm::vec3 forward_;
    glm::vec3 strafe_;
    glm::vec3 up_;
    glm::mat4 projection_;
    glm::mat4 view_;
};


#endif // PX_CG_CAMERA_HPP
