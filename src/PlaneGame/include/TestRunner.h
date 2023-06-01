//Why 'TestRunner.h'? So I don't pollute Main with a lot of header includes.
//I can call my test code here instead

#pragma once 

#include "ConfigReader.h"

namespace PlaneGame
{
    class Tests
    {
    public:
        static void RunTests();
    };

    //I need a better name for tester stuff but I'll refactor it later
    class ConfigReaderTester
    {
    public:
        static void TestOne();
    };
}