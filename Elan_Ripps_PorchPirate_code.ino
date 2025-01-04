//Elan Ripps
//Porch Pirate Detterent Project
//December 2024
// Sources: 
  // Elegoo: Elegoo.com   
  // Colorado School of Mines: https://docs.google.com/document/d/1obryQtgOMd0R7mqLdrUUguU7etnmvPZzWeJ2sHndrh8/edit?tab=t.0#heading=h.426grbg17595
  // Gemini: gemini.google.com

// Include libraries
#include <IRremote.h> // Include the IRremote library for IR communication
#include <TimerFreeTone.h> // Include the TimerFreeTone library for tone generation
#include <pitches.h> // Include the pitches library for musical notes
#include <Servo.h> // Include the Servo library for controlling servos

// Create an instance of the Servo class
Servo myservo; 

// Define pins for IR receiver, tilt sensor, and tone output
const int irReceiverPin = 2;
const int tilt_pin = 3;
const int TONE_PIN = 5;

// Variables to track system state, tilt sensor state, time, and servo position
int systemState = 0; // Variable to track system state
int tiltState = 0; // Variable to track tilt sensor state
int min_counter = 0; // Minutes start at 0
int hr_counter = 0; // Hours start at 0
int pos = 0; // Servo starting point

// Set variables for time
unsigned long previousMillis = 0; // Store last time sensor was read
unsigned long lastMinuteChange = 0; // Store last time a minute passed

// Define IR remote control codes
const unsigned long volUpCode = 0xFF629D;  // Arming code
const unsigned long volDownCode = 0xFFA857;  // Disarming code

// Create an instance of the IRrecv class for receiving IR signals
IRrecv irrecv(irReceiverPin);
decode_results results;


void setup() {
  Serial.begin(9600); 
  irrecv.enableIRIn(); // Enable IR input
  pinMode(tilt_pin, INPUT_PULLUP); // Configure tilt sensor pin as input with internal pull-up resistor
  pinMode(TONE_PIN, OUTPUT); // Configure tone output pin as output
  myservo.attach(9); // Attach the servo to digital pin 9

  // Print a header to the serial monitor
  Serial.println("Time, Systrem State, Motion Detector"); 
}

void loop() {
  
  // Check if a second has passed
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) { // Read sensor every second (1000 milliseconds)
    previousMillis = currentMillis;

    // Calculate time in hours, minutes, and seconds
    unsigned long seconds = (currentMillis / 1000) % 60;
    unsigned long minutes = (currentMillis / (1000 * 60)) % 60;
    unsigned long hours = currentMillis / (1000 * 60 * 60);    
    if (currentMillis - lastMinuteChange >= 60000) { // Calculate if a minute has passed
      min_counter++;
      lastMinuteChange = currentMillis; 
    }
    if (currentMillis - (lastMinuteChange + (min_counter * 60000)) >= 3600000) { // Calculate if an hour has passed
      hr_counter++;
    } 
    //The previous 7 lines was debugged by Gemini

    // Log all data to serial 
    if (currentMillis < 60000){  // If under a minute record only seconds
      Serial.print(seconds); 
      Serial.print("s, "); 
    }
    else if (currentMillis < 3600000){ // If under an hour record only minutes and seconds
      Serial.print(min_counter); 
      Serial.print("m ");
      Serial.print(seconds); 
      Serial.print("s, "); 
    }

    else{ // If over an hour record hours, minutes, and seconds
      Serial.print(hr_counter); 
      Serial.print("h ");
      Serial.print(min_counter); 
      Serial.print("m ");
      Serial.print(seconds); 
      Serial.print("s, "); 
    }
    
    // Log the system state
    tiltState = digitalRead(tilt_pin);
    if (systemState == 1) { 
      // Output "System Armed" for Armed state
    } else {
      Serial.print("System Disarmed, "); // Output "System Disarmed" for Disarmed state
    }

    // Log the motion state
    if (tiltState == 1) { 
      Serial.println("Motion Detected"); // Output "Motion Detected" for tilt state: 1
    } 
    else {
      Serial.println("No Motion Detected"); // Output "No Motion Detected" for tilt state: 0
    } 

      // Activate detterent if both the system is armed and the tilt is detected
      if (systemState == 1 && tiltState == 1) {
        buzzer_alarm(); // Blaring alarm goes off
        for (pos = 0; pos <= 90; pos += 1) { // Pop odor bag
        myservo.write(pos);              
        delay(30);     // The previous 3 lines borrowed code from Elegoo                  
      } 
      }  
    }

  // Set up IR Code and Arming/Disarming system
  //This stanza borrowed some lines of code from Colorado School of Mines
  if (irrecv.decode(&results)) { // Check if a valid IR signal has been received
    if (results.value == volUpCode) { // If the received code matches the arming(volume up) code
      systemState = 1;
      armed_buzzer(); // Play confirmation tone that the code was recieved
    } 
    else if (results.value == volDownCode) { // If the received code matches the volume down code
      systemState = 0;
      disarmed_buzzer(); // Play confirmation tone that the code was recieved
    } 
    irrecv.resume(); // Receive the next value
  }
}

// Set up deterrnet alarm with passive buzzer 
// The following 35 lines used code from Colorado School of Mines 
void buzzer_alarm(){
  float tempo = 180; //beats per minute
  int melody[] = {A6, A6, A6}; // notes in the melody (as note names or as frequencies)
  float duration[] = {h, h, h}; // lengths as number of beats per note in melody
  // Array items can be multiple bytes. sizeof()/sizeof(arr[0]) gives array length
  int numNotes = sizeof(melody)/sizeof(melody[0]);
  play_song(tempo, melody, duration, numNotes); 
}

// Set up confirmation alarm with passive buzzer
void armed_buzzer(){
  float tempo = 200; //beats per minute
  int melody[] = {C6, C6}; // notes in the melody (as note names or as frequencies)
  float duration[] = {q, q}; // lengths as number of beats per note in melody
  // Array items can be multiple bytes. sizeof()/sizeof(arr[0]) gives array length
  int numNotes = sizeof(melody)/sizeof(melody[0]);
  play_song(tempo, melody, duration, numNotes); 
}

// Set up confirmation alarm with passive buzzer
void disarmed_buzzer(){
  float tempo = 200; // Beats per minute
  int melody[] = {D6}; // Notes in the melody (as note names or as frequencies)
  float duration[] = {h}; // Lengths as number of beats per note in melody
  int numNotes = sizeof(melody)/sizeof(melody[0]);
  play_song(tempo, melody, duration, numNotes); 
}

// Set up for detterent alarm 
void play_song(float tempo, int melody[], float duration[], int numNotes) {
  float beat_delay = (1/tempo) * 60 * 1000; //ms per beat
  for (int thisNote = 0; thisNote < numNotes; thisNote++) { // Loop through the notes in the array.
    TimerFreeTone(TONE_PIN, melody[thisNote], duration[thisNote]*beat_delay); // Play thisNote for duration
    delay(50); // Short delay between notes.
  } 
}

