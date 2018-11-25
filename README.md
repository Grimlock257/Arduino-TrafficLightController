# Introduction
For the 'Embedded Systems' strand of my 'Systems Technology' module at Nottingham Trent University, I had to develop a traffic light controller (TLC).

To develop this system I used an Elegoo Uno R3 along with components (listed [here](#parts-list "Parts List")). My solution to this problem goes beyond the core requirements for the coursework piece, as it implemented a class, enum, and uses a 7 segment display and 8 bit shift register.

# Running the program
I have not yet finished the wiring diagram for this project. You can try and build the circuit from the below image / using the code if you wish.

1. Launch TLC.ino using Arduino IDE or other IDE that supports Arduino
2. Select the correct Arduino board type (for this project I used Arduino Uno)
3. Select the correct COM port
4. Compile & upload the program to the Arduino board
5. Should work!

![Circuit image](https://i.imgur.com/Kwn7Fjp.jpg)

# Problem Specification
The TLC needed to be able to function in 3 distinct modes, outlined below:
1. Regular Mode - Three LEDs should alternate (only 1 LED on at a time) to simulate a traffic light, the three colours being red, yellow and green. The sequence is:
   1. Red for 2 seconds
   2. Yellow for 1 second
   3. Green for 2 seconds
   4. Yellow for 1 second 
2. Pedestrian Mode - This mode utilises an ultrasonic range finder in order to determine the distance of a 'pedestrian'. The sequence outlines in regular mode happens until a pedestrian is detected to be within 10cm of the sensor, one this happens, a buzzer sounds and the yellow LED flashes 10 times at 0.1s intervals, and then returns to the sequence.
3. Night Mode - This mode uses a light dependent resistor (LDR) in order to work out how much ambient light is present. The LEDs alternate in the sequence outlined in regular mode until the ambient light falls below the set threshold, once this happens, the red LED will stay on unless something is detected to be within 10cm of th ultrasonic range finder, if this is the case the green LED will light until either the ambient light is great enough for night mode to deactivate or the object is no longer within 10cm.


# Parts List
The following list covers the core components used for solution (all included in [this](https://www.amazon.co.uk/Project-Complete-Ultimate-TUTORIAL-controller/dp/B01IUY62RM/ "Elegoo Ultimate Starter Kit") kit):
1. Elegoo Uno R3
2. Red, Green and Yellow LED
3. Piezo Buzzer
4. Ultrasonic Sensor
5. Light Dependant Resistor
6. Buttons
7. 8 Bit Shift Register (IC74HC595)
8. 7 Segment Display
9. Resistors (220Ω and 1kΩ)
10. Diode Rectifier (1N4007)
10. Wires
11. Breadboard
