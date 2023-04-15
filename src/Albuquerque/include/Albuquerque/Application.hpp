#pragma once
#include <cstdint>

struct GLFWwindow;

class Application
{
public:
    void Run();

protected:

    static constexpr int windowWidth = 1600;
    static constexpr int windowHeight = 900;
   

    void Close();
    bool IsKeyPressed(int32_t key);
    virtual void AfterCreatedUiContext();
    virtual void BeforeDestroyUiContext();
    virtual bool Initialize();
    virtual bool Load();
    virtual void Unload();
    virtual void RenderScene();
    virtual void RenderUI(double dt);
    virtual void Update(double dt);

    //I think this only called once in awhile so copy is fine. No ownership anyways
    void SetWindowTitle(const char* winTitle);

private:
    GLFWwindow* _windowHandle = nullptr;
    void Render(double dt);
};