//
// Created by jan on 08/10/18.
//

#ifndef LFNS_UTILS_H
#define LFNS_UTILS_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>

namespace base {
    class Utils {
    public:
        static std::string FindDelimiter(std::string string, std::vector<std::string> delimiters = {",", " ", "\t"});

        static std::vector<int> StringToIntVector(std::string string) {
            return StringToIntVector(string, FindDelimiter(string));
        }

        static std::vector<std::size_t> StringToSizeVector(std::string string) {
            return StringToSizeVector(string, FindDelimiter(string));
        }

        static std::vector<double> StringToDoubleVector(std::string string) {
            return StringToDoubleVector(string, FindDelimiter(string));
        }

        static std::vector<std::string> StringToStringVector(std::string string) {
            return StringToStringVector(string, FindDelimiter(string));
        }

        static std::vector<int> StringToIntVector(std::string string, std::string delimiter);

        static std::vector<std::size_t> StringToSizeVector(std::string string, std::string delimiter);

        static std::vector<double> StringToDoubleVector(std::string string, std::string delimiter);

        static std::vector<std::string> StringToStringVector(std::string string, std::string delimiter);

        static std::string getTimeFromClocks(time_t clocks);

        template<typename T>
        static void addOnlyNew(std::vector<T> &original_vector, const std::vector<T> &new_vector) {
            for (std::size_t i = 0; i < new_vector.size(); i++) {
                if (std::find(original_vector.begin(), original_vector.end(), new_vector[i]) == original_vector.end()) {
                    original_vector.push_back(new_vector[i]);
                }
            }
        }

        template<typename T>
        static bool elementsOverlap(std::vector<T> &original_vector, const std::vector<T> &new_vector) {
            for (std::size_t i = 0; i < new_vector.size(); i++) {
                if (std::find(original_vector.begin(), original_vector.end(), new_vector[i]) != original_vector.end()) {
                   return true;
                }
            }
            return false;
        }

        template<typename T>
        static std::vector<T> getOverlappingElements(std::vector<T> &original_vector, const std::vector<T> &new_vector) {
            std::vector<T> overlapping_elements;
            for (std::size_t i = 0; i < new_vector.size(); i++) {
                if (std::find(original_vector.begin(), original_vector.end(), new_vector[i]) != original_vector.end()) {
                    overlapping_elements.push_back(new_vector[i]);
                }
            }
            return overlapping_elements;
        }

        template<typename Identifier>
        static std::map<Identifier, std::size_t> getIndexMapFromVector(std::vector<Identifier> vector) {
            std::map<Identifier, std::size_t> map;
            for (std::size_t i = 0; i < vector.size(); i++) {
                map.insert(std::make_pair(vector[i], i));
            }
            return map;
        };
    };
}
#endif //LFNS_UTILS_H
