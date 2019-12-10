
// Include the SX1272 and SPI library: 
#include "SX1272.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

int e;
String st;

#define DHTPIN 5
#define DHTTYPE    DHT11   
#define NODEID 1

//set ThingSpeak API key here
#define APIKEY "APIKEYHERE"

DHT_Unified dht(DHTPIN, DHTTYPE);

//ThingSpeak maximum rate: 1 message/15 seconds
uint32_t delayms = 15000;

//LoRa maximum packet size: 255 bytes
char packet[250];

void setup()
{
  delay(5000);
  dht.begin();
  
  // Power ON the module
  e = sx1272.ON();
  Serial.write("Setting power ON: state %d\n", e);
  
  // Set transmission mode
  //e = sx1272.setMode(4);
  //Serial.write("Setting Mode: state %d\n", e);

  pinMode(LED_BUILTIN, OUTPUT);

  //Manually setting LoRa mode
  //Spreading Factor = 10
  e = sx1272.setSF(10);
  //Bandwidth = 125 Khz
  e = sx1272.setBW(BW_125);
  //Coding Rate = 4/5
  e = sx1272.setCR(CR_5);
  //Preamble Length = 8 (Default)
  e = sx1272.setPreambleLength(8);
  
  // Set header
  e = sx1272.setHeaderON();
  Serial.write("Setting Header ON: state %d\n", e);
  
  // Select frequency channel (CH_12_900 = 915MHz)
  e = sx1272.setChannel(CH_12_900);
  Serial.write("Setting Channel: state %d\n", e);
  
  // Set CRC
  e = sx1272.setCRC_ON();
  Serial.write("Setting CRC ON: state %d\n", e);
  
  // Select output power (Max, High or Low)
  e = sx1272.setPower('x');
  Serial.write("Setting Power: state %d\n", e);
  
  // Set the node address
  e = sx1272.setNodeAddress(1);
  Serial.write("Setting Node address: state %d\n", e);
  
  // Print a success message
  Serial.write("SX1272 successfully configured\n\n");
  
  delay(1000);
}

void loop(void)
{
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    
    //Start building message - Message template: 'NODEID;APIKEY;temp_value;hum_value'
    st = String(NODEID) + ";" + APIKEY;
    
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
      st += ";" + String(-1);
    }
    else {
      
      st += ";" + String(event.temperature);
      Serial.print(F("Temperature: "));
      Serial.print(event.temperature);
      Serial.println(F("°C"));
      }
      
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    st += ";" + String(-1);
    
    }
    else {
      Serial.print(F("Humidity: "));
      Serial.print(event.relative_humidity);
      Serial.println(F("%"));
    st += ";" + String(event.relative_humidity);
    
    }

    char buf[st.length()+1];

    st.toCharArray(buf, st.length()+1);
    
    Serial.print(buf);
    
    //Send LoRa packet
    sx1272.sendPacketMAXTimeout(0, buf, sizeof(buf));
    Serial.write("\nSending...\n");
  
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);  

    //Downlink for a few seconds
    e = sx1272.receivePacketTimeout(3000);

    //No recipient verification, blink if _any_ message received - TODO: directed message? (is it possible?)
    if( e == 0 )
    {
      for(unsigned int i = 0; i < sx1272.packet_received.length; i++)
      {
          packet[i]=(char)sx1272.packet_received.data[i];
      }
      Serial.println(packet);

      digitalWrite(LED_BUILTIN, HIGH); 
      delay(100); 
      digitalWrite(LED_BUILTIN, LOW); 
      delay(100);     
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW); 
    }


    delay(delayms);
}
