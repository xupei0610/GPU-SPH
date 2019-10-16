#ifndef PX_CG_ITEMS_PILLAR_HPP
#define PX_CG_ITEMS_PILLAR_HPP

#include "item.hpp"

namespace px { namespace item {
    class Pillar;
}}

class px::item::Pillar : public Item
{
public:
    Material material;

    explicit Pillar(Material && mat);
    explicit Pillar(Material const& mat);
    ~Pillar() override;

    void init() override;
    void render(Shader &shader) override;

    glm::vec3 pos() override;
    void pos(glm::vec3 const &p) override;
    glm::vec3 scal() override;
    void scal(glm::vec3 const &s) override;
    glm::vec3 rot() override;
    void rot(glm::vec3 const &r) override;
    
protected:
    unsigned int vao[3], vbo[6];

    inline glm::mat4 &model() {return model_;}
    void updateModel();

private:
    int _n_indices;
    glm::vec3 pos_;
    glm::vec3 scal_;
    glm::vec3 rot_;
    glm::mat4 model_;
};


#endif
