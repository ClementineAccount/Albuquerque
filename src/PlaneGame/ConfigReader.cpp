#pragma once

#include <ConfigReader.h>
#include <TestRunner.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace PlaneGame
{
	void ConfigReader::ParseConfigFile(std::string_view filePath, bool outputContents)
	{
		std::ifstream file(filePath.data());
		if (file.is_open())
		{
			std::string buffer;
			while (std::getline(file, buffer))
			{
				if (outputContents)
					std::cout << buffer << std::endl;
				
				size_t setter_pos = buffer.find_first_of("=");

				//Ignore anything without that equal sign
				if (setter_pos != std::string::npos)
				{
					//To Do: After we profile the speed of parsing, this is an area of
					//optimization we can consider by not copying string values or something...
					//If we are comfortable with the spaces at the end

					std::string variable_name = buffer.substr(0, setter_pos - 1);
					
					//+2 is the right offset from equal sign and first whitespace
					std::string variable_value_string = buffer.substr(setter_pos + 2, buffer.size());
					variables_map.emplace(std::move(variable_name), std::move(variable_value_string));
				}
			}
		}
		file.close();
	}

	void ConfigReader::ReloadConfigFile(std::string_view filePath, bool outputContents)
	{
		std::ifstream file(filePath.data());
		if (file.is_open())
		{
			std::string buffer;
			while (std::getline(file, buffer))
			{
				if (outputContents)
					std::cout << buffer << std::endl;

				size_t setter_pos = buffer.find_first_of("=");

				//Ignore anything without that equal sign
				if (setter_pos != std::string::npos)
				{
					//To Do: After we profile the speed of parsing, this is an area of
					//optimization we can consider by not copying string values or something...
					//If we are comfortable with the spaces at the end

					std::string variable_name = buffer.substr(0, setter_pos - 1);

					//+2 is the right offset from equal sign and first whitespace

					//Why different? First load shouldn't bother to check. Only if reloading update values only if doesn't match
					std::string variable_value_string = buffer.substr(setter_pos + 2, buffer.size());
					if (variables_map.find(variable_name.data()) == variables_map.end())
					{
						variables_map.emplace(std::move(variable_name), std::move(variable_value_string));
					}
					else
					{
						//Although since it's string it might be faster to just update them anyways than bother with the string comparison
						variables_map.at(variable_name) = std::move(variable_value_string);
					}
				}
			}
		}
		file.close();

	}

	bool ConfigReader::GetDataString(std::string_view dataName, std::string& data_string)
	{
		//i dont even know if string view is used correctly here but im tired

		//Check if the data u seek even exists here
		if (variables_map.find(dataName.data()) == variables_map.end())
		{
			//if not i prob should output a proper message but just fail unintrusively
			std::cout << dataName << " not found in config file\n";
			return false;
		}

		data_string = variables_map.at(dataName.data());
		return true;
	}
}