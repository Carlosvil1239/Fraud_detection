# Fraud Detection Performance Improvement

This repository contains the implementation developed for the Bachelor's Thesis **"Improving performance of fraud detection software"**.

The project implements a credit card fraud detection pipeline in modern C++. The application is based on a sequence model: historical transaction data are used to train a Markov-style transition model, and new transaction sequences are later scored using sliding windows per entity.

The main objective of the project is not to propose a new fraud detection model, but to improve the structure and performance of the software implementation. The final version includes both a sequential backend and a parallel backend based on Intel oneAPI Threading Building Blocks.

## Project Structure

```text
include/        Header files
src/            Source files
CMakeLists.txt  CMake build configuration
README.md       Project description
```

The implementation is organized around two main executables:

- `train_markov_model`: builds a transition model from a transaction dataset.
- `fraud_detector`: loads a trained model and detects anomalous transaction sequences.

## Dataset Format

The input dataset is expected to contain one transaction per line, using a comma-separated format similar to:

```text
entity_id,transaction_id,state
```

Example:

```text
P759LJMWV5,PVA7422TZHPN,HNN
DR23QBC13L,Y71RI5CSWNPG,LNS
C815ZV4PBP,12FFV15K740D,LNL
```

The first field identifies the entity, while the last field represents the symbolic transaction state used by the Markov model.

## Build Instructions

The project uses CMake.

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

If Intel oneAPI TBB is available, the parallel backend can be built and used.

## Training

To train a model from a dataset using the sequential backend:

```bash
./train_markov_model --input credit-card.dat --output model.txt --backend seq
```

Using the TBB backend:

```bash
./train_markov_model --input credit-card.dat --output model.txt --backend tbb
```

The training stage reads the dataset, extracts the symbolic states and counts transitions between consecutive states of the same entity. The result is written as a model file.

## Fraud Detection

To run the detector using the sequential backend:

```bash
./fraud_detector --input credit-card.dat --model model.txt --window 5 --threshold 0.9 --backend seq
```

Using the TBB backend:

```bash
./fraud_detector --input credit-card.dat --model model.txt --window 5 --threshold 0.9 --backend tbb
```

The detector keeps a sliding window for each entity and computes an anomaly score using the trained transition model. Events whose score is above the threshold are counted as outliers.

## Benchmarking

The implementation reports execution statistics such as:

- Total processed events
- Execution time
- Throughput
- Number of detected outliers
- Latency statistics
- Predictor statistics

These values are used to compare the sequential and parallel versions of the application.

## Technologies Used

- C++
- CMake
- Intel oneAPI Threading Building Blocks
- Markov transition models
- Sliding-window anomaly scoring

## Academic Context

This project was developed as part of the Bachelor's Degree in Applied Mathematics and Computing at Universidad Carlos III de Madrid.

Author: Carlos Villacañas Iglesias  
Thesis title: Improving performance of fraud detection software
