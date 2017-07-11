/**
 * Applies a Lossy Counting Model on a data stream.
 * This example uses a synthetic stream which contains
 * normal distributed ASCII letters
 *
 * As the creation of random numbers is a bottleneck, this implementation
 * pre-calculates them
 */

#include <iostream>
#include <random>
#include <chrono>
#include <util/statistics.hpp>
#include <lossyCountingModel.hpp>

std::normal_distribution<> getDistribution(){
  char lower = '0';
  char upper = 'z';
  char mean  = (upper + lower) / 2;
  return std::normal_distribution<>(mean, 4);
}

int main(int argc, char ** argv){
  // define input stream
  long   stream_size = 1e8;
  double frequency   = 0.05;
  double error       = 0.005 * frequency;
  int    pre_cal_win = 10000;
  int    prog_int    = 10;    // print progess each 10%
  std::vector<char> buffer;

  // use a random normal distribution of [0-z]
  std::random_device rd;
  std::mt19937 gen(rd());
  auto dist = getDistribution();

  // setup counting model
  LossyCountingModel<char> lcm(frequency, error);
  auto lcm_state = lcm.getState();
  buffer.resize(lcm_state.w * pre_cal_win);
  printState(lcm_state);

  std::chrono::high_resolution_clock::duration time_acc{0};
  int num_windows = stream_size / lcm_state.w;
  for(int win = 0; win<num_windows; ++win){
    // calculate numbers
    if(win % pre_cal_win == 0){
#pragma omp parallel for schedule(static, 1000)
      for(unsigned int i=0; i<lcm_state.w * pre_cal_win; ++i){
        buffer[i] = std::round(dist(gen));
      }
    }

    // process stream
    auto begin = std::chrono::high_resolution_clock::now();
    auto begptr = buffer.begin() + (win % pre_cal_win) * lcm_state.w;
    lcm.processWindow(begptr);
    auto end   = std::chrono::high_resolution_clock::now();
    time_acc += end-begin;

    // print progress
    if(win % (num_windows / prog_int) == 0){
      double prog = static_cast<double>(win) / static_cast<double>(num_windows);
      std::cout << "Progress: " << prog * 100 << "%" << std::endl;
    }
  }

  // compute results
  auto results   = lcm.computeOutput();

  printPerformance(lcm.getState(), sizeof(char), time_acc);

  // output results
  std::cout << "Histogram:" << std::endl;
  for(const auto & kv : results){
    std::cout << "{" << kv.first << ", " << kv.second << "}" << std::endl;
  }
  return 0;
}

