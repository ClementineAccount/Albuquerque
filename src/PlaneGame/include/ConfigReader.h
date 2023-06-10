//I can move this to Albuquerque main codebase later after prototyping
#pragma once

#include <string_view>
#include <string>
#include <unordered_map>

namespace PlaneGame {


    class ConfigReader
    {
    public:
        void ParseConfigFile(std::string_view filePath, bool outputContents = false);


        // To Do: probably make some template version that lets u get the data directly in correct type but for now I let the other classes 
        // handle converting it to the type they like
        // Probably could return an error code instead of bool for failure but i dont care rn


       bool GetDataString(std::string_view dataName,  std::string& data_string);


       void ReloadConfigFile(std::string_view filePath, bool outputContents = false);

    private:
        std::unordered_map<std::string, std::string> variables_map;
    };
}

