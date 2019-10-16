#include "menu.hpp"
#include "app.hpp"

using namespace px;

const int Menu::FONT_SIZE = 40;

std::size_t Menu::Page::addButton(Button const &btn)
{
    buttons.push_back(btn);
    return buttons.size()-1;
}

Menu::Button &Menu::Page::button(std::size_t id)
{
    return buttons.at(id);
}

void Menu::Button::render(shader::Text &text_shader)
{
    text_shader.render(title, x, y, font_scale, color, anchor);
}

std::size_t Menu::addPage(Page const &page)
{
    pages_.push_back(page);
    return pages_.size()-1;
}

Menu::Page & Menu::page(std::size_t id)
{
    return pages_.at(id);
}


Menu::Menu()
    : rectangle_shader(nullptr), text_shader(nullptr),
      fbo(0), bto(0), rbo(0), current_page_(0)
{
}

Menu::~Menu()
{
    glDeleteBuffers(1, &fbo);
    glDeleteTextures(1, &bto);
    glDeleteRenderbuffers(1, &rbo);
    delete rectangle_shader;
    delete text_shader;
}

void Menu::error(std::string const &msg)
{
    throw AppError("App Error: " + msg);
}

void Menu::init(int font_size)
{
    if (fbo == 0) glGenFramebuffers(1, &fbo);
    if (bto == 0) glGenTextures(1, &bto);
    if (rbo == 0) glGenRenderbuffers(1, &rbo);

    if (rectangle_shader == nullptr)
    {
        rectangle_shader = new shader::Rectangle;
        rectangle_shader->init();
    }
    if (text_shader == nullptr)
    {
        static const unsigned char font_dat[] = {
#include "font/North_to_South.dat"
        };
        text_shader = new shader::Text;
        text_shader->init();
        text_shader->setFont(font_dat, sizeof(font_dat), font_size);
    }
}

void Menu::activatePage(std::size_t page_id)
{
    if (page_id < pages_.size())
        current_page_ = page_id;
    else
        error("Failed to load menu page with id " + std::to_string(page_id));
}

void Menu::resize(unsigned int w, unsigned int h)
{
    glBindTexture(GL_TEXTURE_2D, bto);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGB, w, h,
                 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bto, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        error("Failed to generate frame buffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    frame_width_ = static_cast<float>(w);
    frame_height_ = static_cast<float>(h);
}

void Menu::update()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    App::instance()->scene()->render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Menu::render()
{
    auto app = App::instance();
    app->showCursor(true);

    rectangle_shader->render(-1.0f, -1.0f, 2.0f, 2.0f,
                             glm::vec4(0.0f, 0.0f, 0.0f, 0.75f), bto);
    
    if (currentPage() < pages_.size())
    {
        for (auto &b : pages_.at(currentPage()).buttons)
        {
            b.update();
            b.render(*text_shader);
        }
    }
}
