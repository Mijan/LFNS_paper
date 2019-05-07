/*
 * EstimationOptions.h
 *
 *  Created on: Feb 24, 2015
 *      Author: jan
 */

#ifndef DP_GMM_ESTIMATIONOPTIONS_H_
#define DP_GMM_ESTIMATIONOPTIONS_H_

#include <iostream>

namespace DP_GMM {

class EstimationOptions {
public:
	EstimationOptions();
	virtual ~EstimationOptions();

	std::string data_file_name;
	std::string output_file_name;
	int num_dp_iterations;
	int max_num_points;
	int print_frequency;
	bool read_hyper_params;
	std::string hyper_params_file_name;

	void handleOptions(int argc, char * argv[]);
	void printInfo(std::size_t num_points, std::size_t num_dimensions);

private:
	//Default values
	std::string DATA_FILE_NAME;
	std::string OUTPUT_FILE_NAME;
	int NUM_DP_ITERATIONS;
	int NUM_DIM;
	int MAX_NUM_POINTS;
	int PRINT_FREQUENCY;
	std::string HYPER_PARAMS_FILE_NAME;
};

} /* namespace DP_GMM */

#endif /* DP_GMM_ESTIMATIONOPTIONS_H_ */
