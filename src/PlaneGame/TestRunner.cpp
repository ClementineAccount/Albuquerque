#pragma once 


#include "include/TestRunner.h"

//spdlog handles the logging for it more efficently by default so we ok
#include <iostream>

namespace PlaneGame
{
	void ConfigReaderTester::TestOne()
	{
		std::cout << "Test One()\n";
		ConfigReader instance;
		instance.ParseConfigFile("Data/Test.txt");
	}

	void Tests::RunTests()
	{
		PlaneGame::ConfigReaderTester::TestOne();
	}

}