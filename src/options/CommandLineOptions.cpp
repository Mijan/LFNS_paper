//
// Created by jan on 12.01.17.
//

#include <iostream>
#include "CommandLineOptions.h"
#include "../base/IoUtils.h"

namespace options {
    CommandLineOptions::CommandLineOptions() : config_file_name(""), output_file_name(""), desc() {

        desc.add_options()("help", "produce help message")("config_file",
                                                           po::value<std::string>(&config_file_name),
                                                           "Config file. A config file must always be provided!")(
                "output_file,O", po::value<std::string>(&output_file_name), "Output file")(
                "experiment,E", po::value<std::vector<std::string> >(&experiments)->multitoken(),
                "The performed experiments");

    }

    CommandLineOptions::~CommandLineOptions() {}

    int CommandLineOptions::handleCommandLineOptions(int argc, char **argv) {

        po::positional_options_description p;
        p.add("config_file", -1);

        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        std::string problem_name = base::IoUtils::getFileName(config_file_name);
        std::string file_ending = base::IoUtils::getFileEnding(problem_name);
        if (file_ending.compare("xml") != 0) {
            if (problem_name.size() == 0) {
                throw std::runtime_error(
                        "No config file was provided, please provide the problem file name with its relative or absolute location");
            } else {
                throw std::runtime_error(
                        "Provided config file does not seem to be a .xml file. Please provide a xml problem file name with its relative or absolute location");
            }

        }

        if (!base::IoUtils::isPathAbsolute(config_file_name)) {
            fs::path full_path(fs::current_path());
            config_file_name = full_path.string() + "/" + config_file_name;
        }

        if (output_file_name.empty()) {
            std::string file_location = base::IoUtils::getFileLocation(config_file_name);
            output_file_name = file_location + "/results.txt";
            std::cerr << "No output file provided, results will be written into " << output_file_name << std::endl;
        }
        if (output_file_name.find_last_of(".") == std::string::npos) {
            std::stringstream os;
            os << "The provided output file name " << output_file_name
               << " is not a file. Please provide a filename in the format '/path/to/outputfile.txt'!" << std::endl;
            throw std::runtime_error(os.str());
        }
        if (output_file_name.empty()) {
            fs::path full_path(fs::current_path());
            output_file_name = full_path.string() + "/results.txt";
        }
        if (!base::IoUtils::isPathAbsolute(output_file_name)) {
            fs::path full_path(fs::current_path());
            output_file_name = full_path.string() + "/" + output_file_name;
        }
        if (base::IoUtils::getFileName(output_file_name).size() == 0) {
            output_file_name = output_file_name + "results.txt";
            std::cerr << "No output file name provided! Using default value for filename:" << output_file_name
                      << std::endl;
        }
        return 1;
    }
}

