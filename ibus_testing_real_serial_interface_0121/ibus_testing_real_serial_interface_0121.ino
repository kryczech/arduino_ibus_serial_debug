#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

byte readbuffer[64];
int i;
int cspin = 7;
boolean read_byte = false;
int buffer_index = 0;
int buffer_max = 64;
int cksum;
long lastrcv;
long lastsend;
boolean lastsendgood = true;
byte lastmessagetype = 0x03;
const int buttonPin = 2;
const int faultPin = 6;
const int ledPin = 13;

void setup()  
{
  // initialize buffer array to 0's
  memset(readbuffer, 0, sizeof(readbuffer));
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600, SERIAL_8E1);
  // Serial.begin(9600);
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("Hello, world?");
  mySerial.flush();
  
  pinMode(cspin, OUTPUT);
  digitalWrite(cspin, HIGH);
  pinMode(faultPin, OUTPUT);
  digitalWrite(faultPin, HIGH);
  
  lastrcv = millis();
  lastsend = millis();
}

void loop() // run over and over
{
// Somehow I think I'm having to slow this down to accurately read messages. 
// Trying to just add a delay x number of microseconds to see how it does.
  delayMicroseconds(1100);
  
// Check mySerial to see if we need to send something. 
// Do I really need to time constrain it here?
// if ((millis() - lastsend) > 500) {
  if (mySerial.available()) {
    mySerial.println("got input");
    lastsendgood = sendMessage(mySerial.read());
  }
  lastsend = millis();
  // }

// Do timeout of buffer after not receiving anything for 10ms
  if ((millis() - lastrcv) > 15) {
    memset(readbuffer, 0, sizeof(readbuffer));
    buffer_index = 0;
    read_byte = false;
    buffer_max = 64;
    lastrcv = millis();
    // mySerial.println("Timeout.");
    return;
  } 
  
  
  
  // If there's something to be read, read it.  
  if (Serial.available()) {
    readbuffer[buffer_index] = Serial.read();
    read_byte = true;
  }
  
  // If this is byte 2, then set buffer_max to it's value.
  // Also set cksum to xor of byte 1 and byte 2.
  if (read_byte) {
    if (buffer_index == 1) {
      buffer_max = readbuffer[buffer_index] + 2;
      cksum = readbuffer[0] ^ readbuffer[1];
    } else if ((buffer_index > 1 ) && (buffer_index < buffer_max)) {
      cksum = cksum ^ readbuffer[buffer_index];
    }
  }
  
  // Reset buffer_index when it is buffer_max - 1.
  if (buffer_index == (buffer_max - 1)) {
    if ((readbuffer[0] == 0x68) || (readbuffer[2] == 0x68)) {
      if (cksum == 0) {
        mySerial.print("Good message: ");
        mySerial.print(millis());
        mySerial.print(": ");
        for (i = 0; i < buffer_max; i++) {
          mySerial.print(readbuffer[i], HEX);
          mySerial.print(" ");
        }
        mySerial.println();
      } else {
        mySerial.print("Invalid message. cksum: ");
        mySerial.println(cksum, HEX);
        for (i = 0; i < buffer_max; i++) {
          mySerial.print(readbuffer[i], HEX);
          mySerial.print(" ");
        }
        mySerial.println();
      }
    }
    memset(readbuffer, 0, sizeof(readbuffer));
    buffer_index = 0;
    read_byte = false;
    lastrcv = millis();
  }
    
  // Increment index if we put something into the buffer
  if (read_byte == true) {
    read_byte = false;
    buffer_index++;
    lastrcv = millis();
  }
}

boolean sendMessage(byte messageType)
{ 
  // mySerial.print("Sending ...  ");
  int messagelen;
  byte volup[6] = { 0xF0, 0x04, 0x68, 0x32, 0x11, 0xBF };
  byte voldown[6] = { 0xF0, 0x04, 0x68, 0x32, 0x10, 0xBE };
  byte pressmode[6] = { 0xF0, 0x4, 0x68, 0x48, 0x23, 0xF7 };
  byte releasemode[6] = { 0xF0, 0x4, 0x68, 0x48, 0xA3, 0x77 };
  byte bradrox[20] = { 0x68, 0x12, 0x3B, 0x23, 0x62, 0x10, 0x42, 0x52, 0x41, 0x44, 0x52, 0x4F, 0x58, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x40 };
  
  // digitalWrite(faultPin, HIGH);
  switch (messageType) {
    
    case 0x2B: // + for vol up
      mySerial.println("Sending vol up.");
      messagelen = 6;
      for (int k = 0; k < messagelen; k++) {
        Serial.write(volup[k]);
      }
      break;
      
    case 0x2D: // - for vol down
      mySerial.println("Sending vol down.");
      messagelen = 6;
      for (int k = 0; k < messagelen; k++) {
        Serial.write(voldown[k]);
      }
      break;
      
    case 0x42: // B for BRADROX
      mySerial.println("Brad rocks!");
      messagelen = 20;
      for (int k = 0; k < messagelen; k++) {
        Serial.write(bradrox[k]);
      }
      break;
      
    case 0x6D: // m for mode
      mySerial.println("Sending Mode.");
      messagelen = 6;
      for (int k = 0; k < messagelen; k++) {
        Serial.write(pressmode[k]);
      }
      delay(150);
      for (int k = 0; k < messagelen; k++) {
        Serial.write(releasemode[k]);
      }
      break;
  }
  // digitalWrite(faultPin, LOW);
  
  boolean echogood = true;
  /* for (i = 0; i < messagelen; i++) {
    if (Serial.read() != outmessage[i]) {
      echogood = false;
    }
  } 
  
  if (echogood) {
    mySerial.println("Got echo!");
  } else {
    mySerial.println("No echo :(");
  } */
  
  return echogood;
  
}

