#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glfw.hpp"
#include "app.hpp"

#include "scenes/sph_scene.hpp"
#include "scene_helper/controllable_camera.hpp"
#include "items/floor.hpp"
#include "items/skybox.hpp"
#include "utils/random.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <iomanip>


class MainScene : public px::scene::SPHScene
{
public:
    bool pause;
    px::shader::Text text;

    explicit MainScene(SceneParameter scene_param)
        : SPHScene(scene_param),
          pause(false), camera_(nullptr)
    {}
    ~MainScene() override = default;

    void init() override
    {
        text.init();
        const unsigned char font_dat[] = {
            #include "font/Just_My_Type.dat"
        };
        text.setFont(font_dat, sizeof(font_dat), 40);

        camera_ = std::make_shared<px::scene_helper::ControllableCamera>();
        camera_->init();

        addItem(std::make_shared<px::item::Skybox>());

        px::scene::SPHScene::camera(std::shared_ptr<px::Camera>(camera_.get()));

        px::scene::SPHScene::init();
        resetCamera();
        px::App::instance()->showCursor(false);

        setupMenu();
    }

    void update(float dt) override
    {
        auto app = px::App::instance();
        if (app->keyReleased(px::App::Key::R)) restart();
        if (app->keyReleased(px::App::Key::B)) resetCamera();
        else camera_->update(dt);
        if (app->keyReleased(px::App::Key::P)) {
            pause = !pause;
            std::cout << camera()->position().x << " "
                      << camera()->position().y << " "
                      << camera()->position().z << " "
                      << camera()->roll() << " "
                      << camera()->pitch() << " "
                      << camera()->yaw() << std::endl;
        }
        if (pause)
        {
            px::scene::SPHScene::update(0.f);
            return;
        }
        if (app->keyReleased(px::App::Key::M))
        {
            use_cuda = !use_cuda;
            restart();
        }
        if (app->keyReleased(px::App::Key::T))
        {
            tap_open = !tap_open;
        }
        if (app->keyReleased(px::App::Key::Z))
        {
            show_box = !show_box;
        }
        if (app->keyReleased(px::App::Key::X))
        {
            show_obs = !show_obs;
        }
        if (app->keyReleased(px::App::Key::Left))
        {
            scene_param.n_obstacles--;
            if (scene_param.n_obstacles < 0)
                scene_param.n_obstacles = 0;
        }
        if (app->keyReleased(px::App::Key::Right))
        {
            scene_param.n_obstacles++;
        }
        if (app->keyReleased(px::App::Key::One))
        {
            scenery = Scenery::Empty;
            restart();
        }
        else if (app->keyReleased(px::App::Key::Two))
        {
            scenery = Scenery::Normal;
            restart();
        }
        else if (app->keyReleased(px::App::Key::Three))
        {
            scenery = Scenery::SurfaceTension;
            restart();
        }

        // process input events
        static auto last_key = px::App::Key::Unknown;
        static auto sum_dt = 0.f;
        static auto key_count = 0;
#define HOLD_KEY(Key)                                           \
    (last_key == Key && sum_dt > 0.01f && key_count == 10)
#define STICKY_KEY_CHECK(Key, Cmd)                              \
    if (app->keyPressed(Key))                  \
    {                                                           \
        if (last_key != Key || sum_dt > 0.1f || HOLD_KEY(Key))  \
        {                                                       \
            { Cmd }                                             \
            sum_dt = 0; if (key_count < 10) ++key_count;        \
        }                                                       \
        else sum_dt += dt;                                      \
        if (last_key != Key)                                    \
        { last_key = Key; key_count = 0; }                      \
    }
#define INCREASE_STEP_SIZE scene_param.step_size += .00002f;
#define DECREASE_STEP_SIZE if (scene_param.step_size > .00002f) scene_param.step_size -= 0.00002f;

        STICKY_KEY_CHECK(px::App::Key::Up, INCREASE_STEP_SIZE)
        else
        STICKY_KEY_CHECK(px::App::Key::Down, DECREASE_STEP_SIZE)
        else if (app->keyReleased(px::App::Key::H))
        {
            scene_param.pressure_force = !scene_param.pressure_force;
        }
        if (app->keyReleased(px::App::Key::J))
        {
            scene_param.viscosity_force = !scene_param.viscosity_force;
        }
        if (app->keyReleased(px::App::Key::K))
        {
            scene_param.surface_tension = !scene_param.surface_tension;
        }
        if (app->keyReleased(px::App::Key::L))
        {
            scene_param.gravity_force = !scene_param.gravity_force;
        }


        px::scene::SPHScene::update(dt);
    }
    void render() override
    {
        px::scene::SPHScene::render();
        constexpr float scale = .4f;
        const static glm::vec4 color(1.f);

        auto app = px::App::instance();

        auto h = 10.f;
        auto w = app->framebufferWidth() - 10;
        // fps, right top corner
        text.render("FPS: " + std::to_string(app->fps()),
                    w, h, scale, color,
                    px::shader::Text::Anchor::RightTop);
        // info
        text.render("Mode: " + std::string(isUseCuda() ? "GPU" : "CPU"),
                    10, h, scale, color,
                    px::shader::Text::Anchor::LeftTop);
        // h += 20;
        // text.render("Simulation Step Size: " + std::to_string(scene_param.step_size),
        //             10, h, scale, color,
        //             px::shader::Text::Anchor::LeftTop);
        h += 20;
        text.render("Particles: " + std::to_string(particles()),
                    10, h, scale, color,
                    px::shader::Text::Anchor::LeftTop);
        // h += 20;
        // text.render("Max Obstacles: " + std::to_string(scene_param.n_obstacles),
        //             10, h, scale, color,
        //             px::shader::Text::Anchor::LeftTop);
        h += 20;
        text.render("Pressure Force: " + std::string(scene_param.pressure_force ? "Enable" : "Disable"),
                    10, h, scale, color,
                    px::shader::Text::Anchor::LeftTop);
        h += 20;
        text.render("Viscosity Force: " + std::string(scene_param.viscosity_force ? "Enable" : "Disable"),
                    10, h, scale, color,
                    px::shader::Text::Anchor::LeftTop);
        h += 20;
        text.render("Surface Tension: " + std::string(scene_param.surface_tension ? "Enable" : "Disable"),
                    10, h, scale, color,
                    px::shader::Text::Anchor::LeftTop);
        h += 20;
        text.render("Gravity Force: " + std::string(scene_param.gravity_force ? "Enable" : "Disable"),
                    10, h, scale, color,
                    px::shader::Text::Anchor::LeftTop);
        // pause or not
        // if (pause)
        //     text.render("Pausing......",
        //                 10, app->framebufferHeight()-20, scale*1.2f, color,
        //                 px::shader::Text::Anchor::LeftBottom);

        // // prompt
        // h = app->framebufferHeight() - 25;
        // text.render("Press R to reset the scene",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press M to switch between GPU and CPU",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press Z to toggle the container",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press X to toggle the obstacles",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Hold Up and Down to adjust simulation step size",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press Left and Right to add or remove obstacles",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press T to toggle the tap",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press H to toggle pressure force",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press J to toggle viscosity force",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press K to toggle surface tension",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
        // h -= 20;
        // text.render("Press L to toggle gravity force",
        //             w, h, scale, color,
        //             px::shader::Text::Anchor::RightTop);
    }

    void setupMenu()
    {
        auto menu = px::App::instance()->menu();
        auto page = menu->addPage(px::Menu::Page());
        menu->page(page).addButton(px::Menu::Button(
            "pausss...iiing", 0, 0,
            1.f, glm::vec4(1.f, 1.f, 1.f, 1.f),
            px::shader::Text::Anchor::CenterTop,
            [](px::Menu::Button& btn)
            {
                btn.x = 0.5f * px::App::instance()->framebufferWidth();
                btn.y = 0.5f * px::App::instance()->framebufferHeight() - 100.f;
            }
        ));
        menu->page(page).addButton(px::Menu::Button(
            "resume", 0, 0,
            0.4f, glm::vec4(1.f, 1.f, 1.f, 1.f),
            px::shader::Text::Anchor::CenterTop,
            [](px::Menu::Button& btn)
            {
                btn.x = 0.5f * px::App::instance()->framebufferWidth();
                btn.y = 0.5f * px::App::instance()->framebufferHeight() - 20.f;

                auto btn_x = 0.5f * px::App::instance()->windowWidth();
                auto btn_y = 0.5f * px::App::instance()->windowHeight() - 20.f;
                auto cursor = px::App::instance()->cursorPosition();
                if (cursor.x > btn_x - 70.f && cursor.x < btn_x + 70.f &&
                    cursor.y > btn_y - 10.f && cursor.y < btn_y + 15.f)
                {
                    if (px::App::instance()->mouseReleased(px::App::MouseButton::Left))
                        px::App::instance()->toggleMenu();
                    else btn.font_scale = 0.6f;
                }
                else btn.font_scale = 0.4f;
            }
        ));
        menu->page(page).addButton(px::Menu::Button(
            "fullscreen", 0, 0,
            0.4f, glm::vec4(1.f, 1.f, 1.f, 1.f),
            px::shader::Text::Anchor::CenterTop,
            [](px::Menu::Button& btn)
            {
                btn.x = 0.5f * px::App::instance()->framebufferWidth();
                btn.y = 0.5f * px::App::instance()->framebufferHeight() + 20.f;
                auto btn_x = 0.5f * px::App::instance()->windowWidth();
                auto btn_y = 0.5f * px::App::instance()->windowHeight() + 20.f;
                auto cursor = px::App::instance()->cursorPosition();
                if (cursor.x > btn_x - 70.f && cursor.x < btn_x + 70.f &&
                    cursor.y > btn_y - 10.f && cursor.y < btn_y + 15.f)
                {
                    if (px::App::instance()->mouseReleased(px::App::MouseButton::Left))
                        px::App::instance()->setFullscreen(!px::App::instance()->fullscreen());
                    else btn.font_scale = 0.6f;
                }
                else btn.font_scale = 0.4f;
            }
        ));
        menu->page(page).addButton(px::Menu::Button(
            "quit", 0, 0,
            0.4f, glm::vec4(1.f, 1.f, 1.f, 1.f),
            px::shader::Text::Anchor::CenterTop,
            [](px::Menu::Button& btn)
            {
                btn.x = 0.5f * px::App::instance()->framebufferWidth();
                btn.y = 0.5f * px::App::instance()->framebufferHeight() + 60.f;
                auto btn_x = 0.5f * px::App::instance()->windowWidth();
                auto btn_y = 0.5f * px::App::instance()->windowHeight() + 60.f;
                auto cursor = px::App::instance()->cursorPosition();
                if (cursor.x > btn_x - 70.f && cursor.x < btn_x + 70.f &&
                    cursor.y > btn_y - 10.f && cursor.y < btn_y + 15.f)
                {
                    if (px::App::instance()->mouseReleased(px::App::MouseButton::Left))
                        px::App::instance()->closeWindow();
                    else btn.font_scale = 0.6f;
                }
                else btn.font_scale = 0.4f;
            }
        ));
    }

    void resetCamera()
    {
        camera()->position(glm::vec3(0.f, .25f, .75f));
        camera()->pitch(6.f);
        camera()->roll(0.f);
        camera()->yaw(0.f);
        camera_->forwardSpeed(camera_->FORWARD_SPEED*0.075f);
        camera_->backwardSpeed(camera_->BACKWARD_SPEED*0.075f);
        camera_->sidestepSpeed(camera_->SIDESTEP_SPEED*0.075f);
        camera_->upSpeed(camera_->UP_SPEED*0.075f);
    }

    MainScene(MainScene const &) = delete;
    MainScene(MainScene &&) = delete;
    MainScene &operator=(MainScene const &) = delete;
    MainScene &operator=(MainScene &&) = delete;

private:
    std::shared_ptr<px::scene_helper::ControllableCamera> camera_;
};


MainScene::SceneParameter parse(int argc, char *argv[])
{
    MainScene::SceneParameter scene_param
    (
            0.0008, 5, glm::vec3(0.f, -9.8f, 0.f)
    );

    if (argc == 2)
    {
#define PARSE_TRY(cmd, name, line)                                   \
    try                                                              \
    {                                                                \
        cmd;                                                         \
    }                                                                \
    catch (...)                                                      \
    {                                                                \
        throw std::invalid_argument("[Error] Failed to parse `" +    \
                                    std::string(name) +              \
                                    "` at line " +                   \
                                    std::to_string(line));           \
    }

#define PARAM_CHECK(name, param_size, param, line)                                          \
    if (param.size() < param_size + 1 ||                                                    \
        (param.size() > param_size + 1 && param[param_size+1][0]!='#'))                     \
    {                                                                                       \
        throw std::invalid_argument("[Error] Failed to parse `" +                           \
                                    std::string(name) +                                     \
                                    "` at line " +                                          \
                                    std::to_string(line) + " (" +                           \
                                    std::to_string(param_size) + " parameters expected, " + \
                                    std::to_string(param.size() - 1) + " provided)");       \
    }

#define S2D(var) std::stof(var)
#define S2I(var) std::stoi(var)

        std::string config_str;
        std::ifstream file(argv[1]);
        if (!file.is_open())
            throw std::invalid_argument("[Error] Failed to open scene file `" + std::string(argv[1]) + "`.");
        try
        {
            config_str.resize(file.seekg(0, std::ios::end).tellg());
            file.seekg(0, std::ios::beg).read(&config_str[0], static_cast<std::streamsize>(config_str.size()));
        }
        catch (...)
        {
            throw std::invalid_argument("[Error] Failed to read scene file `" + std::string(argv[1]) + "`.");
        }
        std::istringstream buffer(config_str);
        int ln = 0;
        std::string line;
        while (std::getline(buffer, line, '\n'))
        {
            ln++;
            auto cmd_s = std::find_if_not(line.begin(), line.end(), isspace);
            if (cmd_s == line.end() || *cmd_s == '#')
                continue;
            std::istringstream buf(line);
            std::vector<std::string> param;
            std::copy(
                std::istream_iterator<std::string>(buf), std::istream_iterator<std::string>(),
                std::back_inserter(param)
            );
             if (param[0] == "step_size")
            {
                PARAM_CHECK(param[0], 1, param, ln)
                PARSE_TRY(scene_param.step_size = std::abs(S2D(param[1])), param[0], ln);
            }
            else if (param[0] == "max_steps")
            {
                PARAM_CHECK(param[0], 1, param, ln)
                PARSE_TRY(scene_param.max_steps = std::abs(S2I(param[1])), param[0], ln);
            }
            else if (param[0] == "gravity")
            {
                PARAM_CHECK(param[0], 3, param, ln)
                PARSE_TRY(scene_param.gravity.x = S2D(param[1]), param[0], ln);
                PARSE_TRY(scene_param.gravity.y = S2D(param[2]), param[0], ln);
                PARSE_TRY(scene_param.gravity.z = S2D(param[3]), param[0], ln);
            }
            else
            {
                std::cout << "[WARN] skip unrecognized parameter `" << param[0] << "` at line " << ln << std::endl;
            }
        }
        std::cout << "[Info] Loaded scene file `" << argv[1] << "`" << std::endl;
    }
    else if (argc > 2)
    {
        std::cout << 
            "Usage:\n"
            "  " << argv[0] << " <scene_file>" <<
            std::endl;
        exit(1);
    }

    return scene_param;

}

int main(int argc, char *argv[])
{
    auto scene_param = parse(argc, argv);

    px::glfw::init();
    auto a = px::App::instance();
    a->setScene(std::make_shared<MainScene>(scene_param));
#if defined(__APPLE__)
    a->init(false);
#else
    a->init(true);
#endif
    while (a->run());
    a->close();

    px::glfw::terminate();
    return 0;
}
