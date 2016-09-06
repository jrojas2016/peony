/*
 * Heart Rate Calculation library.
 * Based on Pulse Sensor Amped 1.4 by Joel Murphy and Yury Gitman
 * See http://www.pulsesensor.com
 * and https://github.com/WorldFamousElectronics/PulseSensor_Amped_Arduino
 * 
 * Portions Copyright (c) 2016 Bradford Needham, North Plains, Oregon
 * @bneedhamia, https://www.needhamia.com
 * Licensed under the MIT License, a copy of which
 * should have been included with this software.
 * This software is not intended for medical use.
 */

#include "PulseSensorBPM.h"

/*
 * Returns the current version of the library.
 */
long PulseSensorBPM::getVersion() {
  return(PULSE_AMPED_BPM_VERSION);
}

/*
 * Constructs a Pulse Sensor manager.
 * pulse_pin = Analog Input. Arduino pin number
 *   to read the pulse signal from.  For example, A0
 * sample_interval_ms = the expected time between calls
 *   to PulseSensorBPM::readSensor(), in Milliseconds.
 *   The application is responsible for calling readSensor()
 *   every sample_interval_ms, either through an Timer interrupt
 *   or by using delayMicroseconds().  See the Examples.
 *   
 * This code assumes that the application constructs
 * a PulseSensorBPM object only once.
 */
PulseSensorBPM::PulseSensorBPM(int pulse_pin, unsigned long sample_interval_ms) {
  pinPulse = pulse_pin;
  sampleIntervalMs = sample_interval_ms;

  // pulse_pin needs no setup for analogRead().

  // Initialize (seed) the pulse detector
  IBI = 600;                  // 600ms per beat = 100 Beats Per Minute (BPM)
  Pulse = false; 
  sampleCounter = 0;
  lastBeatTime = 0;
  P = 512;                    // peak at 1/2 the input range of 0..1023
  T = 512;                    // trough at 1/2 the input range.
  thresh = 525;               // threshold a little above the trough
  amp = 100;                  // beat amplitude 1/10 of input range.
  firstBeat = true;           // looking for the first beat
  secondBeat = false;         // not yet looking for the second beat in a row
}

/*
 * Returns the most recent sample read from the pulse sensor.
 * Range 0..1023
 */
int PulseSensorBPM::getSignal() {
  return(Signal);
}

/*
 * Returs the most recent BPM (Beats Per Minute) calculation.
 */
int PulseSensorBPM::getBPM() {
  return(BPM);
}

/*
 * Returns the most recent IBI (Inter-Beat Inverval, in milliseconds)
 * calculation.
 */
int PulseSensorBPM::getIBI() {
  return(IBI);
}

/*
 * Returns the curent state of the 'Pulse' variable,
 * which is true if we belive we're currently sampling the major pulse
 * part of the waveform.
 * 
 * Used in the original code to drive an LED.
 */
boolean PulseSensorBPM::isPulse() {
  return(Pulse);
}

/*
 * Reads and processes a sample from the Pulse Sensor.
 * Returns true if the start of a pulse was found
 * (the variable QS in the original code), false otherwise.
 */
boolean PulseSensorBPM::readSensor() {
  boolean QS = false;                        // value to return. True if we found the start of a pulse.
  
  Signal = analogRead(pinPulse);             // read a sample from the pulse sensor
  
  sampleCounter += sampleIntervalMs;         // keep track of the time in mS with this variable
  int N = sampleCounter - lastBeatTime;      // monitor the time since the last beat to avoid noise

  //  find the peak and trough of the pulse wave
  if(Signal < thresh && N > (IBI/5)*3){      // avoid dichrotic noise by waiting 3/5 of last IBI
    if (Signal < T){                         // T is the trough
      T = Signal;                            // keep track of lowest point in pulse wave 
    }
  }

  if(Signal > thresh && Signal > P){         // thresh condition helps avoid noise
    P = Signal;                              // P is the peak
  }                                          // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250){                              // avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
      Pulse = true;                          // set the Pulse flag when we think there is a pulse
      IBI = sampleCounter - lastBeatTime;    // measure time between beats in mS
      lastBeatTime = sampleCounter;          // keep track of time for next pulse

      if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
        secondBeat = false;                  // clear secondBeat flag
        for(int i=0; i<=9; i++){             // seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;                      
        }
      }

      if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
        firstBeat = false;                   // clear firstBeat flag
        secondBeat = true;                   // set the second beat flag
        return(QS);                          // IBI value is unreliable so discard it
      }   


      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                  // clear the runningTotal variable    

      for(int i=0; i<=8; i++){                // shift data in the rate array
        rate[i] = rate[i+1];                  // and drop the oldest IBI value 
        runningTotal += rate[i];              // add up the 9 oldest IBI values
      }

      rate[9] = IBI;                          // add the latest IBI to the rate array
      runningTotal += rate[9];                // add the latest IBI to runningTotal
      runningTotal /= 10;                     // average the last 10 IBI values 
      BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
      QS = true;                              // set Quantified Self flag (we detected a beat)
    }                       
  }

  if (Signal < thresh && Pulse == true){   // when the values are going down, the beat is over
    Pulse = false;                         // reset the Pulse flag so we can do it again
    amp = P - T;                           // get amplitude of the pulse wave
    thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
    P = thresh;                            // reset these for next time
    T = thresh;
  }

  if (N > 2500){                           // if 2.5 seconds go by without a beat
    thresh = 512;                          // set thresh default
    P = 512;                               // set P default
    T = 512;                               // set T default
    lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
    firstBeat = true;                      // set these to avoid noise
    secondBeat = false;                    // when we get the heartbeat back
  }

  return(QS);
}
