#ifndef PX_CG_ITEMS_BALL_HPP
#define PX_CG_ITEMS_BALL_HPP

#include "sphere.hpp"

namespace px { namespace item {
    class Ball;
    class TextureBall;
}}

class px::item::Ball : public px::item::Sphere
{
public:
    Material material;

    explicit Ball(Material && mat);
    explicit Ball(Material const& mat);
    ~Ball() override;

    void init() override;
    void render(Shader &shader) override;

protected:
    unsigned int vao, vbo[3];
private:
    std::size_t n_indices_;
};

class px::item::TextureBall : public px::item::Sphere
{
public:
    TextureBall();
    ~TextureBall() override;

    void init() override;
    void render(Shader &shader) override;

protected:
    unsigned int vao, vbo[5], texture[4];
private:
    std::size_t n_indices_;
};


#endif
