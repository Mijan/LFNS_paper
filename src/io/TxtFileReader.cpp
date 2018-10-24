//
// Created by jan on 12/10/18.
//

#include "TxtFileReader.h"

#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "../base/Utils.h"

namespace io {
    TxtFileReader::TxtFileReader(std::string txt_file_name) : _txt_file_lines() {
        std::ifstream txt_stream(txt_file_name.c_str());
        if (!txt_stream.is_open()) {
            std::ostringstream os;
            os << "File " << txt_file_name << " could not be read!"
               << std::endl;
            throw std::runtime_error(os.str());
        }

        std::string line;
        while (std::getline(txt_stream, line)) { _txt_file_lines.push_back(line); }
        txt_stream.close();
    }

    TxtFileReader::~TxtFileReader() {}

    std::vector<std::string> TxtFileReader::_readLineWithCaption(std::string caption, std::string delimiter) {

        std::vector<std::string> entries;
        bool entries_found = false;
        std::string line;

        for (std::size_t line_nbr = 0; line_nbr < _txt_file_lines.size(); line_nbr++) {

            line = _txt_file_lines[line_nbr];
            if (line.compare(caption) == 0) {
                if (entries_found) {
                    std::ostringstream os;
                    os << "Multiple lines with '" << caption << "' found!" << std::endl;
                    throw std::runtime_error(os.str());
                }
                entries = _readStrippedDelimitterdLine(_txt_file_lines[line_nbr + 1], delimiter);
                entries_found = true;
            }
        }
        if (!entries_found) {
            std::ostringstream os;
            os << "'" << caption << "' line not found!" << std::endl;
            throw std::runtime_error(os.str());
        }
        return entries;
    }


    std::vector<std::string> TxtFileReader::_readStrippedDelimitterdLine(std::string line, std::string delimiter) {

        std::vector<std::string> entries;

        std::size_t new_pos = 0;
        std::size_t del_pos = line.find(delimiter);

        while (del_pos != std::string::npos) {
            std::string sub_string = line.substr(new_pos, del_pos - new_pos);
            new_pos = del_pos + delimiter.size();
            del_pos = line.find(delimiter, new_pos + 1);

            std::string tmp = boost::erase_all_copy(sub_string, " ");
            tmp = boost::erase_all_copy(tmp, "\t");
            entries.push_back(tmp);
        }
        std::string sub_string = line.substr(new_pos, del_pos);
        std::string tmp = boost::erase_all_copy(sub_string, " ");
        tmp = boost::erase_all_copy(tmp, "\t");
        entries.push_back(tmp);
        return entries;
    }

    std::vector<std::string> TxtFileReader::_splitInTwo(std::string line, std::string delimiter) {

        std::vector<std::string> two_strings = _readStrippedDelimitterdLine(line, delimiter);

        if (two_strings.size() != 2) {
            std::ostringstream os;
            os << "line must have exactly one token '" << delimiter
               << "', current line is: " << line << std::endl;
            throw std::runtime_error(os.str());
        }

        return two_strings;
    }

    std::vector<std::string> TxtFileReader::_readCaptionLines(std::string caption) {
        std::vector<std::string> reaction_lines;
        bool reactions_found = false;
        std::string line;
        for (std::size_t line_nbr = 0; line_nbr < _txt_file_lines.size(); line_nbr++) {

            line = _txt_file_lines[line_nbr];

            if (reactions_found) {
                if (line.length() == 0) { return reaction_lines; }
                reaction_lines.push_back(line);
            }

            if (line.compare(caption) == 0) {
                reactions_found = true;
            }
        }

        if (!reactions_found) {
            std::ostringstream os;
            os << caption << " line not found!" << std::endl;
            throw std::runtime_error(os.str());
        }
        return reaction_lines;
    }
}
