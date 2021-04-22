// based on esp32 arduino core libraries v1.0.6

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const int offset = 20;
const int bufSize = 300;

int bufIndex = 0;
int buf[bufSize];

const int pulsePin = 34;

const char *ssid = "Nokia 6.1 Plus";
const char *pass = "V@fancul0!";

const char *udpAddress = "192.168.43.55";
const int udpPort = 8000;

WiFiUDP udp;
OSCMessage msg("/hr");

int cnt = 0;
int computeHrEvery = 50;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < bufSize; i++) {
    buf[i] = 0;
  }
}

void loop() {
  int val = analogRead(pulsePin);
  buf[bufIndex] = val;
  bufIndex = (bufIndex + 1) % bufSize;

  //Serial.println(val);

  // Compute HR with current buffer
  if (cnt == computeHrEvery) {
    cnt = 0;
    
    // compute min max
    int minBuf = 4096;
    int maxBuf = 0;

    for (int i = 0; i < bufSize; i++) {
      int v = buf[(bufIndex + i) % bufSize];
      if (v < minBuf) minBuf = v;
      if (v > maxBuf) maxBuf = v;
    }

    // mean crossing algorithm
    int mean = minBuf + (maxBuf - minBuf) / 2 + offset;
    bool above = buf[bufIndex] > mean;
    int nbIndexes = 0;
    int indexes[bufSize];
  
    for (int i = 0; i < bufSize; i++) {
      int v = buf[(bufIndex + i) % bufSize];
      if (v > mean && !above) {
        indexes[nbIndexes] = i;
        nbIndexes++;
        above = true;
      }
  
      if (v < mean) above = false;
    }

    int deltas[nbIndexes - 1];
    for (int i = 0; i < nbIndexes - 1; i++) {
      deltas[i] = (indexes[i + 1] - indexes[i]) * 10;
    }

    float sum = 0;
    for (int i = 0; i < nbIndexes - 1; i++) {
      sum += deltas[i];
    }
    sum /= (nbIndexes - 1);

    msg.empty();
    msg.add(sum);
    udp.beginPacket(udpAddress, udpPort);
    msg.send(udp);
    udp.endPacket();
  }
  
  cnt++;

  delay(10);
}
