//
// Created by jan on 05.04.16.
//

#include <iomanip>
#include "MeasurementModel.h"
#include <algorithm>

namespace models {
    using namespace std::placeholders;

    MeasurementModel::MeasurementModel(base::RngPtr rng, MeasurementModelData measurement_model_data)
            : ParserBaseObject(rng, measurement_model_data), _initialized(false),
              _measurement_parsers(measurement_model_data.getNumMeasurements()),
              _log_likelihood_parsers(measurement_model_data.getNumMeasurements()),
              _measurement_model_data(measurement_model_data),
              _measurement_ptr(measurement_model_data.getNumMeasurements()),
              _measurement(measurement_model_data.getNumMeasurements()), _output_measurement_ext_by_int(),
              _input_measurement_ext_by_int() {

        for (int i = 0; i < measurement_model_data.getNumMeasurements(); i++) {
            _measurement_ptr[i] = &_measurement[i];
            _output_measurement_ext_by_int[i] = i;
            _input_measurement_ext_by_int[i] = i;
        }
        _initialize();
    }

    MeasurementModel::~MeasurementModel() {}

    void
    MeasurementModel::computeMeasurement(std::vector<double> *measurement, const std::vector<double> &state, double t,
                                         const std::vector<double> &theta) {

        if (!_all_inputs_defined) {
            std::stringstream os;
            os << "The input order for MeasurementModel must be defined before measurement can be computed!"
               << std::endl;
            _printInputs();
            throw std::runtime_error(os.str());
        }
        _updateTheta(theta);
        _updateState(state.data());
        _createRandomNumbers();

        if (measurement->size() != _measurement_parsers.size()) {
            std::stringstream os;
            os << "Tried to simulate " << measurement->size() << " measurements, but model has "
               << _measurement_parsers.size() << " measurements" << std::endl;
            throw std::runtime_error(os.str());
        }
        for (std::size_t i = 0; i < _measurement_parsers.size(); i++) {
            try {
                (*measurement)[_output_measurement_ext_by_int[i]] = _measurement_parsers[i].Eval();
            } catch (mu::ParserError &e) {
                std::ostringstream os;
                os << "Parser error for equation measurement simulation"
                   << " and expression : " << e.GetExpr() << std::endl;
                os << "Message:  " << e.GetMsg() << "\n";
                os << "Formula:  " << e.GetExpr() << "\n";
                os << "Token:    " << e.GetToken() << "\n";
                os << "Position: " << e.GetPos() << "\n";
                os << "Errc:     " << e.GetCode() << "\n";
                os << "Hidden Species: " << std::endl;
                for (std::size_t i = 0; i < _state_ptrs.size(); i++) {
                    os << "\t" << _base_data.getSpeciesNames()[i] << ": " << *_state_ptrs[i] << std::endl;
                }
                throw std::runtime_error(os.str());
            }

        }
    }

    double MeasurementModel::getLogLikelihood(const std::vector<double> &state, const std::vector<double> &measurements,
                                              const std::vector<double> &theta) {
        if (!_all_inputs_defined) {
            std::stringstream os;
            os << "The input order for MeasurementModel must be defined before log likelihood can be computed!"
               << std::endl;
            _printInputs();
            throw std::runtime_error(os.str());
        }
        _updateTheta(theta);
        _updateState(state.data());
        _updateMeasurement(measurements);
        try {
            double log_likelihood = 0;
            for (std::size_t i = 0; i < _log_likelihood_parsers.size(); i++) {
                mu::Parser &_log_likelihood = _log_likelihood_parsers[i];
                log_likelihood += _log_likelihood.Eval();
            }
            return log_likelihood;
        } catch (mu::ParserError &e) {
            std::ostringstream os;
            os << "Parser error for equation log likelihood computation"
               << " and expression : " << e.GetExpr() << std::endl;
            os << "Message:  " << e.GetMsg() << "\n";
            os << "Formula:  " << e.GetExpr() << "\n";
            os << "Token:    " << e.GetToken() << "\n";
            os << "Position: " << e.GetPos() << "\n";
            os << "Measured State: " << std::endl;
            for (std::size_t i = 0; i < _measurement_ptr.size(); i++) {
                os << "\t" << _measurement_model_data.getMeasurementNames()[i] << ": " << *_measurement_ptr[i]
                   << std::endl;
            }
            os << "Hidden State:" << std::endl;
            for (std::size_t i = 0; i < _state_ptrs.size(); i++) {
                os << "\t" << _base_data.getSpeciesNames()[i] << ": " << *_state_ptrs[i] << std::endl;
            }
            throw std::runtime_error(os.str());
        }
    }


    LikelihoodFct_ptr MeasurementModel::getLikelihoodFct() {
        return std::make_shared<LikelihoodFct>(std::bind(&MeasurementModel::getLogLikelihood, this, _1, _2, _3));
    }

    void MeasurementModel::printInfo(std::ostream &os) {
        os << "----------   Measurement Model   ----------" << std::endl;
        ParserBaseObject::printInfo(os);
        std::size_t max_output_name = 0;
        for (const std::string &name : _measurement_model_data.getMeasurementNames()) {
            if (max_output_name < name.size()) { max_output_name = name.size(); }
        }

        os << "\n\nMeasurements:" << std::endl;
        for (std::size_t i = 0; i < _measurement_parsers.size(); i++) {
            os << std::setw(max_output_name + 7) << _measurement_model_data.getMeasurementNames()[i] << " = "
               << _measurement_parsers[i].GetExpr() << std::endl;
        }
        os << std::endl;

        os << "Log Likelihood:" << std::endl;
        for (std::size_t i = 0; i < _measurement_model_data.getNumMeasurements(); i++) {
            os << std::setw(max_output_name + 7) << _measurement_model_data.getMeasurementNames()[i] << ":\t"
               << _log_likelihood_parsers[i].GetExpr()
               << std::endl;
        }

    }

    void MeasurementModel::_initialize() {
        try {
            if (!_allPointerSet()) {
                std::stringstream os;
                os << "Tried to initialize MeasurementModel before all pointers have been set. The pointers are:"
                   << std::endl;
                printPointer(os);
                throw std::runtime_error(os.str());
            }

            for (const std::string measurement_name : _measurement_model_data.getMeasurementNames()) {
                std::size_t index = _measurement_model_data.getMeasurementIndex(measurement_name);
                _initializeParser(_log_likelihood_parsers[index]);
                _log_likelihood_parsers[index].SetExpr(
                        _measurement_model_data.getLogLikelihoodEquationForMeasurment(measurement_name));
                for (const std::string measurement_name_ : _measurement_model_data.getMeasurementNames()) {
                    _log_likelihood_parsers[index].DefineVar(measurement_name_,
                                                             _measurement_ptr[_measurement_model_data.getMeasurementIndex(
                                                                     measurement_name_)]);
                }

                _initializeParser(_measurement_parsers[index]);
                _measurement_parsers[index].SetExpr(
                        _measurement_model_data.getMeasurementEquationForMeasurment(measurement_name));


            }
            _initialized = true;
        } catch (const std::exception &e) {
            std::stringstream os;
            os << "Failed to initialize MeasurementModel:" << std::endl;
            os << "\t" << e.what() << std::endl;
            throw std::runtime_error(os.str());
        }
    }

    bool MeasurementModel::_allPointerSet() const {

        if (!BaseObject::_allPointerSet()) { return false; }

        for (std::size_t i = 0; i < _measurement_ptr.size(); ++i) {
            if (!_measurement_ptr[i]) { return false; }
        }
        return true;
    }

    void MeasurementModel::printPointer(std::ostream &os) const {
        BaseObject::printPointer(os);
        for (std::size_t measurement_index = 0;
             measurement_index < _measurement_model_data.getNumMeasurements(); measurement_index++) {
            os << _measurement_model_data.getMeasurementNames()[measurement_index] << ":\t"
               << _measurement_ptr[measurement_index] << std::endl;
        }
    }


    const std::vector<std::string> &MeasurementModel::getMeasurementNames() const {
        return _measurement_model_data.getMeasurementNames();
    };


    void MeasurementModel::setOutputMeasurementOrder(std::vector<std::string> measurement_order) {

        _output_measurement_ext_by_int.clear();
        for (int i = 0; i < measurement_order.size(); i++) {
            std::string measurement_name = measurement_order[i];
            bool is_measurement = _measurement_model_data.isMeasurementName(measurement_name);
            if (is_measurement) {
                int state_index = _measurement_model_data.getMeasurementIndex(measurement_name);
                _input_measurement_ext_by_int[state_index] = i;
            } else {
                std::stringstream ss;
                ss << "Failed to set order of output measurements for MeasurementModel. Output state "
                   << measurement_name << " requested, but no such measurement defined!" << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        _all_inputs_defined = _allInputsDefined();
    }

    void MeasurementModel::setInputMeasurementOrder(std::vector<std::string> measurement_order) {
        _input_measurement_ext_by_int.clear();
        for (int i = 0; i < measurement_order.size(); i++) {
            std::string measurement_name = measurement_order[i];
            bool is_measurement = _measurement_model_data.isMeasurementName(measurement_name);
            if (is_measurement) {
                int state_index = _measurement_model_data.getMeasurementIndex(measurement_name);
                _input_measurement_ext_by_int[state_index] = i;
            }
        }
        _all_inputs_defined = _allInputsDefined();
    }


    void MeasurementModel::_updateMeasurement(const std::vector<double> &measurement) {
        for (int i = 0; i < _measurement_ptr.size(); i++) {
            *_measurement_ptr[i] = measurement[_input_measurement_ext_by_int[i]];
        }
    }


    bool MeasurementModel::_allInputsDefined() {
        if (!ParserBaseObject::_allInputsDefined()) { return false; }
        for (int i = 0; i < _measurement_model_data.getNumMeasurements(); i++) {
            if (!_input_measurement_ext_by_int.count(i)) { return false; }
            if (!_output_measurement_ext_by_int.count(i)) { return false; }
        }
        return true;
    }

    void MeasurementModel::_printInputs() {
        ParserBaseObject::_printInputs();
        std::cout << "Measurements:" << std::endl;
        for (int i = 0; i < _measurement_model_data.getNumMeasurements(); i++) {
            std::string measurement_name = _measurement_model_data.getMeasurementNames()[i];
            std::cout << measurement_name << ":\t";
            std::cout << " input nbr ";
            if (_input_measurement_ext_by_int.count(i)) { std::cout << _input_parameter_ext_by_int[i]; }
            std::cout << " output nbr ";
            if (_output_measurement_ext_by_int.count(i)) { std::cout << _output_measurement_ext_by_int[i]; }
            std::cout << std::endl;
        }
    }


} /* namespace models */

