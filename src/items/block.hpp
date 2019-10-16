#ifndef PX_CG_ITEMS_BLOCK_HPP
#define PX_CG_ITEMS_BLOCK_HPP

#include "item.hpp"

namespace px { namespace item {
class Block;
}}

class px::item::Block : public px::Item
{
public:
    Material material;

    explicit Block(Material && mat);
    explicit Block(Material const& mat);
    ~Block() override;

    glm::vec3 pos() override;
    void pos(glm::vec3 const &p) override;
    glm::vec3 scal() override;
    void scal(glm::vec3 const &s) override;
    glm::vec3 rot() override;
    void rot(glm::vec3 const &s) override;

    void init() override;
    void render(Shader &shader) override;

protected:
    inline glm::mat4 &model() {return model_;}
    void updateModel();

protected:
    unsigned int vao, vbo[3];
private:
    std::size_t n_indices_;


    glm::vec3 pos_;
    glm::vec3 scal_;
    glm::vec3 rot_;
    glm::mat4 model_;
};

#endif //PX_CG_ITEMS_BLOCK_HPP
