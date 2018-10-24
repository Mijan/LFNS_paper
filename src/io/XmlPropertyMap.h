//
// Created by jan on 24/10/18.
//

#ifndef LFNS_XMLPROPERTYMAP_H
#define LFNS_XMLPROPERTYMAP_H


#include <string>
#include <map>
#include <iostream>
#include "XmlFileReader.h"

namespace io {
    class XmlPropertyMap {
    public:
        XmlPropertyMap();

        XmlPropertyMap(XmlMap map);

        virtual ~XmlPropertyMap();

        std::string at(std::string);

        std::string safeAt(std::string);

        std::string getValueForKey(std::string key_list_name, std::string key, std::string value_list_name);

        bool empty();

        friend std::ostream &operator<<(std::ostream &output, const XmlPropertyMap &prop_map) {
            output << "Key: \t \tValue: \t\tAttributes" << std::endl;
            for (XmlMap::const_iterator it = prop_map._map.cbegin();
                 it != prop_map._map.cend(); it++) {
                output << it->first << "\t\t" << it->second.entry;
                if (!it->second.attributes.empty()) {
                    output << "\t\t";
                    for (const std::pair<std::string, std::string> &att : it->second.attributes) {
                        output << att.first << ": " << att.second << ", ";
                    }
                    output << std::endl;
                } else {

                    output << std::endl;
                }
            }
            return output;
        }

    private:
        XmlMap _map;
    };
}


#endif //LFNS_XMLPROPERTYMAP_H
