cmake_minimum_required(VERSION 2.8)

project(DP_GMM)

# Version number
set(DP_GMM_VERSION_MAJOR 2)
set(DP_GMM_VERSION_MINOR 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_BUILD_TYPE Release)

set(Boost_ADDITIONAL_VERSIONS 1.54 1.54.0)

set(BUILD_EXE 1 CACHE FILEPATH "Determines if executable estimate_density will be build. 0: no, 1: yes")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CMAKE_SOURCE_DIR}/src)
add_library(DPGMM SHARED src/DataReader.cpp src/IOUtils.cpp src/AdaptiveRejectionSampler.cpp src/DPGMMEstimator.cpp src/DPGMMLogger.cpp src/DPMixtureComponent.cpp src/HyperParameters.cpp src/IndicatorParameters.cpp src/MultivariateNormalDistribution.cpp src/PostLogAlpha.cpp src/PostLogBeta.cpp src/WishartDistribution.cpp)


install(TARGETS DPGMM DESTINATION lib)
install(FILES src/DataReader.h src/IOUtils.h src/AdaptiveRejectionSampler.h src/BoostDistributions.h src/GaussMixtureComponent.h src/DPMixtureComponent.h src/DPGMMEstimator.h src/DPGMMLogger.h src/HyperParameters.h src/MatrixTypes.h src/MultivariateNormalDistribution.h src/LogConcaveFunction.h src/WishartDistribution.h DESTINATION include)

####################################################################################################
## Packages
####################################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${DP_GMM_SOURCE_DIR}/cmake/Modules/")
## Boost
find_package(Boost 1.54.0 REQUIRED COMPONENTS program_options system filesystem)

## Eigen
find_package(Eigen3)
if (NOT Eigen3_FOUND)
    message("Eigen3 library not found (you can set Eigen3 location through the environment variable EIGEN3_ROOT). Shipped Eigen3 library in ${CMAKE_SOURCE_DIR}/../eigen3.4 will be used!")
    if (NOT DEFINED ENV{EIGEN3_ROOT})
        set(ENV{EIGEN3_ROOT} "${CMAKE_SOURCE_DIR}/../eigen3.4/" CACHE FILEPATH "The location of the Eigen library")
    endif ()
    find_package(Eigen3 REQUIRED)
endif ()
include_directories(${EIGEN3_INCLUDE_DIR})

######################################################################################################
## Build Sequential version
######################################################################################################
add_executable(estimate_density src/DataReader.cpp src/IOUtils.cpp src/AdaptiveRejectionSampler.cpp src/DPGMMEstimator.cpp src/DPGMMLogger.cpp src/DPMixtureComponent.cpp src/EstimationOptions.cpp src/estimate_density.cpp src/HyperParameters.cpp src/IndicatorParameters.cpp src/MultivariateNormalDistribution.cpp src/PostLogAlpha.cpp src/PostLogBeta.cpp src/WishartDistribution.cpp)
install(TARGETS estimate_density DESTINATION bin)
target_link_libraries(estimate_density ${Boost_LIBRARIES})
MESSAGE( "\n Executable will be build!\n")
