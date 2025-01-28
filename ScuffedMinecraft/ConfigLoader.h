#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <variant>

namespace ConfigLoader {


    // Example Definitions, LEAVE bUseCustomConfiguration, its used to determine wether to leave the configMap Populated or not!
    // Currently Supported: int, bool and std::string
#define CONFIG_VARIABLES(_) \
        _(bool, bUseCustomConfiguration, false) \
        _(int, ExampleInt, 42) \
        _(std::string, ExampleString, "DefaultValue")

    namespace {
        struct ConfigVariable {
            using ConfigValue = std::variant<bool*, int*, std::string*>;
            ConfigValue value;
        };

        std::unordered_map<std::string, ConfigVariable> configMap;

#define DEFINE_VARIABLE(type, name, defaultValue) type name = defaultValue;
        CONFIG_VARIABLES(DEFINE_VARIABLE)
#undef DEFINE_VARIABLE

            std::string Trim(const std::string& str) {
            size_t first = str.find_first_not_of(" \t");
            if (first == std::string::npos)
                return "";
            size_t last = str.find_last_not_of(" \t");
            return str.substr(first, last - first + 1);
        }

        void AssignValue(const std::string& name, const std::string& value) {
            auto it = configMap.find(name);
            if (it == configMap.end()) {
                std::cerr << "Warning: Unknown configuration name '" << name << "'\n";
                return;
            }

            try {
                auto& var = it->second.value;
                if (auto* boolVar = std::get_if<bool*>(&var)) {
                    if (value == "true" || value == "1") {
                        **boolVar = true;
                    }
                    else if (value == "false" || value == "0") {
                        **boolVar = false;
                    }
                    else {
                        throw std::invalid_argument("Invalid bool value");
                    }
                }
                else if (auto* intVar = std::get_if<int*>(&var)) {
                    **intVar = std::stoi(value);
                }
                else if (auto* stringVar = std::get_if<std::string*>(&var)) {
                    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                        **stringVar = value.substr(1, value.size() - 2);
                    }
                    else {
                        throw std::invalid_argument("Invalid string format");
                    }
                }
            }
            catch (const std::exception&) {
                std::cerr << "Warning: Invalid value for '" << name << "': " << value << '\n';
            }
        }
    }

    void InitializeConfigMap() {
#define ADD_TO_CONFIG_MAP(type, name, defaultValue) configMap[#name] = {&name};
        CONFIG_VARIABLES(ADD_TO_CONFIG_MAP)
#undef ADD_TO_CONFIG_MAP
    }

    void LoadConfig(const std::string& filePath = "./Config/Config.ini") {
        std::ifstream configFile(filePath);
        if (!configFile) {
            std::cerr << "Error: Could not open config file: " << filePath << '\n';
            return;
        }

        std::string line;
        while (std::getline(configFile, line)) {
            line = Trim(line);
            if (line.empty() || line.front() == '[' || line.front() == ';') {
                continue;
            }

            size_t equalsPos = line.find('=');
            if (equalsPos == std::string::npos) {
                std::cerr << "Warning: Malformed line in config file: " << line << '\n';
                continue;
            }

            std::string name = Trim(line.substr(0, equalsPos));
            std::string value = Trim(line.substr(equalsPos + 1));

            AssignValue(name, value);
        }
    }

}