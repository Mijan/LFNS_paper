//
// Created by jan on 24/10/18.
//

#include <sstream>
#include "XmlPropertyMap.h"
#include "../base/Utils.h"

namespace io {
    XmlPropertyMap::XmlPropertyMap() : _map() {}

    XmlPropertyMap::XmlPropertyMap(XmlMap map) : _map(map) {}

    XmlPropertyMap::~XmlPropertyMap() {};

    std::string XmlPropertyMap::at(std::string key) {
        try {
            return _map.at(key).entry;
        } catch (const std::exception &e) {
            std::ostringstream os;
            os << "Accessing value for '" << key << "' failed!" << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    std::string XmlPropertyMap::safeAt(std::string key) {
        try {
            return _map.at(key).entry;
        } catch (const std::exception &e) {
            return "";
        }
    }

    std::string XmlPropertyMap::getValueForKey(std::string key_list_name, std::string key,
                                               std::string value_list_name) {
        std::vector<std::string> keys = base::Utils::StringToStringVector(at(key_list_name));
        std::vector<std::string> values = base::Utils::StringToStringVector(at(value_list_name));
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
            std::stringstream os;
            os << "Could not find value for " << key << " in list " << at(key_list_name) << std::endl;
            throw std::runtime_error(os.str());
        }
        if (values.size() == 1) { return values[0]; }

        if (keys.size() != values.size()) {
            std::ostringstream os;
            os << "Number of values " << values.size() << " does not match number of keys " << keys.size() <<
               std::endl << "\tValues: " << value_list_name << std::endl << "\tKeys: " << key_list_name << std::endl;
            throw std::runtime_error(os.str());
        }

        std::string value;
        bool key_found = false;
        for (std::size_t index = 0; index < keys.size(); index++) {
            if (keys[index].compare(key) == 0) {
                if (key_found) {
                    std::ostringstream os;
                    os << "Multiple keys " << key << " in list of keys:" << key_list_name << " found! " << std::endl;
                    throw std::runtime_error(os.str());
                }
                value = values[index];
                key_found = true;
            }
        }

        if (key_found) {
            return value;
        } else {
            std::ostringstream os;
            os << "Could not find key " << key << " in list of keys:" << key_list_name << std::endl;
            throw std::runtime_error(os.str());
        }
    }


    bool XmlPropertyMap::empty() { return _map.empty(); }
}