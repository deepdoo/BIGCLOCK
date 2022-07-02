// BIG LED CLOCK
// https://github.com/deepdoo/BIGCLOCK
// Virtual COM Port Drivers 
// https://ftdichip.com/drivers/vcp-drivers/

#include <DS3232RTC.h>
// https://github.com/PaulStoffregen/Time
// https://github.com/JChristensen/DS3232RTC
#include <IRremote.h>
// https://github.com/Arduino-IRremote/Arduino-IRremote
#define DATAPIN   (2)   // 74HC595のDSへ
#define LATCHPIN  (4)  // 74HC595のST_CPへ
#define CLOCKPIN  (3)  // 74HC595のSH_CPへ

byte count = 0;
const int RECV_PIN = 12;
IRrecv irrecv(RECV_PIN);
decode_results results;
boolean mode = true; //true normal, false setup mode
boolean showdot = true;
boolean showDot = true;
int now_t = 0;

byte a = 2 - 1;
byte b = 8 - 1;
byte c = 1 - 1;
byte d = 5 - 1;
byte e = 7 - 1;
byte f = 4 - 1;
byte g = 6 - 1;
byte dot = 3 - 1;

long zero = 1 << a | 1 << b | 1 << c | 1 << d | 1 << e | 1 << f;
long one = 1 << b | 1 << c;
long two = 1 << a | 1 << b | 1 << d | 1 << e | 1 << g;
long thre = 1 << a | 1 << b | 1 << c | 1 << d | 1 << g;
long four = 1 << b | 1 << c | 1 << f | 1 << g;
long five = 1 << a | 1 << c | 1 << d | 1 << f | 1 << g;
long six = 1 << a | 1 << c | 1 << d | 1 << e | 1 << f | 1 << g;
long seven = 1 << a | 1 << b | 1 << c;
long eight = 1 << a | 1 << b | 1 << c | 1 << d | 1 << e | 1 << f | 1 << g;
long nine = 1 << a | 1 << b | 1 << c | 1 << d | 1 << f | 1 << g;

long patterns[] = {
  zero, one, two, thre, four, five, six, seven, eight, nine
};

void show(unsigned long val )
{
  digitalWrite(LATCHPIN, LOW);
  for ( int i = 0; i < 32; i++ )
  {
    digitalWrite(DATAPIN, !!(val & (1L << i)));

    digitalWrite(CLOCKPIN, HIGH);
    digitalWrite(CLOCKPIN, LOW);
  }
  digitalWrite(LATCHPIN, HIGH);
}

void setup()
{

  pinMode(DATAPIN, OUTPUT);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);

  show(patterns[8] | 1 << dot | (patterns[8] | 1 << dot) << 8 | (patterns[8] | 1 << dot) << 16 | (patterns[8] | 1 << dot) << 24);

  delay(1000);
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true);

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
}

void loop()
{

  if (irrecv.decode(&results)) {

    Serial.println(results.value, HEX);
    switch (results.value) {
      case 0x5AF1484A: case 0x73508B9E:
        Serial.println("mode");
        mode = !mode;
        break;
      case 0x5AF148A8: case 0x29728F39:
        Serial.println("hour minus");
        if (!mode) {
          setTime((hour() + 23) % 24, minute(), second(), day(), month(), year()); //時、分、秒、日、月、年の順で入力
          RTC.set(now());
        }
        break;
      case 0x5AF14828: case 0xDA882442:
        Serial.println("hour add");
        if (!mode) {
          setTime((hour() + 1) % 24, minute(), second(), day(), month(), year()); //時、分、秒、日、月、年の順で入力
          RTC.set(now());
        }
        break;
      case 0x5AF14888: case 0x452C5CE0:
        Serial.println("minute add");
        if (!mode) {
          setTime(hour(), (minute() + 1) % 60, second(), day(), month(), year()); //時、分、秒、日、月、年の順で入力
          RTC.set(now());
        }
        break;
      case 0x5AF14848: case 0xEAA22D24:
        Serial.println("minute minus");
        if (!mode) {
          setTime(hour(), (minute() + 59) % 60, second(), day(), month(), year()); //時、分、秒、日、月、年の順で入力
          RTC.set(now());
        }
        break ;
      case 0x5AF148C8: case 0xB02818A8:
        showdot = !showdot;
        break ;
    }
    irrecv.resume();
  }
  if (hour() + minute()  != now_t) {
    digitalClockDisplay();
    now_t = hour() + minute();
  }
  int h1 = hour() / 10;
  int h2 = hour() % 10;
  int m1 = minute() / 10;
  int m2 = minute() % 10;
  long ph1 = patterns[h1];
  if (h1 == 0)
    ph1 = 0;

  if (mode) {
    if (showdot && showDot)
      show(ph1 | (patterns[h2] | 1 << dot) << 8 | patterns[m1] << 16 | patterns[m2] << 24);
    else
      show(ph1 | patterns[h2] << 8 | patterns[m1] << 16 | patterns[m2] << 24);
    if (count % 2 == 0)
      showDot = !showDot;
  } else {
    show(ph1 | 1 << dot | (patterns[h2] | 1 << dot) << 8 | (patterns[m1] | 1 << dot) << 16 | (patterns[m2] | 1 << dot) << 24);
  }
  count++;
  delay(400);
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(' ');
  Serial.print(day());
  Serial.print(' ');
  Serial.print(month());
  Serial.print(' ');
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(':');
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
