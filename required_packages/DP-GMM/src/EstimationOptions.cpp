/*
 * EstimationOptions.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: jan
 */

#include "EstimationOptions.h"

#include <limits>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace DP_GMM {

EstimationOptions::EstimationOptions()  {
	DATA_FILE_NAME = "";
	OUTPUT_FILE_NAME = "";
	NUM_DP_ITERATIONS = 200;
	MAX_NUM_POINTS = std::numeric_limits<int>::max();
	PRINT_FREQUENCY = 10;
	HYPER_PARAMS_FILE_NAME = "";

}

EstimationOptions::~EstimationOptions() {
	// TODO Auto-generated destructor stub
}

void EstimationOptions::handleOptions(int argc, char * argv[]) {
	po::options_description desc("Allowed options");
	desc.add_options()("help", "produce help message")("data_file",
			po::value<std::string>(&data_file_name)->default_value(
					DATA_FILE_NAME),
			"problem file. A problem file must always be provided!")(
			"output_file,O",
			po::value<std::string>(&output_file_name)->default_value(
					OUTPUT_FILE_NAME),
			"The name of the output file where the results will be stored")(
			"num_dp_it,d",
			po::value<int>(&num_dp_iterations)->default_value(
					NUM_DP_ITERATIONS),
			"The parameters used for the simulation. The number of parameters must be equal to the number of parameters needed by the model!")("max_num_pts,m",
			po::value<int>(&max_num_points)->default_value(MAX_NUM_POINTS),
			"the maximal number of datapoints used for the density estimation")(
			"verbose,p",
			po::value<int>(&print_frequency)->default_value(PRINT_FREQUENCY),
			"the frequncy of iterations after which the output is written")(
			"hyer_params_file,h",
			po::value<std::string>(&hyper_params_file_name)->default_value(
					HYPER_PARAMS_FILE_NAME), "File with hyperparameters");

	po::positional_options_description p;
	p.add("data_file", -1);
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::store(
			po::command_line_parser(argc, argv).options(desc).positional(p).run(),
			vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << "\n";
		abort();
	}

	if (output_file_name.size() == 0) {
		fs::path full_path(fs::current_path());
		output_file_name = full_path.string() + "/dpgmm_estimation_results.txt";
	}
	if (!hyper_params_file_name.empty()) {
		read_hyper_params = true;
	}else{
		read_hyper_params = false;
	}
}

void EstimationOptions::printInfo(std::size_t num_points, std::size_t num_dimensions) {
	std::cout
			<< "Dirichlet Process Gaussian Mixture Model density estimation is being performed."
			<< std::endl << std::endl;
	std::cout << "The data is read from " << data_file_name
			<< " and is assumed to be " << num_dimensions << " dimensional."
			<< std::endl;
	std::cout << "The output will be saved " << " every " << print_frequency
			<< " iterations in " << output_file_name << std::endl << std::endl;
	if (read_hyper_params) {
		std::cout << "Hyper parameters are read from " << hyper_params_file_name
				<< std::endl;
	} else {
		std::cout << "No hyper parameters file provided!" << std::endl;
	}
	std::cout << std::endl;
	if (max_num_points < num_points) {
		std::cout << "Only " << max_num_points
				<< " data points of the original " << num_points
				<< " will be used!" << std::endl;
	} else {
		std::cout << num_points << " data points will be used!" << std::endl;
	}
	std::cout << std::endl;
	std::cout << "The number of DP-GMM iterations is " << num_dp_iterations
			<< " and the results will be written every " << print_frequency
			<< " iterations into the file " << output_file_name << std::endl
			<< std::endl;

}

}/* namespace DP_GMM */
