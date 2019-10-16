#ifndef PX_CG_ITEMS_SKYBOX_HPP
#define PX_CG_ITEMS_SKYBOX_HPP

#include "item.hpp"
#include "shaders/skybox.hpp"

namespace px {namespace item {
    class Skybox;
}}

class px::item::Skybox : public px::Item
{
public:
    Skybox();
    ~Skybox() override = default;

    void init() override;
    void post_render() override;
protected:
    shader::Skybox shader;
    
};

#endif