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
			while (std::getline(file, buffer))
			{
				std::cout << buffer << std::endl;
				size_t setter_pos = buffer.find_first_of("=");

				//Ignore anything without that equal sign
				if (setter_pos != std::string::npos)
				{
					//To Do: After we profile the speed of parsing, this is an area of
					//optimization we can consider by not copying string values or something...
					
					//If we are comfortable with the spaces at the end
					std::string variable_name = buffer.substr(0, setter_pos - 1);
					std::string variable_value_string = buffer.substr(setter_pos + 1, buffer.size());

					variables_map.emplace(std::move(variable_name), std::move(variable_value_string));
				}
			}
		}
	}
}