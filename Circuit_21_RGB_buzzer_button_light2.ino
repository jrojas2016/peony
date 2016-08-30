/*
  SparkFun Inventor's Kit
  Example sketch 21

  BASIC BLE CONTROL

  Turn an LED on and off using BLE and either a phone or tablet. Android and iOS devices     only!

  Based off of the BLE LED example written by Intel Corporation and included with the       Curie BLE Arduino Library.

*/

#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device (the board you're programming)
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service


//set BLE characteristic
BLEUnsignedCharCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int vibPin = 3; // pin to use for the LED
const int RED_PIN = 11;
const int GREEN_PIN = 10;
const int BLUE_PIN = 9;
const int buzzerPin = 7;
const int ledPin = 4;
const int sensorPin = 0;
const int buttonPin = 13;

int lightCal;
int lightVal;

//create a variable to store a counter and set it to 0
int counter = 0;

/*
  This sketch uses the buzzer to play songs.
  The Arduino's tone() command will play notes of a given frequency.
  We'll provide a function that takes in note characters (a-g),
  and returns the corresponding frequency from this table:

  note  frequency
  c     262 Hz
  d     294 Hz
  e     330 Hz
  f     349 Hz
  g     392 Hz
  a     440 Hz
  b     494 Hz
  C     523 Hz

  For more information, see http://arduino.cc/en/Tutorial/Tone
*/

// We'll set up an array with the notes we want to play
// change these values to make different songs!

// Length must equal the total number of notes and spaces

const int songLength = 4;

// Notes is an array of text characters corresponding to the notes
// in your song. A space represents a rest (no tone)

char notesc[] = "ccc "; // a space represents a rest
char notesd[] = "ddd "; // a space represents a rest
char notese[] = "eee "; // a space represents a rest
char notesf[] = "fff "; // a space represents a rest
char notesg[] = "cfcg"; // a space represents a rest
char notesa[] = "aaa "; // a space represents a rest
char notesb[] = "bbb "; // a space represents a rest

// Beats is an array of values for each note and rest.
// A "1" represents a quarter-note, 2 a half-note, etc.
// Don't forget that the rests (spaces) need a length as well.

int beats[] = {1, 1, 1, 1};

// The tempo is how fast to play the song.
// To make the song play faster, decrease this value.

int tempo = 150;

int frequency(char note)
{
  // This function takes a note character (a-g), and returns the
  // corresponding frequency in Hz for the tone() function.

  int i;
  const int numNotes = 8;  // number of notes we're storing

  // The following arrays hold the note characters and their
  // corresponding frequencies. The last "C" note is uppercase
  // to separate it from the first lowercase "c". If you want to
  // add more notes, you'll need to use unique characters.

  // For the "char" (character) type, we put single characters
  // in single quotes.

  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523};

  // Now we'll search through the letters in the array, and if
  // we find it, we'll return the frequency for that note.

  for (i = 0; i < numNotes; i++)  // Step through the notes
  {
    if (names[i] == note)         // Is this the one?
    {
      return (frequencies[i]);    // Yes! Return the frequency
    }
  }
  return (0); // We looked through everything and didn't find it,
  // but we still need to return a value, so return 0.
}

void setup()
{
  // set LED pin to output mode
  pinMode(vibPin, OUTPUT);
    //set the three pin variables as outputs
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  lightCal = analogRead(sensorPin);
  
  // Set up the pushbutton pins to be an input:
  pinMode(buttonPin, INPUT);
  
  // set advertised local name and service UUID:
  blePeripheral.setLocalName("101 Board");
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(switchCharacteristic);

  // set the initial value for the characeristic:
  switchCharacteristic.setValue(0);

  // begin advertising BLE service:
  blePeripheral.begin();
}

void loop()
{

   //Take a reading using analogRead() on sensor pin and store it in lightVal
  lightVal = analogRead(sensorPin);
  

  //if lightVal is less than our initial reading (lightCal) minus 50 it is dark and
  //turn pin 9 HIGH. The (-50) part of the statement sets the sensitivity. The smaller
  //the number the more sensitive the circuit will be to variances in light.
  if (lightVal < lightCal - 50)
  {
    digitalWrite(ledPin, HIGH);
  }

  //else, it is bright, turn pin 9 LOW
  else
  {
    digitalWrite(ledPin, LOW);
  }

  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central)
  {
    // while the central is still connected to peripheral:
    while (central.connected())
    {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written())
      {
        // any value other than 0, turn on the LED
        if (switchCharacteristic.value()==0)
        {
          digitalWrite(RED_PIN, LOW);
          digitalWrite(GREEN_PIN, LOW);
          digitalWrite(BLUE_PIN, LOW);
          digitalWrite(vibPin, LOW);
        }
        
        if (switchCharacteristic.value()==1){ 
          //digitalWrite(vibPin, HIGH);
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(GREEN_PIN, LOW);
          digitalWrite(BLUE_PIN, LOW);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notesc[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
  }
   

    //while (true) {}
        }

        if (switchCharacteristic.value()==2)
        {
          digitalWrite(RED_PIN, LOW);
          digitalWrite(GREEN_PIN, HIGH);
          digitalWrite(BLUE_PIN, LOW);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notesd[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
  }

    //while (true) {}
        }
                if (switchCharacteristic.value()==3)
        {
          digitalWrite(RED_PIN, LOW);
          digitalWrite(GREEN_PIN, LOW);
          digitalWrite(BLUE_PIN, HIGH);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notese[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
  }

    //while (true) {}
        }
                if (switchCharacteristic.value()==4)
        {
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(GREEN_PIN, HIGH);
          digitalWrite(BLUE_PIN, LOW);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notesf[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
  }

   //while (true) {}
        }
                if (switchCharacteristic.value()==5)
        {
          digitalWrite(RED_PIN, LOW);
          digitalWrite(GREEN_PIN, HIGH);
          digitalWrite(BLUE_PIN, HIGH);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notesg[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
  }

    //while (true) {}
        }
                if (switchCharacteristic.value()==6)
        {
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(GREEN_PIN, LOW);
          digitalWrite(BLUE_PIN, HIGH);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notesa[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
 }

    //while (true) {}
        }
                if (switchCharacteristic.value()==7)
        {
          digitalWrite(RED_PIN, HIGH);
          digitalWrite(GREEN_PIN, HIGH);
          digitalWrite(BLUE_PIN, HIGH);
          digitalWrite(vibPin, HIGH);
          delay(2000);
          digitalWrite(vibPin, LOW);
           int i, duration;
            
          for (i = 0; i < songLength; i++) // step through the song arrays
  {
    duration = beats[i] * tempo;  // length of note/rest in ms

    
      tone(buzzerPin, frequency(notesb[i]), duration);
      delay(duration);            // wait for tone to finish
    
    
    delay(tempo / 10);            // brief pause between notes
    
  }

    //while (true) {}
        }
        //else turn the LED off
//        else
  //      {
    //      digitalWrite(vibPin, LOW);
      //    digitalWrite(RED_PIN, LOW);
        //  digitalWrite(GREEN_PIN, LOW);
          //digitalWrite(BLUE_PIN, LOW);
        //}
      }
    }
  }
}


