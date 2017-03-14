#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"
#include "ticks.h"
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

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
        // cout << time_stamp << endl;
        for (size_t i = 0; i < data_n; ++i) {
            long l = data[i];
            double radian = urg.index2rad(i);
            long x = static_cast<long>(l * cos(radian));
            long y = static_cast<long>(l * sin(radian));
            cout << x << " " << y << endl;
        }
    }
}


int main(int argc, char *argv[]) {
    // Connects to the sensor
    if (!urg.open("192.168.0.10", 10940, Urg_driver::Ethernet)) {
        cout << "Urg_driver::open(): " << ": " << urg.what() << endl;
        return 1;
    }

    urg.start_measurement(Urg_driver::Distance, 0, 0);
    char buff[256] = "";
    while (true) {
      vector<long> data;
      long time_stamp = 0;

      if (!urg.get_distance(data, &time_stamp)) {
        cout << "Urg_driver::get_distance(): " << urg.what() << endl;
        exit(1);
      }
      struct timeval now;
      gettimeofday(&now, NULL);
      struct tm *pnow = localtime(&now.tv_sec);
      sprintf(buff, "%04d/%02d/%02d %02d:%2d:%2d.%06d", pnow->tm_year + 1900, pnow->tm_mon+1, pnow->tm_mday, pnow->tm_hour, pnow->tm_min, pnow->tm_sec, now.tv_usec);
      std::cout << buff << std::endl;
      print_data(urg, data, time_stamp);
    }

    urg.stop_measurement();

    urg.close();
    return 0;
}
