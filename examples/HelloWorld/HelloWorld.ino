#include <Wire.h> 
#include <T123.h>

T123 lcd(0x3A,12,4);  // set the LCD address to 3A for a 12 chars and 4 (actually 3) line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.clear();
 
  // Print a message to the LCD.
  lcd.setCursor(0,0); // col 0, row 0
  lcd.print("Hello, world!");
}

void loop()
{
}
