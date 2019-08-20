cmake_minimum_required(VERSION 2.8)
project(LFNS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_MATH_DISABLE_FLOAT128")

set(CMAKE_BUILD_TYPE Release)
####################################################################################################
## Packages
####################################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LFNS_SOURCE_DIR}/cmake/Modules/")

## Boost
find_package(Boost 1.54.0 REQUIRED COMPONENTS program_options system filesystem mpi serialization)
include_directories(${Boost_INCLUDE_DIR})

## Eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

## Sundials
find_package(Sundials REQUIRED COMPONENTS cvode nvecserial)
include_directories(${SUNDIALS_INCLUDE_DIR})
MESSAGE("Including Sundials directory ${SUNDIALS_INCLUDE_DIR}")

## DP-GMM
find_package(DPGMM REQUIRED)
include_directories(${DPGMM_INCLUDE_DIR})
MESSAGE("Including DPGMM directory ${DPGMM_INCLUDE_DIR}")

## muparser
find_package(muparser)
include_directories(${MUPARSER_INCLUDE_DIR})
MESSAGE("Including muParser directory ${MUPARSER_INCLUDE_DIR}")

## MPI
set(CMAKE_CXX_COMPILER "mpicxx")
set(CMAKE_LINKER "mpicxx")
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})


set(BASE_SOURCE_FILES src/base/EigenMatrices.h src/base/IoUtils.cpp src/base/IoUtils.h src/base/MathUtils.cpp src/base/MathUtils.h src/base/MultivariateNormal.cpp src/base/MultivariateNormal.h src/base/RandomDistributions.h src/base/Utils.cpp src/base/Utils.h src/base/EigenSerialization.h)
set(IO_SOURCE_FILES src/io/TxtFileReader.cpp src/io/TxtFileReader.h src/io/ModelReactionReader.cpp src/io/ModelReactionReader.h src/io/ParserReader.cpp src/io/ParserReader.h src/io/InitialValueReader.h src/io/InitialValueReader.cpp src/io/MeasurementModelReader.h src/io/MeasurementModelReader.cpp src/io/XmlPropertyMap.cpp src/io/XmlPropertyMap.h src/io/XmlFileReader.cpp src/io/XmlFileReader.h src/io/ConfigFileInterpreter.cpp src/io/ConfigFileInterpreter.h src/io/IoSettings.h)
set(OPTIONS_SOURCE_FILES src/options/CommandLineOptions.cpp src/options/CommandLineOptions.h src/options/DataOptions.h src/options/LFNSOptions.cpp src/options/LFNSOptions.h src/options/SimulationOptions.cpp src/options/SimulationOptions.h src/options/ComputeLikelihoodOptions.cpp src/options/ComputeLikelihoodOptions.h src/options/MCMCOptions.cpp src/options/MCMCOptions.h)
set(LFNS_SOURCE_FILES src/LFNS/LFNSSettings.h src/LFNS/LFNS.cpp src/LFNS/LFNS.h src/LFNS/DeadParticleSet.cpp src/LFNS/DeadParticleSet.h src/LFNS/LiveParticleSet.cpp src/LFNS/LiveParticleSet.h src/LFNS/LFNSParticle.cpp src/LFNS/LFNSParticle.h src/LFNS/LFNSLogger.cpp src/LFNS/LFNSLogger.h src/LFNS/PosteriorEstimator.cpp src/LFNS/PosteriorEstimator.h src/LFNS/LFNSSampler.cpp src/LFNS/LFNSSampler.h src/sampler/SliceSampler.cpp src/sampler/SliceSampler.h)
set(LFNS_SEQ_SOURCE_FILES src/LFNS/seq/LFNSSeq.cpp src/LFNS/seq/LFNSSeq.h)
set(LFNS_MPI_SOURCE_FILES src/LFNS/mpi/LFNSMpi.h src/LFNS/mpi/LFNSMpi.cpp src/LFNS/mpi/LFNSWorker.cpp src/LFNS/mpi/LFNSWorker.h src/LFNS/mpi/MpiRequest.cpp src/LFNS/mpi/MpiRequest.h src/LFNS/mpi/RequestQueue.cpp src/LFNS/mpi/RequestQueue.h src/LFNS/mpi/MpiTags.h)
set(SAMPLE_SOURCE_FILES src/sampler/Sampler.h src/sampler/UniformSampler.h src/sampler/UniformSampler.cpp src/sampler/DensityEstimation.h src/sampler/DpGmmSampler.h src/sampler/DpGmmSampler.cpp src/sampler/EllipsoidSampler.h src/sampler/EllipsoidSampler.cpp src/sampler/KernelDensityEstimation.h src/sampler/KernelDensityEstimation.cpp src/sampler/GaussianSampler.h src/sampler/GaussianSampler.cpp src/sampler/KernelSupportEstimation.cpp src/sampler/KernelSupportEstimation.h src/sampler/RejectionSupportSampler.h src/sampler/RejectionSupportSampler.cpp src/sampler/DensityEstimation.cpp src/sampler/SamplerSettings.h)
set(SIMULATOR_SOURCE_FILES src/simulator/Simulator.h src/simulator/SimulatorSsa.cpp src/simulator/SimulatorSsa.h src/simulator/StoppingCriterion.h src/simulator/SimulatorOde.h src/simulator/SimulatorOde.cpp src/simulator/SimulatorHybrid.cpp src/simulator/SimulatorHybrid.h)
set(PARTICLE_FILTER_SOURCE_FILES src/particle_filter/ParticleFilter.h src/particle_filter/ParticleFilter.cpp src/particle_filter/SmcParticle.h src/particle_filter/SmcParticleSet.h src/particle_filter/SmcParticleSet.cpp src/particle_filter/ParticleFilterSettings.h src/particle_filter/MultLikelihoodEval.h)
set(MODELS_SOURCE_FILES src/models/BaseData.h src/models/BaseData.cpp src/models/BaseObject.h src/models/BaseObject.cpp src/models/ChemicalReactionNetwork.h src/models/ChemicalReactionNetwork.cpp src/models/ModelReactionData.h src/models/ModelReactionData.cpp src/models/ParserBaseObject.h src/models/ParserBaseObject.cpp src/models/ParserData.h src/models/ParserData.cpp src/models/InitialValueProvider.h src/models/InitialValueProvider.cpp src/models/InitialValueData.cpp src/models/InitialValueData.h src/models/MeasurementModel.h src/models/MeasurementModel.cpp src/models/MeasurementModelData.h src/models/MeasurementModelData.cpp src/models/InputPulse.cpp src/models/InputPulse.h src/models/FullModel.cpp src/models/FullModel.h src/models/ModelSettings.h)
set(MCMC_SOURCE_FILES src/MCMC/MCMCSampler.cpp src/MCMC/MCMCSampler.h src/MCMC/MCMC.cpp src/MCMC/MCMC.h src/MCMC/MCMCParticles.cpp src/MCMC/MCMCParticles.h src/MCMC/MCMCLogger.cpp src/MCMC/MCMCLogger.h src/MCMC/MCMCSettings.h)

add_library(base SHARED ${BASE_SOURCE_FILES})
add_library(io SHARED ${IO_SOURCE_FILES})
add_library(options SHARED ${OPTIONS_SOURCE_FILES})
add_library(lfns SHARED ${LFNS_SOURCE_FILES})
add_library(lfnsSeq SHARED ${LFNS_SOURCE_FILES} ${LFNS_SEQ_SOURCE_FILES})
add_library(lfnsMpi SHARED ${LFNS_SOURCE_FILES} ${LFNS_MPI_SOURCE_FILES})
add_library(sampler SHARED ${SAMPLE_SOURCE_FILES})
add_library(simulator SHARED ${SIMULATOR_SOURCE_FILES})
add_library(particle_filter SHARED ${PARTICLE_FILTER_SOURCE_FILES})
add_library(models SHARED ${MODELS_SOURCE_FILES})
add_library(MCMC SHARED ${MCMC_SOURCE_FILES})

target_link_libraries(base ${Boost_LIBRARIES} ${Eigen3_LIBRARIES})
target_link_libraries(io ${Boost_LIBRARIES} ${Eigen3_LIBRARIES})
target_link_libraries(options ${Boost_LIBRARIES})
target_link_libraries(lfns ${Boost_LIBRARIES} base sampler)
target_link_libraries(lfnsSeq ${Boost_LIBRARIES} base sampler lfns)
target_link_libraries(lfnsMpi ${Boost_LIBRARIES} base sampler lfns)
target_link_libraries(sampler ${Boost_LIBRARIES} ${Eigen3_LIBRARIES} ${DPGMM_LIBRARIES})
target_link_libraries(simulator ${Boost_LIBRARIES} ${Eigen3_LIBRARIES} ${SUNDIALS_LIBRARIES} base)
target_link_libraries(particle_filter ${Boost_LIBRARIES} ${Eigen3_LIBRARIES} base simulator)
target_link_libraries(models ${Boost_LIBRARIES} ${Eigen3_LIBRARIES} ${MUPARSER_LIBRARIES} base io)
target_link_libraries(MCMC ${Boost_LIBRARIES} base sampler)


if (MPI_COMPILE_FLAGS)
    set_target_properties(lfnsMpi PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif ()

if (MPI_LINK_FLAGS)
    set_target_properties(lfnsMpi PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif ()

add_executable(lfns_seq lfns_seq.cpp LFNSSetup.cpp LFNSSetup.h GeneralSetup.cpp GeneralSetup.h)
add_executable(lfns_mpi lfns_mpi.cpp LFNSSetup.cpp LFNSSetup.h GeneralSetup.cpp GeneralSetup.h)
add_executable(simulate simulate.cpp SimulationSetup.cpp SimulationSetup.h GeneralSetup.cpp GeneralSetup.h)
add_executable(evaluate_likelihood evaluate_likelihood.cpp LikelihoodSetup.cpp LikelihoodSetup.h GeneralSetup.cpp GeneralSetup.h)
add_executable(mcmc ${MCMC_SOURCE_FILES} GeneralSetup.cpp GeneralSetup.h mcmc.cpp MCMCSetup.cpp MCMCSetup.h)

target_link_libraries(lfns_seq base models particle_filter lfns lfnsSeq options io)
target_link_libraries(lfns_mpi base models particle_filter lfns lfnsMpi options io)
target_link_libraries(simulate base models simulator options io)
target_link_libraries(evaluate_likelihood base models simulator options particle_filter io)
target_link_libraries(mcmc ${Boost_LIBRARIES} base models sampler particle_filter options io)

install(TARGETS mcmc lfns_seq lfns_mpi simulate evaluate_likelihood DESTINATION bin)
install(TARGETS base models simulator options io particle_filter sampler lfns lfnsSeq lfnsMpi DESTINATION lib)


install(FILES src/base/EigenMatrices.h src/base/IoUtils.h src/base/MathUtils.h src/base/MultivariateNormal.h src/base/RandomDistributions.h src/base/Utils.h DESTINATION include)
install(FILES src/io/TxtFileReader.cpp src/io/TxtFileReader.h src/io/ModelReactionReader.h src/io/ParserReader.h src/io/InitialValueReader.h src/io/MeasurementModelReader.h src/io/XmlPropertyMap.h src/io/XmlFileReader.h src/io/ConfigFileInterpreter.h DESTINATION include)
install(FILES src/options/CommandLineOptions.h src/options/DataOptions.h src/options/LFNSOptions.h DESTINATION include)
install(FILES src/LFNS/LFNSSettings.h src/LFNS/LFNS.h src/LFNS/DeadParticleSet.h src/LFNS/LiveParticleSet.h src/LFNS/LFNSParticle.h src/LFNS/LFNSLogger.h src/LFNS/PosteriorEstimator.h src/LFNS/LFNSSampler.h DESTINATION include)
install(FILES src/LFNS/seq/LFNSSeq.h DESTINATION include)
install(FILES src/LFNS/mpi/LFNSMpi.h src/LFNS/mpi/LFNSWorker.h src/LFNS/mpi/MpiRequest.h src/LFNS/mpi/RequestQueue.h src/LFNS/mpi/MpiTags.h DESTINATION include)
install(FILES src/sampler/Sampler.h src/sampler/UniformSampler.h src/sampler/DensityEstimation.h src/sampler/DpGmmSampler.h src/sampler/EllipsoidSampler.h src/sampler/KernelDensityEstimation.h src/sampler/GaussianSampler.h src/sampler/KernelSupportEstimation.h src/sampler/RejectionSupportSampler.h DESTINATION include)
install(FILES src/simulator/Simulator.h src/simulator/SimulatorSsa.h src/simulator/StoppingCriterion.h src/simulator/SimulatorOde.h src/simulator/SimulatorExceptions.h DESTINATION include)
install(FILES src/particle_filter/ParticleFilter.h src/particle_filter/SmcParticle.h src/particle_filter/SmcParticleSet.h DESTINATION include)
install(FILES src/models/BaseData.h src/models/BaseObject.h src/models/ChemicalReactionNetwork.h src/models/ModelReactionData.h src/models/ParserBaseObject.h src/models/ParserData.h src/models/InitialValueProvider.h src/models/InitialValueData.h src/models/MeasurementModel.h src/models/MeasurementModelData.h DESTINATION include)