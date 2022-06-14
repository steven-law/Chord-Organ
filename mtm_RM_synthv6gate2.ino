/* custom firmware for the chord organ module from music thing modular designed and created by Tom Whitwell
    aviable at thonk
    with alot of help from "Wolle Wer". Thank you once again at this point
    this Firmware trys to be several synthvoices in one with nice audio-modulation and FX abilty.
    in the system are 4 waveformgenrators, 2 filters in series, wavefolder, bitcrusher and delay.
    the first filter is controled by the signal coming from stationCV Jack, the second filter is controlled
    by the stationknob aviable at page 0. You may control the first filters resonance with startknob also on page 0
    stationKnob = A9
    stationCV = A8
    startKnob = A7
    startCV = A6
    tasterButton = Pin 8
    triggerCV = Pin 9
    Output = dac1

    startCV changes Pitch within a range of 3.3V equal 40 Notes, notes are quantized
    stationCV changes filter1.frequency i.E. external envelope
    with tasterButton you can switch thru different pages.
    Each Page has the two knobs Station- and startKnob to change different parameters

    page (stationKnob, startKnob)
    initial page 0 (filter2.frequency, filter1.resonance)
    page 1 (mixer1.gain(Note1), (mixer1.gain(note2,3,4)
    page 2 (chord selection, waveform1,2,3,4.selection)
    page 3 (wavefolder1, bitcrusher1)
    page 4 (delay1.time <200ms, delay1.feedback)

  //todo:
  //EnvelopeGenerator with trigger in that actually sends a control value (dc2) to the filter (dc2 * 8184)
  //Envelopegenerator Attack/Release control on a new page controlled by stationKnob/startKnob
  //
  //
*/
#define BOUNCE_PIN 9
#include <synth_waveform.h>
#include <Bounce2.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dc2;            //xy=171.05627822875977,606.3250160217285
AudioSynthWaveform       waveform3;      //xy=195.80627059936523,430.00000762939453
AudioSynthWaveform       waveform1;      //xy=198.80627059936523,334.00000762939453
AudioSynthWaveform       waveform2;      //xy=199.80627059936523,383.00000762939453
AudioSynthWaveform       waveform4;      //xy=208.80627059936523,480.00000762939453
AudioSynthWaveformDc     dc3;            //xy=275.75000047683716,673.75
AudioEffectEnvelope      envelope1;      //xy=283.2500343322754,557.5000171661377
AudioMixer4              mixer1;         //xy=358.80627059936523,371.00000762939453
AudioMixer4              mixer4;         //xy=460.7500114440918,498.75001335144043
AudioFilterStateVariable filter1;        //xy=511.8062438964844,386.9999885559082
AudioSynthWaveformDc     dc1;            //xy=549.8062744140625,614.0000114440918
AudioMixer4              mixer2;         //xy=648.8062744140625,397.0000114440918
AudioEffectWaveFolder    wavefolder1;    //xy=815.8062744140625,460.0000114440918
AudioEffectBitcrusher    bitcrusher1;    //xy=846.8062744140625,350.0000114440918
AudioEffectDelay         delay1;         //xy=1016.8062744140625,601.0000114440918
AudioMixer4              mixer3;         //xy=1020.8062744140625,418.0000114440918
AudioFilterStateVariable filter3;        //xy=1020.8062744140625,493.0000114440918
AudioMixer4              mixer5;         //xy=1186.8062744140625,413.00000762939453
AudioOutputAnalog        dac1;           //xy=1322.8063125610352,418.9999885559082
AudioConnection          patchCord1(dc2, envelope1);
AudioConnection          patchCord2(waveform3, 0, mixer1, 2);
AudioConnection          patchCord3(waveform1, 0, mixer1, 0);
AudioConnection          patchCord4(waveform2, 0, mixer1, 1);
AudioConnection          patchCord5(waveform4, 0, mixer1, 3);
AudioConnection          patchCord6(dc3, 0, mixer4, 1);
AudioConnection          patchCord7(envelope1, 0, mixer4, 0);
AudioConnection          patchCord8(mixer1, 0, filter1, 0);
AudioConnection          patchCord9(mixer4, 0, filter1, 1);
AudioConnection          patchCord10(filter1, 0, mixer2, 0);
AudioConnection          patchCord11(dc1, 0, wavefolder1, 1);
AudioConnection          patchCord12(mixer2, 0, wavefolder1, 0);
AudioConnection          patchCord13(wavefolder1, bitcrusher1);
AudioConnection          patchCord14(bitcrusher1, 0, mixer3, 0);
AudioConnection          patchCord15(delay1, 0, filter3, 0);
AudioConnection          patchCord16(mixer3, delay1);
AudioConnection          patchCord17(mixer3, 0, mixer5, 0);
AudioConnection          patchCord18(filter3, 2, mixer3, 1);
AudioConnection          patchCord19(mixer5, dac1);
AudioControlSGTL5000     sgtl5000_1;     //xy=896.8062744140625,740.0000114440918
// GUItool: end automatically generated code



//Constants
const int tasterPin = 8;     //taster at Pin 8
const int triggerPin = 9;    //Jack for Gate at Pin 9

const int L1Pin = 3;       // LED1 at Pin 3
const int L2Pin = 4;      // LED2 at Pin 4
const int L3Pin = 5;      // LED3 at Pin 5
const int L4Pin = 6;      // LED4 at Pin 6
const int L5Pin = 11;     // LED5 at Pin 11
Bounce bounce = Bounce();
//Variables
int mode = 0;                // Variable for case switching
int DetectionMode = 0;       //detection for stationKnob hook up
int DetectionMode2 = 0;       //detection for startKnob hook up
int tasterState = 0;         // Variable to save tasterstate
int triggerState = 0;     // Variable to save triggerstate

float Note2[8] = {1.25992, 1.18920, 1.25992, 1.33484, 1.33484, 1.25992, 1.33484, 1.33484};  //{4, 3, 4, 3, 5, 4, 3, 3}Frequency proportion to 2nd note
float Note3[8] = {1.49830, 1.49830, 1.49830, 1.49830, 1.49830, 1.49830, 1.49830, 1.41421};  //{7, 7, 7, 7, 7, 7, 7, 6};Frequency proportion to 3rd note
float Note4[8] = {2.00000, 2.00000, 1.88775, 1.78179, 1.88775, 1.58739, 1.58739, 2.00000};  //{12, 12, 11, 10, 12, 8, 8, 12};Frequency proportion to 4th note
int noteArray;
float Hertz[61] = {65.4064, 69.957, 73.41619, 77.78175, 82.40689, 87.30706, 92.49861, 97.99886, 103.8262, 110.0000, 116.5409, 123.4708,
                   130.8128, 138.5913, 146.8324, 155.5635, 164.8138, 174.6141, 184.9972, 195.9977, 207.6523, 220.0000, 233.0819, 246.9417,
                   261.6256, 277.1826, 293.6648, 311.1270, 329.6276, 349.2282, 369.9944, 391.9954, 415.3047, 440.0000, 466.1638, 493.8833,
                   523.2511, 554.3653, 587.3295, 622.2540, 659.2551, 698.4565, 739.9888, 783.9909, 830.6094, 880.0000, 932.3275, 987.7666,
                   1046.502, 1108.731, 1174.659, 1244.508, 1318.510, 1396.913, 1479.978, 1567.982, 1661.219, 1760.000, 1864.655, 1975.533, 2093.005
                  };

//variables to handle each´s page last knob position
int filterCutoff = 1023;
int resoNance = 0;
int volume1 = 1023;
int volume234 = 1023;
int chordSelect = 0;
int wfSelect = 439;
int waveFolder = 100;
int bitCrusher = 1023;
int delayTime = 0;
int feedBack = 0;
int attAck = 5;
int reLease = 200;

void setup() {
  Serial.begin(19200); // USB is always 12 or 480 Mbit/sec
  //setting up the audio components
  //===========================================================================
  //analogReadResolution(12);
  AudioMemory(200);

  waveform1.begin(WAVEFORM_SAWTOOTH);
  waveform1.amplitude(1);
  waveform1.frequency(65.41);
  waveform1.pulseWidth(0.15);

  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform2.amplitude(1);
  waveform2.frequency(65.41);
  waveform2.pulseWidth(0.15);

  waveform3.begin(WAVEFORM_SAWTOOTH);
  waveform3.amplitude(1);
  waveform3.frequency(65.41);
  waveform3.pulseWidth(0.15);

  waveform4.begin(WAVEFORM_SAWTOOTH);
  waveform4.amplitude(1);
  waveform4.frequency(65.41);
  waveform4.pulseWidth(0.15);

  //mixer1 to controll Waveform1 with stationKnob; Waveform2-4 with startKnob on page 1
  mixer1.gain(0, 0.25);
  mixer1.gain(1, 0.25);
  mixer1.gain(2, 0.25);
  mixer1.gain(3, 0.25);

  envelope1.delay(0);
  envelope1.attack(200);
  envelope1.hold(0);
  envelope1.decay(500);
  envelope1.sustain(0.8);
  envelope1.release(500);

  dc2.amplitude(1);

  dc3.amplitude(0);
  mixer4.gain(0, 1);
  mixer4.gain(1, 1);

  //filter1 is controlled by the stationCV input
  filter1.frequency(26);                                                  //set initial Filter1 freq to 16000Hz
  filter1.resonance(0);                                                      //set Resonance to 0
  filter1.octaveControl(7);

  //filter2 is controlled by the stationKnob on page 0
  // filter2.frequency(16000);                                                  //set initial Filter2 freq to 16000Hz
  // filter2.resonance(0);                                                      //set Resonance to 0

  //mixer2 is just there, no need for it actually
  mixer2.gain(0, 1);                                                         //set Mixer2(Channel, Gain) (0-3, 0-1)
  mixer2.gain(1, 1);

  //dc1 for wavefolder input controlled by stationKnob on page 3
  dc1.amplitude(0.05);                                                       //set dc1 for Wavefolder Input

  //bitcrusher1 controlled by startKnob on page 3
  bitcrusher1.bits(16);
  bitcrusher1.sampleRate(44100);

  //delay1 time controlled by stationKnob; feedback controlled by startKnob on page 4
  delay1.delay(0, 3);                                                        //set Delay(OutChannel, saved Samples) (0-7, 3-449*) *maximum

  //filter3 HighPassfilter to get rid of the muddy sound in feedbackloop
  filter3.frequency(200);                                                   //set initial Filter3 freq to 16000Hz
  filter3.resonance(0);                                                      //set Resonance to 0

  //mixer3 for feedback controlled by startKnob on page 4
  mixer3.gain(0, 1);                                                         //setze Mixer2(Kanal, Gain) (0-3, 0-1)
  mixer3.gain(1, 0);

  //mixer5 again just a mixer to pass audio thru
  mixer5.gain(0, 0.5);



  //setting up the audio components DONE!!
  //===========================================================================

  //setting up the Pins as In´s n Out´s
  //============================================================================
  pinMode(tasterPin, INPUT);                                                 //taster is input
  pinMode(triggerPin, INPUT);                                             //trigger is input
  pinMode(L1Pin, OUTPUT);                                                    // Set LEDPin as output
  pinMode(L2Pin, OUTPUT);                                                    // Set LEDPin as output
  pinMode(L3Pin, OUTPUT);                                                    // Set LEDPin as output
  pinMode(L4Pin, OUTPUT);                                                    // Set LEDPin as output
  pinMode(L5Pin, OUTPUT);                                                    // Set LEDPin as output
  bounce.attach( BOUNCE_PIN, INPUT ); // USE EXTERNAL PULL-UP
  bounce.interval(3); // interval in ms
  // SET A VARIABLE TO STORE THE LED STATE
  int triggerState = LOW;
}




void loop() {
  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  bounce.update();
  tasterState = digitalRead(tasterPin);                                       //read tasterPin and set tasterState
  //triggerState = digitalRead(triggerPin);                                  //read triggerPin and set triggerState

  int startCV = analogRead(A8);
  int startKnob = analogRead(A7);                                             //lies Analog7 und setze auf startKnob
  int stationKnob = analogRead(A9);                                           //lies Analog9 und setze auf stationKnob
  int stationCV = analogRead(A6);                                             //lies Analog6 und setze auf stationCV
  int triggerlock = 0;

  //1V/Octave works, though only till 3.3V so its a 40 Note Range beginning from C1 65.4064Hz Hertzarray goes to 60, so you could transpose those 40 Notes :)
  //works independently from pageselection
  //PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP
  int hertzseriell = map(startCV, 0, 1023, 0, 40);
  waveform1.frequency(Hertz[hertzseriell]); //WF1 plays rootnote
  waveform2.frequency(Hertz[hertzseriell] * Note2[noteArray]); //WF2 plays rootnote * Frequencyproportionn from "Note2"-array
  waveform3.frequency(Hertz[hertzseriell] * Note3[noteArray]); //WF3 plays rootnote * Frequencyproportionn from "Note3"-array
  waveform4.frequency(Hertz[hertzseriell] * Note4[noteArray]); //WF4 plays rootnote * Frequencyproportionn from "Note4"-array
  //PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP

  //filter1 freuquency modulation from station CV Jack, independently from pageselection
  //filter1.frequency(map(stationCV, 0, 1023, 0, 8184));
  //mixer3.gain(0, map(stationCV, 0, 511, 0, 1));


  // <Bounce>.changed() RETURNS true IF THE STATE CHANGED (FROM HIGH TO LOW OR LOW TO HIGH)
  if ( bounce.changed() ) {
    // THE STATE OF THE INPUT CHANGED
    // GET THE STATE
    int deboucedInput = bounce.read();
    if ( deboucedInput == HIGH ) {
      envelope1.noteOn(); //trigger envelope1 to start envelope
    }
    if ( deboucedInput == LOW ) {
      envelope1.noteOff(); //end of gate, start release
    }
    digitalWrite(L5Pin, deboucedInput);
  }
  


  //TasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTaster
  //=======================================================================================================
  if (tasterState == HIGH)                                                    //when tasterState is High
  {
    mode++;                                                                   //count case mode + 1
    DetectionMode = 0;                                                        //everytime we press the button, ddetectionmode is resetted to 0
    DetectionMode2 = 0;
    delay(200);                                                               //delay zur Entprellung
  }
  //=======================================================================================================
  //TasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTasterTaster


  digitalWrite(L1Pin, LOW);                                               //Led 1 aus
  digitalWrite(L2Pin, LOW);                                               //Led 2 aus
  digitalWrite(L3Pin, LOW);                                               //Led 3 aus
  digitalWrite(L4Pin, LOW);                                               //Led 4 aus

  //begin of page switching and what happens in each page
  //==========================================================================

  switch (mode) {
    //00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    case 0: //Filter + Resonance

      if ((DetectionMode == 0) && (abs(stationKnob - filterCutoff) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode = 1;
      }
      if (DetectionMode == 1) {
        filterCutoff = stationKnob;
      }
      mixer4.gain(0, ((float)filterCutoff / 1023));       //Filter1 filters cutoff

      //--------------------------------------------------------------------------
      if ((DetectionMode2 == 0) && (abs(startKnob - resoNance) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode2 = 1;
      }
      if (DetectionMode2 == 1) {
        resoNance = startKnob;
      }
      filter1.resonance((float)map(resoNance, 0, 1023, 0, 5));                //controls Resonance
      break;
    case 1: //Volume1 + 2,3,4
      //1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111
      digitalWrite(L1Pin, HIGH);                                              //Led 1 an

      if ((DetectionMode == 0) && (abs(stationKnob - volume1) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode = 1;
      }
      if (DetectionMode == 1) {
        volume1 = stationKnob;
      }
      mixer1.gain(0, ((float)volume1 / 3069));                            //Gain Mixer1 Eingang 0
      //---------------------------------------------------------------------------------------------------------------
      if ((DetectionMode2 == 0) && (abs(startKnob - volume234) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode2 = 1;
      }
      if (DetectionMode2 == 1) {
        volume234 = startKnob;
      }
      mixer1.gain(1, ((float)volume234 / 3069));                              //Gain Mixer1 Eingang 1
      mixer1.gain(2, ((float)volume234 / 3069));                              //Gain Mixer1 Eingang 2
      mixer1.gain(3, ((float)volume234 / 3069));                              //Gain Mixer1 Eingang 3
      break;

    //2222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222
    case 2: //Chordtype + Waveformtype
      digitalWrite(L2Pin, HIGH);                                              //Led 2 an
      if ((DetectionMode == 0) && (abs(stationKnob - chordSelect) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode = 1;
      }
      if (DetectionMode == 1) {
        chordSelect = stationKnob;
      }
      noteArray = map(chordSelect, 0, 1023, 0, 7);                       //stationKnob stores value 0-7 into noteArray
      //----------------------------------------------------------------------------------------------------------------------------------------
      if ((DetectionMode2 == 0) && (abs(startKnob - wfSelect) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode2 = 1;
      }
      if (DetectionMode2 == 1) {
        wfSelect = startKnob;
      }
      waveform1.begin(map(wfSelect, 0, 1023, 0, 12));                        //waveform selection with startKnob
      waveform2.begin(map(wfSelect, 0, 1023, 0, 12));                        //waveform selection with startKnob
      waveform3.begin(map(wfSelect, 0, 1023, 0, 12));                        //waveform selection with startKnob
      waveform4.begin(map(wfSelect, 0, 1023, 0, 12));                        //waveform selection with startKnob
      break;

    //3333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333
    case 4: //wavefolder + bitcrusher
      digitalWrite(L4Pin, HIGH);                                              //Led 3 an
      if ((DetectionMode == 0) && (abs(stationKnob - waveFolder) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode = 1;
      }
      if (DetectionMode == 1) {
        waveFolder = stationKnob;
      }
      dc1.amplitude((float)waveFolder / 1023);                              //dc-value 0-1 for Wavefolder controlled by stationKnob
      //-----------------------------------------------------------------------------------------------------------------------------
      if ((DetectionMode2 == 0) && (abs(startKnob - bitCrusher) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode2 = 1;
      }
      if (DetectionMode2 == 1) {
        bitCrusher = startKnob;
      }
      bitcrusher1.sampleRate(map(bitCrusher, 0, 1023, 1, 44100));              //samplerate 1-44100Hz controlled by startKnob
      break;

    //4444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444
    case 5: //delaytime + feedback
      digitalWrite(L1Pin, HIGH);
      digitalWrite(L2Pin, HIGH);

      if ((DetectionMode == 0) && (abs(stationKnob - delayTime) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode = 1;
      }
      if (DetectionMode == 1) {
        delayTime = stationKnob;
      }
      delay1.delay(0, map(delayTime, 0, 1023, 3, 200));                     //delaytime 3-200ms controlled by stationknob
      //--------------------------------------------------------------------------------------------------------------------------------------
      if ((DetectionMode2 == 0) && (abs(startKnob - feedBack) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode2 = 1;
      }
      if (DetectionMode2 == 1) {
        feedBack = startKnob;
      }
      mixer3.gain(1, ((float)feedBack / 1023));                              //delayfeedback 0-1 controlled by startknob
      break;

    //00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    case 3: //Envelope1 Attack Release
      digitalWrite(L3Pin, HIGH);
      if ((DetectionMode == 0) && (abs(stationKnob - attAck) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode = 1;
      }
      if (DetectionMode == 1) {
        attAck = stationKnob;
      }
      envelope1.attack((float)map(attAck, 0, 1023, 0, 2047));       //Filter1 filters cutoff

      //--------------------------------------------------------------------------
      if ((DetectionMode2 == 0) && (abs(startKnob - reLease) < 10)) {         // Potiwert muss in die Naehe des letzten Wertes kommen
        DetectionMode2 = 1;
      }
      if (DetectionMode2 == 1) {
        reLease = startKnob;
      }
      envelope1.release((float)map(reLease, 0, 1023, 0, 4091));                //controls Resonance
      break;
    default:                                                                  //next taster push brings us back to page 0
      mode = 0;
  }
  //end of page switching
  //==================================================================================================



}
