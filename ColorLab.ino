#include "rgbrfid2.h"
#include <SoftwareSerial.h>

#define DEBUG true
SoftwareSerial pinSerial(RX_PIN, TX_PIN);

// Current and target colors
unsigned char colorPins[3] = {RED_PIN, GREEN_PIN, BLUE_PIN};
unsigned char currentColor[3] = {0,0,0};
unsigned char targetColor[3] = {255,255,255};

// RFID tag id's
char *redTags[NUM_TAGS];
char *greenTags[NUM_TAGS];
char *blueTags[NUM_TAGS];
char *yellowTags[NUM_TAGS];
char *cyanTags[NUM_TAGS];
char *magentaTags[NUM_TAGS];
char *rainbowTags[NUM_TAGS];
char *whiteTags[NUM_TAGS];
char *purpleTags[NUM_TAGS];

QMessage lastMessage = NONE;
unsigned char queueLength = 0;
QMessage* messageQueue = NULL;

int noDiskCount = 0;

// Initialization
void setup()
{
  // Serial communication for Debug
  // Set SerialMonitor to 115200 in IDE
  if(DEBUG)
    Serial.begin(115200);

  Debug("", "Setup...");
  
  // Set RFID tag id's
  setTags();
  
  // Digital pin 9 used to enable/disable the RFID reader
  pinMode(RFID_CONTROL_PIN,OUTPUT); 
  
  // Ready LED
  pinMode(READY_LED_PIN, OUTPUT);
  
  activateRFID();

  // Initialize the SoftwareSerial on pin 8 to communicate
  // with the RFID reader at 2400 bauds.
  pinSerial.begin(2400);
  
  // Allocate memory
  messageQueue = new QMessage[QUEUE_LENGTH];
  for(int i=0; i<QUEUE_LENGTH; i++)
    messageQueue[i] = NONE;
}

unsigned char modulo = 1;
unsigned long cycles = 0;

void loop()
{  
  cycles++;
  
  char key[11] = {0,0,0,0,0,0,0,0,0,0,0};
  if( readSerial(key) > 0)
  {
    // We have a new key ...
    processKey(key);

    Debug("RFID Key: ", key);
  }
  else
  {
    noDiskCount++;
    if(noDiskCount > MAX_NO_DISK_COUNT)
    {
      // There are no more key on the RFID reader
      Debug("No disk ...", noDiskCount);
      
      noDiskCount = 0;
      postMessage(EMPTY);
      
      checkForTargets();
    }
  }
  
  readQueue();
  setLEDs();
}

void setTags()
{
  redTags[0] = "70006D6078";
  redTags[1] = "70006D3C91";
  redTags[2] = "70006D3C43";
  redTags[3] = "02007F5281";
  
  blueTags[0] = "70006D5E08";
  blueTags[1] = "70006D389D";
  blueTags[2] = "70006D2C26";
  
  greenTags[0] = "70006E618D";
  greenTags[1] = "70006D4926";
  greenTags[2] = "70006D5A61";
  greenTags[3] = "0200B2D1CF";
  
  yellowTags[0] = "70006D5B40";
  yellowTags[1] = "70006D233E";
  yellowTags[2] = "70006D493B";
  
  magentaTags[0] = "70006D5874";
  magentaTags[1] = "70006D40A3";
  magentaTags[2] = "70006D5094";
  
  cyanTags[0] = "70006D4868";
  cyanTags[1] = "70006D486B";
  cyanTags[2] = "70006D58DA"; 
  cyanTags[3] = "020080FA93";
  
  rainbowTags[0] = "70006D3CEC";
  rainbowTags[1] = "70006F2106";
  rainbowTags[2] = "70006D3A9F";
  rainbowTags[3] = "0200802558";
  
  whiteTags[0] = "70006D6142";
  whiteTags[1] = "70006E2C78";
  whiteTags[2] = "70006D6243";
  
  purpleTags[0] = "70006D3B9E";
  purpleTags[1] = "70006D5AD9";
  purpleTags[2] = "70006D2C1B";
}

void readQueue()
{
  while(queueLength > 0)
  {
    QMessage msg = getMessage();
    Debug("Message: ", msg);
    
    if(msg == RANDOM)
    {
      if(lastMessage == EMPTY)
      {
        modulo = SLOW_FADE;
        targetColor[0] = random(255);
        targetColor[1] = random(255);
        targetColor[2] = random(255);
      }
 
      continue;
    }
    
    if(msg == lastMessage)
      continue;
      
    // We have a new command ...
    modulo = FAST_FADE;
      
    // Turn ON the LED if there is no disk in the lamp
    setReadyLED(msg);
        
    switch(msg)
    {
      case RED:
        targetColor[0] = 255;
        targetColor[1] = 0;
        targetColor[2] = 0;
      break;
      
      case GREEN:
        targetColor[0] = 0;
        targetColor[1] = 255;
        targetColor[2] = 0;
      break;
      
      case BLUE:
        targetColor[0] = 0;
        targetColor[1] = 0;
        targetColor[2] = 255;
      break;
      
      case YELLOW:
        targetColor[0] = 255;
        targetColor[1] = 200;
        targetColor[2] = 0;
      break;
      
      case CYAN:
        targetColor[0] = 0;
        targetColor[1] = 255;
        targetColor[2] = 255;
      break;
      
      case MAGENTA:
        targetColor[0] = 255;
        targetColor[1] = 0;
        targetColor[2] = 155;
      break;
      
      case WHITE:
        targetColor[0] = 255;
        targetColor[1] = 255;
        targetColor[2] = 255;
      break;
      
      case PURPLE:
        targetColor[0] = 135;
        targetColor[1] = 0;
        targetColor[2] = 140;
      break;
      
      case EMPTY:
        modulo = SLOW_FADE;
        targetColor[0] = random(255);
        targetColor[1] = random(255);
        targetColor[2] = random(255);
      break;
      
      case RAINBOW:
        displayRainbow();
       break;
    }
    
    lastMessage = msg;
  }
}

unsigned long alternate = 0;
void setLEDs()
{  
  if(cycles % modulo != 0)
    return;
    
  if(modulo == FAST_FADE)
  {
    for(int i=0; i<3; i++)
    {
      if(currentColor[i] < targetColor[i])
        currentColor[i] += 1;
      else if(currentColor[i] > targetColor[i])
        currentColor[i] -= 1;
        
      analogWrite(colorPins[i], currentColor[i]);
    }
  }
  else
  {
    int i = alternate++ % 3;
    if(currentColor[i] < targetColor[i])
      currentColor[i] += 1;
    else if(currentColor[i] > targetColor[i])
      currentColor[i] -= 1;
      
    analogWrite(colorPins[i], currentColor[i]);
  }
}


void Debug(char* prefix, char* msg)
{
  if(DEBUG)
  {
    Serial.print(prefix);
    Serial.println(msg);
  }
}

void Debug(char* prefix, int val)
{
  if(DEBUG)
  {
    Serial.print(prefix);
    Serial.println(val, DEC);
  }
}


void activateRFID()
{
  digitalWrite(RFID_CONTROL_PIN, LOW); 
  delay(ACTIVATE_RFID_WAIT);
}

void deactivateRFID()
{
  digitalWrite(RFID_CONTROL_PIN, HIGH); 
  pinSerial.flush();
}

int readSerial(char* key)
{
  activateRFID();
  
  // Check if there is data coming from the RFID reader
  if(pinSerial.available()) 
  {
    // There is some data to read.
    // Resetting all values ...
    int inputVal = 0;     
    int byteCount = 0;
    char rfidKey[KEY_LENGTH]; 
   
   // Check for the opcode
    if((inputVal = pinSerial.read()) == SERIAL_OPCODE) 
    {
      // This is the beginning of a new code.
      // Reading all 10 bytes
      byteCount = 0; 
      while(byteCount < KEY_LENGTH) 
      {
        if( pinSerial.available()) 
        { 
          inputVal = pinSerial.read(); 
          if((inputVal == SERIAL_OPCODE) || (inputVal == SERIAL_ENDCODE)) 
          {
            // Break when receiving a control character
            break;
          } 
          
          // This is a valid character.
          // Add the value to the key ...
          rfidKey[byteCount] = inputVal;
          byteCount++;          
        } 
      } 
      
      if(byteCount == KEY_LENGTH) 
      {        
        // We have a complete key.
        strncpy(key, rfidKey, KEY_LENGTH);
        deactivateRFID();
        
        return byteCount;
      } 
    } 
  } 
  
  return 0;
}

void processKey(char* key)
{
  noDiskCount = 0;
  
  for(int i = 0; i < NUM_TAGS; i++)
  {
    if(strncmp(key, magentaTags[i], 10) == 0)
    {
      postMessage(MAGENTA);
    }
    else if(strncmp(key, cyanTags[i], 10) == 0)
    {
      postMessage(CYAN);
    }
    else if(strncmp(key, redTags[i], 10) == 0)
    {
      postMessage(RED);
    }
    else if(strncmp(key, blueTags[i], 10) == 0)
    {
      postMessage(BLUE);
    }
    else if(strncmp(key, rainbowTags[i], 10) == 0)
    {
      postMessage(RAINBOW);
    }
    else if(strncmp(key, greenTags[i], 10) == 0)
    {
      postMessage(GREEN);
    }
    else if(strncmp(key, yellowTags[i], 10) == 0)
    {
      postMessage(YELLOW);
    }
    else if(strncmp(key, whiteTags[i], 10) == 0)
    {
      postMessage(WHITE);
    }
    else if(strncmp(key, purpleTags[i], 10) == 0)
    {
      postMessage(PURPLE);
    }
  }
}

void postMessage(QMessage msg)
{
  if(queueLength >= QUEUE_LENGTH)
  {
    // Flush the oldest event and shift all messages.
    Debug("Queue full ...", queueLength);
    
    shiftMessages();
  }
  
  messageQueue[queueLength++] = msg;
}

QMessage getMessage()
{
  if(queueLength > 0)
  {
    QMessage msg = messageQueue[0];
    shiftMessages();
    return msg;
  }
  
  return NONE;
}

void shiftMessages()
{
  if(queueLength == 0)
    return;
  
  for(int i=0; i<queueLength-1; i++)
    messageQueue[i] = messageQueue[i+1];
    
  queueLength -= 1;
}

void setReadyLED(QMessage msg)
{
  if(msg == EMPTY)
    digitalWrite(READY_LED_PIN, HIGH);
  else
    digitalWrite(READY_LED_PIN, LOW);   
}




void checkForTargets()
{
  boolean allTheSame = true;
  
  for(int i=0; i<3; i++)
  {
    if(currentColor[i] != targetColor[i])
    {
      allTheSame = false;
      break;
    }
  }
  
  if(allTheSame)
    postMessage(RANDOM);
}


void displayRainbow()
{
  int r, g, b;
  
  analogWrite(RED_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  delay(SLOW_FADE);

  // Red
  for(int i = 0; i < 256; i++)
  {
     analogWrite(RED_PIN, i);
     delay(SLOW_FADE);
  }
  
  // Orange/Yellow
  for(int i = 0; i < 166; i++)
  {
    analogWrite(GREEN_PIN, i);
    delay(SLOW_FADE);
  }
  
  // Green
  for(int i = 255; i > 0; i--)
  {
     analogWrite(RED_PIN, i);
     delay(SLOW_FADE);
  }
  
  // Cyan/Blue
  for(int i = 0; i < 256; i++)
  {
     analogWrite(BLUE_PIN, i);
     delay(SLOW_FADE);
  }
  for(int i = 255; i > 0; i--)
  {
     analogWrite(GREEN_PIN, i);
     delay(SLOW_FADE);  
  }
  
  // Indigo
  for(int i = 0; i < 76; i++)
  {
    analogWrite(RED_PIN, 75);
    delay(SLOW_FADE);
  }
  for(int i = 255; i > 129; i--)
  {
    analogWrite(BLUE_PIN, i);
    delay(SLOW_FADE);
  }
  
  // Violet
  for(int i = 75; i < 239; i++)
  {
    analogWrite(RED_PIN, i);
    delay(SLOW_FADE);
  }
  for(int i = 130; i < 239; i++)
  {
    analogWrite(BLUE_PIN, i);
    delay(SLOW_FADE);
  }
  for(int i = 0; i < 130; i++)
  {
    analogWrite(GREEN_PIN, i);
    delay(SLOW_FADE);
  }
   
  // White
  for(int i = 238; i < 256; i++)
  {
    analogWrite(RED_PIN, i);
    delay(SLOW_FADE);
  }
  for(int i = 130; i < 256; i++)
  {
    analogWrite(GREEN_PIN, i);
    delay(SLOW_FADE);
  }
  for(int i = 238; i < 256; i++)
  {
     analogWrite(BLUE_PIN, i);
     delay(SLOW_FADE);
  }
  
  // Black
  for(int i = 0; i < 120; i++)
  {
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
    delay(SLOW_FADE);
  }

}
