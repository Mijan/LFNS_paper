//
// Created by jan on 05.04.16.
//

#include <iomanip>
#include "MeasurementModel.h"
#include "ModelExceptions.h"
#include <algorithm>

namespace models {
    using namespace std::placeholders;

    MeasurementModel::MeasurementModel(base::RngPtr rng, MeasurementModelData measurement_model_data)
            : ParserBaseObject(rng, measurement_model_data), _initialized(false),
              _measurement_parsers(measurement_model_data.getNumMeasurements()),
              _log_likelihood_parsers(measurement_model_data.getNumMeasurements()),
              _measurement_model_data(measurement_model_data),
              _measurement_ptr(measurement_model_data.getNumMeasurements()),
              _measurement(measurement_model_data.getNumMeasurements()) {

        for (int i = 0;
             i < measurement_model_data.getNumMeasurements(); i++) { _measurement_ptr[i] = &_measurement[i]; }
    }

    MeasurementModel::~MeasurementModel() {}

    void
    MeasurementModel::computeMeasurement(std::vector<double> *measurement, const std::vector<double> &state, double t) {

        if (!_initialized) {
            std::stringstream os;
            os << "MeasurementModel must be initialized before measurement can be computed!"
               << std::endl;
            if (!_allPointerSet()) { printPointer(os); }
            throw std::runtime_error(os.str());
        }
        if (_perturbation_fct) { (*_perturbation_fct)(state.data(), t); }
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
                (*measurement)[i] = _measurement_parsers[i].Eval();
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
                                              double t) {
        if (!_initialized) {
            std::stringstream os;
            os << "MeasurementModel must be initialized before measurement can be computed!"
               << std::endl;
            if (!_allPointerSet()) { printPointer(os); }
            throw std::runtime_error(os.str());
        }
        if (_perturbation_fct) { (*_perturbation_fct)(state.data(), t); }
        _updateState(state.data());
        _updateMeasurement(measurements);
        try {
            double log_likelihood = 0;
            for (std::size_t i = 0; i < _log_likelihood_parsers.size(); i++) {
                mu::Parser &_log_likelihood = _log_likelihood_parsers[i];
                log_likelihood += _log_likelihood.Eval();
                if (std::isnan(log_likelihood)) {
                    std::stringstream ss;
                    ss << "Computed likelihood is NaN!\n\tExpression: " << _log_likelihood.GetExpr() << std::endl;
                    ss << "\tPointer values:" << std::endl;
                    printPointerValue(ss);
                    throw ModelException(ss.str());
                }
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

        os << "Measurements:" << std::endl;
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

    void MeasurementModel::printPointerValue(std::ostream &os) const {
        BaseObject::printPointerValue(os);
        for (std::size_t measurement_index = 0;
             measurement_index < _measurement_model_data.getNumMeasurements(); measurement_index++) {
            os << _measurement_model_data.getMeasurementNames()[measurement_index] << ":\t"
               << *_measurement_ptr[measurement_index] << std::endl;
        }
    }


    const std::vector<std::string> &MeasurementModel::getMeasurementNames() const {
        return _measurement_model_data.getMeasurementNames();
    };


    std::size_t MeasurementModel::getNumMeasurements() const {
        return _measurement.size();
    }

    void MeasurementModel::_updateMeasurement(const std::vector<double> &measurement) {
        for (int i = 0; i < _measurement_ptr.size(); i++) {
            *_measurement_ptr[i] = measurement[i];
        }
    }
} /* namespace models */

