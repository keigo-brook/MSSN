#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace qrk;
using namespace std;

Urg_driver urg;

namespace
{
    void print_data(const Urg_driver& urg,
                    const vector<long>& data, long time_stamp)
    {
    // \~japanese 全てのデータの X-Y の位置を表示
    // \~english Prints the X-Y coordinates for all the measurement points
        size_t data_n = data.size();
        cout << time_stamp << endl;
        for (size_t i = 0; i < data_n; ++i) {
            long l = data[i];
            double radian = urg.index2rad(i);
            long x = static_cast<long>(l * cos(radian));
            long y = static_cast<long>(l * sin(radian));
            cout << x << " " << y << endl;
        }
    }
}

void handler(int s) {
    cout << "stop: " << s << endl;
    urg.stop_measurement();
    exit(0);
}

void long_scan(int capture_times, int sps) {
  int interval = 1 / sps, capture_count = 0;
  while (true) {
    if (capture_times > 0 && capture_count == capture_times) break;
    sleep(interval);
    urg.start_measurement(Urg_driver::Distance, 1, 0);
    long time_stamp = 0;
    vector<long> data;
    if (!urg.get_distance(data, &time_stamp)) {
      cout << "Urg_driver::get_distance(): " << urg.what() << endl;
      exit(1);
    }
    print_data(urg, data, time_stamp);
    urg.stop_measurement();
    capture_count++;
  }
}

void short_scan(int capture_times, int sps) {
  int skip_scan;
  switch (sps) {
  case 40:
    skip_scan = 0;
    break;
  case 10:
    skip_scan = 3;
    break;
  case 4:
    skip_scan = 9;
    break;
  default:
    cout << "Invalid skip per scan: " << sps << endl;
    exit(1);
  }
  // Gets measurement data
  urg.start_measurement(Urg_driver::Distance, capture_times, skip_scan);
  int capture_count = 0;
  while (true) {
    if (capture_times > 0 && capture_count == capture_times) break;
    vector<long> data;
    long time_stamp = 0;

    if (!urg.get_distance(data, &time_stamp)) {
      cout << "Urg_driver::get_distance(): " << urg.what() << endl;
      exit(1);
    }
    print_data(urg, data, time_stamp);
    capture_count++;
  }
  urg.stop_measurement();
}

// sps: scan per second
void scan(int capture_times, int sps) {
  if (sps < 40) {
    long_scan(capture_times, sps);
  } else {
    short_scan(capture_times, sps);
  }
}

int main(int argc, char *argv[]) {
    // Connects to the sensor
    if (!urg.open("192.168.0.10", 10940, Urg_driver::Ethernet)) {
        cout << "Urg_driver::open(): " << ": " << urg.what() << endl;
        return 1;
    }

    string message;
    int mode = 0, p_id;
    signal(SIGINT, handler);
    while (true) {
      if ((p_id = fork()) == 0) {
        // child process
        if (mode == 0) {
          scan(0, 1);
        } else {
          scan(0, 40);
        }
      } else {
        // parent process
        while (cin >> message) {
          if (mode == 0 && message == "Detect") {
            mode = 1;
            break;
          } else if (mode == 1 && message == "Undetect") {
            mode = 0;
            break;
          }
        }
        kill(p_id, SIGINT);
        wait(NULL);
      }
    }

    urg.close();
    return 0;
}
