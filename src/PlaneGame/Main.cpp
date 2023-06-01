#include <ProjectApplication.hpp>
#include <TestRunner.h>


int main(int argc, char* argv[])
{
    PlaneGame::Tests::RunTests();

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
