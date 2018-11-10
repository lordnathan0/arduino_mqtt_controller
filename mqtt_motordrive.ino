#include <localInternet.h>  // Loads WiFiSSID and WiFiPSK

#include <SparkFun_TB6612.h>


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

//////////////////////
// WiFi Definitions //
//////////////////////

const char MQTT[] = "192.168.0.200";

const char TOPIC_X[] = "controller/x";
const char TOPIC_Y[] = "controller/y";

//////////////////////
// Motor Definitions //
//////////////////////
#define AIN1 4
#define BIN1 7
#define AIN2 0
#define BIN2 8
#define PWMA 5
#define PWMB 6
#define STBY 13

const int offsetA = 1;
const int offsetB = 1;
const int MOTOR_MAX = 255;

Motor motorx = Motor(AIN1, AIN2, PWMA, offsetA, STBY);


/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the Thing
const int DIGITAL_PIN = 12; // Digital pin to be read

WiFiClient espClient;
PubSubClient client(espClient);

void setup() 
{
  initHardware();
  connectWiFi();
  setupMDNS();
  client.setServer(MQTT, 1883);
  client.setCallback(callback);
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 String inString = "";
 for (int i=0;i<length;i++) {
      inString += (char)payload[i];
 }
  Serial.print(inString);
  Serial.println();
  float cmd = inString.toFloat();
  if (cmd > 1)
  {
    cmd = 1;
  }
  else if (cmd < -1) 
  {
    cmd = -1;
  }

  if (strcmp(topic,TOPIC_X)){
    motorx.drive(cmd*MOTOR_MAX,10);
     Serial.print(cmd*MOTOR_MAX);
    Serial.println();
  }
}

void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("ESP8266 Client")) {
  Serial.println("connected");
  // ... and subscribe to topic
  client.subscribe(TOPIC_X);
  client.subscribe(TOPIC_Y);
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}

void connectWiFi()
{
  byte ledStatus = LOW;
  Serial.println();
  Serial.println("Connecting to: " + String(WiFiSSID));
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMDNS()
{
  // Call MDNS.begin(<domain>) to set up mDNS to point to
  // "<domain>.local"
  if (!MDNS.begin("thing")) 
  {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

}

void initHardware()
{
  Serial.begin(9600);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  // Don't need to set ANALOG_PIN as input, 
  // that's all it can be.
}
