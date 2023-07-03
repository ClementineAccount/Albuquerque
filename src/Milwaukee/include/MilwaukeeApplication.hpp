#pragma once

#include <Albuquerque/Application.hpp>


#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string_view>
#include <vector>
#include <memory>
#include <queue>
#include <functional>
#include <chrono>


namespace Milwaukee
{



//Stolen from Fwog
class Timer
{
    using microsecond_t = std::chrono::microseconds;
    using myclock_t = std::chrono::high_resolution_clock;
    using timepoint_t = std::chrono::time_point<myclock_t>;
public:
    Timer()
    {
        timepoint_ = myclock_t::now();
    }

    void Reset()
    {
        timepoint_ = myclock_t::now();
    }

    double Elapsed_us() const
    {
        timepoint_t beg_ = timepoint_;
        return static_cast<double>(std::chrono::duration_cast<microsecond_t>(myclock_t::now() - beg_).count());
    }

private:
    timepoint_t timepoint_;
};



class DrawFrameBuffer
{
public:
    DrawFrameBuffer() = delete;
    DrawFrameBuffer(int32_t width, int32_t height);

    void DrawPixel(int32_t x, int32_t y, glm::vec4 color, int32_t x_offset = 0, int32_t y_offset = 0);

    void Resize(int32_t width, int32_t height);

    glm::vec4 clear_color{0.2f, 0.2f, 0.2f, 1.0f};
    uint32_t  fbo_id{0};
    uint32_t  tex_id{0};

    int32_t width;
    int32_t height;
};


//Pixel drawing canvas
class Canvas
{
public:
    Canvas() = delete;
    Canvas(int32_t width, int32_t height, int32_t origin_x = 0, int32_t origin_y = 0);


    int32_t width;
    int32_t height;

    int32_t origin_x = 0;
    int32_t origin_y = 0;

    static constexpr glm::vec4 default_clear_color{0.2f, 0.2f, 0.2f, 1.0f};
    glm::vec4 clear_color{0.2f, 0.2f, 0.2f, 1.0f};
    std::vector<glm::vec4> canvas_color_buffer;



    void DrawPixel(int32_t x, int32_t y, glm::vec4 color);
    void ClearCanvas(glm::vec4 color = default_clear_color);
    void DrawCanvasToFBO(DrawFrameBuffer& FBO) const;

    void Resize(int32_t set_width, int32_t set_height);
    void SetOrigin(int32_t origin_x, int32_t origin_y);
};

struct Light
{
    //Directional light by default
public:
    float intensity = 0.6f;
    glm::vec3 direction = glm::normalize(glm::vec3(1.0f, 3.0f, 3.0f));

};


struct Sphere
{
public:
    bool is_rendering = true;

    //considering the member variable option instead of pass by reference the sphere
    void intersection_check_ray(glm::vec3 ray_origin, glm::vec3 ray_direction);

    float radius;
    glm::vec3 center;
};

class RaycastScene
{


private:
    std::vector<Sphere> sphere_list;

    glm::vec4 background_color;

    //Could change sphere to surface I guess? It checks against the sphere_list anyways
    glm::vec2 IntersectRaySphere(glm::vec3 origin, glm::vec3 ray, glm::vec3 sphere_center, float sphere_radius);

    //float compute_lighting(glm::vec3 point, glm::vec3 normal, glm::vec3 v, Light light, float specular_power);

    void ClosestIntersection(glm::vec3 origin, glm::vec3 ray, float t_min, float t_max, float& closest_t, Sphere* closest_sphere);
    glm::vec4 TraceRay(glm::vec3 startPoint, glm::vec3 ray, float t_min, float t_max);

    static constexpr float inf = std::numeric_limits<float>::max();
};


class MilwaukeeApplication final : public Albuquerque::Application
{
protected:
    void AfterCreatedUiContext() override;
    void BeforeDestroyUiContext() override;

    bool Load() override;
    void RenderScene(double dt) override;
    void RenderUI(double dt) override;
    void Update(double dt) override;

private:

    //void CreateBuffers();
    void ClearFBO(uint32_t fbo, glm::vec4 color);
    void ClearCanvas();

    void DrawPixelsToScreen();

    //Render commands
    void DrawPixel(int32_t x, int32_t y, glm::vec4 color, int32_t x_offset = 0, int32_t y_offset = 0);
    void DrawPixelCentreOrigin(int32_t x, int32_t y, glm::vec4 color);
    void DrawPixelLineNaive(int32_t x_start, int32_t y_start, int32_t x_end, int32_t y_end, glm::vec4 color);
    void DrawLineBresenhamNaive(glm::i32vec2 start_pos, glm::i32vec2 end_pos, glm::vec4 color);
    void DrawLineBresenham(glm::i32vec2 start_pos, glm::i32vec2 end_pos, glm::vec4 color, bool centerOrigin = true);
    void DrawFilledSquare(glm::i32vec2 center, glm::vec4 color, int32_t length = 30, bool centerOrigin = false);

    void DrawFilledSquareCanvas(glm::i32vec2 center, glm::vec4 color, int32_t length = 30);

    void DrawPixel();

    //Some 'scenes' which are just collection of function calls
    void BuildSceneOneCommands();
    void RenderSceneOne();
    void RenderSceneTwo();

    void FillScreenBenchmarks();

    //Simple sphere projection with rays
    void RenderSceneThree();

    void RenderSpheresDelay(double dt);

    void RenderSpheresRealTime(double dt);

    void BrushControlCallback(double xoffset, double yoffset);

    bool MakeShader(std::string_view vertexShaderFilePath, std::string_view fragmentShaderFilePath);

private:

    uint32_t _shaderProgram;

    std::queue<std::function<void()>> renderCommandQueue;

    //for the default framebuffer
    glm::vec4 clear_screen_color_default_fbo{0.4f, 0.4f, 0.4f, 1.0f};
    glm::vec4 clear_screen_color{0.2f, 0.2f, 0.2f, 1.0f};

    uint32_t currently_binded_fbo;

    //default framebuffer
    uint32_t screen_draw_fbo = 0;

    static constexpr int32_t starting_brush_length = 30;
    int32_t current_brush_length;

    bool is_screen_dirty = true;

    bool is_rendering_paused = false;

    std::unique_ptr<DrawFrameBuffer> draw_framebuffer;
    std::unique_ptr<Canvas> draw_canvas;
    double elapsed_time_seconds = 0.0f;
};


}