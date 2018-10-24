//
// Created by jan on 24/10/18.
//

#ifndef LFNS_XMLFILEREADER_H
#define LFNS_XMLFILEREADER_H


#include <string>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <vector>

namespace io {
    typedef boost::property_tree::ptree bt_ptree;

    class XmlEntry {
    public:
        std::string getAttributeValue(std::string name_) {
            std::string value = "";
            for (std::pair<std::string, std::string> &att: attributes) {
                if (att.first.compare(name_) == 0) {
                    if (value.length() == 0) {
                        value = att.second;
                    } else {
                        std::stringstream ss;
                        ss << "Tried to access attribute " << name_ << " of node " << name
                           << ", but multiple attributes by that name found!" << std::endl;
                        throw std::runtime_error(ss.str());
                    }
                }
            }
            if (value.length() == 0) {
                std::stringstream ss;
                ss << "Tried to access attribute " << name_ << " of node " << name
                   << ", but no such attributes by that name found!" << std::endl;
                throw std::runtime_error(ss.str());
            }
            return value;
        }

        std::string name;
        std::string entry;
        std::vector<std::pair<std::string, std::string> > attributes;
    };

    typedef std::map<std::string, XmlEntry> XmlMap;

    class XmlFileReader {
    public:
        XmlFileReader(std::string file_name);

        XmlMap getEntryMap(std::string parent_path, std::string sub_tree_name);

        std::vector<XmlMap>
        getEntryMaps(std::string parent_path, std::string sub_tree_name, std::string sub_tree_key = "",
                     std::string sub_tree_value = "");

        std::vector<XmlMap> getEntryMaps(bt_ptree root_node, std::string sub_tree_name, std::string sub_tree_key = "",
                                         std::string sub_tree_value = "");

        std::string getEntry(std::string path);

        std::vector<XmlEntry> getEntries(std::string path, std::string node_name);

        std::string getXmlFilePath();

        XmlMap getEntriesForTree(bt_ptree &p_tree);

        std::vector<bt_ptree> getSubtrees(bt_ptree root_node, std::string sub_tree_name, std::string sub_tree_key = "",
                                          std::string sub_tree_value = "");

        std::vector<bt_ptree>
        getSubtrees(std::string parent_path, std::string sub_tree_name, std::string sub_tree_key = "",
                    std::string sub_tree_value = "");

    private:
        std::string _xml_file_name;
        std::string _xml_file_path;
        bt_ptree _pt;
    };
}


#endif //LFNS_XMLFILEREADER_H
