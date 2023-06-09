#include <ProjectApplication.hpp>
#include <TestRunner.h>



int main(int argc, char* argv[])
{

    Tutorials::ProjectApplication application;
    application.Run();
    return 0;
}


//This was for compiling a version that didn't have a console window. I bet there is a better way tho

//int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
//{
//    ProjectApplication application;
//    application.Run();
//    return 0;
//}
