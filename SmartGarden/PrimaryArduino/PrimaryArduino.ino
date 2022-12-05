#include <Wire.h>
#include <LiquidCrystal_74HC595.h>


LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);


/*----------------------------------Setup and assign value to variable----------------------------------*/

// Variables related to soil moisture sensor
char capacitiveSoilSensor = A3;
int soilMoisturePercent = 0;


// Variables related to water control
int waterLED = 3;
int waterButtonPin = 2;
bool waterButtonState = false; // Button state of water (false in default)
bool waterAuto_On = false; // State of water pump in automatic mode
bool waterManual_On = false; // State of water pump in manual mode


// Variables related to UV light control
int lightLED = 5;
int lightButtonPin = 6;
bool lightButtonState = false; // Button state of light (false in default)
bool lightManual_On = false; // State of UV light in manual mode


// Varaibles related to manual control override
int redLED = 9;
int manualButtonPin = 4;
bool manualMode = false; // Flag to activate manual mode


char thermometer = A0;
int celcius = 0;

int resetPin = 7;

// Variables related to I2C communication
byte h = 0; // Water pump off (manual)
byte w = 2; // Water pump on (manual)

byte l = 4; // UV Light on
byte f = 6; // UV Light off

byte e = 8; // Manual mode on
byte y = 10; // Manual mode off

byte a = 12; // Water pump on (auto)
byte u = 14; // Water pump off (auto)


// Declare icons for LCD

byte humidityIcon[8] = // Icon of water droplet
{
    0b00100,
    0b00100,
    0b01010,
    0b01010,
    0b10001,
    0b11001,
    0b11101,
    0b01110
};


byte temperatureIcon[8] = // Icon of thermometer
{
    0b00100,
    0b01010,
    0b01010,
    0b01110,
    0b01110,
    0b11111,
    0b11111,
    0b01110
};

byte celciusIcon[8] = // Character ° for celcius
{
    0b00111,
    0b00101,
    0b00111,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

byte nodeIcon_1[8] = // Icon of primary node
{
    0b01111,
    0b01001,
    0b01101,
    0b01101,
    0b01101,
    0b01111,
    0b00000,
    0b00000
};

byte nodeIcon_2[8] = // Icon of secondary node
{
    0b11110,
    0b10010,
    0b11010,
    0b10110,
    0b10010,
    0b11110,
    0b00000,
    0b00000
};


byte autoConnectionIcon[8] = // Icon for auto connection animation
{
    0b00000,
    0b00000,
    0b10001,
    0b10001,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

byte manualConnectionIcon_1[8] = // Icon for manual connection animation 1
{
    0b00000,
    0b00000,
    0b11000,
    0b11000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

byte manualConnectionIcon_2[8] = // Icon for manual connection animation 2
{
    0b00000,
    0b00000,
    0b00011,
    0b00011,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};


/*----------------------------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(9600); // Set baud rate at 9600

  Wire.begin(5); // Establish communication to secondary Arduino

  lcd.begin(16,2); // Setup LCD display


  // Create character of customised icon
  lcd.createChar(0, humidityIcon);
  lcd.createChar(1, temperatureIcon);
  lcd.createChar(2, celciusIcon);

  lcd.createChar(3, nodeIcon_1);
  lcd.createChar(4, nodeIcon_2);

  lcd.createChar(5, autoConnectionIcon);
  lcd.createChar(6, manualConnectionIcon_1);
  lcd.createChar(7, manualConnectionIcon_2);


  // Setup strings and icons on LCD that reamains unchange

  // Setup 'Mode' on LCD
  lcd.setCursor(0, 0); 
  lcd.print("Mode: "); 

  // Setup primary node icon
  lcd.setCursor(13,0);
	lcd.write(byte(3)); 

  // Setup secondary node icon
  lcd.setCursor(15,0);
	lcd.write(byte(4)); 

  // Setup soil humidity display
  lcd.setCursor(0,1);
	lcd.write(byte(0)); // humidity icon on LCD
  lcd.setCursor(4, 1); 
  lcd.print("%"); // '%' next to moisture value

  // Setup temperature icon
  lcd.setCursor(8,1);
	lcd.write(byte(1)); 

  // Setup ° and celcius on LCD
  lcd.setCursor(12,1);
	lcd.write(byte(2)); 
  lcd.setCursor(13, 1);
  lcd.print("C"); 


  pinMode(waterLED, OUTPUT);
  pinMode(waterButtonPin, INPUT);

  pinMode(lightLED, OUTPUT);
  pinMode(lightButtonPin, OUTPUT);

  pinMode(redLED, OUTPUT);
  pinMode(manualButtonPin, OUTPUT);

}

void loop() {
  delay(100);

  if (manualMode == true) { // Turn on manual mode if true
    Wire.beginTransmission(4); 
    Wire.write(e); // Sent manual mode command to secondary Arduino
    Wire.endTransmission(); 

  } 
  
  else { // Turn off manual mode
    if (waterAuto_On == true) { // Automatic mode for water pump if true
      Wire.beginTransmission(4); 
      Wire.write(a); // Sent flag for water pump along serial communication
      Wire.endTransmission(); 
    }
    
    else { 
      Wire.beginTransmission(4);
      Wire.write(y); // Sent flag for water pump along serial communication
      Wire.endTransmission(); 

      lcd.setCursor(14,0); // Display 'autoConnectionIcon' icon
	    lcd.write(byte(5));

    }
  }

  soilMoisture(); // Process soil moisture data

  if (manualMode == true) {
    waterButton(); // Manual control on water pump
    lightButton(); // Manual control on UV light strip
    manualAnimation(); // Play animation of manual icons
  }

  detectMode(); // Flash red LED light if in manual mode
  displayMode(); // Display mode and mositure of soil
}


/*----------------------------------------------------------------------------------------------------*/

int airTemp() { // Get air temperature from TMP-36
  int reading = analogRead(thermometer);
  float voltage = reading * (5000 / 1024.0); // calculate analogue value to voltage
  celcius = (voltage - 500) / 10; // Change voltage to celcius

  return celcius; // return to  displayMode()
}


int soilMoisture() { // Get soil moisture from capacitive moisture soil sensor
  int soilMoistureRaw = analogRead(capacitiveSoilSensor);
  soilMoisturePercent = map (soilMoistureRaw,550,0,0,100); // Calculate moisture percentage by adding soil density

  if (soilMoisturePercent < 35) { // Sent flag if moisture less than 35 %
    waterAuto_On = true;
  }
  
  else {
    waterAuto_On = false;
  }

  // Serial monitor for soil moisture
  Serial.print(">>> MOISTURE: "); Serial.print(soilMoisturePercent); Serial.println(" %"); 
  return soilMoisturePercent;

}


void waterButton() {
  waterButtonState = digitalRead(waterButtonPin);

  if (waterButtonState == HIGH) {
    waterManual_On = !waterManual_On; // Swap boolean state
    delay(180);

  }

  if (waterManual_On == true) {

    digitalWrite(waterLED, HIGH); // Flash blue LED
    Serial.println("!> Water ON"); // Serial monitor for when water pump is turned on

    Wire.beginTransmission(4); // Send signal to activate UV light
    Wire.write(a);
    Wire.endTransmission();

  }

  else {
    digitalWrite(waterLED, LOW); // Flash blue LED

    Wire.beginTransmission(4); 
    Wire.write(u);// Send signal to stop water pump
    Wire.endTransmission();

  }
}


void lightButton() {
  lightButtonState = digitalRead(lightButtonPin); // Press UV light button

  if (lightButtonState == HIGH) {
    lightManual_On = !lightManual_On; // Swap boolean state
    delay(180);

  }


  if (lightManual_On) {
    digitalWrite(lightLED, HIGH); // Flash yellow LED
    Serial.println("!> Light ON"); // Serial monitor for when UV light is turned on

    Wire.beginTransmission(4);
    Wire.write(l); // Send signal to activate UV light
    Wire.endTransmission(); 

  } 
  
  else {
    digitalWrite(lightLED, LOW); // Close yellow LED

    Wire.beginTransmission(4); 
    Wire.write(f); // Send signal to stop UV light
    Wire.endTransmission(); // Terminate transsmission

  }
}

void detectMode() {
  int manualButtonState = digitalRead(manualButtonPin);

  if (manualButtonState == HIGH) {
    manualMode = !manualMode; // Swap boolean state
    delay(200);
  }

  if (manualMode == true) {
    digitalWrite(redLED, HIGH); // Open red LED if in manual mode
  }
  
  else {
    digitalWrite(redLED, LOW); // Close red LED if in manual mode
  }
}


void displayMode() { // Display variable that always change on LCD
  
  if (manualMode == true) { // Display 'MANUAL' on LCD when in manual mode
    lcd.setCursor(6,0);
    lcd.print("MANUAL");
  } 
  
  else {
    lcd.setCursor(6,0);
    lcd.print("AUTO  "); // Display 'AUTO' on LCD when in automatic mode
  }

  lcd.setCursor(2,1);
  lcd.print(soilMoisturePercent);
  
  celcius = airTemp(); // Get celcius value from airTemp() method
  lcd.setCursor(10,1);
  lcd.print(celcius); // Display live value of celcius

}

void manualAnimation() { // PLay animation for manual control using two icons
  lcd.setCursor(14,0);
	lcd.write(byte(6));
  delay(150);

  lcd.setCursor(14,0);
	lcd.write(byte(7));
  delay(150);

}
