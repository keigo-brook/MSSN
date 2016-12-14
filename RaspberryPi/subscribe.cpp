#include <string>
#include <unistd.h>
#include <vector>
#include <sstream>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#define QOS         1
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

std::vector<std::string> split(const std::string &str, char sep) {
  std::vector<std::string> v;
  std::stringstream ss(str);
  std::string buffer;
  while( std::getline(ss, buffer, sep) ) {
    v.push_back(buffer);
  }
  return v;
}


void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    payloadptr = (char *)message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[])
{
    int i, opt;
    opterr = 0;

    std::string address, clientID;
    std::vector<std::string> topic;

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
              topic = split(optarg, ' ');
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
    MQTTClient client, client2;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, address.c_str(), clientID.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_create(&client2, address.c_str(), "RPi-idsdetect", MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    MQTTClient_setCallbacks(client2, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    if ((rc = MQTTClient_connect(client2, &conn_opts)) != MQTTCLIENT_SUCCESS)
      {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
      }
    //printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
    //       "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, topic[0].c_str(), QOS);
    MQTTClient_subscribe(client2, topic[1].c_str(), QOS);

    do
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

