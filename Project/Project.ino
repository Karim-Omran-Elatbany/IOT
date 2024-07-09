#include <ESP8266WiFi.h>           // Include the WiFi library
#include <PubSubClient.h>                             // Include the MQTT library
#include "DHT.h" 
#define FAN_PIN D3
#define GAS_V_PIN A0 
#define BUZZ_PIN D8 
#define LAMB_PIN D5 // Import DHT library   
#define DHT_PIN D7 
#define SOUND_PIN D0
unsigned long lastEvent = 0;
bool lamb_state=false;// Digital pin connected to the DHT sensor
int Gas_Value;
bool state=false;
DHT dht(DHT_PIN, DHT11);                                // Initialize DHT sensor
const char* ssid = "ghhg";                          // WiFi SSID
const char* password = "1236547891";      //wifi password
const char* broker = "192.168.137.1";  // broker ip
const char* topic = "temp_sens";
const char* fantopic="fan_state";
const char* gas_topic="gas";
const char* buzz_topic="buzz";
unsigned long lastTempUpdate = 0;
const int port = 1883;    
WiFiClient espClient;                                 // Create an object of the WiFiClient class
PubSubClient client(espClient);  
void setup() {
  Serial.begin(115200);                               // Initialize serial communication at baudrate of 115200
  // wifi begin
  WiFi.begin(ssid, password);                         // Attempt to connect to the Wi-Fi network
  while (WiFi.status() != WL_CONNECTED) {             // Wait until the NodeMCU is successfully connected
    delay(1000);                                      // Wait 1 second before rechecking Wi-Fi connection statuss
    Serial.println("Connecting to WiFi...");          // A message indicating an attempt to connect to Wi-Fi
  }
  Serial.println("Connected to WiFi.");               // A message indicating a successful connection
  // mqtt 
  client.setServer(broker, port);                     // Connect to the MQTT broker
  client.connect("NodeMCU_Publisher");                // Connect to MQTT broker with the name "NodeMCU_Publisher"
  Serial.println("Connected to MQTT broker.");

  client.setCallback(on_message);   
  client.subscribe(fantopic);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(LAMB_PIN, OUTPUT);
  pinMode(GAS_V_PIN,INPUT);
 
  pinMode(SOUND_PIN, INPUT);
                               
  dht.begin();                                          // Start DHT sensor
}

const int MAX_MESSAGE_LENGTH = 128; 
char fanstate[MAX_MESSAGE_LENGTH]; 

void on_message(char* topic, byte* message, unsigned int length) {
  Serial.print("Message received: "); 
  if (length >= MAX_MESSAGE_LENGTH) {
    Serial.println("Message too long, ignoring...");
    return; // Exit the function early
  }

  // Copy each byte from the message into the fanstate array
  for (int i = 0; i < length; i++) {
    fanstate[i] = (char)message[i]; // Store the byte as a character in fanstate
  }

  // Null-terminate the fanstate string to ensure it's properly terminated
  fanstate[length] = '\0';

  // Print the fanstate to the Serial Monitor
  Serial.println(fanstate);
}

void loop() {
temp_on_fan(); 
sound_sensor();// Wait a few seconds between measurements
gas_sensor();
fan_on();
  
}
void temp_on_fan(){
  
  float h = dht.readHumidity();                         // Read humidity
  float temp = dht.readTemperature();  

   // Check if any reads failed (to try later)
  if (isnan(h) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor.");
    return;
  }
  else{
 // Read temperature as Celsius
   char temperature[8]; // Buffer big enough for 7-character float
  dtostrf(temp, 6, 2, temperature);
  
//  const char* message = "Turn On";                    // The message to be published
  client.publish(topic, temperature);                     // Publish the message to the specified topic
  Serial.print("Published Temperature: ");   
    delay(1000);  // A message prefix
  Serial.println(temperature);
 
//   Print temperature
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C ");

  // Print humidity
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
     }
    
    } 
  
 void gas_sensor(){
  delay(2000);
  Gas_Value = analogRead(GAS_V_PIN);
 

  char Gas_V[8]; // Buffer big enough for 7-character float
  dtostrf(Gas_Value, 6, 2, Gas_V);
  if(Gas_Value!=0){
  client.publish(gas_topic,Gas_V); 
  Serial.print("Sensor Value: ");
  Serial.println(Gas_Value);
  }
    delay(2000);
  if(Gas_Value >500){
    Serial.println("gas!");
    digitalWrite(BUZZ_PIN,HIGH);
    delay(2500);
    client.publish(buzz_topic,"ON, There is gas Leaking");}
    else{
      digitalWrite(BUZZ_PIN,LOW);}
   delay(1000);
 }
void sound_sensor(){
  int sensor_value = digitalRead(SOUND_PIN); // Read the value from the sound sensor

  if (sensor_value == HIGH) { // If sound is detected
    Serial.println("Clap detected!");
    state = !state; // Toggle the state
    Serial.println(state);
  }

  // Update the LED based on the state
  digitalWrite(LAMB_PIN, state ? HIGH : LOW);
  }
void fan_on() {
    // Check the fanstate variable
        client.loop();
    if (strcmp(fanstate, "on") == 0) {
        // If fanstate is "on", turn on the fan
        digitalWrite(FAN_PIN, HIGH);
        Serial.println("The fan is on");
    } else {
        // If fanstate is not "on", turn off the fan
        digitalWrite(FAN_PIN, LOW);
    }
}
