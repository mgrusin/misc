// "Trigger" example sketch for Lilypad MP3 Player
// Mike Grusin, SparkFun Electronics
// http://www.sparkfun.com

// This sketch (which is preloaded onto the board by default),
// will play a specific audio file when one of the five trigger
// inputs (labeled T1 - T5) is momentarily grounded.

// You can place up to five audio files on the micro-SD card.
// These files should have the desired trigger number (1 to 5)
// as the first character in the filename. The rest of the
// filename can be anything you like. Long file names will work,
// but will be translated into short 8.3 names. We recommend using
// 8.3 format names without spaces, but the following characters
// are OK: .$%'-_@~`!(){}^#&. The VS1053 can play a variety of 
// audio formats, see the datasheet for information.

// By default, a new trigger will interrupt a playing file, except
// itself. (In other words, a new trigger won't restart an 
// already-playing file). You can easily change this behavior by
// modifying the global variables "interrupt" and "interruptself"
// below.

// This sketch can output serial debugging information if desired
// by changing the global variable "debugging" to true. Note that
// this will take away trigger inputs 4 and 5, which are shared 
// with the TX and RX lines. You can keep these lines connected to
// trigger switches and use the serial port as long as the triggers
// are normally open (not grounded) and remain ungrounded while the
// serial port is in use.

// Uses the SdFat library by William Greiman, which is supplied
// with this archive, or download from http://code.google.com/p/sdfatlib/

// Uses the SFEMP3Shield library by Bill Porter, which is supplied
// with this archive, or download from http://www.billporter.info/

// License:
// We use the "beerware" license for our firmware. You can do
// ANYTHING you want with this code. If you like it, and we meet
// someday, you can, but are under no obligation to, buy me a
// (root) beer in return.

// Have fun! 
// -your friends at SparkFun

// Revision history:
// 1.0 initial release MDG 2012/11/01


// We'll need a few libraries to access all this hardware!

#include <SPI.h>            // To talk to the SD card and MP3 chip
#include <SdFat.h>          // SD card file system
#include <SFEMP3Shield.h>   // MP3 decoder chip
#include <wire.h>           // Wire (I2C) library

// Constants for a few outputs we'll be using:

const int ROT_LEDR = 10; // Red LED in rotary encoder (optional)
const int EN_GPIO1 = A2; // Amp enable + MIDI/MP3 mode select
const int SD_CS = 9;     // Chip Select for SD card

//float bluelight;
//float blueenvelope;
const float TWOPI = 6.283185307179586476925286766559;

// Create library objects:

SFEMP3Shield MP3player;
SdFat sd;

// Set debugging = true if you'd like status messages sent 
// to the serial port. Note that this will take over trigger
// inputs 4 and 5. (You can leave triggers connected to 4 and 5
// and still use the serial port, as long as you're careful to
// NOT ground the triggers while you're using the serial port).

boolean debugging = true;


void setup()
{
  int x, index;
  SdFile file;
  byte result;
  char tempfilename[13];

  // If serial port debugging is inconvenient, you can connect
  // a LED to the red channel of the rotary encoder to blink
  // startup error codes:
  
  pinMode(ROT_LEDR,OUTPUT);
  digitalWrite(ROT_LEDR,HIGH);  // HIGH = off

  // The board uses a single I/O pin to select the
  // mode the MP3 chip will start up in (MP3 or MIDI),
  // and to enable/disable the amplifier chip:
  
  pinMode(EN_GPIO1,OUTPUT);
  digitalWrite(EN_GPIO1,LOW);  // MP3 mode / amp off


  // If debugging is true, initialize the serial port:
  // (The 'F' stores constant strings in flash memory to save RAM)
  
  if (debugging)
  {
    Serial.begin(9600);
    Serial.println(F("Lilypad MP3 Player trigger sketch"));
  }
  
  // Initialize the SD card; SS = pin 9, half speed at first

  if (debugging) Serial.print(F("initialize SD card... "));

  result = sd.begin(SD_CS, SPI_HALF_SPEED); // 1 for success
  
  if (result != 1) // Problem initializing the SD card
  {
    if (debugging) Serial.print(F("error, halting"));
    errorBlink(1); // Halt forever, blink LED if present.
  }
  else
    if (debugging) Serial.println(F("success!"));
  
  // Start up the MP3 library

  if (debugging) Serial.print(F("initialize MP3 chip... "));

  result = MP3player.begin(); // 0 or 6 for success

  // Check the result, see the library readme for error codes.

  if ((result != 0) && (result != 6)) // Problem starting up
  {
    if (debugging)
    {
      Serial.print(F("error code "));
      Serial.print(result);
      Serial.print(F(", halting."));
    }
    errorBlink(result); // Halt forever, blink red LED if present.
  }
  else
    if (debugging) Serial.println(F("success!"));

  // Set the VS1053 volume. 0 is loudest, 255 is lowest (off):

  MP3player.setVolume(30,30);
  
  // Turn on the amplifier chip:
  
  digitalWrite(EN_GPIO1,HIGH);
  delay(2);
}


void loop()
{
  if (!MP3player.isPlaying())
    MP3player.playMP3("TARDIS.mp3");

  Mp3whilePlaying();
  
  delay(5);

  int regAddress = 0x32;  //first axis-acceleration-data register on the ADXL345
  int x, y, z;
  static int count;

  // read the current acceleration from the ADXL345:
  readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345
  //each axis reading is 10 bits in two bytes. combine these into one int.
  x = (((int)buff[1]) << 8) | buff[0];   
  y = (((int)buff[3])<< 8) | buff[2];
  z = (((int)buff[5]) << 8) | buff[4];
  
  // from experimentation, we know that 1g is about 230.
  // We'll look for values a little outside that range.
  // We also want to look for a sustained acceleration, not single spikes,
  // so we'll wait until we have 10 values in a row that fall outside that range.
  
  if ((z > 238) || (z < 220))
    count++;
  else
    count = 0;
    
  if (count > 10) // bingo! let's play the noise and light the lights
  {
	// reset the light values to 0
    bluelight = 0.0;
    blueenvelope = 0.0;

	// turn on the amplifier
    digitalWrite(5,HIGH);

	// start playing the sound
    sprintf(trackName, "TARDIS.MP3");
    playMP3(trackName);

	// done playing, turn off the amplifier
    digitalWrite(5,LOW);

	// if the light is still some level of "on", fade it to off
    while (bluelight > 0)
    {
      Mp3whilePlaying();
      delay(10);
    }
  }
  // ADXL delay
  delay(10);

}


void Mp3whilePlaying() // called repeatedly by MP3 player code between data transfers.
// typically runs every few ms, don't dawdle more than 100ms here or the MP3 chip will starve!
{
  static float bluelight, blueenvelope;
  
  // slowly pulse the blue light.
  // we'll do this by generating a cosine wave from the incrementing value bluelight
  // we'll also slowly brighten the blue light using the blueenvelope value
  
  // increment bluelight value (for cosine wave)
  bluelight += 0.04;
  
  // keep the above value between 0 and two * PI
  if (bluelight > TWOPI) bluelight = 0.0;

  // increment the brightness value
  blueenvelope += 0.2;
  
  // 127 is maximum brightness
  if (blueenvelope > 127.0) blueenvelope = 127.0;
  
  // generate the brightness of the light using bluelight and blueenvelope
  analogWrite(ROT_LEDR,cos(bluelight) * -1 * blueenvelope + blueenvelope);
}

void readFrom(int device, byte address, int num, byte buff[]) 
{
  Wire.beginTransmission(device); //start transmission to device 
  Wire.send(address);        //sends address to read from
  Wire.endTransmission(); //end transmission
  
  Wire.beginTransmission(device); //start transmission to device (initiate again)
  Wire.requestFrom(device, num);    // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())    //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.receive(); // receive a byte
    i++;
  }
  Wire.endTransmission(); //end transmission
}

void errorBlink(int blinks)
{
  // The following function will blink the red LED in the rotary
  // encoder (optional) a given number of times and repeat forever.
  // This is so you can see any startup error codes without having
  // to use the serial monitor window.

  int x;

  while(true) // Loop forever
  {
    for (x=0; x < blinks; x++) // Blink the given number of times
    {
      digitalWrite(ROT_LEDR,LOW); // Turn LED ON
      delay(250);
      digitalWrite(ROT_LEDR,HIGH); // Turn LED OFF
      delay(250);
    }
    delay(1500); // Longer pause between blink-groups
  }
}

