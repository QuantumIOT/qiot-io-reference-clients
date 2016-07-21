/*
 Exercise things
 BWC 2016


*/

#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>

// Update these with values suitable for your network.

const char* ssid = "Starfleet Communication";
const char* password = "Network23";
const char* mqtt_server = "qiot.io";

#define AIO_USERNAME    "bill"
#define AIO_KEY         "mango-jam"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

IPAddress timeServerIP; // time.nist.gov NTP server address
//const char* ntpServerName = "time.nist.gov";
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = 1;     // Central European Time

void sendNTPpacket(IPAddress &address);

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
char timestamp[64];
float offset;
int gottime;

unsigned int localPort = 8888;      // local port to listen for UDP packets


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      //gottime = 1
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
//  2011-12-19T15:28:46.493Z

  sprintf(timestamp, "%d-%02d-%2dT%02d:%02d:%02d.000Z", year(), month(), day(), hour(), minute(), second());
  Serial.println(timestamp);  
}



void setup() {
//  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
//  client.setCallback(callback);

  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
//  setSyncProvider(getNtpTime);
//  setSyncInterval(300);
  
  delay(1000);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
//  delay(5000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
char buffy[256];

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("13ddhwowgwdg5bb", AIO_USERNAME, AIO_KEY)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
float flat = 33.028276;
float flon = -96.653479;
time_t prevDisplay = 0; // when the digital clock was displayed

struct Drone {
  float lat;
  float lon;
};

float counter = 0.0;

int dronecount = 0;
void loop() {
  char slat[16];
  char slon[16];
  float lat;
  float lon;
  gottime = 0;
  if (!client.connected()) {
    reconnect();
  }
  prevDisplay = getNtpTime();
  if(prevDisplay > 0)
  {
    setTime(prevDisplay);
    digitalClockDisplay();
    // Now we can publish stuff!
    offset = (float) random(0,10);
    offset = offset - 5;
    offset = offset / 100000;
    lat = flat + offset;
    lon = flon + offset;
    counter = counter + 1.0;
    lat = counter;
    sprintf(buffy, "%s%s%s%s%s%s%s", "{\"messages\":[{\"latitude\":\"", dtostrf(lat,9,6,slat), "\",\"longitude\":\"", dtostrf(lon,10,6,slon), "\",\"timestamp\":\"", timestamp, "\"}]} ");
    Serial.print("Publish message: ");
    Serial.println(buffy);
//    client.publish("1/l/Yrm7DySa4bwJ2q9S", buffy);
    Serial.println("1/l/ajnppojcx3wpsqs"); // ;1/l/ajnppojcx3wpsqs
    client.publish("1/l/ajnppojcx3wpsqs", buffy); // ;1/l/ajnppojcx3wpsqs
    delay(1000);

    
  }
  else
    Serial.println("no time");
  delay(15000);
  if(prevDisplay > 0)
  {
    setTime(prevDisplay);
    digitalClockDisplay();
    // Now we can publish stuff!
    offset = (float) random(0,10);
    offset = offset - 5;
    offset = offset / 100000;
    lat = flat + offset;
    lon = flon + offset;
    lat = counter;
    sprintf(buffy, "%s%s%s%s%s%s%s", "{\"messages\":[{\"latitude\":\"", dtostrf(lat,9,6,slat), "\",\"longitude\":\"", dtostrf(lon,10,6,slon), "\",\"timestamp\":\"", timestamp, "\"}]} ");
    Serial.print("Publish message: ");
    Serial.println(buffy);
    Serial.println("1/l/Yrm7DySa4bwJ2q9S");
    client.publish("1/l/Yrm7DySa4bwJ2q9S", buffy);
//    client.publish("1/l/ajnppojcx3wpsqs", buffy) // ;1/l/ajnppojcx3wpsqs
    delay(1000);   
  }
  else
    Serial.println("no time");
  delay(15000);
}
