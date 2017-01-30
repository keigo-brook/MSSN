#include "MQTTClient.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#define QOS 1
#define TIMEOUT 10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
  printf("Message with token value %d delivery confirmed\n", dt);
  deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen,
             MQTTClient_message *message) {
  int i;
  char *payloadptr;

  payloadptr = (char *)message->payload;
  for (i = 0; i < message->payloadlen; i++) {
    putchar(*payloadptr++);
  }
  putchar('\n');
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);
  return 1;
}

void connlost(void *context, char *cause) {
  printf("\nConnection lost\n");
  printf("     cause: %s\n", cause);
}

struct client_params {
  MQTTClient client;
  std::string address;
  std::string clientID;
  std::string topic;
};

client_params create_subclient(std::string address, std::string cID, std::string topic) {
  MQTTClient client;
  MQTTClient_create(&client, address.c_str(), cID.c_str(),
                    MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  int rc;
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
    printf("Failed to connect, return code %d\n", rc);
    exit(-1);
  }
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;

  client_params cp = { client, address, cID, topic};
  return cp;
}

std::vector<client_params> parse_opt(int i, char *t[]) {
  int opt;

  std::vector<std::string> addresses, cIDs, topics;
  while ((opt = getopt(i, t, "a:c:t:")) != -1) {
    //コマンドライン引数のオプションがなくなるまで繰り返す
    switch (opt) {
    case 'a':
      addresses.push_back(optarg);
      break;
    case 'c':
      cIDs.push_back(optarg);
      break;
    case 't':
      topics.push_back(optarg);
      break;
    default:
      //指定していないオプションが渡された場合
      break;
    }
  }

  if (!(addresses.size() == cIDs.size() && cIDs.size() == topics.empty())) {
    printf("ERROR: option size different\n");
    exit(1);
  }

  std::vector<client_params> clients;
  for (int i = 0; i < (int)addresses.size(); i++) {
    clients.push_back(create_subclient(addresses[i], cIDs[i], topics[i]));
  }
  return clients;
}

/*
  -a ip -c client id -t topic をsubscribeしたいぶんだけ
 */
int main(int argc, char *argv[]) {
  opterr = 0;
  auto clients = parse_opt(argc, argv);
  for (auto it = clients.begin(); it != clients.end(); ++it) {
    MQTTClient_subscribe(it->client, it->topic.c_str(), QOS);
  }

  int ch;
  // printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
  //       "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
  do {
    ch = getchar();
  } while (ch != 'Q' && ch != 'q');

  for (auto it = clients.begin(); it != clients.end(); ++it) {
    MQTTClient_disconnect(it->client, 10000);
    MQTTClient_destroy(&it->client);
  }
  return 0;
}
