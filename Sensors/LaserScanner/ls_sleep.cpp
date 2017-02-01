#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


using namespace qrk;
using namespace std;

Urg_driver urg;

int main(int argc, char *argv[]) {
  // Connects to the sensor
  if (!urg.open("192.168.0.10", 10940, Urg_driver::Ethernet)) {
    cout << "Urg_driver::open(): " << ": " << urg.what() << endl;
    return 1;
  }

  urg.sleep();

  sleep(10);
  urg.wakeup();
  urg.stop_measurement();

  urg.close();
  return 0;
}
