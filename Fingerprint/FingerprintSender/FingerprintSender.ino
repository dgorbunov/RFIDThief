#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"
#include "config.h"

// set up software serial pins for Arduino's w/ Atmega328P's
// FPS (TX) is connected to pin 4 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 5 (Arduino's Software TX)
FPS_GT511C3 fps(14, 12); // (Arduino SS_RX = pin 4, Arduino SS_TX = pin 5)

/*If using another Arduino microcontroller, try commenting out line 53 and
uncommenting line 62 due to the limitations listed in the
library's note => https://www.arduino.cc/en/Reference/softwareSerial . Do
not forget to rewire the connection to the Arduino*/

// FPS (TX) is connected to pin 10 (Arduino's Software RX)
// FPS (RX) is connected through a converter to pin 11 (Arduino's Software TX)
//FPS_GT511C3 fps(10, 11); // (Arduino SS_RX = pin 10, Arduino SS_TX = pin 11)

AdafruitIO_Feed *fingerprintID = io.feed("rfidthief.fingerprintid");

void setup() {
  Serial.begin(115200); //set up Arduino's hardware serial UART
  pinMode(5, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(100);
  fps.Open();         //send serial command to initialize fps
  fps.SetLED(true);   //turn on LED so fps can see fingerprint

  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  digitalWrite(LED_BUILTIN, HIGH);

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
}

void loop() {
  io.run();
  
  // Identify fingerprint test
  if (fps.IsPressFinger()) {
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();
    
       /*Note:  GT-521F52 can hold 3000 fingerprint templates
                GT-521F32 can hold 200 fingerprint templates
                 GT-511C3 can hold 200 fingerprint templates. 
                GT-511C1R can hold 20 fingerprint templates.
       Make sure to change the id depending on what
       model you are using */
    if (id < 200){ //if the fingerprint matches, provide the matching template ID
      Serial.print("Verified ID:");
      Serial.println(id);

      fingerprintID->save(id);
      
      digitalWrite(5, HIGH);
      delay(100);
      
    } else { //if unable to recognize
      fingerprintID->save("unknown");
      Serial.println("Finger not found");
 
    }
  } else {
//    Serial.println("Please press finger");
    
    }
    
    delay(100);
    digitalWrite(5, LOW);
}
