
#include <SPI.h>
#include <Ethernet.h>

// prepare for DHT module reading
#include <dht.h>
#define dht_dpin 2
dht DHT;


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,177);
EthernetServer server(80);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
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
}




void outputPair(EthernetClient client, String label, String value) {
  client.println("\t\""+label+"\": "+value+",");
}




void processRequest(EthernetClient client, String request) {
  
  int idx = request.indexOf("\n");
  Serial.println(request.substring(0, idx));

  
  // send a standard http response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  
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

