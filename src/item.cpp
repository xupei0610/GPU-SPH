#include "item.hpp"

using namespace px;

const glm::mat4 Item::IDENTITY_MAT = glm::mat4(1.f);
const glm::vec3 Item::X_AXIS = glm::vec3(1.f, 0.f, 0.f);
const glm::vec3 Item::Y_AXIS = glm::vec3(0.f, 1.f, 0.f);
const glm::vec3 Item::Z_AXIS = glm::vec3(0.f, 0.f, 1.f);

Item::Item()
{}

void Item::error(std::string const &msg)
{
    throw AppError(msg);
}

void Item::error(std::string const &msg, int code)
{
    throw AppError(msg, code);
}