#ifndef PX_CG_ITEMS_FLOOR_HPP
#define PX_CG_ITEMS_FLOOR_HPP

#include "surface.hpp"

namespace px { namespace item {
    class Floor;
}}

class px::item::Floor : public item::Surface
{
public:
    float x_tiles;
    float y_tiles;
    std::string texture_name;
    
    Floor(float x_tiles=1.f, float y_tiles=1.f, std::string const &texture="floor7");
    ~Floor() override;

    void init() override;
    void render(Shader &shader) override;

protected:
    unsigned int vao, vbo, texture[4];
};

#endif