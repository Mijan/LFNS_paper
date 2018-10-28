#include <iostream>
#include <vector>
#include "src/base/EigenMatrices.h"
#include "src/simulator/SimulatorSsa.h"
#include "src/base/Utils.h"
#include "src/base/IoUtils.h"
#include "src/simulator/SimulatorOde.h"
#include "src/base/MathUtils.h"
#include "src/models/ChemicalReactionNetwork.h"
#include "src/models/InitialValueData.h"
#include "src/models/InitialValueProvider.h"
#include "src/models/MeasurementModel.h"


int testReadModel() {
//    std::string model_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_model.txt";
//    models::ModelReactionData model_data(model_file);
//    models::ChemicalReactionNetwork lac_system(model_data);
//
//    lac_system.printInfo(std::cout);
//
//
//    base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));
//    std::string initial_value_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_initial_states.txt";
//    models::InitialValueData init_data(initial_value_file);
//    models::InitialValueProvider lac_init_value(rng, init_data);
//
//    lac_init_value.printInfo(std::cout);
//
//
//    std::string measurement_file = "/home/jan/crypt/Dropbox/Nested_Sampling_paper/numerical_data/tmp/lacgfp_measurement.txt";
//    models::MeasurementModelData measure_data(measurement_file);
//    models::MeasurementModel lac_measure(rng, measure_data);
//
//    lac_measure.printInfo(std::cout);
    return 1;
}

int main() {

    int ode = testReadModel();
    return ode;
}