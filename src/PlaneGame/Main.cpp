#include <ProjectApplication.hpp>
#include <TestRunner.h>

//prob should change this to something else, argc variable or something?
constexpr bool runTests = false;


int main(int argc, char* argv[])
{
    if (runTests)
    {
        PlaneGame::Tests::RunTests();
    }

    PlaneGame::ProjectApplication application;
    application.Run();
    return 0;
}


//int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
//{
//    ProjectApplication application;
//    application.Run();
//    return 0;
//}
