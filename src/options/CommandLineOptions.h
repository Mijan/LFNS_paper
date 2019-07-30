//
// Created by jan on 24/10/18..
//

#ifndef LFNS_COMMANDLINEOPTIONS_H
#define LFNS_COMMANDLINEOPTIONS_H

#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace options {
    class CommandLineOptions {
    public:
        CommandLineOptions();

        virtual ~CommandLineOptions();

        virtual int handleCommandLineOptions(int argc, char **argv);

        std::string config_file_name;
        std::string output_file_name;
        std::vector<std::string> experiments;

        po::options_description desc;
        po::variables_map vm;
    };
}

#endif //LFNS_COMMANDLINEOPTIONS_H
