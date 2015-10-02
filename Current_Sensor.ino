#include "SevSeg.h"

/* 

	Transition calcuations

	Normal mode
		Maximum input voltage = Vref = 1.218V
		Resolution = ~1.2mV
		Breakpoint: 105mV, 88/1024

	Medium amp mode
		Gain1(V >~ 20mV): ~11 at current above 50mA
		Gain2(V < 10mV): 8.45
		Maximum input voltage: Vref/Gain = 1.218V/11 = 110,7mV
		Resolution: 0.11mV
		Breakpoint: 12mV, 109/1024

	High amp mode - could perhaps be made better with an offset or something. LM358 isn't good enough for this.
		Gain1(V > 10mV): 56
		Gain2(5mV < V < 7.5mV): 45
		Gain3(V < 1mV): >100
		Maxium input voltage: Vref/Gain = 1.218V/50 = 24.36mV
		Resolution: 0.0237mV

*/


// Constants
const float voltage_reference = 1.218; //The supplied reference voltage
const float resistor_value = 1.0; //The value of the sense resistor
const int current_sensor = 0; //The current sensing pin
const int current_sensor_medium_amp = 1;
const int current_sensor_high_amp = 2;
const unsigned long refresh_rate = 100; //How often the measurement is taken

//For amplification
const int mediumBreakpoint = 88;
const int highBreakpoint = 109;


//Object instantiation
SevSeg display;

//variables
float current = 0;
int amplification = 0;

void setup() {
  analogReference(EXTERNAL);

  //Display
  byte numDigits = 3;
  byte digitPins[] = {11, 8, 7};
  //Pins same as the pin number on display, except 1 (E) >> 12
  byte segmentPins[] = {10, 6, 4, 2, 12, 9, 5, 3}; 
  display.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  display.setBrightness(100);

  Serial.begin(9600);
}

void loop() {
  Serial.println();
  
  current = analogRead(current_sensor);
  Serial.print("Base reading"); Serial.println(current);

  if( current < mediumBreakpoint ){
  	//Switch to 10x mode if reading is at 118.9mV approximately

  	current = analogRead(current_sensor_medium_amp);
    Serial.print("Medium amp reading"); Serial.println(current);

  	if( current < highBreakpoint ) {
  		//Switch to 100x mode if reading is at 
  		current = analogRead(current_sensor_high_amp);
      Serial.print("High amp reading"); Serial.println(current);
  		amplification = getHighAmplification(current);

  	} else{
  		//Set amplifiaction for 10x
  		amplification = getMediumAmplification(current);
  	}

  } else {

  	amplification = 1;

  }

  //Convert to miliamps
  float maxVoltage = voltage_reference / ( resistor_value * amplification);
  float convertedCurrent = ( current/1024 ) * maxVoltage;
  convertedCurrent *= 1000; //Convert from A to mA
  Serial.print("Output current: "); Serial.print(convertedCurrent); Serial.println(" mA");
  if( convertedCurrent < 10 ){
  	display.setNumber(convertedCurrent, 2);
  } else if( convertedCurrent < 100 ){
    display.setNumber(convertedCurrent, 1);
  } else {
    display.setNumber(convertedCurrent, 0);
  }

  //Update display
  unsigned long time = millis();
  while(time + refresh_rate >= millis()) {
  	display.refreshDisplay();
  }

}

/*

Should be measured

*/
float getMediumAmplification(float current) {

	return 11;

}

float getHighAmplification(float current) {

	return 50;

}
