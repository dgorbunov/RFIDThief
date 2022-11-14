#include "config.h"

#define PIN_RED D0
#define PIN_GREEN D1
#define PIN_BLUE D2
#define PIN_BUZZER D7
#define PIN_DATA0_IN D5
#define PIN_DATA1_IN D6

#define MAX_CARD_BITS 32
#define pulseWidth 1000     // uS
#define pulseInterval 10000 // uS

const byte tagSize = MAX_CARD_BITS;
char tagData[MAX_CARD_BITS];
unsigned int tagInt = 0;
unsigned int tagIndex = 0;
bool receivingData = false;
bool secureStatus = false;
bool secureLock = false;
unsigned long secureStartTime = 0;
bool fingerScanned = false;
bool fingerLock = false;
bool fingerStartTime = false;
int fingerID = 0;

const int VALID_ID = 8139252; // Good ID

AdafruitIO_Feed *secure = io.feed("rfidthief.securemode");
AdafruitIO_Feed *fingerprintID = io.feed("rfidthief.fingerprintid");

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

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_DATA0_IN, INPUT);
  pinMode(PIN_DATA1_IN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  ledWrite(255, 0, 0);
  tone(PIN_BUZZER, 1000, 200);
  
  attachInterrupt(digitalPinToInterrupt(PIN_DATA0_IN), writeToBuffer0, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_DATA1_IN), writeToBuffer1, FALLING);

  Serial.print("Connecting to Adafruit IO");
  io.connect();

  fingerprintID->onMessage(processFingerprint);
  secure->onMessage(secureMode);

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

  secure->get();
}

void loop() {
  io.run();
  
  if (receivingData && tagIndex == 0) {
    receivingData = false;
    printTag();

    if (secureStatus && tagInt == VALID_ID) {
      ledWrite(0,0,255);
      
      if (fingerScanned == true && fingerID == 0) {
        fingerScanned = false;
        allowAccess();
      } else {
        denyAccess();
      }
    }
    else if (!secureStatus && tagInt == VALID_ID) {
      allowAccess();
    } else {
      denyAccess();
    }
  }
}

void allowAccess() {
  ledWrite(0, 255, 0);
  tone(PIN_BUZZER, 1000, 100);
  delay(100);
  tone(PIN_BUZZER, 1500, 100);
  delay(100);
  tone(PIN_BUZZER, 2000, 200);

  delay(1200);
  ledWrite(255, 0, 0);
}

void denyAccess() {
  ledWrite(0, 0, 0);
  tone(PIN_BUZZER, 1000, 100);
  delay(100);
  
  ledWrite(255, 0, 0);
  tone(PIN_BUZZER, 750, 100);
  delay(100);
  
  ledWrite(0, 0, 0);
  tone(PIN_BUZZER, 500, 200);
  delay(200);
  
  ledWrite(255, 0, 0);

  delay(1000);
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

void processFingerprint(AdafruitIO_Data *data) {
  fingerScanned = true;
  Serial.println("Fingerprint Detected!");
  if (data->toString() == "unknown") fingerID = -1;
  else fingerID = data->toInt();
  Serial.print("Finger ID: ");
  Serial.println(fingerID);
}

void secureMode(AdafruitIO_Data *data) {
  secureStatus = (data->toPinLevel() == HIGH) ? true : false;
  if (secureStatus) {
    Serial.println("Secure Mode Enabled!");
    ledWrite(0,0,255);
  } else {
    Serial.println("Secure Mode Disabled!");
    ledWrite(255,0,0);
  }
}

void ledWrite(byte r, byte g, byte b) {
  analogWrite(PIN_RED, (1023/255) * r);
  analogWrite(PIN_GREEN, (1023/255) * g);
  analogWrite(PIN_BLUE, (1023/255) * b);
}
