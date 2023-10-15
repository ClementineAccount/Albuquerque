//View Data
#include <Albuquerque/Camera.hpp>
#include <Albuquerque/FwogHelpers.hpp>
#include <Albuquerque/DrawObject.hpp>
#include <Albuquerque/Primitives.hpp>

//Temporarily here before I move it again
struct ViewData
{
    ViewData();

    struct ViewUniform {
        glm::mat4 viewProj;
        glm::vec3 eyePos;
    };

    std::optional<Fwog::TypedBuffer<ViewUniform>> viewBuffer;

    //Skybox doesn't have translation
    std::optional<Fwog::TypedBuffer<ViewUniform>> skyboxBuffer;

    void Update(Albuquerque::Camera const& camera);
};