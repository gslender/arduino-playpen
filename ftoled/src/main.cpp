#include "Arduino.h"

#define redPin   A0
#define greenPin A1

void setup()                      // run once, when the sketch starts
{
  pinMode(redPin, OUTPUT);        // sets the digital pin as output
  pinMode(greenPin, OUTPUT);      // sets the digital pin as output
  Serial.begin(115200);

}

void loop() {


	  digitalWrite(redPin, HIGH);
	  digitalWrite(greenPin, LOW);
	  delay(500);
	  digitalWrite(redPin, LOW);
	  digitalWrite(greenPin, HIGH);
	  delay(500);


}
