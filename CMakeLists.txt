cmake_minimum_required(VERSION 3.16)
project(tfg_code LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(TBB REQUIRED)

add_library(fraud_core
        src/training/MarkovTrainer.cpp
        src/training/MarkovTrainerSequential.cpp
        src/training/MarkovTrainerTBB.cpp
        src/inference/FraudDetector.cpp
        src/inference/FraudDetectorSequential.cpp
        src/inference/FraudDetectorTBB.cpp
        src/io/ModelReader.cpp
        src/io/DatasetReader.cpp
        src/inference/OutlierScorer.cpp
        src/inference/MarkovPredictor.cpp
        src/inference/Sink.cpp
        src/stats/Stats.cpp
)

target_include_directories(fraud_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fraud_core PUBLIC TBB::tbb)

add_executable(train_markov_model
        src/train_main.cpp
)

target_link_libraries(train_markov_model PRIVATE fraud_core)

add_executable(fraud_detector
        src/main.cpp
)

target_link_libraries(fraud_detector PRIVATE fraud_core)
