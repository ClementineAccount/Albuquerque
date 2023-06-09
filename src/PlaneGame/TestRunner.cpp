#pragma once 
#include "include/TestRunner.h"

//spdlog handles the logging for it more efficently by default so we ok
#include <iostream>
#include <assert.h>

namespace PlaneGame
{
	//Need to learn better ways to formalize this kinda test runners
	void ConfigReaderTester::TestOne()
	{
		std::cout << "Test One()\n";
		
		//Parse the file first
		ConfigReader instance;
		instance.ParseConfigFile("Data/Test.txt", true);

		//Retrieve the data expected
		std::string string_for_x;
		instance.GetDataString("x", string_for_x);

		//Check the string version matches
		assert(string_for_x == "10");
		
		//Check the data version matches
		int x = std::stoi(string_for_x);
		assert(x == 10);

		//Similar check but for the y value
		std::string string_for_y;
		instance.GetDataString("y", string_for_y);

		assert(string_for_y == "20");
		int y = std::stoi(string_for_y);
		assert(y == 20);

		//Now see if can fail gracefully if I try to search for the non-existant z
		std::string string_for_z;
		assert(instance.GetDataString("z", string_for_z) == false);

		std::cout << "Test One() Done\n";
	}



	void Tests::RunTests()
	{
		PlaneGame::ConfigReaderTester::TestOne();
	}

}