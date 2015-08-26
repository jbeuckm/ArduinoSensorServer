
#include <SPI.h>
#include <Ethernet.h>

// prepare for DHT module reading
#include <dht.h>
#define dht_dpin 5
dht DHT;


//byte mac[] = { 0xD5, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192,168,0,177);
EthernetServer server(80);

void setup() {
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


char new_state[256];

void loop() {
/*
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");

    String request = "";
    
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      
      if (client.available()) {
        char c = client.read();
        
        request += c;

        if (c == '\n' && currentLineIsBlank) {
        
          processRequest(client, request);

          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
*/
  // listen for incoming clients
  EthernetClient client = server.available();

  if (client) {

    Serial.println("Client connected");

    String header = "";
    String body_text = "";

    while (client.connected()) {

      int i = 0;
      int head = 1;
      int body = 0;

      while(client.available()) {
        char c = client.read();
        Serial.print(c);
        if (c == '\r') {
        Serial.print("\\r");
        }
        if (c == '\n') {

          if ( i <= 2 ) {

            // an http request ends with a blank line

            
            if ( head == 1 ) {
              body = 1;
              head = 0;
            }

          }

          i = -1;

        }
        if (head == 1) {
          header += c;
        }
        if ( body == 1 ) {
          new_state[i] = c;
        }
        i++;
        new_state[i] = '\0';
      }
      i = 0;
    }

    sendResponse(client);

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");

    String verb = getRequestVerb(header);
    Serial.println("verb = "+verb);
    if (verb == "POST") {
      updateState(new_state);
    }

  }
}


String getRequestVerb(String header) {
  Serial.println("getRequestVerb()");
  Serial.println(header);
  int idx = header.indexOf("\n");
  String firstLine = header.substring(0, idx);
  
  idx = firstLine.indexOf(" ");
  String method = firstLine.substring(0, idx);

  return method;  
}


void updateState(char *new_state) {
  Serial.println("updateState()");
  Serial.println(new_state);

    // Post data looks like pinD2=On
    if ( strncmp( new_state, "pinD", 4) == 0 ) {
      int pin = new_state[4] - 48; // Convert ascii to int
      Serial.println(pin);
      if ( strncmp( new_state+5, "=On", 3) == 0 ) {
        digitalWrite(pin, 1);
      } 
      else if ( strncmp( new_state+5, "=Off", 4) == 0 ) {
        digitalWrite(pin, 0);
      }
    }
}


void sendResponse(EthernetClient client) {
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close"); 
  
  client.println();
  client.println("{");
  
  DHT.read11(dht_dpin);

  outputPair(client, "temperature", "\""+String((int)DHT.temperature) + "C\"");
  outputPair(client, "humidity", "\""+String((int)DHT.humidity) + "%\"");


  for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
    int sensorReading = analogRead(analogChannel);
    outputPair(client, "analog"+String(analogChannel), String(sensorReading) );
  }
  client.println("}");
}

void outputPair(EthernetClient client, String label, String value) {
  client.println("\t\""+label+"\": "+value+",");
}




