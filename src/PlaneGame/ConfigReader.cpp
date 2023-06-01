#pragma once

#include <ConfigReader.h>
#include <TestRunner.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace PlaneGame
{
	void ConfigReader::ParseConfigFile(std::string_view filePath)
	{
		std::ifstream file(filePath.data());
		if (file.is_open())
		{
			std::string buffer;
			while (file >> buffer)
			{
				//Check if it has the [category]

				std::cout << buffer << std::endl;
			}
		}
	}
}