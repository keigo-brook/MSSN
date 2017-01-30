#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "MQTTClient.h"
#include <string>
#include <string.h>
#include <sstream>
#include <future>
#include <map>

using namespace qrk;
using namespace std;
#define QOS         1
#define TIMEOUT     10000L

MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
MQTTClient_deliveryToken token;
int rc;

int sendMessage(const char* payload) {
  pubmsg.payload = (void *)payload;
  pubmsg.payloadlen = strlen(payload);
  MQTTClient_publishMessage(client, "/LS/data", &pubmsg, &token);
  rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
  // printf("Message with delivery token %d delivered\n", token);
  return rc;
}

Urg_driver urg;
namespace
{
  string pack_data(const vector<long> &data, long time_stamp) {
    size_t data_n = data.size();
    stringstream ss;
    ss << time_stamp << endl;
    for (size_t i = 0; i < data_n; ++i) {
      long l = data[i];
      double radian = urg.index2rad(i);
      long x = static_cast<long>(l * cos(radian));
      long y = static_cast<long>(l * sin(radian));
      ss << x << " " << y << endl;
    }
    return ss.str();
  }
  // send one scan data
  void send_data(const vector<long>& data, long time_stamp) {
    string payload = pack_data(data, time_stamp);
    sendMessage(payload.c_str());
  }

}

void handler(int s) {
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
    send_data(data, time_stamp);
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
  int data_count = 0;
  string payload = "";
  while (true) {
    if (capture_times > 0 && capture_count == capture_times) break;
    vector<long> data;
    long time_stamp = 0;

    if (!urg.get_distance(data, &time_stamp)) {
      cout << "Urg_driver::get_distance(): " << urg.what() << endl;
      exit(1);
    }
    if (data_count < 10) {
      payload += pack_data(data, time_stamp);
      data_count++;
    } else {
      std::thread t([payload] { sendMessage(payload.c_str()); });
      t.detach();
      payload = "";
      data_count = 0;
    }
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

vector<string> split(const string &s, char delim) {
  vector<string> elems;
  stringstream ss(s);
  string item;
  while (getline(ss, item, delim)) {
    if (!item.empty()) {
      elems.push_back(item);
    }
  }
  return elems;
}

int main() {
    // Connects to the sensor
    if (!urg.open("192.168.0.10", 10940, Urg_driver::Ethernet)) {
        cout << "Urg_driver::open(): " << ": " << urg.what() << endl;
        return 1;
    }

    string payload;
    MQTTClient_create(&client, "tcp://192.168.101.188:1883", "LS", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    string message;
    int p_id = getpid();
    int mode = 0;
    signal(SIGINT, handler);
    while (true) {
      if (p_id == 0) {
        // child process
        if (mode == 0) {
          scan(0, 1);
        } else {
          scan(0, 40);
        }
      } else {
        // parent process
        while (cin >> message) {
          // cout << m.size();
          cout << message;
          if (message == "Q") {
            return 0;
          } else if (mode == 0 && (message == "2" || message == "Detect")) {
            mode = 1;
            break;
          } else if (mode == 1 && message == "0") {
            mode = 0;
            break;
          }
        }
        kill(p_id, SIGINT);
        wait(NULL);
      }
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    urg.close();
    return 0;
}
