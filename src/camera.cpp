#include "camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>


using namespace px;

const float Camera::DEFAULT_FILM_HEIGHT = 640.f;
const float Camera::DEFAULT_FILM_WIDTH = 480.f;
const float Camera::DEFAULT_NEAR_CLIP = 0.01f;
const float Camera::DEFAULT_FAR_CLIP = 150.f;
const float Camera::DEFAULT_FOV = 45.f;

Camera::Camera()
    : force_update_projection(true),
      force_update_view(true),
      film_width_(DEFAULT_FILM_WIDTH),
      film_height_(DEFAULT_FILM_HEIGHT),
      near_clip_(DEFAULT_NEAR_CLIP),
      far_clip_(DEFAULT_FAR_CLIP),
      fov_(DEFAULT_FOV),
      pitch_(0),
      yaw_(90.f),
      roll_(0.f),
      position_(0.f, 0.f, -1.f)
{}

void Camera::nearClip(float near)
{
    force_update_projection = true;
    near_clip_ = near;
}

void Camera::farClip(float far)
{
    force_update_projection = true;
    far_clip_ = far;
}

void Camera::clip(float near, float far)
{
    nearClip(near);
    farClip(far);
}

void Camera::filmHeight(float height)
{
    force_update_projection = true;
    film_height_ = height;
}

void Camera::filmWidth(float width)
{
    force_update_projection = true;
    film_width_ = width;
}

void Camera::filmSize(float width, float height)
{
    filmWidth(width);
    filmHeight(height);
}

void Camera::fov(float fov)
{
    force_update_projection = true;
    fov_ = fov;
    if (fov_ > 89.f)
        fov_ = 89.f;
    else if (fov_ < 30.f)
        fov_ = 30.f;
}

void Camera::pitch(float pitch)
{
    force_update_view = true;
    pitch_ = pitch;
    if (pitch_ > 89.f)
        pitch_ = 89.f;
    else if (pitch_ < -89.f)
        pitch_ = -89.f;
}

void Camera::yaw(float yaw)
{
    force_update_view = true;
    yaw_ = yaw;
    if (yaw_ > 360.f)
        yaw_ -= 360.f;
    else if (yaw_ < -360.f)
        yaw_ += 360.f;
}

void Camera::roll(float roll)
{
    force_update_view = true;
    roll_ = roll;
    if (roll_ > 360.f)
        roll_ -= 360.f;
    else if (roll_ < -360.f)
        roll_ += 360.f;
}

void Camera::position(glm::vec3 const &pos)
{
    force_update_view = true;
    position_ = pos;
}

void Camera::updateProjection()
{
    projection_ = glm::perspectiveFov(GLM_ANG(fov()),
                                      filmWidth(), filmHeight(),
                                      nearClip(), farClip());

//    std::cout << "Projection Matrix" << std::endl;
//    std::cout << film_width_ << " " << film_height_ << " " << near_clip_ << " " << far_clip_ << " " << fov_ << std::endl;
//    std::cout << projection_[0][0] << " " << projection_[1][0] << " " << projection_[2][0] << " " << projection_[3][0] << std::endl;
//    std::cout << projection_[0][1] << " " << projection_[1][1] << " " << projection_[2][1] << " " << projection_[3][1] << std::endl;
//    std::cout << projection_[0][2] << " " << projection_[1][2] << " " << projection_[2][2] << " " << projection_[3][2] << std::endl;
//    std::cout << projection_[0][3] << " " << projection_[1][3] << " " << projection_[2][3] << " " << projection_[3][3] << std::endl;
}

void Camera::updateView()
{
    //    glm::quat orient = glm::quat(glm::vec3(GLM_ANG(_pitch), GLM_ANG(_yaw), 0));

    glm::quat pitch = glm::angleAxis(GLM_ANG(pitch_), glm::vec3(1.f,0.f,0.f));
    glm::quat yaw = glm::angleAxis(GLM_ANG(yaw_), glm::vec3(0.f,1.f,0.f));
    glm::quat orient = pitch * yaw;

    glm::mat4 rot = glm::mat4_cast(glm::normalize(orient));
    glm::mat4 trans = glm::translate(glm::mat4(1.f), -position());

    view_ = rot * trans;

    strafe_.x = -view_[0][0]; // strafe is the left direction
    strafe_.y = -view_[1][0];
    strafe_.z = -view_[2][0];
    forward_.x = -view_[0][2];
    forward_.y = -view_[1][2];
    forward_.z = -view_[2][2];
    up_.x = view_[0][1];
    up_.y = view_[1][1];
    up_.z = view_[2][1];

//    std::cout << "View Matrix" << std::endl;
//    std::cout << position().x  << " " << position().y << " " << position().z << std::endl;
//    std::cout << view_[0][0] << " " << view_[1][0] << " " << view_[2][0] << " " << view_[3][0] << std::endl;
//    std::cout << view_[0][1] << " " << view_[1][1] << " " << view_[2][1] << " " << view_[3][1] << std::endl;
//    std::cout << view_[0][2] << " " << view_[1][2] << " " << view_[2][2] << " " << view_[3][2] << std::endl;
//    std::cout << view_[0][3] << " " << view_[1][3] << " " << view_[2][3] << " " << view_[3][3] << std::endl;
}
