//
// Created by jan on 08/10/18.
//

#include <sstream>
#include <stdexcept>
#include "IoUtils.h"
#include "Utils.h"
#include <algorithm>

namespace base {
    std::string IoUtils::appendToFileName(const std::string fileName,
                                          const std::string ending_string) {
        size_t pos = fileName.find_last_of(".");
        if (pos == std::string::npos) {
            std::stringstream os;
            os << "The provided name " << fileName
               << " is not a file. Please provide a filename in the format 'filename.txt'!" << std::endl;
            throw std::runtime_error(os.str());
        }
        std::string fileStart;
        std::string fileEnd;
        std::string file;
        fileStart.assign(fileName.begin(), fileName.begin() + pos);
        fileEnd.assign(fileName.begin() + pos, fileName.end());

        std::stringstream ss;
        ss << fileStart << "_" << ending_string << fileEnd;
        file = ss.str();
        return file;
    }

    std::string IoUtils::setFileEnding(const std::string fileName,
                                       const std::string string) {
        size_t pos = fileName.find_last_of(".");
        std::string fileStart;
        std::string file;
        fileStart.assign(fileName.begin(), fileName.begin() + pos);
        file = fileStart + "." + string;
        return file;
    }

    std::string IoUtils::getFileEnding(const std::string file_name) {
        size_t pos = file_name.find_last_of(".");

        if (pos == std::string::npos) {
            return "";
        }
        std::string file_ending;
        file_ending.assign(file_name.begin() + pos + 1,
                           file_name.end());
        return file_ending;
    }

    std::string IoUtils::getAbsolutePath(std::string path, std::string base_path) {
        if (path.find_first_of("/") == 0) {
            return path;
        } else {
            return base_path + path;
        }
    }

    std::string IoUtils::getFileLocation(std::string full_file_path) {
        size_t end_of_path = full_file_path.find_last_of("/");
        std::string file_path;
        file_path.assign(full_file_path.begin(),
                         full_file_path.begin() + end_of_path);
        return file_path;
    }

    std::string IoUtils::getFileName(std::string full_file_path) {
        size_t end_of_path = full_file_path.find_last_of("/");
        std::string file_name;
        file_name.assign(full_file_path.begin() + end_of_path + 1,
                         full_file_path.end());
        return file_name;
    }


    bool IoUtils::isPathAbsolute(std::string path) {
        if (path.find("/") > 0) {
            return false;
        }
        return true;
    }

    bool IoUtils::doesFileExists(std::string file_name) {
        std::ifstream file_stream(file_name);
        return file_stream.good();
    }


    std::string IoUtils::intToString(size_t nbr) {
        std::stringstream os;
        os << nbr;
        return os.str();
    }

    std::vector<double> IoUtils::readVector(std::string input_file_name) {
        std::vector<double> out_vector;

        std::ifstream data_file(input_file_name.c_str());
        if (!data_file.is_open()) {
            std::ostringstream os;
            os << "Faild to read vector. Could not open file " << input_file_name << "!" << std::endl;
            throw std::runtime_error(os.str());
        }
        while (!data_file.eof()) {
            std::string val_str;
            std::getline(data_file, val_str);

            if (val_str.size() > 0) {
                out_vector = Utils::StringToDoubleVector(val_str, Utils::FindDelimiter(val_str));
            }
        }
        data_file.close();
        return out_vector;
    }


    std::vector<std::vector<double> > IoUtils::readVectorOfVectors(std::string input_file_name) {
        std::vector<std::vector<double> > out_vector;

        std::ifstream data_file(input_file_name.c_str());
        if (!data_file.is_open()) {
            std::ostringstream os;
            os << "Faild to read vector of vectors. Could not open file " << input_file_name << "!" << std::endl;
            throw std::runtime_error(os.str());
        }
        while (!data_file.eof()) {
            std::string val_str;
            std::getline(data_file, val_str);

            if (val_str.size() > 0) {
                out_vector.push_back(Utils::StringToDoubleVector(val_str, Utils::FindDelimiter(val_str)));
            }
        }
        data_file.close();
        return out_vector;
    }


    std::vector<std::vector<std::vector<double> > >
    IoUtils::readMultiline(std::string input_file_name, int num_lines_for_multiline, int max_traj) {
        std::vector<std::vector<std::vector<double> > > out_vector;
        std::ifstream data_file(input_file_name.c_str());
        if (!data_file.is_open()) {
            std::ostringstream os;
            os << "Faild to read vector. Could not open file " << input_file_name << "!" << std::endl;
            throw std::runtime_error(os.str());
        }

        std::string val_str;
        int num_lines = 0;
        while (std::getline(data_file, val_str)) {
            if (val_str.size() > 0) { num_lines++; }
        }
        data_file.clear();
        data_file.seekg(0, std::ios_base::beg);

        if (num_lines % num_lines_for_multiline != 0) {
            std::stringstream ss;
            ss << "Failed to read multiline from file " << input_file_name << "! Expected multilines with "
               << num_lines_for_multiline << " lines for each output, but a total of " << num_lines << " found!"
               << std::endl;
            throw std::runtime_error(ss.str());
        }

        int num_traj = 0;
        while (!data_file.eof()) {

            std::vector<std::vector<double> > multiline;
            for (int line_nbr = 0; line_nbr < num_lines_for_multiline; line_nbr++) {
                std::getline(data_file, val_str);
                if (val_str.size() > 0) {
                    std::vector<double> line = Utils::StringToDoubleVector(val_str, Utils::FindDelimiter(val_str));
                    if (line_nbr > 0 && line.size() != multiline.size()) {
                        std::stringstream ss;
                        ss << "Data inconsistent! While reading multiline from " << input_file_name
                           << " the first line had " << multiline.size() << " entries, while line nbr " << line_nbr + 1
                           << " has " << line.size() << " entries!" << std::endl;
                        throw std::runtime_error(ss.str());
                    }
                    for (int col_nbr = 0; col_nbr < line.size(); col_nbr++) {
                        if (line_nbr == 0) {
                            multiline.push_back(std::vector<double>(num_lines_for_multiline, 0.0));
                        }
                        (multiline[col_nbr])[line_nbr] = line[col_nbr];
                    }
                }
            }
            if (multiline.size() > 0) {
                out_vector.push_back(multiline);
                if (++num_traj >= max_traj) { return out_vector; };
            }
        }
        return out_vector;
    }

}