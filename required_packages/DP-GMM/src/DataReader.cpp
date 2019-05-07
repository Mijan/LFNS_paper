/*
 * DataReader.cpp
 *
 *  Created on: Mar 13, 2015
 *      Author: jan
 */

#include "DataReader.h"

#include <fstream>
#include <string>
#include <math.h>

namespace DP_GMM {

    DataReader::DataReader() { }

    DataReader::~DataReader() { }

    EiMatrix DataReader::readData(std::string data_file_name) {

        typedef std::vector<double> Row;

        std::ifstream data_file(data_file_name.c_str());
        if (!data_file.is_open()) {
            std::ostringstream os;
            os << "Error opening file " << data_file_name.c_str()
            << " for reading particles. Particles could not be read!!"
            << std::endl;
            throw std::runtime_error(os.str());
        }

        std::vector<Row> data_;
        while (!data_file.eof()) {

            std::string line;
            std::vector<Row> data_matrix;

            while (std::getline(data_file, line)) {
                std::istringstream iss(line);
                double n;
                Row row;
                while (iss >> n) { row.push_back(n); }
                data_.push_back(row);
            }

        }
        data_file.close();

        if (data_.size() > 0 && data_[0].size() > 0) {
            std::cout << "Data read from" << data_file_name << std::endl;
        } else {
            std::ostringstream os;
            os << "Data file " << data_file_name.c_str() << " seems to be empty..."
            << std::endl;
            throw std::runtime_error(os.str());
        }

        std::size_t num_dim = data_[0].size();
        std::size_t num_samples = data_.size();

        EiMatrix data_matrix(num_samples, num_dim);

        for (std::size_t row = 0; row < num_samples; row++) {
            for (std::size_t col = 0; col < num_dim; col++) {
                data_matrix(row, col) = (data_[row])[col];
            }
        }

        return data_matrix;
    }

} /* namespace DP_GMM */
