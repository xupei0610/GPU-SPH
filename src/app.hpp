#ifndef PX_CG_APP_HPP
#define PX_CG_APP_HPP

#include <string>
#include <memory>
#include "glfw.hpp"
#include "menu.hpp"
#include "scene.hpp"

namespace px
{
class App;
}

class px::App
{
public:
    static const unsigned int DEFAULT_WINDOW_HEIGHT;
    static const unsigned int DEFAULT_WINDOW_WIDTH;
    static const char * DEFAULT_WINDOW_TITLE;

    static const GLFWwindowsizefun window_size_callback;
    static const GLFWframebuffersizefun framebuffer_size_callback;
    static const GLFWcursorposfun cursor_position_callback;
    static const GLFWkeyfun keypress_callback;
    static const GLFWmousebuttonfun mousebutton_callback;
    static const GLFWscrollfun scroll_callback;

public:
    enum class Key : int
    {
        W = GLFW_KEY_W,
        S = GLFW_KEY_S,
        A = GLFW_KEY_A,
        D = GLFW_KEY_D,
        Q = GLFW_KEY_Q,
        E = GLFW_KEY_E,
        M = GLFW_KEY_M,
        F = GLFW_KEY_F,
        L = GLFW_KEY_L,
        B = GLFW_KEY_B,
        P = GLFW_KEY_P,
        O = GLFW_KEY_O,
        N = GLFW_KEY_N,
        R = GLFW_KEY_R,
        I = GLFW_KEY_I,
        H = GLFW_KEY_H,
        J = GLFW_KEY_J,
        K = GLFW_KEY_K,
        U = GLFW_KEY_U,
        V = GLFW_KEY_V,
        G = GLFW_KEY_G,
        T = GLFW_KEY_T,
        Z = GLFW_KEY_Z,
        X = GLFW_KEY_X,
        Y = GLFW_KEY_Y,
        Up = GLFW_KEY_UP,
        One = GLFW_KEY_1,
        Two = GLFW_KEY_2,
        Three = GLFW_KEY_3,
        Four = GLFW_KEY_4,
        Down = GLFW_KEY_DOWN,
        Left = GLFW_KEY_LEFT,
        Right = GLFW_KEY_RIGHT,
        Shift = GLFW_KEY_LEFT_SHIFT,
        Space = GLFW_KEY_SPACE,
        Period = GLFW_KEY_PERIOD,
        Comma = GLFW_KEY_COMMA,
        Escape = GLFW_KEY_ESCAPE,
        Unknown = GLFW_KEY_UNKNOWN
    };
    enum class MouseButton : int
    {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT
    };

    static App* instance();
    virtual void init(bool fullscreen);
    virtual bool run();
    virtual void close();
    [[noreturn]]
    virtual void error(std::string const &msg);
    [[noreturn]]
    virtual void error(std::string const &msg, int code);

    virtual void setWindowSize(unsigned int width, unsigned int height);
    virtual void setWindowTitle(const char title[]);
    virtual void setWindowTitle(std::string title);
    virtual void setFullscreen(bool enable);
    virtual void setScene(std::shared_ptr<Scene> scene);
    virtual void setCursorPosition(float x, float y);
    virtual void showCursor(bool enable);
    virtual void toggleMenu();
    virtual void closeWindow();

    inline unsigned int windowHeight() const noexcept
    { return window_height_; }
    inline unsigned int windowWidth() const noexcept
    { return window_width_; }
    inline std::string const &windowTitle() const noexcept
    { return window_title_; }
    inline int framebufferWidth() const noexcept
    { return framebuffer_width_; }
    inline int framebufferHeight() const noexcept
    { return framebuffer_height_; }
    inline float timeGap() const noexcept { return time_gap; }
    inline int fps() const noexcept { return fps_; }
    inline bool fullscreen() const noexcept { return fullscreen_; }
    inline std::shared_ptr<Scene> scene() { return scene_; }
    inline bool onMenu() const noexcept { return on_menu_; }
    inline std::shared_ptr<Menu> menu() noexcept { return menu_; }
    bool cursorShown() noexcept;
    float getTime();

    bool keyTriggered(Key key);
    bool keyHold(Key key);
    bool keyPressed(Key key);
    bool keyReleased(Key key);
    bool mouseTriggered(MouseButton btn);
    bool mouseHold(MouseButton btn);
    bool mousePressed(MouseButton btn);
    bool mouseReleased(MouseButton btn);
    inline glm::vec2 const &cursorPosition() { return cursor_position_; }
    inline glm::vec2 const &scrollOffset() { return scroll_offset_; }


    virtual ~App();

protected:
    App();

    virtual void pollEvents();
    virtual void updateTimeGap();
    virtual void updateWindowSize(int width, int height);
    virtual void updateFramebufferSize(int width, int height);
    virtual void updateCursorPosition(float x, float y);
    virtual void updateKeyPressCondition(int glfw_key, int glfw_action);
    virtual void updateMousePressCondition(int glfw_key, int glfw_action);
    virtual void updateScrollOffset(float x_offset, float y_offset);

protected:
    GLFWwindow *window;
    float time_gap;

private:
    unsigned int window_width_;
    unsigned int window_height_;
    std::string window_title_;
    int framebuffer_width_;
    int framebuffer_height_;
    std::shared_ptr<Scene> scene_;
    glm::vec2 cursor_position_;
    glm::vec2 scroll_offset_;
    int fps_;
    bool fullscreen_;
    std::shared_ptr<Menu> menu_;
    bool on_menu_;

    unsigned int prev_window_width_;
    unsigned int prev_window_height_;
    int prev_window_pos_x_;
    int prev_window_pos_y_;

    std::array<bool, GLFW_KEY_LAST+1> last_key_pressed_;
    std::array<bool, GLFW_KEY_LAST+1> current_key_pressed_;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST+1> last_mouse_pressed_;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST+1> current_mouse_pressed_;
public:
    App &operator=(App const &) = delete;
    App &operator=(App &&) = delete;
};


#endif // PX_CG_APP_HPP