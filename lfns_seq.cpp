#include <iostream>
#include "src/options/LFNSOptions.h"
#include "LFNSSetup.h"
#include "src/base/IoUtils.h"
#include "src/LFNS/seq/LFNSSeq.h"
#include "src/sampler/UniformSampler.h"
#include "src/sampler/EllipsoidSampler.h"
#include "src/sampler/DpGmmSampler.h"
#include "src/sampler/GaussianSampler.h"
#include "src/sampler/KernelDensityEstimation.h"

static std::string model_summary_suffix = "model_summary";
static std::stringstream model_summary_stream;


options::LFNSOptions lfns_options;

int runLFNS(LFNSSetup &lfns_setup);

int main(int argc, char **argv) {
    try {


        base::RngPtr rng = std::make_shared<base::RandomNumberGenerator>(time(NULL));

        sampler::NormalSamplerData data_1(2);
        data_1.bounds = {{1, 5},
                         {2, 10}};
        data_1.mean << 10, 20;

        sampler::NormalSamplerData data_2(2);
        data_2.bounds = {{-1, 0},
                         {-2, -1}};
        data_2.mean << 1, 2;

        sampler::GaussianSampler de(rng, data_1);
        sampler::KernelDensityEstimation kde(rng, std::make_shared<sampler::GaussianSampler>(de), data_1);


        base::EiMatrix samples(5, 2);
        samples << 1, 2, 2, 3, 3, 4, 4, 5, 5, 6;
//        de.updateDensitySamples(samples);
//        de.writeToStream(std::cout);
        kde.updateDensitySamples(samples);
        kde.writeToStream(std::cout);

        std::stringstream stream_out_de;
        {
            boost::archive::binary_oarchive oar_de(stream_out_de);
            oar_de << kde;
        }


        sampler::GaussianSampler de_2(rng, data_2);
        base::EiMatrix samples_2(5, 2);
        samples_2 << 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4, 0.5, 0.5, 0.6;
//        de_2.updateDensitySamples(samples_2);
//        de_2.writeToStream(std::cout);
        sampler::KernelDensityEstimation kde_2(rng, std::make_shared<sampler::GaussianSampler>(de_2), data_2);
        kde_2.updateDensitySamples(samples_2);
        std::cout << "original second.. " << std::endl;
        kde_2.writeToStream(std::cout);

        {
            boost::archive::binary_iarchive iar_de(stream_out_de);
            iar_de >> kde_2;
        }
//        de_2.writeToStream(std::cout);

        std::cout << "copied second.. " << std::endl;
        kde_2.writeToStream(std::cout);
        std::cout << "here we are again" << std::endl;

//        lfns_options.handleCommandLineOptions(argc, argv);
//
//
//        LFNSSetup likelihood_setup(lfns_options);
//        likelihood_setup.setUp();
//        return runLFNS(likelihood_setup);
    } catch (const std::exception &e) {
        std::cerr << "Failed to run LFNS, exception thrown:\n\t" << e.what() << std::endl;
        return 0;
    }
    return 1;

}

// TODO add check if output files can be saved BEFORE code runs!
int runLFNS(LFNSSetup &lfns_setup) {
    lfns_setup.printSettings(model_summary_stream);
    std::cout << model_summary_stream.str();

    std::string model_summary_file_name = base::IoUtils::appendToFileName(lfns_setup.io_settings.output_file,
                                                                          model_summary_suffix);
    std::ofstream model_summary_file_stream(model_summary_file_name.c_str());
    model_summary_file_stream << model_summary_stream.str();
    model_summary_file_stream.close();


    lfns::seq::LFNSSeq lfns(lfns_setup.lfns_settings, lfns_setup.sampler_settings, lfns_setup.rng,
                            lfns_setup.mult_like_eval.getLogLikeFun());
    if (!lfns_setup.lfns_settings.previous_log_file.empty()) {
        lfns.resumeRum(lfns_setup.lfns_settings.previous_log_file);
    }
    if (lfns_setup.particle_filter_settings.use_premature_cancelation) {
        for (particle_filter::ParticleFilter_ptr filter : lfns_setup.particle_filters) {
            filter->setThresholdPtr(lfns.getPointerToThreshold());
        }
        lfns_setup.mult_like_eval.setThresholdPtr(lfns.getPointerToThreshold());
    }

    lfns.runLFNS();
    return 1;
}
