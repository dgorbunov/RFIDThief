#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"

FPS_GT511C3 fps(14, 12); // initializing fingerprint serial lines

/*If using another Arduino microcontroller, try commenting out line 60 and
uncommenting line 69 due to the limitations listed in the
library's note => https://www.arduino.cc/en/Reference/softwareSerial . Do
not forget to rewire the connection to the Arduino*/

void setup()
{
  Serial.begin(9600); //set up Arduino's hardware serial UART
  delay(100);
  fps.Open();         //send serial command to initialize fps
  fps.SetLED(true);   //turn on LED so fps can see fingerprint

  Enroll();          //begin enrolling fingerprint
}

void Enroll()
{
  // Enroll test

  // find open enroll id
  int enrollid = 0;
  bool usedid = true;
  while (usedid == true)
  {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid==true) enrollid++;
  }
  fps.EnrollStart(enrollid);

  // enroll
  Serial.print("Press finger to Enroll #");
  Serial.println(enrollid);
  while(fps.IsPressFinger() == false) delay(100);
  bool bret = fps.CaptureFinger(true);
  int iret = 0;
  if (bret != false)
  {
    Serial.println("Remove finger");
    fps.Enroll1(); 
    while(fps.IsPressFinger() == true) delay(100);
    Serial.println("Press same finger again");
    while(fps.IsPressFinger() == false) delay(100);
    bret = fps.CaptureFinger(true);
    if (bret != false)
    {
      Serial.println("Remove finger");
      fps.Enroll2();
      while(fps.IsPressFinger() == true) delay(100);
      Serial.println("Press same finger yet again");
      while(fps.IsPressFinger() == false) delay(100);
      bret = fps.CaptureFinger(true);
      if (bret != false)
      {
        Serial.println("Remove finger");
        iret = fps.Enroll3();
        if (iret == 0)
        {
          Serial.println("Enrolling Successful");
        }
        else
        {
          Serial.print("Enrolling Failed with error code:");
          Serial.println(iret);
        }
      }
      else Serial.println("Failed to capture third finger");
    }
    else Serial.println("Failed to capture second finger");
  }
  else Serial.println("Failed to capture first finger");
}


void loop()
{
  delay(100000);
}
