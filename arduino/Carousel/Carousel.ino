#include <PubSubClient.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>

int pinDIR = 9;
int pinSTP = 8;
int pinSLP = 7;
int pinRST = 6;
int pinPSAVE = 5;
int pinZERO  = 2;

const int DELAY_PER_STEP = 15;
const int DELAY_PER_STEP_SLOW = 35;
const int STEPS_PER_SLOT = 20;
const int MAX_STEPS = 200;
const int SLOT_TIME = 1000;
const int DIR_LEFT = HIGH;
const int DIR_RIGHT = LOW;
const int ZERO_FIX = 1;
const int SLOTS = 10;
const int MIN_HALF_WIDH = 3;

boolean mqttConnected = false;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };

EthernetClient ethClient;
PubSubClient client(ethClient);

void messageReceived(const char* topic, byte* payload, unsigned int length)
{
  if (length != 2)
  {
    Serial.println("Invalid packet"); 
  }

  if (payload[0] == 120)
  {
    Serial.println("Set card position" + payload[1]); 
    setCard(payload[1]);
  }
  
  if (payload[0] == 125)
  {
    Serial.println("Calirating carousel...");
    findZero();
  }

  if (payload[0] == 130)
  {
    Serial.println("Turn off motor");
    digitalWrite(pinSLP, LOW);
  }

  if (payload[0] == 135)
  {
    Serial.println("Turn on motor");
    digitalWrite(pinSLP, HIGH);
  }
}

void setup()
{
  pinMode(pinDIR, OUTPUT);
  pinMode(pinSTP, OUTPUT);
  pinMode(pinSLP, OUTPUT);
  pinMode(pinRST, OUTPUT);

  pinMode(pinPSAVE, OUTPUT);

  pinMode(pinZERO, INPUT);

  // DVR8825 setup
  digitalWrite(pinSLP, HIGH);
  digitalWrite(pinRST, HIGH);
  digitalWrite(pinPSAVE,  HIGH);

  // direction setup
  digitalWrite(pinDIR, DIR_LEFT);

  // configure serial
  Serial.begin(9600);  
  Serial.println("Loading...");

  // configure ethernet
  if (Ethernet.begin(mac) == 0) {
  Serial.println("Failed to configure Ethernet using DHCP");
  // no point in carrying on, so do nothing forevermore:
  for(;;)
    ;
}
  Serial.println(Ethernet.localIP());

  // configure Mqtt
  Serial.println("Connecting...");
  client.setServer("10.239.71.133",1883);
  // mqttConnected = client.connect("carouselClient1");
  // if (mqttConnected)
  //{
  // Serial.println("Connected");
  //}
  //else
  //{
  //  Serial.println("Disconnected");
  //}
  
  client.setCallback(messageReceived);
  Serial.println("Initialized");
  findZero();
}

void powerSave(int mode)
{
  digitalWrite(pinPSAVE, mode > 0 ? HIGH : LOW);
}

void oneStep(int dir, int dps)
{
  digitalWrite(pinDIR, dir);
  digitalWrite(pinSTP, HIGH);
  delay(dps);
  digitalWrite(pinSTP, LOW);
  delay(dps);
}

void advance(int steps, int dir, int dps)
{
  for (int i=steps; i > 0; i--)
  {
    oneStep(dir, dps);
  }
}

void findZero()
{
  powerSave(0);
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_LEFT, DELAY_PER_STEP);
    int val = digitalRead(pinZERO);
    if (val == 1)
    {
      break;
    }
  }
  delay(SLOT_TIME);
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_RIGHT, DELAY_PER_STEP_SLOW);
    int val = digitalRead(pinZERO);
    if (val == 0)
    {
      break;
    }
  }
  delay(SLOT_TIME);
  int width = 0;
  for (int i=MAX_STEPS; i > 0; i--)
  {
    oneStep(DIR_LEFT, DELAY_PER_STEP_SLOW);
    width++;
    if (width > MIN_HALF_WIDH)
    {
      int val = digitalRead(pinZERO);
      if (val == 0)
      {
        break;
      }
    }
  }
  delay(SLOT_TIME);
  advance((width / 2) + ZERO_FIX, DIR_RIGHT, DELAY_PER_STEP_SLOW);
  delay(SLOT_TIME);

  powerSave(1);
}

void setCard(int slotNumber)
{
  int sN = slotNumber <= 0 ? 1 : (slotNumber > SLOTS ? SLOTS : slotNumber);
  powerSave(0);
  for (int i=sN; i > 0; i--)
  {
    advance(STEPS_PER_SLOT, DIR_LEFT, DELAY_PER_STEP);
  }
  powerSave(1);
}

void loop()
{
  //setCard(random(1, 10));
  // delay(15000);
  if (!client.connected()) {
    reconnect();
  }
client.loop();  
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ArduinoMegaCarousel-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      if (client.subscribe("/ifs/carousel", 1))
      {
        Serial.println("Subscribed");
      }
      else
      {
        Serial.println("Failed to subscribe");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

