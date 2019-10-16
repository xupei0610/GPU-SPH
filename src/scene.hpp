#ifndef PX_CG_SCENE_HPP
#define PX_CG_SCENE_HPP

#include <memory>
#include "camera.hpp"
#include "item.hpp"

namespace px
{
class Scene;
}

class px::Scene
{
public:
    Scene();
    virtual ~Scene();

    virtual void init();
    virtual void update(float dt);
    virtual void render();
    virtual void resize(unsigned int width, unsigned int height);

    inline void camera(std::shared_ptr<Camera> cam) {camera_ = cam;}
    inline std::shared_ptr<Camera> camera() noexcept { return camera_; }

    inline std::vector<std::shared_ptr<Item> > &items() noexcept { return items_; }
    virtual std::size_t addItem(std::shared_ptr<Item> const &item);

public:
    std::shared_ptr<Camera> camera_;
    std::vector<std::shared_ptr<Item> > items_;

protected:
    unsigned int camera_param_ubo;
};

#endif // PX_CG_SCENE_HPP
