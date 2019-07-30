//
// Created by jan on 31/10/18.
//


#include "src/options/ComputeLikelihoodOptions.h"
#include "LikelihoodSetup.h"
#include "src/base/IoUtils.h"
#include "src/base/MathUtils.h"

static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;

options::ComputeLikelihoodOptions likelihood_options;

int computeLikelihood(LikelihoodSetup &likelihood_setup);

int main(int argc, char **argv) {
    try {
        likelihood_options.handleCommandLineOptions(argc, argv);


        LikelihoodSetup likelihood_setup(likelihood_options);
        likelihood_setup.setUp();
        return computeLikelihood(likelihood_setup);
    } catch (const std::exception &e) {
        std::cerr << "Failed to run likelihood computation exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
    return 1;

}

// TODO add check if output files can be saved BEFORE code runs!
int computeLikelihood(LikelihoodSetup &likelihood_setup) {
    likelihood_setup.printSettings(model_summary_stream);

    std::cout << model_summary_stream.str();

    std::string model_summary_file_name = base::IoUtils::appendToFileName(likelihood_setup.io_settings.output_file,
                                                                          model_summary_suffix);
//    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
//    model_summary_file_stream << model_summary_stream.str();
//    model_summary_file_stream.close();

    std::string output_file_str = base::IoUtils::appendToFileName(likelihood_setup.io_settings.output_file,
                                                                  likelihood_setup.experiments[0]);
    std::ofstream output_file_file_stream(output_file_str.c_str());

    std::cout << "\n\nLog Likelihoods: " << std::endl;
    time_t tic = clock();
    std::vector<double> log_likelihoods;

    likelihood_setup.mult_like_eval.setVerbose(true);
    likelihood_setup.mult_like_eval.setStream(&output_file_file_stream);
    for (std::vector<double> parameter : likelihood_setup.parameters) {
        for (std::size_t computation_nbr = 0; computation_nbr < likelihood_setup.num_computations; computation_nbr++) {

            double log_likelihood = -DBL_MAX;
            try {
                log_likelihood = likelihood_setup.mult_like_eval.compute_log_like(parameter);
            } catch (const std::exception &e) {
                std::cerr << "Failed to compute likelihood for parameter ";
                for (double &p : parameter) { std::cerr << p << " "; }
                std::cerr << "\n\t" << e.what();
            }
            log_likelihoods.push_back(log_likelihood);
//                std::cout << log_likelihood << std::endl;
        }
    }
    output_file_file_stream.close();

    time_t toc = clock();

    double max_log = -DBL_MAX;
    for (double log_likelihood : log_likelihoods) { max_log = max_log < log_likelihood ? log_likelihood : max_log; }

    double log_like_mean = 0;
    if (likelihood_setup.num_computations > 1) {
        log_like_mean = base::MathUtils::sumOfLog(log_likelihoods, max_log) -
                        std::log(likelihood_setup.num_computations);

        double stddev = 0;
        for (double log_likelihood : log_likelihoods) {
            stddev += (exp(log_likelihood - log_like_mean) - 1) * (exp(log_likelihood - log_like_mean) - 1);
        }
        stddev = log(stddev) - std::log(likelihood_setup.num_computations) + 2 * log_like_mean;
        std::cout << "log mean: " << log_like_mean << " \tlog stddev: " << 0.5 * stddev << "\t";
    }
    std::cout << "time needed: " << base::Utils::getTimeFromClocks(toc - tic) << std::endl << std::endl;

    return 1;
}
