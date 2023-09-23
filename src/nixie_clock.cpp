#include <Arduino.h>
#include <ezTime.h>
#include <ESP8266WiFi.h>

#define MY_SSID "cjweiland"
#define MY_WIFI_PASSWORD "areallygoodkey"

#define SHIFT_CLOCK_PIN D5
#define LATCH_CLOCK_PIN D6
#define SERIAL_DATA_PIN D7

#define PWM_PIN D1
#define BLINK_PIN D2

Timezone myTZ;
int lastMinute = 0;

#define IN18
//#define IN14

//stuff to handle pin swapping
#ifdef IN14
unsigned char OutputToBCD[10] =
{
  8, //0
  2, //1
  3, //2
  7, //3
  6, //4
  4, //5
  5, //6
  1, //7
  0, //8
  9  //9
};

//1 -> 1 << 0
//2 -> 1 << 6
//4 -> 1 << 5
//8 -> 1 << 7
unsigned char BCDToSerial1[10] =
{
  (0 << 7) | (0 << 5) | (0 << 6) | (0 << 0),
  (0 << 7) | (0 << 5) | (0 << 6) | (1 << 0),
  (0 << 7) | (0 << 5) | (1 << 6) | (0 << 0),
  (0 << 7) | (0 << 5) | (1 << 6) | (1 << 0),
  (0 << 7) | (1 << 5) | (0 << 6) | (0 << 0),
  (0 << 7) | (1 << 5) | (0 << 6) | (1 << 0),
  (0 << 7) | (1 << 5) | (1 << 6) | (0 << 0),
  (0 << 7) | (1 << 5) | (1 << 6) | (1 << 0),
  (1 << 7) | (0 << 5) | (0 << 6) | (0 << 0),
  (1 << 7) | (0 << 5) | (0 << 6) | (1 << 0),
};

//1 -> 1 << 4
//2 -> 1 << 2
//4 -> 1 << 1
//8 -> 1 << 3
unsigned char BCDToSerial2[10] =
{
  0,
  (0 << 3) | (0 << 1) | (0 << 2) | (1 << 4),
  (0 << 3) | (0 << 1) | (1 << 2) | (0 << 4),
  (0 << 3) | (0 << 1) | (1 << 2) | (1 << 4),
  (0 << 3) | (1 << 1) | (0 << 2) | (0 << 4),
  (0 << 3) | (1 << 1) | (0 << 2) | (1 << 4),
  (0 << 3) | (1 << 1) | (1 << 2) | (0 << 4),
  (0 << 3) | (1 << 1) | (1 << 2) | (1 << 4),
  (1 << 3) | (0 << 1) | (0 << 2) | (0 << 4),
  (1 << 3) | (0 << 1) | (0 << 2) | (1 << 4)
};
#endif

#ifdef IN18
unsigned char OutputToBCD[10] =
{
  1, //0
  5, //1
  4, //2
  7, //3
  9, //4
  8, //5
  2, //6
  3, //7
  6, //8
  0  //9
};

//1 -> 1 << 3
//2 -> 1 << 1
//4 -> 1 << 0
//8 -> 1 << 2
unsigned char BCDToSerial1[10] =
{
  (0 << 2) | (0 << 0) | (0 << 1) | (0 << 3),
  (0 << 2) | (0 << 0) | (0 << 1) | (1 << 3),
  (0 << 2) | (0 << 0) | (1 << 1) | (0 << 3),
  (0 << 2) | (0 << 0) | (1 << 1) | (1 << 3),
  (0 << 2) | (1 << 0) | (0 << 1) | (0 << 3),
  (0 << 2) | (1 << 0) | (0 << 1) | (1 << 3),
  (0 << 2) | (1 << 0) | (1 << 1) | (0 << 3),
  (0 << 2) | (1 << 0) | (1 << 1) | (1 << 3),
  (1 << 2) | (0 << 0) | (0 << 1) | (0 << 3),
  (1 << 2) | (0 << 0) | (0 << 1) | (1 << 3),
};

//1 -> 1 << 7
//2 -> 1 << 5
//4 -> 1 << 4
//8 -> 1 << 6
unsigned char BCDToSerial2[10] =
{
  0,
  (0 << 6) | (0 << 4) | (0 << 5) | (1 << 7),
  (0 << 6) | (0 << 4) | (1 << 5) | (0 << 7),
  (0 << 6) | (0 << 4) | (1 << 5) | (1 << 7),
  (0 << 6) | (1 << 4) | (0 << 5) | (0 << 7),
  (0 << 6) | (1 << 4) | (0 << 5) | (1 << 7),
  (0 << 6) | (1 << 4) | (1 << 5) | (0 << 7),
  (0 << 6) | (1 << 4) | (1 << 5) | (1 << 7),
  (1 << 6) | (0 << 4) | (0 << 5) | (0 << 7),
  (1 << 6) | (0 << 4) | (0 << 5) | (1 << 7)
};
#endif



#ifdef IN18

#endif

unsigned char NumberToSerial(int number)
{
  int n1 = number % 10;
  int n2 = number / 10;
  unsigned char r1 = BCDToSerial1[OutputToBCD[n1]];
  unsigned char r2 = BCDToSerial2[OutputToBCD[n2]];
  //Serial.println("BCD1 for " + String(n1) + " = " + String(OutputToBCD[n1]));
  //Serial.println("Serial for " + String(OutputToBCD[n1]) + " = " + String(r1) + " or 0x" + String(r1, HEX) + " or 0b" + String(r1, BIN));
  //Serial.println("Serial for " + String(number) + " is 0b" + String(r1 | r2, BIN));
  return r1 | r2;
}

void OutputTime(int hours, int minutes)
{
  Serial.println("Time is " + String(hours) + ":" + String(minutes));
  unsigned char hourByte = NumberToSerial(hours);
  unsigned char minuteByte = NumberToSerial(minutes);

  digitalWrite(LATCH_CLOCK_PIN, LOW);
  shiftOut(SERIAL_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, hourByte);
  shiftOut(SERIAL_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, minuteByte);
  digitalWrite(LATCH_CLOCK_PIN, HIGH);

  lastMinute = minutes;
}

void ClearDisplay()
{
  digitalWrite(LATCH_CLOCK_PIN, LOW);
  shiftOut(SERIAL_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, 0xFF);
  shiftOut(SERIAL_DATA_PIN, SHIFT_CLOCK_PIN, MSBFIRST, 0xFF);
  digitalWrite(LATCH_CLOCK_PIN, HIGH);
}

void debugloop(int delayMS)
{
  for (int i = 0; i < 10; i++)
  {
    OutputTime(11 * i, 11 * i);
    delay(delayMS);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Started!");
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(LATCH_CLOCK_PIN, OUTPUT);
  pinMode(SERIAL_DATA_PIN, OUTPUT);
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(BLINK_PIN, OUTPUT);
  analogWriteFreq(100);
  digitalWrite(LED_BUILTIN, HIGH);
  analogWrite(PWM_PIN, 512);
  digitalWrite(BLINK_PIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(MY_SSID, MY_WIFI_PASSWORD);
  debugloop(1000);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.println("Waiting for wifi to connect...");
  }

  waitForSync();
  myTZ.setLocation("America/New_York");
  OutputTime(myTZ.hour(), myTZ.minute());
}

//bool on = true;
void loop()
{
  events(); //ezTime events()

  //uncomment for blinky colon
#ifdef IN18
  digitalWrite(BLINK_PIN, (myTZ.second() % 2) == 0);
#endif

  int hour = myTZ.hour();
  int minute = myTZ.minute();

  if (hour == 4 && minute == 10)
  {
    //roll through all of the digits so all of them get a chance to turn on
    //once in a while (preserves life of the tubes)
    debugloop(1000 * 10);
  }

  //between these hours, turn completely off since no one is likely around to read the clock!
  if (false)//hour > 1 && hour < 6)
  {
    ClearDisplay();
  }
  else
  {
    //otherwise, show the time!
    OutputTime(hour, minute);
  }
  delay(100);
}