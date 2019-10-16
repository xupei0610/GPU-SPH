#ifndef PX_CG_ITEMS_LIGHTING_BALL_HPP
#define PX_CG_ITEMS_LIGHTING_BALL_HPP

#include "shaders/lamp.hpp"
#include "item.hpp"
#include "sphere.hpp"
#include <memory>

namespace px {namespace item {
    class LightingBall;
    class LightingBallCollection;
}}

class px::item::LightingBall : public px::item::Sphere
{
public:
    LightingBall();
    ~LightingBall() override = default;
    virtual glm::vec3 color();
    virtual void color(glm::vec3 const &c);

    bool light() override {return true;}
    glm::vec3 ambient() override;
    void ambient(glm::vec3 const &c) override;
    glm::vec3 diffuse() override;
    void diffuse(glm::vec3 const &c) override;
    glm::vec3 specular() override;
    void specular(glm::vec3 const &c) override;
    glm::vec3 lightAttenuation() override;
    void lightAttenuation(glm::vec3 const &a) override;

private:
    glm::vec3 color_;
    glm::vec3 ambient_;
    glm::vec3 diffuse_;
    glm::vec3 specular_;
    glm::vec3 attenuation_;
};

class px::item::LightingBallCollection : public px::Item
{
public:
    float radius;

    LightingBallCollection();
    ~LightingBallCollection() override = default;

    void init() override;
    void post_render() override;

    std::size_t add(std::shared_ptr<LightingBall> b);
    inline std::vector<std::shared_ptr<LightingBall> > const &balls() {return balls_;}

    inline std::size_t size() { return position_.size(); }
    inline std::vector<glm::vec3> const &position() const noexcept { return position_; }
    inline std::vector<glm::vec3> const &color() const noexcept { return color_; }

    void position(std::size_t id, glm::vec3 const &p);
    void color(std::size_t id, glm::vec3 const &c);
    bool updatePosition() {return update_position_;}
    bool updateColor() {return update_color_;}

protected:
    shader::Lamp shader;
private:
    std::vector<glm::vec3> position_;
    std::vector<glm::vec3> color_;
    std::vector<std::shared_ptr<LightingBall> > balls_;
    bool update_color_;
    bool update_position_;
};

#endif
