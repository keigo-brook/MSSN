#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "MQTTClient.h"

#define QOS         1
#define TIMEOUT     10000L

int sendMessage(const char* payload, const char* address, const char* clientID, const char* topic) {
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;
  int rc;

  MQTTClient_create(&client, address, clientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;

  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
      printf("Failed to connect, return code %d\n", rc);
      exit(-1);
    }
  pubmsg.payload = (void *)payload;
  pubmsg.payloadlen = strlen(payload);
  pubmsg.qos = QOS;
  pubmsg.retained = 0;
  MQTTClient_publishMessage(client, topic, &pubmsg, &token);
  printf("Waiting for up to %d seconds for publication of %s\n"
         "on topic %s for client with ClientID: %s\n",
         (int)(TIMEOUT/1000), payload, topic, clientID);
  rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
  printf("Message with delivery token %d delivered\n", token);
  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);
  return rc;
}

int main(int argc, char* argv[]) {
    int i, opt;
    opterr = 0;
    std::string address, clientID, topic;

    while ((opt = getopt(argc, argv, "a:c:t:")) != -1) {
        //コマンドライン引数のオプションがなくなるまで繰り返す
        switch (opt) {
            case 'a':
                address = optarg;
                break;
            case 'c':
                clientID = optarg;
                break;
            case 't':
                topic = optarg;
                break;
            default:
                //指定していないオプションが渡された場合
                printf("Usage: %s [-f] [-g] [-h argment] arg1 ...\n", argv[0]);
                break;
        }
    }

    if (address.empty() || clientID.empty() || topic.empty()) {
        printf("ERROR: option null\n");
        exit(1);
    }
    
    std::string payload;
    while (std::cin>> payload) {
        sendMessage(payload.c_str(), address.c_str(), clientID.c_str(), topic.c_str());
    }

    return 0;
}
