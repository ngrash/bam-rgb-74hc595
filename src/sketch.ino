/*
   TODO
   - use 8-bit timer
   - use volatile variables
   - double buffer
*/
#define SI_PIN 12
#define SCK_PIN 13
#define RCK_PIN 11
#define NUM_LEDS 24

#define BAM_MAX_VALUE 15
#define WHEEL_MAX_BRIGHTNESS 100

#define BAM_POSITIONS 4
#define OUT_BYTES 3
#define TIMER_COUNT 60000

long _lastBrightnessChange;
byte _brightness[NUM_LEDS];

volatile byte _bamCounter;
volatile byte _bamBytes[BAM_POSITIONS][OUT_BYTES];

byte _wheelPosition[8];

#define WHEEL_COLORS 8
byte _colors[][3] = {
  {255,   0,   0},
  {255, 127,   0},
  {255, 255,   0},
  {127, 255,   0},
  {  0, 255,   0},
  {  0, 255, 127},
  {  0, 255, 255},
  {  0, 127, 255},
  {  0,   0, 255},
  {127,   0, 255},
  {255,   0, 255},
  {255,   0, 127}
};

void setup()
{
  pinMode(SI_PIN, OUTPUT);
  pinMode(SCK_PIN, OUTPUT);
  pinMode(RCK_PIN, OUTPUT);
  digitalWrite(RCK_PIN, LOW);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = TIMER_COUNT;
  TCCR1B |= (1 << CS10);
  TIMSK1 |= (1 << TOIE1);
  interrupts();

  for(int i = 0; i < 8; i++) {
    _wheelPosition[i] = i;
  }
}

ISR(TIMER1_OVF_vect)
{
  TCNT1 = TIMER_COUNT;
  bam();
}

void loop()
{
  long currentMillis = millis();
  if(currentMillis - _lastBrightnessChange >= 50) {
    _lastBrightnessChange = currentMillis;

    for(int i = 0; i < 8; i++) {
      _wheelPosition[i]++;
      if(_wheelPosition[i] > WHEEL_COLORS) _wheelPosition[i] = 0;

      byte pos = i*3;
      byte *color = _colors[_wheelPosition[i]];
      _brightness[pos+0] = color[0];
      _brightness[pos+1] = color[1];
      _brightness[pos+2] = color[2];
    }

    bamCalc(_brightness, NUM_LEDS);
  }
}

void bam()
{
  _bamCounter++;
  if(_bamCounter > BAM_MAX_VALUE) _bamCounter = B00000001;

  if(_bamCounter == B00000001) bamWrite(0);
  if(_bamCounter == B00000010) bamWrite(1);
  if(_bamCounter == B00000100) bamWrite(2);
  if(_bamCounter == B00001000) bamWrite(3);
  if(_bamCounter == B00010000) bamWrite(4);
  if(_bamCounter == B00100000) bamWrite(5);
  if(_bamCounter == B01000000) bamWrite(6);
  if(_bamCounter == B10000000) bamWrite(7);
}

void bamCalc(byte leds[], byte ledsSize) {
  for(int pos = 0; pos < BAM_POSITIONS; pos++) {
    bamBytes(pos, leds, ledsSize, _bamBytes[pos]);
  }
}

void bamBytes(byte pos, byte leds[], byte ledsSize, volatile byte *buffer) {
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
    shiftOut(SI_PIN, SCK_PIN, LSBFIRST, _bamBytes[pos][i]);
  }
  digitalWrite(RCK_PIN, HIGH);
  digitalWrite(RCK_PIN, LOW);
}

