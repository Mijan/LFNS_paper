//
// Created by jan on 27/10/18.
//

#include "FullModel.h"
#include "../base/Utils.h"
#include "../base/IoUtils.h"

namespace models {
    using namespace std::placeholders;

    FullModel::FullModel(ChemicalReactionNetwork_ptr dynamics, InitialValueProvider_ptr init_val,
                         MeasurementModel_ptr measurement) : dynamics(dynamics), initial_value_provider(init_val),
                                                             measurement_model(measurement), _param_names(),
                                                             _parameter(), _unfixed_parameter_indices(), _inputs() {
        _param_names = dynamics->getParameterNames();
        base::Utils::addOnlyNew<std::string>(_param_names, init_val->getParameterNames());
        base::Utils::addOnlyNew<std::string>(_param_names, measurement->getParameterNames());
        _parameter.resize(_param_names.size());
        for (int i = 0; i < _param_names.size(); i++) {
            dynamics->setPointer(&_parameter[i], _param_names[i]);
            initial_value_provider->setPointer(&_parameter[i], _param_names[i]);
            measurement_model->setPointer(&_parameter[i], _param_names[i]);
            _unfixed_parameter_indices.push_back(i);
            _parameter[i] = -1;
        }
    }


    FullModel::FullModel(models::ModelSettings &settings, base::RngPtr rng, std::string experiment) : dynamics(nullptr),
                                                                                                      initial_value_provider(
                                                                                                              nullptr),
                                                                                                      measurement_model(
                                                                                                              nullptr),
                                                                                                      _param_names(
                                                                                                              settings.param_names),
                                                                                                      _parameter(
                                                                                                              settings.param_names.size()),
                                                                                                      _unfixed_parameter_indices(),
                                                                                                      _inputs() {
        dynamics = std::make_shared<models::ChemicalReactionNetwork>(
                settings.model_file);
        models::InitialValueData init_data(settings.initial_value_file);
        if (init_data.getNumInitialValues() != dynamics->getNumSpecies()) {
            std::stringstream ss;
            ss << "Initial states not properly set! Model dynamics require " << dynamics->getNumSpecies()
               << " species, but initial states for " << init_data.getNumInitialValues() << " states defined!"
               << std::endl;
            std::vector<std::string> dynamics_states = dynamics->getSpeciesNames();
            std::vector<std::string> init_states = init_data.getInitialStates();
            ss << "Model dynamics species: " << std::endl;
            for (std::string species: dynamics_states) { ss << species << std::endl; }
            ss << "\nProvided initial states for: " << std::endl;
            for (std::string species: init_states) { ss << species << std::endl; }
            throw std::runtime_error(ss.str());
        }
        initial_value_provider = std::make_shared<models::InitialValueProvider>(rng, init_data);
        initial_value_provider->setOutputStateMapping(dynamics->getSpeciesNames());

        models::MeasurementModelData measure_data(settings.measurement_file);
        measurement_model = std::make_shared<models::MeasurementModel>(rng, measure_data);
        measurement_model->setStateOrder(dynamics->getSpeciesNames());

        for (int i = 0; i < _param_names.size(); i++) {
            dynamics->setPointer(&_parameter[i], _param_names[i]);
            initial_value_provider->setPointer(&_parameter[i], _param_names[i]);
            measurement_model->setPointer(&_parameter[i], _param_names[i]);
            _unfixed_parameter_indices.push_back(i);
            _parameter[i] = -1;
        }

        std::map<std::string, double>::iterator it = settings.fixed_parameters.begin();
        for (it; it != settings.fixed_parameters.end(); it++) {
            fixParameter(it->first, it->second);
        }

        if (experiment.size() > 0 && settings.input_datas.count(experiment) > 0) {
            for (models::InputData input_data : settings.input_datas[experiment]) {
                models::InputPulse pulse(input_data);
                addInputPulse(pulse);
            }
        }
    }

    FullModel::FullModel(const FullModel &rhs) : dynamics(rhs.dynamics),
                                                 initial_value_provider(rhs.initial_value_provider),
                                                 measurement_model(rhs.measurement_model),
                                                 _param_names(rhs._param_names),
                                                 _parameter(rhs._parameter),
                                                 _unfixed_parameter_indices(rhs._unfixed_parameter_indices),
                                                 _inputs(rhs._inputs) {
        for (int i = 0; i < _param_names.size(); i++) {
            dynamics->setPointer(&_parameter[i], _param_names[i]);
            initial_value_provider->setPointer(&_parameter[i], _param_names[i]);
            measurement_model->setPointer(&_parameter[i], _param_names[i]);
        }
    }

    FullModel::~FullModel() {}

    SetParameterFct_ptr FullModel::getParameterSettingFct() {
        return std::make_shared<SetParameterFct>(std::bind(&FullModel::setParameter, this, _1));
    }

    void FullModel::setParameter(const std::vector<double> &theta) {
        if (theta.size() != _unfixed_parameter_indices.size()) {
            std::stringstream ss;
            ss << "Expected " << _unfixed_parameter_indices.size() << " parameters, but provided theta has "
               << theta.size() << " entries!" << std::endl;
            throw std::runtime_error(ss.str());
        }
        for (int i = 0; i < theta.size(); i++) {
            int index = _unfixed_parameter_indices[i];
            _parameter[index] = theta[i];
        }
    }

    std::vector<std::string> FullModel::getUnfixedParamteters() {
        std::vector<std::string> unfixed_params;
        for (int &i : _unfixed_parameter_indices) { unfixed_params.push_back(_param_names[i]); }
        return unfixed_params;
    }

    void FullModel::fixParameter(std::string param_name, double val) {
        std::vector<std::string>::iterator it = std::find(_param_names.begin(), _param_names.end(), param_name);
        bool param_found = it != _param_names.end();
        if (param_found) {
            try {
                int index = it - _param_names.begin();
                _parameter[index] = val;
                std::vector<int>::iterator it = std::find(_unfixed_parameter_indices.begin(),
                                                          _unfixed_parameter_indices.end(), index);
                _unfixed_parameter_indices.erase(it);
            } catch (const std::exception &e) {
                std::ostringstream os;
                os << "Failed to fix parameter " << param_name << ":" << std::endl;
                os << "\t" << e.what() << std::endl;
                throw std::runtime_error(os.str());
            }
        } else {
            std::cerr << "Tried to fix parameter " << param_name << ", but no such parameter defined." << std::endl;
        }
    }


    void FullModel::printInfo(std::ostream &os) {
        dynamics->printInfo(os);
        initial_value_provider->printInfo(os);
        measurement_model->printInfo(os);
    }

    void FullModel::addInputPulse(InputPulse pulse) {
        std::vector<std::string>::iterator it = std::find(_param_names.begin(), _param_names.end(), pulse.input_name);
        bool param_found = it != _param_names.end();;
        if (param_found) {
            if (_inputs.pulses.empty()) { _inputs.modified_parameter = std::vector<double>(_param_names.size(), 0.0); }
            int param_index = it - _param_names.begin();
            pulse.parameter_index = param_index;
            if (_inputs.input_pram_indices.count(param_index) == 0) {
                _inputs.input_pram_indices.insert(param_index);
                dynamics->setPointer(&_inputs.modified_parameter[param_index], pulse.input_name);
                initial_value_provider->setPointer(&_inputs.modified_parameter[param_index], pulse.input_name);
                measurement_model->setPointer(&_inputs.modified_parameter[param_index], pulse.input_name);
            }

            _inputs.pulses.push_back(pulse);
            dynamics->setPerturbationFct(getInputFunction());
            initial_value_provider->setPerturbationFct(getInputFunction());
            measurement_model->setPerturbationFct(getInputFunction());
        }
    }

    void FullModel::evaluateInput(const double *state, double t) {
        if (!_inputs.input_pram_indices.empty()) {
            for (int index : _inputs.input_pram_indices) { _inputs.modified_parameter[index] = 0; }//_parameter[index]; }
            for (InputPulse &input : _inputs.pulses) {
                if (input.pulseActive(t)) {
                    _inputs.modified_parameter[input.parameter_index] += input._input_strength;
                }
            }
        }
    }


    PerturbationFct_ptr FullModel::getInputFunction() {
        return std::make_shared<PerturbationFct>(std::bind(&FullModel::evaluateInput, this, _1, _2));
    }

    std::vector<double> FullModel::getDiscontTimes() {        return _inputs.getDisContTime();    }

    double FullModel::root(const double *state, double t) {

        double r = 1;
        for (InputPulse &pulse : _inputs.pulses) {
            for (int i = 0; i < pulse.pulse_beginnings.size(); i++) {
                r *= (t - pulse.pulse_beginnings[i]) * (t - pulse.pulse_ends[i]);
            }
            r = r / pulse.pulse_ends.back();
        }
        return r;
    }

    RootFct_ptr FullModel::getRootFct() { return std::make_shared<RootFct>(std::bind(&FullModel::root, this, _1, _2)); }


    MeasurementFct_ptr FullModel::getMeasurementFct() {
        return std::make_shared<MeasurementFct>(
                std::bind(&MeasurementModel::computeMeasurement, this->measurement_model.get(), _1, _2, _3));
    }
}