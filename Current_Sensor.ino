#include "SevenSeg.h"

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

	FIRST CALIBRATION DATA
		High Amp absolute error: +1.4927 mA

*/

// Constants
const float voltage_reference = 1.218; //The supplied reference voltage
const float resistor_value = 1.0; //The value of the sense resistor
const int current_sensor = 0; //The current sensing pin
const int current_sensor_medium_amp = 2;
const int current_sensor_high_amp = 1;
const unsigned long refresh_rate = 100; //How often the measurement is taken

//Amplification switching limits
const int mediumBreakpoint = 88;
const int highBreakpoint = 109;


/* Calibration data
	
	Each reading is calibrated according to a linera function:
		y = ax +b, where x = reading and y = target

*/
const float noAmpA = 0.00108285451;
const float noAmpB = -0.013783994333;

const float mediumAmplification = 11;
const float mediumAmpA = 0.000917541660;
const float mediumAmpB = 0.006843216990;


const float highAmplification = 50;
const float highAmpA = 0.001035374060;
const float highAmpB = 0.001344642533;

//Display
/* Pins same as the pin number on display, except 1 (E) >> 12
  a:    10
  b:    6
  c:    4
  d:    2
  e:    12
  f:    9
  g:    5
  dp:   3
*/
SevenSeg display( 10, 6, 4, 2, 12, 9, 5 );
const int numDigits = 3;
int digitalPins[numDigits] = { 11, 8, 7 };

//variables
float current = 0;
double convertedCurrent = 0;
int amplification = 0;
float linearErrorCorrectionA = 0;
float linearErrorCorrectionB = 0;



void setup() {

	analogReference(EXTERNAL);

	//Display
	display.setDigitPins(numDigits, digitalPins);
	display.setDPPin(3);

	display.setTimer(2);
	display.startTimer();

	Serial.begin(9600);

}

void loop() {

	//Debugging
	Serial.println();
	delay(500);
	
	//Program
	readCurrent();
	convertCurrentReadingToMilliamps();
	displayCurrent();

}



void readCurrent(){

	current = analogRead(current_sensor);
	Serial.print("Base reading "); Serial.print(current); Serial.print(", ~"); Serial.print( ( ( current / 1024 ) * voltage_reference * 1000 ) ); Serial.println("mA");

	if( current < mediumBreakpoint ){
	//Switch to 10x mode if reading is at 118.9mV approximately

		current = analogRead(current_sensor_medium_amp);

		Serial.print("Medium amp reading "); Serial.println(current);

		if( current < highBreakpoint ) {
		//Switch to 100x mode if reading is at 
			current = analogRead(current_sensor_high_amp);
			Serial.print("High amp reading "); Serial.println(current);
			linearErrorCorrectionA = highAmpA;
			linearErrorCorrectionB = highAmpB;
			amplification = highAmplification;

		} else{
			
			//Set amplifiaction for 10x
			amplification = mediumAmplification;
			linearErrorCorrectionA = mediumAmpA;
			linearErrorCorrectionB = mediumAmpB;

		}

	} else {

		amplification = 1;
		linearErrorCorrectionA = noAmpA;
		linearErrorCorrectionB = noAmpB;

	}

}



void convertCurrentReadingToMilliamps(){

	//Max Voltage is the ceiling for the chosen amplifier
	float maxVoltage = voltage_reference / ( resistor_value * amplification);

	//Convert the relative voltage reading to a milliamps readout
	convertedCurrent = ( current/1024 ) * maxVoltage;
	//Apply linear correction function
	convertedCurrent = convertedCurrent * linearErrorCorrectionA + linearErrorCorrectionB;

	//Convert from A to mA
	convertedCurrent *= 1000; 
	
}



void displayCurrent(){

	Serial.print("Display current: "); Serial.print(convertedCurrent); Serial.println(" mA (approximate)");
	
	if( convertedCurrent >= 1000 ) {
		/*
		int digitOne = convertedCurrent/1000;
		int digitTwo = ( (int)convertedCurrent % 1000 ) / 100;
		char displayText[] = { '1', '.', '2', 'A', '\0' };

		Serial.println(displayText);*/
		//Something wacky is going on with the display driver, so I'll bypass this
		if( convertedCurrent < 1100 ){
			display.write("1.0A");
		} else if( convertedCurrent < 1200 ) {
			display.write("1.1A");
		} else if( convertedCurrent >= 1023 ) {
			display.write("Err");
		} else {
			display.write("1.2A");
		}

	} else if( convertedCurrent < 0.2 ){
    //minimum current allowd is 0.2 mA. Accuracy has probably given up before then
		display.write("NCu");
	} else {
		display.write(convertedCurrent);
	}

}



ISR(TIMER2_COMPA_vect){
	display.interruptAction();
}
