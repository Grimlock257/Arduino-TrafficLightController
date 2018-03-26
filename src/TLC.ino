/*
Author: Adam Watson
Course: Computer Systems Engineering (Year 1)
Module: Systems Technology, Embedded Systems strand, coursework 1
*/

#define PINBTN1 5
#define PINBTN2 6
#define PINBTN3 7

#define PINLATCH A4
#define PINCLOCK A3
#define PINDATA A5

#define BUZZTONE 100

#define SEG0 B11111100
#define SEG1 B01100000
#define SEG2 B11011010
#define SEG3 B11110010

/* Display the specified number on the seven segment display */
void printSevenSegment(byte value) {
	digitalWrite(PINLATCH, LOW);
	shiftOut(PINDATA, PINCLOCK, LSBFIRST, value);
	digitalWrite(PINLATCH, HIGH);
}

/* Represents a whole traffic light set up (LEDs, Ultrasonic Sensor, LDR and Buzzer) */
class TLC {
private:
	enum LedColour : byte {
		NONE, RED, YELLOW, GREEN
	};

	const byte pinRed;
	const byte pinYellow;
	const byte pinGreen;
	const byte pinEcho;
	const byte pinTrig;
	const byte pinBuzz;

	const byte pinLDR;

	bool isPedLEDOn = false;
	bool isPedFlashing = false;

	byte distanceThreshold; // Centimeters
	float lightThreshold;
	float distance; // Store distance of the closest object in the range of the ultrasonic sensor

	// Sequence array stored in second chunks
	static const byte ledSeqSize = 6;
	byte ledSeq[ledSeqSize] = { LedColour::RED, LedColour::RED, LedColour::YELLOW, LedColour::GREEN, LedColour::GREEN, LedColour::YELLOW };
	byte ledSeqPos = -1;
	byte pedFlashes = 0;
	int ledSeqTime = 100;
	int ledSeqElapsed = 0;

	/*
	Calculate distance of object using ultrasonic sensor. Send short pulse
	then convert time read from the analog input into centimeters
	*/
	float calculateDistance() {
		digitalWrite(pinTrig, HIGH);
		delayMicroseconds(5);
		digitalWrite(pinTrig, LOW);

		return pulseIn(pinEcho, HIGH) / 58;
	}

public:
	TLC(byte aRed, byte aYellow, byte aGreen, byte aEcho, byte aTrig, byte aBuzz, byte aLDR, byte aDistanceThreshold, float aLightThreshold) :
		pinRed(aRed),
		pinYellow(aYellow),
		pinGreen(aGreen),
		pinEcho(aEcho),
		pinTrig(aTrig),
		pinBuzz(aBuzz),

		pinLDR(aLDR),

		distanceThreshold(aDistanceThreshold),
		lightThreshold(aLightThreshold) {
	}

	void setup() {
		// Set the pin modes (digital then analog)
		pinMode(pinRed, OUTPUT);
		pinMode(pinYellow, OUTPUT);
		pinMode(pinGreen, OUTPUT);
		pinMode(pinEcho, INPUT);
		pinMode(pinTrig, OUTPUT);
		pinMode(pinBuzz, OUTPUT);

		pinMode(pinLDR, INPUT);

		// Make sure pins are initially not active
		digitalWrite(pinRed, LOW);
		digitalWrite(pinGreen, LOW);
		digitalWrite(pinTrig, LOW);
		digitalWrite(pinBuzz, LOW);
	}

	void update() {
		byte ledColour;

		extern byte operationMode;

		switch (operationMode) {
		case 1:
			Serial.println("Mode 1 (Regular)");
			printSevenSegment(SEG1);
			ledColour = regularMode();
			break;
		case 2:
			Serial.println("Mode 2 (Pedestrian)");
			printSevenSegment(SEG2);
			ledColour = pedestrianMode();
			break;
		case 3:
			Serial.println("Mode 3 (Night)");
			printSevenSegment(SEG3);
			ledColour = nightMode();
			break;
		default:
			Serial.println("Please select a mode");
			printSevenSegment(SEG0);
		}

		switch (ledColour) {
		case LedColour::RED:
			digitalWrite(pinYellow, LOW);
			digitalWrite(pinGreen, LOW);
			digitalWrite(pinRed, HIGH);
			delay(ledSeqTime);
			break;
		case LedColour::YELLOW:
			digitalWrite(pinRed, LOW);
			digitalWrite(pinGreen, LOW);
			digitalWrite(pinYellow, HIGH);
			delay(ledSeqTime);
			break;
		case LedColour::GREEN:
			digitalWrite(pinRed, LOW);
			digitalWrite(pinYellow, LOW);
			digitalWrite(pinGreen, HIGH);
			delay(ledSeqTime);
			break;
		default:
			digitalWrite(pinRed, LOW);
			digitalWrite(pinYellow, LOW);
			digitalWrite(pinGreen, LOW);
			delay(ledSeqTime);
		}
	}

	/* Regular mode - LED sequence: RED YELLOW GREEN YELLOW (durations 2, 1, 2, 1 respectively) */
	byte regularMode() {
		return (((ledSeqElapsed += ledSeqTime) %= (1000 - ledSeqTime)) == 0) ? ledSeq[++ledSeqPos %= ledSeqSize] : ledSeq[ledSeqPos];
	}

	/* This purely decided whether the yellow LED needs to be on or off, of it the flashed as elapsed it's time */
	byte flashPed() {
		if (pedFlashes++ < 20) {
			isPedFlashing = true;
			tone(pinBuzz, BUZZTONE);
			return (isPedLEDOn = !isPedLEDOn) ? LedColour::YELLOW : LedColour::NONE;
		} else {
			isPedFlashing = false;
			noTone(pinBuzz);
			return 255; // Return 255 as it's the highest value from a byte and is used as a check in the pedestrianMode() method
		}
	}

	/*
	Pedestrian mode where lights behave as regular mode unless object detected to
	be within the specified distance, if object in range, sound buzzer and flash
	yellow LED 10 times, if object leaves scene, return to regular mode
	*/
	byte pedestrianMode() {
		distance = calculateDistance();

		Serial.println("Distance: " + String(distance) + "cm");

		/*
		Check if calculated distance below distance threshold, if so sound buzzer and
		make yellow LED flash 10 times and then stop the buzzer, then return to regular
		mode until an object in range again
		*/
		if (distance < distanceThreshold) {
			byte led = flashPed();
			return (led == 255) ? regularMode() : led;
		} else {
			if (!isPedFlashing) {
				pedFlashes = 0;
				return regularMode();
			} else {
				return flashPed();
			}
		}
	}

	/*
	Night mode, lights behave differently if the ambient light is below specified
	threshold. If so, red LED to be on constantly until object detected to be within
	distance threshold, if so, turn green LED on *instead* until the object moves
	out of range, then turn red LED back on again. Otherwise, return regular mode
	*/
	byte nightMode() {
		float ldrValue = analogRead(pinLDR);
		Serial.println("LDR Value: " + String(ldrValue));

		// Check if LDR value below specified threshold, if so, turn red LED on until
		// object in range, if so turn green LED on. Otherwise, do regular mode
		if (ldrValue < lightThreshold) {
			distance = calculateDistance();

			Serial.println("Distance: " + String(distance) + "cm");

			return (distance < distanceThreshold) ? LedColour::GREEN : LedColour::RED;
		} else {
			return regularMode();
		}
	}
};

byte operationMode = 0;

// Create an instance of our TLC class to represent a TLC system
TLC tlc(13, 12, 11, 10, 9, 8, A0, 10, 700);

/* This is where code is executed once upon program launch by the Arduino */
void setup() {
	// Call the setup method of the TLC instance to setup pins
	tlc.setup();

	// Set the pin mode of the button pins
	pinMode(PINBTN1, INPUT_PULLUP);
	pinMode(PINBTN2, INPUT_PULLUP);
	pinMode(PINBTN3, INPUT_PULLUP);

	// Set the pin mode of the seven segment display pins
	pinMode(PINLATCH, OUTPUT);
	pinMode(PINCLOCK, OUTPUT);
	pinMode(PINDATA, OUTPUT);

	// Attach an interupt for our buttons so they function at any time
	attachInterrupt(0, btnISR, CHANGE);

	// Begin the serial communication with a baud rate of 9600
	Serial.begin(115200);
}

/* Loop function, which is the code that is executed over and over by the Ardunio */
void loop() {
	// Check if user changed mode from serial monitor
	if (Serial.available()) {
		byte inVal = Serial.parseInt();

		switch (inVal) {
		case 0:
		case 1:
		case 2:
		case 3:
			operationMode = inVal;
		}
	}
	// Update the TLC each time the loop is ran
	tlc.update();

	// Separate each iteration of the loop in serial monitor
	Serial.println("--------------------------------------");
}

/* This is ran when an interrupt is received on via pin 2 for button detection */
void btnISR() {
	if (digitalRead(PINBTN1) == LOW) {
		operationMode = 1;
	} else if (digitalRead(PINBTN2) == LOW) {
		operationMode = 2;
	} else if (digitalRead(PINBTN3) == LOW) {
		operationMode = 3;
	}
}
