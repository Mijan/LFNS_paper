//
// Created by jan on 24/10/18.
//

#include "XmlFileReader.h"
#include "../base/Utils.h"
#include <boost/property_tree/xml_parser.hpp>

namespace io {

    XmlFileReader::XmlFileReader(std::string file_name) : _xml_file_name(file_name), _xml_file_path(), _pt() {
        boost::property_tree::read_xml(file_name, _pt);
        _xml_file_path = file_name.substr(0, file_name.find_last_of("/") + 1);
    }


    XmlMap XmlFileReader::getEntryMap(std::string parent_path, std::string sub_tree_name) {

        std::vector<XmlMap> maps = getEntryMaps(parent_path, sub_tree_name);

        if (maps.size() != 1) {
            std::ostringstream os;
            os << "Exactly one tree '" << sub_tree_name << "' expected, but " << maps.size() <<
               " provided!" << std::endl;
            throw std::runtime_error(os.str());
        }
        return maps[0];
    };

    std::vector<XmlMap>
    XmlFileReader::getEntryMaps(std::string parent_path, std::string sub_tree_name, std::string sub_tree_key,
                                std::string sub_tree_value) {
        return getEntryMaps(_pt.get_child(parent_path), sub_tree_name, sub_tree_key, sub_tree_value);
    }


    std::vector<XmlMap>
    XmlFileReader::getEntryMaps(bt_ptree root_node, std::string sub_tree_name, std::string sub_tree_key,
                                std::string sub_tree_value) {

        std::vector<XmlMap> entry_maps;
        std::vector<bt_ptree> p_trees = getSubtrees(root_node, sub_tree_name,
                                                    sub_tree_key, sub_tree_value);
        for (bt_ptree &p_tree : p_trees) {
            entry_maps.push_back(getEntriesForTree(p_tree));
        }
        return entry_maps;
    }

    std::string XmlFileReader::getEntry(std::string path) { return _pt.get_child(path).get_value<std::string>(); }

    std::vector<XmlEntry> XmlFileReader::getEntries(std::string path, std::string node_name) {
        std::vector<XmlEntry> entries;
        for (bt_ptree::value_type &v : _pt.get_child(path)) {
            if (v.first.compare(node_name) == 0) {
                XmlEntry entry;
                entry.name = v.first;
                entry.entry = v.second.get_value<std::string>("");
                std::vector<std::pair<std::string, std::string> > attributes;

                if (v.second.count("<xmlattr>") > 0) {
                    bt_ptree &attributes_tree = v.second.get_child("<xmlattr>");

                    for (bt_ptree::value_type &at : attributes_tree.get_child("")) {
                        attributes.push_back(std::make_pair<std::string, std::string>(at.first.data(),
                                                                                      at.second.get_value<std::string>(
                                                                                              "")));
                    }
                }
                entry.attributes = attributes;

                entries.push_back(entry);
            }
        }
        return entries;
    }

    XmlMap XmlFileReader::getEntriesForTree(bt_ptree &p_tree) {
        XmlMap entry_map;
        for (bt_ptree::value_type &v : p_tree.get_child("")) {
            XmlEntry entry;
            entry.name = v.first;
            entry.entry = v.second.get_value<std::string>("");
            std::vector<std::pair<std::string, std::string> > attributes;

            if (v.second.count("<xmlattr>") > 0) {
                bt_ptree &attributes_tree = v.second.get_child("<xmlattr>");

                for (bt_ptree::value_type &at : attributes_tree.get_child("")) {
                    attributes.push_back(std::make_pair<std::string, std::string>(at.first.data(),
                                                                                  at.second.get_value<std::string>(
                                                                                          "")));
                }
            }
            entry.attributes = attributes;

            entry_map[entry.name] = entry;
        }
        return entry_map;
    }

    std::string XmlFileReader::getXmlFilePath() { return _xml_file_path; }

    std::vector<bt_ptree> XmlFileReader::getSubtrees(bt_ptree root_node,
                                                     std::string sub_tree_name, std::string sub_tree_key,
                                                     std::string sub_tree_value) {
        try {
            std::vector<bt_ptree> result_trees;

            for (bt_ptree::value_type &v : root_node.get_child("")) {
                if (v.first.compare(sub_tree_name) == 0) {
                    if (sub_tree_key.length() > 0) {
                        std::string keys = v.second.get<std::string>(sub_tree_key);
                        bool key_is_contained = false;
                        std::vector<std::string> keys_vector = base::Utils::StringToStringVector(keys);
                        for (std::string &key : keys_vector) {
                            if (key.compare(sub_tree_value) == 0) {
                                key_is_contained = true;
                            }
                        }
                        if (key_is_contained) {
                            result_trees.push_back(v.second);
                        }
                    } else {

                        result_trees.push_back(v.second);

                    }
                }
            }
            return result_trees;
        } catch (const std::exception &e) {
            std::ostringstream os;
            os << "Getting subtree " << sub_tree_name << " with key  " << sub_tree_key << " = " << sub_tree_value <<
               " failed " << std::endl << "\t" << e.what() << std::endl;
            throw std::runtime_error(os.str());
        }
    }


    std::vector<bt_ptree> XmlFileReader::getSubtrees(std::string parent_path, std::string sub_tree_name,
                                                     std::string sub_tree_key,
                                                     std::string sub_tree_value) {
        return getSubtrees(_pt.get_child(parent_path), sub_tree_name, sub_tree_key, sub_tree_value);
    }
}