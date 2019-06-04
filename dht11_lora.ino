// Include the SX1272 and SPI library: 
#include "SX1272.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

int e;
String st;

#define DHTPIN 5
#define DHTTYPE    DHT11   

char message1 [] = "Packet 1, wanting to see if received packet is the same as sent packet";
char message2 [] = "Packet 2, broadcast test";

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayms = 20000;

void setup()
{
  delay(5000);
  dht.begin();
  // Print a start message
  Serial.write("SX1272 module and Raspberry Pi: send packets with ACK and retries\n");
  
  // Power ON the module
  e = sx1272.ON();
  Serial.write("Setting power ON: state %d\n", e);
  
  // Set transmission mode
  //e = sx1272.setMode(4);
  //Serial.write("Setting Mode: state %d\n", e);

  e = sx1272.setSF(7);
  e = sx1272.setBW(BW_125);
  e = sx1272.setCR(CR_5);
  e = sx1272.setPreambleLength(8);
  
  // Set header
  e = sx1272.setHeaderON();
  Serial.write("Setting Header ON: state %d\n", e);
  
  // Select frequency channel
  e = sx1272.setChannel(CH_12_900);
  Serial.write("Setting Channel: state %d\n", e);
  
  // Set CRC
  e = sx1272.setCRC_OFF();
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
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      Serial.print(event.temperature);
      Serial.println(F("°C"));
    }
    st = "{\"temp\": ";
    st += String(event.temperature);
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      Serial.print(event.relative_humidity);
      Serial.println(F("%"));
    }


    st += ", \"hum\": ";
    st += String(event.relative_humidity);
    st += "}";
    char buf[st.length()+1];

    st.toCharArray(buf, st.length()+1);
    
    Serial.print(buf);
    
    sx1272.sendPacketMAXTimeout(0, buf, sizeof(buf));
    Serial.write("\nSending...\n");
    delay(delayms);
}
