//
// Created by jan on 08/10/18.
//

#ifndef LFNS__IOUTILS_H
#define LFNS__IOUTILS_H

#include <string>
#include <vector>
#include <ios>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace base {
    class IoUtils {
    public:
        static std::string appendToFileName(const std::string fileName,
                                            const std::string ending_string);

        static std::string getAbsolutePath(std::string path, std::string base_path);

        static std::string getFileLocation(std::string full_file_path);

        static std::string getFileName(std::string full_file_path);

        static bool isPathAbsolute(std::string path);

        static bool doesFileExists(std::string file_name);

        template<typename T>
        static void writeVector(std::string output_file_name,
                                const std::vector<T> &out_vector, std::string delimmiter = ", ",
                                std::ios_base::openmode mode = std::ios_base::out) {
            std::ofstream data_file(output_file_name.c_str(), mode);
            if (!data_file.is_open()) {
                std::ostringstream os;
                os << "Could not open file " << output_file_name << "!" << std::endl;
                throw std::runtime_error(os.str());
            }
            std::size_t entry_nbr = 0;
            for (entry_nbr; entry_nbr < out_vector.size() - 1;
                 entry_nbr++) {

                data_file << out_vector[entry_nbr] << delimmiter;
            }
            data_file << out_vector[entry_nbr] << std::endl;
            data_file.close();
        }

        template<typename T>
        static void writeVectorCol(std::string output_file_name, const std::vector<T> &out_vector,
                                   std::ios_base::openmode mode = std::ios_base::out) {
            std::ofstream data_file(output_file_name.c_str(), mode);
            if (!data_file.is_open()) {
                std::ostringstream os;
                os << "Could not open file " << output_file_name << "!" << std::endl;
                throw std::runtime_error(os.str());
            }
            std::size_t entry_nbr = 0;
            for (entry_nbr; entry_nbr < out_vector.size() - 1;
                 entry_nbr++) {

                data_file << out_vector[entry_nbr] << std::endl;
            }
            data_file << out_vector[entry_nbr] << std::endl;
            data_file.close();
        }

        static std::vector<double> readVector(std::string input_file_name);

        static std::vector<std::vector<double> > readVectorOfVectors(std::string input_file_name);

        static std::vector<std::vector<std::vector<double> > >
        readMultiline(std::string input_file_name, int num_lines_for_multiline, int max_traj = 1000);

        template<typename T>
        static void writeVectorOfVectors(std::string output_file_name, const std::vector<std::vector<T> > &out_vectors,
                                         bool transposed = false, std::ios_base::openmode mode = std::ios_base::out) {
            std::ofstream data_file(output_file_name.c_str(), mode);
            if (!data_file.is_open()) {
                std::ostringstream os;
                os << "Could not open file " << output_file_name << "!" << std::endl;
                throw std::runtime_error(os.str());
            }

            if (transposed) {
                int num_entries = out_vectors[0].size();
                for (std::size_t entry = 0; entry < num_entries; entry++) {
                    std::size_t vector_nbr = 0;
                    for (vector_nbr; vector_nbr < out_vectors.size() - 1; vector_nbr++) {
                        data_file << out_vectors[vector_nbr][entry] << ", ";
                    }
                    data_file << out_vectors[vector_nbr][entry] << std::endl;
                }
            } else {
                for (std::size_t vector_nbr = 0; vector_nbr < out_vectors.size(); vector_nbr++) {
                    std::vector<T> out_vector = out_vectors[vector_nbr];
                    std::size_t entry_nbr = 0;
                    if (!out_vector.empty()) {
                        for (entry_nbr; entry_nbr < out_vector.size() - 1;
                             entry_nbr++) {

                            data_file << out_vector[entry_nbr] << ", ";
                        }
                        data_file << out_vector[entry_nbr] << std::endl;
                    }
                }
            }
            data_file.close();
        }

        static std::string setFileEnding(const std::string fileName, const std::string string);

        static std::string getFileEnding(const std::string file_name);

        static std::string intToString(size_t nbr);
    };
}


#endif //LFNS__IOUTILS_H
