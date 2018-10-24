//
// Created by jan on 24/10/18.
//

#ifndef LFNS_CONFIGFILEINTERPRETER_H
#define LFNS_CONFIGFILEINTERPRETER_H

#include <string>
#include <vector>
#include <map>
#include "XmlFileReader.h"

namespace io {

    class ConfigFileInterpreter {
    public:
        ConfigFileInterpreter(std::string instance_file_name);

        virtual ~ConfigFileInterpreter();

        std::string getModelFileName();

        std::string getMeasurementModelFile();

        std::string getInitialConditionsFile();

        std::string getModelType();

        std::map<std::string, std::string> getParameterScales();

        std::map<std::string, std::pair<double, double>> getParameterBounds();

        std::map<std::string, double> getFixedParameters();

        std::map<std::string, std::string> getDataFiles();

        std::map<std::string, std::string> getTimesFiles();

        std::vector<std::string> getExperimentsForLFNS();

        int getNForLFNS();

        int getRForLFNS();

        int getHForLFNS();

        int getDPGMMItForLFNS();

        double getEpsilonForLFNS();

    protected:
        XmlFileReader _reader;

    };
}


#endif //LFNS_CONFIGFILEINTERPRETER_H
