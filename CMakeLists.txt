cmake_minimum_required(VERSION 3.16)
project(tfg_code LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headers live in /include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---------- Core library (shared code) ----------
add_library(fraud_core
        src/training/MarkovTrainer.cpp
        src/io/ModelReader.cpp
        src/io/DatasetReader.cpp
        src/model/OutlierScorer.cpp
        src/pipeline/MarkovPredictor.cpp
        src/pipeline/Sink.cpp
        src/util/Stats.cpp
)

target_include_directories(fraud_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---------- Offline trainer executable ----------
add_executable(train_markov_model
        src/train_main.cpp
)

target_link_libraries(train_markov_model PRIVATE fraud_core)

# ---------- Online detector executable ----------
add_executable(fraud_detector
        src/main.cpp
)

target_link_libraries(fraud_detector PRIVATE fraud_core)
