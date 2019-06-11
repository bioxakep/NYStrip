#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

#define NUM_LEDS 220
#define IRRXPIN 11
#define DATA_PIN 7

uint32_t memColor;
uint32_t currColor;
uint32_t newColor;
uint32_t offColor;
byte r, g, b;
int brightness = 100;
int mode = 0;
int memMode = 0;
int freqHeart = 100;//Ударов в минуту//
IRrecv irRX(IRRXPIN);
Adafruit_NeoPixel NYStrip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);
decode_results results;

void setup() 
{
  Serial.begin(9600);
  NYStrip.begin();
  irRX.enableIRIn();
  r = brightness;
  g = brightness;
  b = brightness;
  offColor = NYStrip.Color(0, 0, 0);
}

void dump(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  if(results->value == 0xF740BF) { off(); memMode = mode; mode = 0; } //Off
  if(results->value == 0xF7C03F) { newColor = memColor; mode = memMode; } //On
  if(results->value == 0xF720DF) { r = brightness; g = 0; b = 0; newColor = NYStrip.Color(r, g, b); mode = 0; } //Red
  if(results->value == 0xF7A05F) { r = 0; g = brightness; b = 0; newColor = NYStrip.Color(r, g, b); mode = 0; } //Green
  if(results->value == 0xF7609F) { r = 0; g = 0; b = brightness; newColor = NYStrip.Color(r, g, b); mode = 0; } //Blue
  if(results->value == 0xF7E01F) { r = brightness; g = brightness; b = brightness; newColor = NYStrip.Color(r, g, b); mode = 0; } //White
  if(results->value == 0xF700FF) { brightness += 10; NYStrip.setBrightness(brightness); currColor = NYStrip.getPixelColor(1); } //Bri+
  if(results->value == 0xF7807F) { brightness -= 10; NYStrip.setBrightness(brightness); currColor = NYStrip.getPixelColor(1); } //Bri-
}

void breathe(int _delay, int _minAmp, int _maxAmp, int _ampOffset, int _pixCnt)
{
  int x[_pixCnt];
  int Amplitude = _minAmp;
  int ampOffset = _ampOffset;
  if(_pixCnt % 2 == 1)
  {
    x[_pixCnt/2] = 0;
    for(int i = 0; i < _pixCnt/2; i++)
    {
      x[_pixCnt/2 + 1 + i] = i + 1;
      x[_pixCnt/2 - 1 -i] = i + 1;
    }
  }
  else
  {                                                                 // 0-0-0-0-0-0-0-0-0-0//
    for(int i = 0; i < _pixCnt/2; i++)                              // 0-1-2-3-4-5-6-7-8-9//
    {                                                               // 5-4-3-2-1-1-2-3-4-5//  
      x[_pixCnt/2 - 1 - i] = i + 1;
      x[_pixCnt/2 + i] = i + 1;
    }
  }

  while (Amplitude >= _minAmp && Amplitude >= 0)
  {
    delay(_delay);
    for (int i = 0; i < NUM_LEDS; i++)
    {
      int brightness = Amplitude - (Amplitude * (pow(x[i], 2))) / (pow(_pixCnt/2,2));
      NYStrip.setPixelColor(i, currColor);
    }
    NYStrip.show();
    if (Amplitude >= _maxAmp) ampOffset = -ampOffset;
    Amplitude += ampOffset;
    if(Amplitude > 255) Amplitude = 255;
  }
}

void HeartBeats(int _freq)
{
  breathe(30, 20, 180, 40, NUM_LEDS);
  breathe(20, 10, 60, 5, NUM_LEDS);
  delay(60000/_freq);
}

void off()
{
  for(int i = 0; i < NUM_LEDS; i++) NYStrip.setPixelColor(i, offColor);
}

void fadeIn(byte _r, byte _g, byte _b)
{
  for(int k=0; k<20;k++)
  {
    for(int p = 0; p < NUM_LEDS; p++) NYStrip.setPixelColor(p, NYStrip.Color(_r*k/20, _g*k/20, _b*k/20));
    NYStrip.show();
  }
}

void loop() { 
  if (irRX.decode(&results)) 
  {
    Serial.println(results.value, HEX);
    dump(&results);
    irRX.resume(); // Receive the next value
  }
  if(mode == 0)
  {
    if(currColor != newColor)
    {
      for(int i = 0; i < NUM_LEDS; i++) NYStrip.setPixelColor(i, newColor);
      NYStrip.show();
      currColor = newColor;
      memColor = currColor;
    }
  }
  else if(mode == 1)
  {
    HeartBeats(freqHeart);
  }
}
