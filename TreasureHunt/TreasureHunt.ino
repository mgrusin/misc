#include <TinyGPS++.h>
#include <SoftwareSerial.h>

TinyGPSPlus gps;
SoftwareSerial lcd_ss(2,3); // RX, TX

double TARGET_LAT = 40.065156, TARGET_LON = -105.210142;

void setup()
{
  lcd_ss.begin(9600);
  Serial.begin(4800);
  
  LCDclear();
  LCDline1();
  lcd_ss.print("Waiting for GPS");
}

void loop()
{
  double range, abscourse;
  int relcourse;
  char data;
  
  while (Serial.available())
  {
    data = Serial.read();
    gps.encode(data);
  }

  if (gps.location.isUpdated())
  {
    range = TinyGPSPlus::distanceBetween(gps.location.lat(),gps.location.lng(),TARGET_LAT,TARGET_LON);

    abscourse = TinyGPSPlus::courseTo(gps.location.lat(),gps.location.lng(),TARGET_LAT,TARGET_LON);

    relcourse = (int)(gps.course.deg() - abscourse);
    if (relcourse < 0) relcourse += 360;
    if (relcourse > 180) relcourse -= 360;
    
    LCDclear();
    LCDline1();
    lcd_ss.print("sat ");
    lcd_ss.print(gps.satellites.value());
    lcd_ss.print(" rng ");
    lcd_ss.print(range,0);
    lcd_ss.print("m");
    LCDline2();
    lcd_ss.print(abscourse,0);
    lcd_ss.print("d ");
    lcd_ss.print(TinyGPSPlus::cardinal(abscourse));
    lcd_ss.print("  ");
    lcd_ss.print(abs(relcourse));
    lcd_ss.print("d ");
    if (abs(relcourse) < 20)
      lcd_ss.print("^");
    else
      if (abs(relcourse) > 160)
        lcd_ss.print("v");
      else
        if (relcourse < 0)
          lcd_ss.print(">");
        else
          lcd_ss.print("<");
  }
}

void LCDclear()
// clear the screen of an SparkFun serial-enabled LCD
{
  LCDline1();
  lcd_ss.print(F("                "));
  LCDline2();
  lcd_ss.print(F("                "));
}

void LCDline1()
// move cursor to start of line 1 on a SparkFun serial-enabled LCD
{
  lcd_ss.write(254);
  lcd_ss.write(128);
}

void LCDline2()
// move cursor to start of line 2 on a SparkFun serial-enabled LCD
{
  lcd_ss.write(254);
  lcd_ss.write(192);
}

