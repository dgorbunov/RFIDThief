#include <RFIDuino.h> //include the RFIDuino Library
#include <limits.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#define pulseWidth 1000     //uS
#define pulseInterval 10000 //uS
#define D0 9
#define D1 10
#define MAX_CARD_BITS 32

const int LOG_TO_EEPROM = 0;
const int ENABLE_BUZZER = 0;
unsigned char data[MAX_CARD_BITS];

//RFIDuino rfid(1.1);     //initialize an RFIDuino object for hardware version 1.1
RFIDuino rfid(1.2);   //initialize an RFIDuino object for hardware version 1.2

byte tagData[5]; //Holds the ID numbers from the tag  
unsigned long tagID; //the lower 4 byted of the tag ID

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");
  
  pinMode(D0, OUTPUT);    // sets the digital pin 0 as output
  pinMode(D1, OUTPUT);    // sets the digital pin 1 as output
  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
}

void loop() {   
  //scan for a tag - if a tag is sucesfully scanned, return a 'true' and proceed
  tagID = 0;
  if(rfid.scanForTag(tagData) == true) { 
    digitalWrite(rfid.led1, LOW);
  
    tagID = ((long)tagData[1] << 24) + ((long)tagData[2] << 16) + ((long)tagData[3] << 8) + tagData[4]; //generate a 4-byte number from the last 4 unique digits of th eID

    Serial.println(tagID); //print                                                                                                                                                                                                                                                                                                                                                                  

    //turn buzzer /led on quickly          
    digitalWrite(rfid.led2, HIGH);     //turn green LED on
   // rfid.successSound();
   // rfid.errorSound();
    delay(100);
    
    /*------------------ Sending Data --------------------------*/

    Serial.println("reset");
    for (int i = 0; i < MAX_CARD_BITS; i++) {
      data[i] = 0;
      // Serial.println(data[i]);
    }

    long y = tagID;
    Serial.println("hit 1");
    decToBinary(y);

    Serial.println("Test"); 
    
    int count = 0;
    Serial.println("hit 2");

    
    for(int i = sizeof(data); i >= 0; i-- ){
      if (data[i] == 0){
        // pull D0 down for pulse width
        digitalWrite(D0, LOW);
        delayMicroseconds(pulseWidth);
        digitalWrite(D0, HIGH);
        Serial.println("0");
        }

       else if (data[i] == 1){
        // pull D1 down for pulse width
        digitalWrite(D1, LOW);
        delayMicroseconds(pulseWidth);
        digitalWrite(D1, HIGH);
        Serial.println("1");
        }
        delayMicroseconds(pulseInterval);
      }
      
    digitalWrite(rfid.led2,LOW);      //turn the green LED off
    digitalWrite(rfid.led1, HIGH);
    
    delay(1000); //delay to prevent the tag from printing time-after time
  }

  else {
    digitalWrite(rfid.led1, HIGH);
  }
}
  

 
int decToBinary(long n) {
    // array to store binary number
    int binaryNum[32];
 
    // counter for binary array
    int i = 0;
    while (n > 0) {
      binaryNum[i] = n % 2; // storing remainder in binary array
      n = n / 2;
      i++;
    }
 
    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--){
        Serial.print(binaryNum[j]);
        data[j] = (char)binaryNum[j];
    }

    Serial.println(); 
}




 
 
  
  
