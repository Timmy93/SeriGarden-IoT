#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define A0 17
#define TX 1
#define RX 3

#define INNAFFIA 1000
// Minuti di attesa tra un'analisi e l'altra
#define LOOP_SLEEP 1
//Loop inattivi
#define NULL_LOOPS 60
#define WAIT 1000
#define MIN_HUM 550
#define MAX_HUM 190
#define MSG_BUFFER_SIZE	128

// Define the plants that are analysed
const int analyzed_plants[][2] = {
  // Non usare: D3, D4
  // OK: D0, D1, D2, D5, D6, D7, D8
  {D5, 1},
  {D6, 2},
  {D7, 3},
  {D8, 4}
};

//Define the plants that are 
const int watered_plants[][2] = {
  // Non usare: D4, D8, D3 (problemi avvio)
  // OK: D0, D1, D2, D5, D6, D7
   {D0, 1},
   {D1, 2},
   {D2, 3}
//   {D3, 4}
};

// Initialize clients
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int iteration = 0;
int ID = -1;
int first_run = 1;

void setup() {
  //Initial setup
  Serial.begin(9600);
  Serial.println("Inizializzazione... ");
  setupPin(true);
  //Get ID
  ID = ESP.getChipId();
  Serial.print("NodeMCU ID: ");
  Serial.println(ID);
  delay(3000);
  //setupPin(true);
  //Connect to WiFi, MQTT server and subscribe to MQTT topic
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  Serial.println("Connettività configurata");
  
}

void setupPin(bool bootTime) {
  //Setup all plant to anlyse
  int len = sizeof(analyzed_plants) / sizeof(analyzed_plants[0]);
  int pin = -1;
  for (int i = 0; i < len; i++) {
    pin = analyzed_plants[i][0];
    if (bootTime) {
      pinMode(pin, OUTPUT);
    }
    initializeThisPin(pin, bootTime, LOW);
  }
  //Setup all plants to water
  len = sizeof(watered_plants) / sizeof(watered_plants[0]);
  for (int i = 0; i < len; i++) {
    pin = watered_plants[i][0];
    if (bootTime) {
      pinMode(pin, OUTPUT);
    }
    initializeThisPin(pin, bootTime, HIGH);
  }
}

void initializeThisPin(int pin, bool bootTime, int status){
  if (bootTime && (pin == D3 || pin == D4)) {
    //During boot this PIN must be HIGH
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, status);
  }
}

// the loop function runs over and over again forever
void loop() {
  setupPin(false);
  int len = sizeof(analyzed_plants) / sizeof(analyzed_plants[0]);
  // Connect to WiFi and MQTT server
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (first_run == 1) {
    first_run = 0;
    // In fase di debug, fai un ciclo di irrigazione con tutte le pompe
    debug_pumps();
    greeting();
    Serial.println("Checking in...");
    delay(5000);
  }
  if (iteration % NULL_LOOPS == 0) {
    //Measure humidity for all the connected plants
    Serial.println("Start plant analysis... ");
    for (int i = 0; i < len; i++) {
      measure_hum(analyzed_plants[i]);
    }
    Serial.println("Plant analysis completed");
    iteration = 0;
  } else {
    Serial.print(".");
  }
  iteration += 1;
  delay(LOOP_SLEEP*1000);
}

void greeting() {
  String topic = "greeting";
  String payload = "s_";
  //TODO
  char topic_c[MSG_BUFFER_SIZE];
  char payload_c[MSG_BUFFER_SIZE];
  //Create output
  topic.toCharArray(topic_c, MSG_BUFFER_SIZE);
  payload = payload + ID;
  payload.toCharArray(payload_c, MSG_BUFFER_SIZE);
  //Publish message
  client.publish(topic_c, payload_c);
}

void measure_hum (const int plant_details[]) {
  //Define variables
  int pin = plant_details[0];
  int plant_id = plant_details[1];
  String topic = "sensor/";
  String payload = "d2_";
  char topic_c[MSG_BUFFER_SIZE];
  char payload_c[MSG_BUFFER_SIZE];
  //Read value
  int hum_v = read_pin(pin);
  //Create output
  topic = topic + ID;
  topic.toCharArray(topic_c, MSG_BUFFER_SIZE);
  payload = payload + hum_v + "_" + plant_id;
  payload.toCharArray(payload_c, MSG_BUFFER_SIZE);
  Serial.println(topic);
  Serial.println(payload_c);
  //Publish message
  client.publish(topic_c, payload_c);
}

int read_pin(int pin_to_read) {
  int value;
  float perc;
  String message = "";
  digitalWrite(pin_to_read, HIGH);
  delay(WAIT);
  value = analogRead(A0);
  delay(WAIT);
  digitalWrite(pin_to_read, LOW);
  perc = 100-((value-MAX_HUM)*100/(MIN_HUM-MAX_HUM));
  message = message + "Sensor: " + pin_to_read + " - Value: " + value + " - %: " + perc;
  Serial.println(message);
  return (int)perc;
}

void innaffiamo (int ilpin, int tempo){
//  int on = LOW;
//  int off = HIGH;
//  if (ilpin == D3) {
//    Serial.println("Special watering!");
//    on = HIGH;
//    off = LOW;
//  }
  int propagazione_acqua = 5000;
  if (DEBUG == 1) {
    propagazione_acqua = 500;
  }
  printf("Inizio ad innaffiare [pin: %d]\n", ilpin);
  digitalWrite(ilpin, LOW);   
  delay(tempo);              
  printf("Finisco di innaffiare, attendo la propagazione dell'acqua\n");
  digitalWrite(ilpin, HIGH);
  delay(propagazione_acqua);
}

// Connect to WiFi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SECRET_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      subscribe_topic();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void subscribe_topic() {
  Serial.println("Subscribing to topic");
  String topic = "";
  char topic_c[MSG_BUFFER_SIZE];
  //Create output
  topic = "water2/";
  topic = topic + ID;
  topic.toCharArray(topic_c, MSG_BUFFER_SIZE);
  Serial.print(topic);
  Serial.println(" - Sottoscritto!");
  client.subscribe(topic_c);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("MQTT message received");
  char* token_topic;
  char* token_payload;
  int water_id = -1;
  int ms = -1;
  int plant_id = -1;
  int required_sensor = -1;
  //Parse received payload
  char p_payload[length+1] = "";
  parsePayload(p_payload, payload, length);
  String message = "";
  message = message + "Message arrived [" + topic + "]["+p_payload+"]";
  Serial.println(message);
  
  //Extract info: 
  //  Parsing topic
  token_topic = strtok(topic, "water2/");
  required_sensor = atoi(token_topic);

  //  Parsing payload
  if (required_sensor == ID || 1) {
    // water2/12041068 - w_26_5439_2
    token_payload = strtok(p_payload, "_");
    token_payload = strtok(NULL, "_");
    water_id = atoi(token_payload);
    token_payload = strtok(NULL, "_");
    ms = atoi(token_payload);
    token_payload = strtok(NULL, "_");
    plant_id = atoi(token_payload);
    Serial.print("plant_num: ");
    Serial.println(plant_id);
    
    //Water the plant
    waterThisPlant(plant_id, ms, water_id);
  } else {
    Serial.print("Unexpected topic or sensor: ");
    Serial.println(required_sensor);
  }
  
}

// Convert the received message in a string
void parsePayload(char* parsed_s, byte* payload, unsigned int length) {
  for (int i = 0; i < length; i++) {
    parsed_s[i] = (char)payload[i];
  }
  parsed_s[length] = '\0';
}

void debug_pumps() {
  int pin;
  if (DEBUG == 1) {
  int len = sizeof(watered_plants) / sizeof(watered_plants[0]);
    for (int i = 0; i < len; i++) {
      pin = watered_plants[i][0];
      innaffiamo(pin, 3000);
    }
  }
}

/**
Executes the watering of this plant
*/
void waterThisPlant(int plant_id, int duration, int watering_id) {
  String message = "";
  message = message + "Plant ID: [" + plant_id + "] - Water ID: [" + watering_id + "] - Milliseconds [" + duration + "]";
  Serial.println(message);
  int pin = getPlantToWater(plant_id);
  if (pin > -1) {
    innaffiamo(pin, duration);
    sendWateringAck(plant_id, watering_id);
  } else {
    Serial.println("Ignoring request - Not handling this plant");
  }
}

int getPlantToWater(int plant_id) {
  int len = sizeof(watered_plants) / sizeof(watered_plants[0]);
  for (int i = 0; i < len; i++) {
    if (watered_plants[i][1] == plant_id) {
      return watered_plants[i][0];
    }
  }
  return -1;
}

// Acknoledge backend that watering was successfull
void sendWateringAck(int plant_id, int watering_id) {
  String topic = "sensor/";
  String payload = "w_";
  char topic_c[MSG_BUFFER_SIZE];
  char payload_c[MSG_BUFFER_SIZE];
  //Create output
  topic = topic + ID;
  topic.toCharArray(topic_c, MSG_BUFFER_SIZE);
  payload = payload + watering_id;
  payload.toCharArray(payload_c, MSG_BUFFER_SIZE);
  //Publish message
  client.publish(topic_c, payload_c);
}