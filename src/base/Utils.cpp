//
// Created by jan on 08/10/18.
//

#include <sstream>
#include <cmath>
#include "Utils.h"

namespace base {
    std::string Utils::FindDelimiter(std::string string, std::vector<std::string> delimiters) {
        for (std::string delimiter : delimiters) {
            std::size_t found = string.find(delimiter);
            if (found != std::string::npos) {
                return delimiter;
            }
        }

        return " ";
    }

    std::vector<int> Utils::StringToIntVector(std::string string, std::string delimiter) {
        std::vector<int> vector;
        std::istringstream ss(string);

        while (std::getline(ss, string, delimiter.c_str()[0])) {
            if (!string.empty()) {
                string.erase(remove_if(string.begin(), string.end(), isspace), string.end());
                string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
                vector.push_back(std::atoi(string.c_str()));
            }
        }
        return vector;
    }

    std::vector<std::size_t> Utils::StringToSizeVector(std::string string, std::string delimiter) {
        std::vector<std::size_t> vector;
        std::istringstream ss(string);

        while (std::getline(ss, string, delimiter.c_str()[0])) {
            if (!string.empty()) {
                string.erase(remove_if(string.begin(), string.end(), isspace), string.end());
                string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
                vector.push_back((std::size_t) std::atoi(string.c_str()));
            }
        }
        return vector;
    }

    std::vector<double> Utils::StringToDoubleVector(std::string string, std::string delimiter) {
        std::vector<double> vector;
        std::istringstream ss(string);

        while (std::getline(ss, string, delimiter.c_str()[0])) {
            if (!string.empty()) {
                string.erase(remove_if(string.begin(), string.end(), isspace), string.end());
                string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
                vector.push_back(std::atof(string.c_str()));
            }
        }
        return vector;
    }

    std::vector<std::string> Utils::StringToStringVector(std::string string, std::string delimiter) {
        std::vector<std::string> vector;
        std::istringstream ss(string);

        while (std::getline(ss, string, delimiter.c_str()[0])) {
            if (!string.empty()) {
                string.erase(remove_if(string.begin(), string.end(), isspace), string.end());
                string.erase(std::remove(string.begin(), string.end(), '\t'), string.end());
                vector.push_back(string);
            }
        }
        return vector;
    }

    std::string Utils::getTimeFromClocks(time_t clocks) {
        double seconds_dbl = ((double) clocks / (double) CLOCKS_PER_SEC);
        int hours = (int) floor(seconds_dbl / 3600.0);
        seconds_dbl -= hours * 3600.0;
        int minutes = (int) floor(seconds_dbl / 60.0);
        seconds_dbl -= minutes * 60;

        std::ostringstream output;
        if (hours > 0) {
            output << hours << " hours, ";
        }
        if (minutes > 0) {
            output << minutes << " minutes, ";
        }
        output << seconds_dbl << " seconds";
        return output.str();
    }

}