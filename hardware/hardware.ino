//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <rom/rtc.h> 
#include <math.h>  // https://www.tutorialspoint.com/c_standard_library/math_h.htm 
#include <ctype.h>

// ADD YOUR IMPORTS HERE



#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

#include <DHT.h>
#include <FastLED.h>


 

// DEFINE VARIABLES
#define ARDUINOJSON_USE_DOUBLE      1 

#define DHTTYPE DHT22
#define DHTPIN 32

#define NUM_LEDS 7
#define DATA_PIN 26

CRGB ledArr[NUM_LEDS];

// DEFINE THE CONTROL PINS FOR THE DHT22 




// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620157584";                    // Add your ID number here
static const char* subtopic[]    = {"620157584_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "broker.emqx.io";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
const char* ssid       = "MonaConnect";     // Add your Wi-Fi ssid
const char* password   = ""; // Add your Wi-Fi password 




// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);
 

/* Declare your functions below */ 
double convert_Celsius_to_fahrenheit(double c);
double convert_fahrenheit_to_Celsius(double f);
double calcHeatIndex(double Temp, double Humid);


/* Init class Instances for the DHT22 etcc */
 DHT dht(DHTPIN, DHTTYPE);
  

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables 


void setup() {
  Serial.begin(115200);  // INIT SERIAL  

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(ledArr, NUM_LEDS);
  // INITIALIZE ALL SENSORS AND DEVICES
  dht.begin();
  
  /* Add all other necessary sensor Initializations and Configurations here */


  initialize();     // INIT WIFI, MQTT & NTP 
  // vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS INT THIS LAB, THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
 }
  


void loop() {
    // put your main code here, to run repeatedly:       
    vTaskDelay(1000 / portTICK_PERIOD_MS);    
}




  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {
        // Add code here to check if a button(S) is pressed
        // then execute appropriate function if a button is pressed  

        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
           
    for( ;; ) {
          // #######################################################
          // ## This function must PUBLISH to topic every second. ##
          // #######################################################
   
          // 1. Read Humidity and save in variable below
          double h = dht.readHumidity();
           
          // 2. Read temperature as Celsius   and save in variable below
          double t = dht.readTemperature();    
 

          if(isNumber(t)){

          JsonDocument doc; // Create JSon object
          char message[1100]  = {0};

          // Add key:value pairs to JSon object
          doc["id"] = "620157584";
          doc["timestamp"]  = getTimeStamp();
          doc["humidity"]  = h;
          doc["temperature"]  = t;
          doc["heat index"]  = convert_fahrenheit_to_Celsius(calcHeatIndex(convert_Celsius_to_fahrenheit(t),h));

          serializeJson(doc, message);  // Seralize / Covert JSon object to JSon string and store in char* array

          if(mqtt.connected() ){
            publish(pubtopic, message);
          }
            
              // ##Publish update according to ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "heatindex": 30}’

              // 1. Create JSon object
              
              // 2. Create message buffer/array to store serialized JSON object
              
              // 3. Add key:value pairs to JSon object based on above schema

              // 4. Seralize / Covert JSon object to JSon string and store in message array
               
              // 5. Publish message to a topic sobscribed to by both backend and frontend                

          }

          
            
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

 

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}


void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


  // PROCESS MESSAGE
  const char* type = doc["type"]; 

  if (strcmp(type, "controls") == 0){

    const int leds = doc["leds"];
    const int red = doc["color"]["r"];
    const int green = doc["color"]["g"];
    const int blue = doc["color"]["b"];
    const int alpha = doc["color"]["a"];
    const int brightness = doc["brightness"];
    // 1. EXTRACT ALL PARAMETERS: NODES, RED,GREEN, BLUE, AND BRIGHTNESS FROM JSON OBJECT

    // 2. ITERATIVELY, TURN ON LED(s) BASED ON THE VALUE OF NODES. Ex IF NODES = 2, TURN ON 2 LED(s)

    for(int x=0; x<leds; x++){
      ledArr[x] = CRGB( red, green, blue); // R, G, B range for each value is 0 to 255
      FastLED.setBrightness( brightness ); // Ranges from 0 to 255
      FastLED.show(); // Send changes to LED array
      delay(50);}

    // 3. ITERATIVELY, TURN OFF ALL REMAINING LED(s).
    for(int x=leds; x<NUM_LEDS; x++){
      ledArr[x] = CRGB::Black;
      FastLED.setBrightness( brightness );
      FastLED.show();
      delay(50); }
   
  }
}


bool publish(const char *topic, const char *payload){   
     bool res = false;
     try{
        res = mqtt.publish(topic,payload);
        // Serial.printf("\nres : %d\n",res);
        if(!res){
          res = false;
          throw false;
        }
     }
     catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
     }
  return res;
}



//***** Complete the util functions below ******

double convert_Celsius_to_fahrenheit(double c){    
  return (c*9/5)+32;
    // CONVERTS INPUT FROM °C TO °F. RETURN RESULTS     
}

double convert_fahrenheit_to_Celsius(double f){    
  return (f-32)*5/9;
    // CONVERTS INPUT FROM °F TO °C. RETURN RESULT    
}

double calcHeatIndex(double Temp, double Humid){;
  return -42.379 + 
               (-2.04901523 * Temp) + (-10.14333127 * Humid) + 
               (-0.22475541 * Temp * Humid) + 
               (-0.00683783 * Temp * Temp) + (-0.05481717 * Humid * Humid) + 
               (-0.00122874 * Temp * Temp * Humid) + (0.00085282 * Temp * Humid * Humid) + 
               (-0.00000199 * Temp * Temp * Humid * Humid);
    // CALCULATE AND RETURN HEAT INDEX USING EQUATION FOUND AT https://byjus.com/heat-index-formula/#:~:text=The%20heat%20index%20formula%20is,an%20implied%20humidity%20of%2020%25
  
}
 
bool isNumber(double number){       
        char item[20];
        snprintf(item, sizeof(item), "%f\n", number);
        if( isdigit(item[0]) )
          return true;
        return false; 
} 
