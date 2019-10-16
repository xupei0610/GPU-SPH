#ifndef PX_CG_MENU_HPP
#define PX_CG_MENU_HPP

#include <functional>
#include "shader.hpp"
#include "shaders/rectangle.hpp"
#include "shaders/text.hpp"

namespace px
{
class App;
class Menu;
}

class px::Menu
{
public:
    static const int FONT_SIZE;
    
    struct Button
    {
        std::string title;
        float x;
        float y;
        float font_scale;
        glm::vec4 color;
        shader::Text::Anchor anchor;
        std::function<void(Button&)> update_callback;

        Button(std::string const &title,
               float x, float y, float font_scale, glm::vec4 const &color, shader::Text::Anchor anchor,
               std::function<void(Button&)> update_fn=[](Button&){})
            : title(title), x(x), y(y), 
              font_scale(font_scale), color(color), anchor(anchor),
              update_callback(update_fn)
        {}
        void update() {update_callback(*this);}
        bool contain(glm::vec2 const &pos) const noexcept;
        bool contain(float x, float y) const noexcept;
        virtual void render(shader::Text &text_shader);
    };

    class Page
    {
    public:
        Page(){}
        std::size_t addButton(Menu::Button const & btn);
        Menu::Button &button(std::size_t id);
        std::vector<Menu::Button> buttons;
    };
public:

    Menu();

    void init(int font_size = FONT_SIZE);
    void update();
    void render();
    void resize(unsigned int w, unsigned int h);
    void activatePage(std::size_t page_id);
    inline std::size_t currentPage() const noexcept { return current_page_; }

    [[noreturn]]
    void error(std::string const & msg);

    std::size_t addPage(Page const &page);
    Page &page(std::size_t id);

    ~Menu();

protected:
    shader::Rectangle *rectangle_shader;
    shader::Text *text_shader;
    unsigned int fbo, bto, rbo;
private:
    std::size_t current_page_;
    std::vector<Page> pages_;
    float frame_width_, frame_height_;
};

#endif