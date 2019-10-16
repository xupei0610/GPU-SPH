#ifndef PX_CG_GLM_HPP
#define PX_CG_GLM_HPP

#include <glm/glm.hpp>

#ifdef GLM_FORCE_RADIANS
#define GLM_ANG(a) (a)
#else
#define GLM_ANG(a) (glm::radians(a))
#endif

#endif // PX_CG_GLM_HPP
