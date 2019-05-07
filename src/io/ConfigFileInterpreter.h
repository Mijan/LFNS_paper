//
// Created by jan on 24/10/18.
//

#ifndef LFNS_CONFIGFILEINTERPRETER_H
#define LFNS_CONFIGFILEINTERPRETER_H

#include <string>
#include <vector>
#include <map>
#include "XmlFileReader.h"
#include "XmlPropertyMap.h"
#include "../base/Utils.h"

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

        std::map<std::string, std::pair<double, double> > getParameterBounds();

        std::map<std::string, double> getFixedParameters();

        std::map<std::string, std::string> getDataFiles(std::vector<std::string> experiments);

        std::map<std::string, std::string> getTimesFiles(std::vector<std::string> experiments);

        std::vector<std::string> getExperimentsForLFNS();

        std::vector<std::string> getExperimentsForSimulations();

        std::vector<std::string> getExperimentsForEvaluateLikelihood();

        int getNForLFNS();

        int getRForLFNS();

        int getHForLFNS();

        int getHForEvaluateLikelihood();

        int getDPGMMItForLFNS();

        double getEpsilonForLFNS();

        std::vector<double> getPulsePeriods(std::string experiment_name);

        std::vector<double> getPulseStrengths(std::string experiment_name);

        std::vector<double> getPulseDurations(std::string experiment_name);

        std::vector<int> getNumPulse(std::string experiment_name);

        std::vector<std::string> getPulseInputNames(std::string experiment_name);

        std::vector<double> getStartingTimes(std::string experiment_name);

        std::vector<double> getParamForSimulation();

        std::vector<double> getParamForEvaluateLikelihood();

        int getNForSimulation();

        std::string getParameterFileforSimulation();

        std::string getParameterFileforEvaluateLikelihood();

        double getInitialTimeForSimulation();

        double getFinalTimeForSimulation();

        double getIntervalForSimulation();

        bool detSpeciesProvided();

        bool stochSpeciesProvided();

        std::vector<std::string> getDetSpecies();

        std::vector<std::string> getStochSpecies();

    protected:
        XmlFileReader _reader;

        std::vector<std::string> _getInputValues(std::string experiment_name, std::string field_name);


    };
}


#endif //LFNS_CONFIGFILEINTERPRETER_H
