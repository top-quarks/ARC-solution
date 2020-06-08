#include <chrono>

struct Timer {
  const int skips = 100, brute_thres = 1000;
  bool running = false;
  double cnt = 0, brute_sum = 0;
  int sample_counter = 0, target = 0;
  double samples = 0, sample_sum = 0, sample_var = 0;
  mt19937 mrand;
  double now() {
    return chrono::high_resolution_clock::now().time_since_epoch().count()*1e-9;
  }
  double start_time;
  void start() {
    assert(!running);
    running = true;
    if (cnt < brute_thres || sample_counter == target)
      start_time = now();
  }
  void stop() {
    assert(running);
    running = false;

    double dt;
    if (cnt < brute_thres || sample_counter == target)
      dt = now()-start_time;

    if (cnt < brute_thres) {
      brute_sum += dt;
    }
    if (sample_counter == target) {
      samples++;
      sample_sum += dt;
      sample_var += dt*dt;
      target = mrand()%skips;
    }

    if (++sample_counter == skips)
      sample_counter = 0;
    cnt++;
  }
  tuple<double,double,double> read() {
    if (cnt <= brute_thres)
      return {cnt, brute_sum, 0};
    double mean = sample_sum/samples;
    double var = (sample_var/samples-mean*mean)/(samples-1);
    return {cnt, mean*cnt, sqrt(var)*cnt};
  }
  void print(const string& label) {
    auto [cnt,sum,std] = read();
    printf("%5.1f ± %4.1f ms - %s\n", sum*1e3, std*1e3, label.c_str());
  }
};
