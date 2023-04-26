#pragma once
#include <cstdint>
struct GLFWwindow;

namespace Albuquerque
{





    class Application
    {
    public:
        void Run();

    protected:

        static constexpr int windowWidth = 1600;
        static constexpr int windowHeight = 900;


        void Close();
        bool IsKeyPressed(int32_t key);
        bool IsKeyRelease(int32_t key);

        bool IsMouseKeyPressed(int32_t key);
        void GetMousePosition(double& mouseX, double& mouseY);

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


        void SetMouseCursorHidden(bool mouseHidden);
        void ToggleMouseCursorMode();

    private:
        GLFWwindow* _windowHandle = nullptr;
        void Render(double dt);
        bool cursor_hidden = false;
    };

}
