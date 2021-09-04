#include <WiFi.h>
#include <ArduinoJson.h>
#include <MAX31855soft.h>

int32_t rawData2 = 0;

const char* ssid     = "MICROTRON TECHNOLOGIES";
const char* password = "micro@440027";

/*
  MAX31855soft(cs, so, sck)

  cs  - chip select
  so  - serial data output
  sck - serial clock input
*/
MAX31855soft myMAX31855_t2(4, 16, 17);

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(5, OUTPUT);      // set the LED pin mode

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(F("MAX6675 OK"));

    server.begin();

  myMAX31855_t2.begin();
    
    while (myMAX31855_t2.getChipID() != MAX31855_ID) {
      Serial.println(F("Chip error")); //(F()) saves string to flash & keeps dynamic memory free
      delay(5000);
    }
}

int value = 0;

void loop(){
  
  while (myMAX31855_t2.detectThermocouple() != MAX31855_THERMOCOUPLE_OK)
  {
    switch (myMAX31855_t2.detectThermocouple())
    {
      case MAX31855_THERMOCOUPLE_SHORT_TO_VCC:
        Serial.println(F("Thermocouple2 short to VCC"));
        break;

      case MAX31855_THERMOCOUPLE_SHORT_TO_GND:
        Serial.println(F("Thermocouple2 short to GND"));
        break;

      case MAX31855_THERMOCOUPLE_NOT_CONNECTED:
        Serial.println(F("Thermocouple2 not connected"));
        break;

      case MAX31855_THERMOCOUPLE_UNKNOWN:
        Serial.println(F("Thermocouple2 unknown error"));
        break;

      case MAX31855_THERMOCOUPLE_READ_FAIL:
        Serial.println(F("Thermocouple2 read error, check chip & cable"));
        break;
    }
    delay(5000);
  }

  rawData2 = myMAX31855_t2.readRawData();

//  Serial.print("Temperature1:");
//  Serial.println(myMAX31855_t1.getTemperature(rawData1));

  Serial.print("Temperature2:");
  Serial.println(myMAX31855_t2.getTemperature(rawData2));
  
  StaticJsonDocument<256> dataJson;
//  dataJson["t1"] = myMAX31855_t1.getTemperature(rawData1);
//  dataJson["t1"] = "100";
//  dataJson["t2"] = myMAX31855_t2.getTemperature(rawData2);
//  char out[128];
//  serializeJson(dataJson, out);

  DynamicJsonDocument doc(1024);
  StaticJsonDocument<256> doc1;
  
  JsonObject json  = doc.createNestedObject("result");
  JsonObject nestedjson = doc1.createNestedObject();
  
  json["success"] = "1";
  json["errorMsg"] = "0";

  
  nestedjson["t1"] = "100";
  nestedjson["t2"] = myMAX31855_t2.getTemperature(rawData2);
  
  json["tempData"] = doc1;
  
  char out[128];
  serializeJson(json, out);

 WiFiClient client = server.available();   

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println();

             
                client.print(out);
              

            // the content of the HTTP response follows the header:
            
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(5, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
