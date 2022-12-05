#include <Wire.h>

/*----------------------------------Setup and assign value to variable----------------------------------*/

// Variables related to water pump control
int relay_2 = 11; // Water pump relay
bool waterOn = false; // Flag for water pump


// Variables related to UV light control
int relay_1 = 10; // UV light relay
char photo_1 = A0; // Photoresistors
char photo_2 = A1;
char photo_3 = A2;
char photo_4 = A3;
bool lightOn = false; // Flag for UV light

bool manualOn = false; // Flag to activate manual mode


void setup() {
  Serial.begin(9600); // Set baud rate at 9600

  Wire.begin(4); // Establish communication to primary Arduino
  Wire.onReceive(recieveEvent);

  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);

}

void loop() {
  if (!manualOn) {
    lightDetection(); // Activate light control function
  }
}

/*----------------------------------------------------------------------------------------------------*/

void lightDetection() {
  int photoResistor_1 = analogRead(photo_1);
  int photoResistor_2 = analogRead(photo_2);
  int photoResistor_3 = analogRead(photo_3);
  int photoResistor_4 = analogRead(photo_4);

  int avgValue = (photoResistor_1 + photoResistor_2 + photoResistor_3 + photoResistor_4) / 4; // Take average data

  // Run value into probabilistic model
  float probNotTurningOn = sigmoidFunc(avgValue); 
  float prob = ((double) rand() / (RAND_MAX));

  if (prob < probNotTurningOn) {
    digitalWrite(relay_1, LOW); // UV light strip off
  }

  else {
    digitalWrite(relay_1, HIGH); // UV light strip on
  }
}


float sigmoidFunc(int mean) { // Probabistic model of light intensity
  return (1 / (1 + exp(-mean+400)));
}



void recieveEvent(int howMany) {

  while (1 < Wire.available()) {
    int w = Wire.read(); // Show byte value encoded
  }
  
  int w = Wire.read();
  Serial.print("!> Byte: "); Serial.println(w); // Serial monitor to check encoded byte

  if (manualOn == true) {
    if (w == 2) { // Water pump on (manual)
      digitalWrite(relay_2, HIGH);
    }

    if (w == 0) { // Water pump off (manual)
      digitalWrite(relay_2, LOW);
    }

    if (w == 4) { // UV Light on
      lightOn = true;
    }

    if (w == 6) { // UV light off
      lightOn = false;
    }

    if (w == 12) { // Water pump on (auto)
      waterOn = true;
    }

    if (w == 14) { // Water pump off (auto)
      waterOn = false;
    }
  }

  if (lightOn == true) {
    Serial.println(">> LIGHT ON");
    digitalWrite(relay_1, HIGH); // Turn on UV light
  }
  
  else {
    digitalWrite(relay_1, LOW); // Turn off UV light
  }

  if (waterOn == true) {
    Serial.println(">> WATER ON");
    digitalWrite(relay_2, HIGH); // Turn on water pump
  }
  
  else {
    digitalWrite(relay_2, LOW); // Turn off water pump
  }

  if (w == 8) { // Manual mode on
    manualOn = true;
    Serial.println("<----------MANUAL---------->"); // Serial monitor to check manual mode
    digitalWrite(relay_2, LOW); // Turn off water pump for good measure

  }

  if (w == 10) { // Manual mode off
    manualOn = false;
    Serial.println("<--------AUTOMATIC-------->"); // Serial monitor to check automatic mode
  }

}
