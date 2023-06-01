//I can move this to Albuquerque main codebase later after prototyping
#pragma once

#include <string_view>
#include <string>
#include <unordered_map>

namespace PlaneGame {


    class ConfigReader
    {
    public:
        void ParseConfigFile(std::string_view filePath);

    private:
        std::unordered_map<std::string, std::string> variables_map;
    };
}

