#include "skybox.hpp"
#include "config.h"
#include "stb_image.h"

using namespace px;

item::Skybox::Skybox()
    : Item()
{}

void item::Skybox::init()
{
    int xp_w, xp_h;
    int xn_w, xn_h;
    int yp_w, yp_h;
    int yn_w, yn_h;
    int zp_w, zp_h;
    int zn_w, zn_h;
    int ch;

    constexpr auto right_face  = ASSET_PATH "/texture/skybox/right.jpg";
    constexpr auto left_face   = ASSET_PATH "/texture/skybox/left.jpg";
    constexpr auto top_face    = ASSET_PATH "/texture/skybox/top.jpg";
    constexpr auto bottom_face = ASSET_PATH "/texture/skybox/bottom.jpg";
    constexpr auto back_face   = ASSET_PATH "/texture/skybox/back.jpg";
    constexpr auto front_face  = ASSET_PATH "/texture/skybox/front.jpg";

    auto xp = stbi_load(right_face, &xp_w, &xp_h, &ch, 3);
    if (!xp) error("Failed to load texture: " + std::string(right_face));
    auto xn = stbi_load(left_face, &xn_w, &xn_h, &ch, 3);
    if (!xn) error("Failed to load texture: " + std::string(left_face));
    auto yp = stbi_load(top_face, &yp_w, &yp_h, &ch, 3);
    if (!yp) error("Failed to load texture: " + std::string(top_face));
    auto yn = stbi_load(bottom_face, &yn_w, &yn_h, &ch, 3);
    if (!yn) error("Failed to load texture: " + std::string(bottom_face));
    auto zp = stbi_load(back_face, &zp_w, &zp_h, &ch, 3);
    if (!zp) error("Failed to load texture: " + std::string(back_face));
    auto zn = stbi_load(front_face, &zn_w, &zn_h, &ch, 3);
    if (!zn) error("Failed to load texture: " + std::string(front_face));

    shader.init(xp, xp_w, xp_h, xn, xn_w, xn_h,
                         yp, yp_w, yp_h, yn, yn_w, yn_h,
                         zp, zp_w, zp_h, zn, zn_w, zn_h);

    stbi_image_free(xp);
    stbi_image_free(xn);
    stbi_image_free(yp);
    stbi_image_free(yn);
    stbi_image_free(zp);
    stbi_image_free(zn);
}

void item::Skybox::post_render()
{
    shader.activate(true);
    shader.render();
    shader.activate(false);
}
