#include <WiFi.h>
#include <ArduinoJson.h>

int32_t rawData1 = 20;
int32_t rawData2 = 40;

const char* ssid     = "MICROTRON TECHNOLOGIES";
const char* password = "micro@440027";

WiFiServer server(80);

void setup(){

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

    server.begin();
}

int value = 0;

void loop(){

  rawData1= rawData1 + 5;
  rawData2= rawData2 + 5;
  

  if(rawData1 >= 80 && rawData2 >= 120) {
    rawData1 = 20;
    rawData2 = 40;
  }

  delay(2000);
  
  StaticJsonDocument<256> dataJson;

  DynamicJsonDocument doc(1024);
  StaticJsonDocument<256> doc1;
  
  JsonObject json  = doc.createNestedObject("result");
  JsonObject nestedjson = doc1.createNestedObject();
  
  json["success"] = "1";
  json["errorMsg"] = "0";

  
  nestedjson["t1"] = rawData1;
  nestedjson["t2"] = rawData2;
  
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
