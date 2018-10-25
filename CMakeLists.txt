cmake_minimum_required(VERSION 2.8)
project(LFNS)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


#set(CMAKE_BUILD_TYPE Release)
####################################################################################################
## Packages
####################################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LFNS_SOURCE_DIR}/cmake/Modules/")

## Boost
find_package(Boost 1.54.0 COMPONENTS program_options system filesystem mpi)
if (NOT Boost_FOUND)
    message("Boost library not found (you can set Boost location with the option -DBOOST_ROOT=<path to boost>). Shipped Boost library in ${CMAKE_SOURCE_DIR}/required_packages will be used!")
    if (NOT DEFINED ENV{BOOST_ROOT})
        set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/required_packages/" CACHE FILEPATH "The location of the Boost library")
    endif ()
    find_package(Boost 1.54.0 REQUIRED COMPONENTS program_options system filesystem mpi)
endif ()
include_directories(${Boost_INCLUDE_DIR})

## Eigen
find_package(Eigen3)
if (NOT Eigen3_FOUND)
    message("Eigen3 library not found (you can set Eigen3 locat:q
    ion through the environment variable EIGEN3_ROOT). Shipped Eigen3 library in ${CMAKE_SOURCE_DIR}/required_packages/eigen3.4 will be used!")
    if (NOT DEFINED ENV{EIGEN3_ROOT})
        set(ENV{EIGEN3_ROOT} "${CMAKE_SOURCE_DIR}/required_packages/eigen3.4/" CACHE FILEPATH "The location of the Eigen library")
    endif ()
    find_package(Eigen3 REQUIRED)
endif ()

## Sundials
find_package(Sundials COMPONENTS cvode nvecserial)
if (NOT SUNDIALS_FOUND)
    message("Sundials library not found (you can set Sundials location through the environment variable MUPARSER_ROOT). Shipped Sundials library in ${CMAKE_SOURCE_DIR}/required_packages/cvode-2.9.0 will be used!")
    if (NOT DEFINED ENV{SUNDIALS_ROOT})
        set(ENV{SUNDIALS_ROOT} "${CMAKE_SOURCE_DIR}/required_packages/cvode-2.9.0/" CACHE FILEPATH
                "The location of the cvode library")
    endif ()
    find_package(Sundials REQUIRED COMPONENTS cvode nvecserial)
endif ()
include_directories(${SUNDIALS_INCLUDE_DIR})
MESSAGE("Including Sundials directory ${SUNDIALS_INCLUDE_DIR}")

## DP-GMM
if (NOT DEFINED ENV{DPGMM_ROOT})
    set(ENV{DPGMM_ROOT} "${CMAKE_SOURCE_DIR}/../../DPGMM_INSTALL/" CACHE FILEPATH
            "The location of the DPGMM library")
endif ()
find_package(DPGMM REQUIRED)
include_directories(${DPGMM_INCLUDE_DIR})
MESSAGE("Including DPGMM directory ${DPGMM_INCLUDE_DIR}")

## muparser
find_package(muparser)
if (NOT MUPARSER_FOUND)
    message("muparser library not found (you can set muparser location through the environment variable MUPARSER_ROOT). Shipped muparser library in ${CMAKE_SOURCE_DIR}/required_packages/muparser-2.2.5 will be used!")
    if (NOT DEFINED ENV{MUPARSER_ROOT})
        set(ENV{MUPARSER_ROOT} "${CMAKE_SOURCE_DIR}/required_packages/muparser-2.2.5/" CACHE FILEPATH
                "The location of the mu_parser library")
    endif ()
    find_package(muparser REQUIRED)
endif ()
include_directories(${MUPARSER_INCLUDE_DIR})
MESSAGE("Including muParser directory ${MUPARSER_INCLUDE_DIR}")

## MPI
set(CMAKE_CXX_COMPILER "mpicxx")
set(CMAKE_LINKER "mpicxx")
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})


set(BASE_SOURCE_FILES src/base/EigenMatrices.h src/base/IoUtils.cpp src/base/IoUtils.h src/base/MathUtils.cpp src/base/MathUtils.h src/base/MultivariateNormal.cpp src/base/MultivariateNormal.h src/base/RandomDistributions.h src/base/Utils.cpp src/base/Utils.h)
set(IO_SOURCE_FILES src/io/TxtFileReader.cpp src/io/TxtFileReader.h src/io/ModelReactionReader.cpp src/io/ModelReactionReader.h src/io/ParserReader.cpp src/io/ParserReader.h src/io/InitialValueReader.h src/io/InitialValueReader.cpp src/io/MeasurementModelReader.h src/io/MeasurementModelReader.cpp src/io/XmlPropertyMap.cpp src/io/XmlPropertyMap.h src/io/XmlFileReader.cpp src/io/XmlFileReader.h src/io/ConfigFileInterpreter.cpp src/io/ConfigFileInterpreter.h)
set(OPTIONS_SOURCE_FILES src/options/CommandLineOptions.cpp src/options/CommandLineOptions.h src/options/DataOptions.h src/options/LFNSOptions.cpp src/options/LFNSOptions.h)
set(LFNS_SOURCE_FILES src/LFNS/LFNSSettings.cpp src/LFNS/LFNSSettings.h src/LFNS/LFNS.cpp src/LFNS/LFNS.h src/LFNS/DeadParticleSet.cpp src/LFNS/DeadParticleSet.h src/LFNS/LiveParticleSet.cpp src/LFNS/LiveParticleSet.h src/LFNS/LFNSParticle.cpp src/LFNS/LFNSParticle.h src/LFNS/LFNSLogger.cpp src/LFNS/LFNSLogger.h src/LFNS/PosteriorEstimator.cpp src/LFNS/PosteriorEstimator.h src/LFNS/LFNSSampler.cpp src/LFNS/LFNSSampler.h)
set(LFNS_SEQ_SOURCE_FILES src/LFNS/seq/LFNSSeq.cpp src/LFNS/seq/LFNSSeq.h)
set(LFNS_MPI_SOURCE_FILES src/LFNS/mpi/LFNSMpi.h src/LFNS/mpi/LFNSMpi.cpp src/LFNS/mpi/LFNSWorker.cpp src/LFNS/mpi/LFNSWorker.h src/LFNS/mpi/MpiRequest.cpp src/LFNS/mpi/MpiRequest.h src/LFNS/mpi/RequestQueue.cpp src/LFNS/mpi/RequestQueue.h src/LFNS/mpi/MpiTags.h)
set(SAMPLE_SOURCE_FILES src/sampler/Sampler.h src/sampler/UniformSampler.h src/sampler/UniformSampler.cpp src/sampler/DensityEstimation.h src/sampler/DpGmmSampler.h src/sampler/DpGmmSampler.cpp src/sampler/EllipsoidSampler.h src/sampler/EllipsoidSampler.cpp src/sampler/KernelDensityEstimation.h src/sampler/KernelDensityEstimation.cpp src/sampler/GaussianSampler.h src/sampler/GaussianSampler.cpp src/sampler/KernelSupportEstimation.cpp src/sampler/KernelSupportEstimation.h src/sampler/RejectionSupportSampler.h src/sampler/RejectionSupportSampler.cpp src/sampler/DensityEstimation.cpp)
set(SIMULATOR_SOURCE_FILES src/simulator/Simulator.h src/simulator/SimulatorSsa.cpp src/simulator/SimulatorSsa.h src/base/StoppingCriterion.h src/simulator/SimulatorOde.h src/simulator/SimulatorOde.cpp)
set(PARTICLE_FILTER_SOURCE_FILES src/particle_filter/ParticleFilter.h src/particle_filter/ParticleFilter.cpp src/particle_filter/SmcParticle.h src/particle_filter/SmcParticleSet.h src/particle_filter/SmcParticleSet.cpp)
set(MODELS_SOURCE_FILES src/models/BaseData.h src/models/BaseData.cpp src/models/BaseObject.h src/models/BaseObject.cpp src/models/ChemicalReactionNetwork.h src/models/ChemicalReactionNetwork.cpp src/models/ModelReactionData.h src/models/ModelReactionData.cpp src/models/ParserBaseObject.h src/models/ParserBaseObject.cpp src/models/ParserData.h src/models/ParserData.cpp src/models/InitialValueProvider.h src/models/InitialValueProvider.cpp src/models/InitialValueData.cpp src/models/InitialValueData.h src/models/MeasurementModel.h src/models/MeasurementModel.cpp src/models/MeasurementModelData.h src/models/MeasurementModelData.cpp)

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


if (MPI_COMPILE_FLAGS)
    set_target_properties(lfnsMpi PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif ()

if (MPI_LINK_FLAGS)
    set_target_properties(lfnsMpi PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif ()


add_executable(test_BEestimation test_BEestimation.cpp)
add_executable(test_lfns test_lfns.cpp)
add_executable(test_sampler test_sampler.cpp)
add_executable(test_simulator test_simulator.cpp)
add_executable(test_particle_filter test_particle_filter.cpp)
add_executable(test_models test_models.cpp)
add_executable(lfns_seq lfns_seq.cpp)
add_executable(lfns_mpi lfns_mpi.cpp)
add_executable(simulate simulate.cpp)

target_link_libraries(test_BEestimation lfns)
target_link_libraries(test_lfns lfnsSeq)
target_link_libraries(test_sampler sampler lfns)
target_link_libraries(test_simulator simulator base)
target_link_libraries(test_particle_filter simulator particle_filter models)
target_link_libraries(test_models base models)
target_link_libraries(lfns_seq base models particle_filter lfns lfnsSeq options)
target_link_libraries(lfns_mpi base models particle_filter lfns lfnsMpi options)
target_link_libraries(simulate base models simulator)
