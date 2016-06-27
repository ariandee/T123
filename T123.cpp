// T123 v1.0 - Arjan D. - 15 March 2015
// LiquidCrystal_I2C V2.0 - Mario H. atmega@xs4all.nl
// Mods for Chinese I2C converter board - Murray R. Van Luyn. vanluynm@iinet.net.au

#include "T123.h"
#include <inttypes.h>
#include "Wire.h"
#include "Arduino.h"


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data (cannot be changed over I2C)
//    M,N = 1; 4x12 display 
//    G = 1; Voltage generator Vlcd = V0 - 0.8Vdd
// 3. Display on/off control: 
//    D = 1; Display on
//    C = 1; Cursor on
//    B = 1; Blinking on
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// T123 constructor is called).

T123::T123(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
  _Addr = lcd_Addr;
  _cols = lcd_cols;
  _rows = lcd_rows;
  //_backlightval = LCD_NOBACKLIGHT;
}

void T123::init(){
	init_priv();
}

void T123::init_priv()
{
	Wire.begin();
	_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_GENERATOR;
	begin(_cols, _rows);  
}

void T123::begin(uint8_t cols, uint8_t lines) {
	if (lines == 4) {
		_displayfunction |= LCD_4LINE;
	}
	if (lines == 2) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	// wait some time before configuring the display
	delayMicroseconds(50000); 
  
	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  
	
	// turn the display on with no default cursor and no default blinking
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	command(LCD_DISPLAYCONTROL | _displaycontrol);  
	
	// Initialize default text direction left to right (increment of cursor position), freeze display
	_displaymode = LCD_ENTRYINCREMENT | LCD_ENTRYDISPLAYFREEZE;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

	// clear the display
	clear();
	
}



/********** high level commands, for the user! */

// clear() - clear display, set cursor position to zero
void T123::clear(){
	command(LCD_CLEARDISPLAY);
	delayMicroseconds(2000);   // this command takes a long time!
}

// home() - set cursor position to zero
void T123::home(){
	command(LCD_RETURNHOME);
}

// setCursor(col, row) - set cursor on specified col, row
void T123::setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x20, 0x40, 0x60 };
	if ( row > _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// noDisplay() - turn the display off
void T123::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// display() - turn the display on
void T123::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// noCursor() - turn the cursor off
void T123::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// cursor() - turn the cursor on
void T123::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// noBlink() - turn cursor blinking off
void T123::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// blink() - turn cursor blinking on
void T123::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// scrollDisplayLeft() - scroll display to the left without changing the RAM
void T123::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

// scrollDisplayRight() - scroll display to the left without changing the RAM
void T123::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// leftToRight() - set text flow left to right
void T123::leftToRight(void) {
	_displaymode |= LCD_ENTRYINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// rightToLeft() - set text flow right to left
void T123::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// autoscroll() - autoscroll display
void T123::autoscroll(void) {
	_displaymode |= LCD_ENTRYDISPLAYSHIFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// noAutoscroll() - freeze display
void T123::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYDISPLAYSHIFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// allows us to fill the first 8 CGRAM locations with custom characters
void T123::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}







/*********** mid level commands, for sending data/cmds */

// send command
inline void T123::command(uint8_t value) {
	send(value, Co);
}

// send character
inline size_t T123::write(uint8_t value) {
	send(value+0x80, Ch);		// strange offset on EA T123A-I2C display (ascii+0x80)
	return 0;
}






/************ low level data pushing commands **********/

// write either command or data
void T123::send(uint8_t value, uint8_t mode) {
	Wire.beginTransmission(_Addr);
	Wire.write(mode);
	Wire.write(value);
	Wire.endTransmission();   
}



// Alias functions

void T123::cursor_on(){
	cursor();
}

void T123::cursor_off(){
	noCursor();
}

void T123::blink_on(){
	blink();
}

void T123::blink_off(){
	noBlink();
}

void T123::load_custom_character(uint8_t char_num, uint8_t *rows){
	createChar(char_num, rows);
}

void T123::printstr(const char c[]){
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	print(c);
}



// unsupported API functions
void T123::off(){}
void T123::on(){}
void T123::setDelay (int cmdDelay,int charDelay) {}
uint8_t T123::status(){return 0;}
uint8_t T123::keypad (){return 0;}
uint8_t T123::init_bargraph(uint8_t graphtype){return 0;}
void T123::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){}
void T123::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){}
void T123::setContrast(uint8_t new_val){}
void T123::setBacklight(uint8_t new_val){}
void T123::noBacklight(){}
void T123::backlight(){}
