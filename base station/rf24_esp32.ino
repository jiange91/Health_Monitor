#include <SPI.h>
#include "RF24.h"
#include "EspMQTTClient.h"
#include <ArduinoJson.h>


EspMQTTClient client(
    "smarthome",
    "unnc@2020",
    "127.0.0.1"
    // "10.160.1.47",          // MQTT Broker server ip
    "mqtt_summer_research", // Can be omitted if not needed
    "unnc2020",             // Can be omitted if not needed
    "Device 1",             // Client name that uniquely identify your device
    1883                    // The MQTT port, default to 1883. this line can be omitted
);

RF24 radio(12, 5);
const uint64_t rAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL};
byte pipeNum = 0;

void setup()
{
  Serial.begin(115200);

  //NRF24L01 setup
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  radio.openReadingPipe(1, rAddress[1]);
  radio.openReadingPipe(5, rAddress[5]);

  radio.startListening();
  Serial.print("Initiated.");

  //MQTT setup
  // Optionnal functionnalities of EspMQTTClient :
  client.enableDebuggingMessages();                                          // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater();                                             // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline"); // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  Serial.println("WiFi and MQTT connected.");
}

void loop()
{

  byte received[7];
  if (radio.available(&pipeNum))
  {
    while (radio.available(&pipeNum))
    {
      radio.read(&received, sizeof(byte[7]));
    }

    Serial.println("Direct print from cushion (pipe5):");
    Serial.print("Healthy score: ");
    Serial.println(received[0]);
    Serial.print("Currently sitted: ");
    Serial.println(received[1]);
    Serial.print("Sedentary: ");
    Serial.println(received[2]);
    Serial.print("Concentrated pressure: ");
    Serial.println(received[3]);
    Serial.print("Unbalanced pressure: ");
    Serial.println(received[4]);
    Serial.print("Lack of shift: ");
    Serial.println(received[5]);
    Serial.print("Stands up too slow: ");
    Serial.println(received[6]);
    Serial.println("----------------------------------");

    //Serial.print("From emitter: ");
    //Serial.println(pipeNum);

    StaticJsonDocument<200> doc;
    JsonObject obj = doc.as<JsonObject>();
    doc["health_score"] = received[0];
    doc["sit_status"] = received[1];
    doc["sedentary"] = received[2];
    doc["concentrated_pressure"] = received[3];
    doc["unbalanced_pressure"] = received[4];
    doc["lacking_shift"] = received[5];
    doc["slow_rising"] = received[6];

    String output;
    serializeJson(doc, output);

    client.loop();
    client.publish("cushion", output);
  }
}
