/* TinyOra
 * a small ircd for ESP8266 Arduino boards (mostly NodeMCU)
 * 
 * Written by Daniel Oaks <daniel@danieloaks.net>.
 * Released as CC0 Public Domain.
 */

// config
#include "defines.h"
// #define WIFI_SSID ""
// #define WIFI_PASS ""

// ircd internals
#define MAX_CLIENTS 5
#define NICKLEN 20
#define USERLEN 10
#define HOSTLEN 30
#define REALNAMELEN 50
#define BUFFERLEN 1024

// wifi stuff
#include <ESP8266WiFi.h>

WiFiServer server(6667);

// includes
#include "clients.h"

int clientCount;
WiFiClient clientConnections[MAX_CLIENTS];
ClientInfo clientInfo [MAX_CLIENTS];
bool clientOccupied[MAX_CLIENTS];

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  // add new clients to our pool
  WiFiClient newClient = server.available();
  if (newClient) {
    if (MAX_CLIENTS < clientCount+1) {
      Serial.print("Client from ");
      Serial.print(newClient.remoteIP());
      Serial.println(" rejected due to client count");

      newClient.print("QUIT * :Max clients connected\r\n");
      newClient.print("ERROR :Max clients connected\r\n");
      delay(1);
      newClient.stop();
    } else {
      Serial.print("Client connected from ");
      Serial.print(newClient.remoteIP());
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clientOccupied[i]) {
          clientConnections[i] = newClient;
          clientOccupied[i] = true;
          Serial.print(" and added as client #");
          Serial.println(i);
          break;
        }
      }
      clientCount++;
    }
  }

  // loop through our connected clients
  WiFiClient c;
  ClientInfo ci;
  for (int clienti = 0; clienti < MAX_CLIENTS; clienti++) {
    if (clientOccupied[clienti]) {
      c = clientConnections[clienti];
      ci = clientInfo[clienti];
      if (!c.connected()) {
        c.stop();
        clientOccupied[clienti] = false;
        clientCount--;
        Serial.print("Client #");
        Serial.print(clienti);
        Serial.println(" disconnected");
        continue;
      }

      //c.print(":irc.example.com NOTICE * :HIHIHIHI!\r\n");

      // read buffer
      int emptybufferindex = 0;
      for (int i = 0; i < BUFFERLEN; i++) {
        if (ci.buffer[i] == '\0') {
          emptybufferindex = i;
          break;
        }
      }
      while (c.available()) {
        char newchar = c.read();
        if (newchar == '\n' || newchar == '\r') {
          Serial.print("Run command from client #");
          Serial.print(clienti);
          Serial.print(": ");
          Serial.println(ci.buffer);
          // empty buffer
          break;
        } else {
          if (BUFFERLEN-1 <= emptybufferindex) {
            c.print("QUIT * :RecvQ Exceeded\r\n");
            c.print("ERROR :RecvQ Exceeded\r\n");
            delay(1);
            c.stop();
            break;
          }

          ci.buffer[emptybufferindex] = newchar;
          emptybufferindex++;
        }
      }

      clientInfo[clienti] = ci;
    }
  }
}
