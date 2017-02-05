#define SI_PIN 12
#define SCK_PIN 13
#define RCK_PIN 11
#define NUM_LEDS 24

#define BAM_MAX_VALUE 15
#define WHEEL_MAX_BRIGHTNESS 100

#define BAM_POSITIONS 4
#define OUT_BYTES 3

/*
   TODO
   - use timer interrupt
*/

long _lastBrightnessChange;
byte _brightness[NUM_LEDS];

void setLed(byte index, byte r, byte g, byte b) {
  int ledPos = index * 3;
  _brightness[ledPos + 0] = r;
  _brightness[ledPos + 1] = g;
  _brightness[ledPos + 2] = b;
}

void setup()
{
  pinMode(SI_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(RCK_PIN, OUTPUT);
  digitalWrite(RCK_PIN, LOW);
}

void loop()
{
  long currentMillis = millis();
  if(currentMillis - _lastBrightnessChange >= 1000) {
    _lastBrightnessChange = currentMillis;

    _brightness[2]++;
    if(_brightness[2] > BAM_MAX_VALUE) _brightness[2] = 0;

    bamCalc(_brightness, NUM_LEDS);
  }

  bam();
}

void wheel(byte *r, byte *g, byte *b) {
  if(*r == 0 && *g == 0 && *b == 0) *r = WHEEL_MAX_BRIGHTNESS;

  if(*r == WHEEL_MAX_BRIGHTNESS && *b == 0) {
    if(*g == WHEEL_MAX_BRIGHTNESS) --*r;
    else ++*g;
  }
  else if(*g == WHEEL_MAX_BRIGHTNESS && *b == 0) {
    if(*r == 0) ++*b;
    else --*r;
  }
  else if(*g == WHEEL_MAX_BRIGHTNESS && *r == 0) {
    if(*b == WHEEL_MAX_BRIGHTNESS) --*g;
    else ++*b;
  }
  else if(*b == WHEEL_MAX_BRIGHTNESS && *r == 0) {
    if(*g == 0) ++*r;
    else --*g;
  }
  else if(*b == WHEEL_MAX_BRIGHTNESS && *g == 0) {
    if(*r == WHEEL_MAX_BRIGHTNESS) --*b;
    else ++*r;
  }
  else if(*r == WHEEL_MAX_BRIGHTNESS && *g == 0) {
    if(*b == 0) ++*g;
    else --*b;
  }
}

byte _bamCounter;
unsigned long _lastBamWrite;

void bam()
{
  unsigned long currentMicros = micros();
  if(currentMicros - _lastBamWrite >= 50) {
    _lastBamWrite = currentMicros;

    _bamCounter++;
    if(_bamCounter > BAM_MAX_VALUE) _bamCounter = B00000001;

    if(_bamCounter == B00000001) bamWrite(1);
    if(_bamCounter == B00000010) bamWrite(2);
    if(_bamCounter == B00000100) bamWrite(3);
    if(_bamCounter == B00001000) bamWrite(4);
    if(_bamCounter == B00010000) bamWrite(5);
    if(_bamCounter == B00100000) bamWrite(6);
    if(_bamCounter == B01000000) bamWrite(7);
    if(_bamCounter == B10000000) bamWrite(8);
  }
}

byte _bamBytes[BAM_POSITIONS][OUT_BYTES];

void bamCalc(byte leds[], byte ledsSize) {
  for(int pos = 1; pos <= BAM_POSITIONS; pos++) {
    bamBytes(pos, leds, ledsSize, _bamBytes[pos-1]);
  }
}

void bamBytes(byte pos, byte leds[], byte ledsSize, byte *buffer) {
  byte out = 0;
  byte outIndex = 0;
  byte bufferIndex = 0;

  for(int ledIndex = 0; ledIndex < ledsSize; ledIndex++) {
    bool isEnabled = bitRead(leds[ledIndex], pos);
    if(isEnabled) {
      byte mask = (1 << (7 - outIndex));
      out = out | mask;
    }

    outIndex++;
    if(outIndex > 7) {
      outIndex = 0;
      buffer[bufferIndex] = out;
      bufferIndex++;
      out = 0;
    }
  }
}

void bamWrite(int pos) {
  for(int i = 0; i < OUT_BYTES; i++) {
    shiftOut(SI_PIN, SCK_PIN, LSBFIRST, _bamBytes[pos-1][i]);
  }
  digitalWrite(RCK_PIN, HIGH);
  digitalWrite(RCK_PIN, LOW);
}

