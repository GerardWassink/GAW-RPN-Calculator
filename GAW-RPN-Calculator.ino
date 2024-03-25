/* ------------------------------------------------------------------------- *
 * Name   : GAW-RPN-Calculator
 * Author : Gerard Wassink
 * Date   : March 2024
 * Purpose: Generate keyboard commands
 * Versions:
 *   0.1  : Initial code base
 *   0.2  : Most functions built in
 *------------------------------------------------------------------------- */
#define progVersion "0.2"                     // Program version definition
/* ------------------------------------------------------------------------- *
 *             GNU LICENSE CONDITIONS
 * ------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ------------------------------------------------------------------------- *
 *       Copyright (C) March 2024 Gerard Wassink
 * ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- *
 *                                				Positioning of onscreen indicators
 * ------------------------------------------------------------------------- */
// item		    startpos	  endpos    values
// ----------	--------    ------    --------------------------------
// gonioState     0          2      rad, grd (spaces when Degree mode)
// shiftState     4          4      f, g


/* ------------------------------------------------------------------------- *
 *       Compiler directives to switch debugging on / off
 *       Do not enable debug when not needed, Serial takes space and time!
 * ------------------------------------------------------------------------- */
#define DEBUG 1

#if DEBUG == 1
  #define debugstart(x) Serial.begin(x)
  #define debug(x) Serial.print(x)
  #define debugln(x) Serial.println(x)
#else
  #define debugstart(x)
  #define debug(x)
  #define debugln(x)
#endif


/* ------------------------------------------------------------------------- *
 *                                              Definitions for the keyboard
 * ------------------------------------------------------------------------- */
#define ROWS 5                              // five rows for keyboard
#define COLS 8                              // eight columns for keyboard


/* ------------------------------------------------------------------------- *
 *                                                          Library includes
 * ------------------------------------------------------------------------- */
#include <Wire.h>                       // I2C comms library
#include <LiquidCrystal_I2C.h>          // LCD library

#include <Keypad.h>                     // Keypad library

#include <String.h>                     // String library

#include <math.h>                       // Arduino math library

#include <Keypad.h>                     // Keypad library

#include <EEPROM.h>                     // EEPROM library to save settings

/* ------------------------------------------------------------------------- *
 *                                                            Create objects
 * ------------------------------------------------------------------------- */
LiquidCrystal_I2C display(0x25,20,4);   // Instantiate display object


/* ------------------------------------------------------------------------- *
 *                                                      Calculator variables
 * ------------------------------------------------------------------------- */
int precision = 4;                      // default precision = 4

const unsigned int statDEG=0;           // goniometric state
const unsigned int statRAD=1;
const unsigned int statGRD=2;
unsigned int gonioStatus = statDEG;     // default to degrees

const unsigned int noShift=0;
const unsigned int shiftF=1;
const unsigned int shiftG=2;
unsigned int stateShift=noShift;        // shift default to 0 (off)

double E = 2.718281828459045;           // constant for the natural number

int statRegLo = 2;                      // Lo register for statistics
int statRegHi = 7;                      // Hi register for statistics

/* ------------------------------------------------------------------------- *
 *                                             Stack variables X, Y, Z and T
 * ------------------------------------------------------------------------- */
double stack[4] = {0, 0, 0, 0};
int X=0, Y=1, Z=2, T=3;

/* ------------------------------------------------------------------------- *
 *                                                Storage register variables
 * Registers in array (30 in total)
 *   0-9,      0 -  9
 *   .0-.9    10 - 19
 *   A-E,     20 - 24
 *   .A-.E,   25 - 29
 * ------------------------------------------------------------------------- */
double Reg[30];

/* ------------------------------------------------------------------------- *
 *                                                   Define keypad variables
 *  Conforming to a HP-15C calculator
 *  Key values indicate row and column number
 * ------------------------------------------------------------------------- */
  char keys[ROWS][COLS] = {
  //   7     8     9    10    11    12    13    14     <== pins
  //   1     2     3     4     5     6     7     8     <== columns
    {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18}, // row 1, pin 2
    {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28}, // row 2, pin 3
    {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38}, // row 3, pin 4
    {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48}, // row 4, pin 5
    {0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58}, // row 5, pin 6
  };
  byte rowPins[ROWS] = {2,3,4,5,6};               // row pins of the keypad
  byte colPins[COLS] = {7,8,9,10,11,12,13,17};    // column pins of the keypad
  
  char key = 0x00;                                // global keypres

/* ------------------------------------------------------------------------- *
 *       Create object for Keypad
 * ------------------------------------------------------------------------- */
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);


/* ------------------------------------------------------------------------- *
 *                                                                   setup()
 * ------------------------------------------------------------------------- */
void setup() {
  debugstart(115200);

  display.init();                       // Initialize display
  display.backlight();                  // Backlights on by default
  
  //                                                            Intro Screen
  // Assemble and show intro text and versionb
  //
  LCD_display(display, 0, 0, "GAW RPN calculator  ");
  String myString = "version ";
  myString.concat(progVersion);
  myString.concat("          ");
  LCD_display(display, 1,  0, myString.substring(0,20) );
  LCD_display(display, 2,  0, "by Gerard Wassink" );
  LCD_display(display, 3,  0, "GNU license v3" );

  delay(3000);

  display.init();                       // Re-initialize display
  display.backlight();                  // Backlights on by default

  clearRegs();                          // Clear all registers
  DEG();                                // Make mode degrees by default

  // -----------------------------------
  // ------------ TEST AREA ------------
#if DEBUG == 1

  FIX(9);
  showStack();

  #include "statisticsTest.h"             // Include local file

#endif
  // ------------ TEST AREA ------------
  // -----------------------------------

}

/* ------------------------------------------------------------------------- *
 *                                                                    loop()
 * ------------------------------------------------------------------------- */
void loop() {

  /* 
   * Read key from keypad
   */
  String showchar="";
  key = keypad.getKey();               // get key press
  
  if (key) {  
    showchar = String(key, HEX);
    debugln(showchar + " - key pressed");
    switch (stateShift) {

      case noShift: { handleNoShift(); break; }

      case shiftF:  { handleShiftF();  break; }

      case shiftG:  { handleShiftG();  break; }

      default: { break; }
    }

    showStack();

  }

}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in noShift state
 * ------------------------------------------------------------------------- */
void handleNoShift() {
  switch (key) {
    case 0x11: { SQRT();        break; }
    case 0x12: { EtoX();        break; }
    case 0x13: { TENtoX();      break; }
    case 0x14: { POW();         break; }
    case 0x15: { OneOverX();    break; }
    case 0x16: { CHS();         break; }
    case 0x17: { /*   7  */     break; }
    case 0x18: { /*   8  */     break; }

    case 0x21: { /* SST  */     break; }
    case 0x22: { /* GTO  */     break; }
    case 0x23: { SIN();         break; }
    case 0x24: { COS();         break; }
    case 0x25: { TAN();         break; }
    case 0x26: { /* EEX  */     break; }
    case 0x27: { /*   4  */     break; }
    case 0x28: { /*   5  */     break; }

    case 0x31: { /* R/S   */    break; }
    case 0x32: { /* GSB   */    break; }
    case 0x33: { rollDown();    break; }
    case 0x34: { swapXY();      break; }
    case 0x35: { /* BSP   */    break; }
    case 0x36: { doEnter();     break; }
    case 0x37: { /*   1   */    break; }
    case 0x38: { /*   2   */    break; }

    case 0x41: { /* ON   */     break; }
    case 0x42: { makeShiftF();  break; }
    case 0x43: { makeShiftG();  break; }
    case 0x44: { /* STO  */     break; }
    case 0x45: { /* RCL  */     break; }
    case 0x46: { /* ENTER*/     break; }
    case 0x47: { /*   0  */     break; }
    case 0x48: { /*   .  */     break; }

    case 0x51: { /*   9  */     break; }
    case 0x52: { DIVIDE();      break; }
    case 0x53: { /*   6  */     break; }
    case 0x54: { MULTIPLY();    break; }
    case 0x55: { /*   3  */     break; }
    case 0x56: { SUBTRACT();    break; }
    case 0x57: { /* E +  */     break; }
    case 0x58: { ADD();         break; }

    default: { break; }
  }
}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in shiftF state
 * ------------------------------------------------------------------------- */
void handleShiftF() {
  switch (key) {

    case 0x11: { /*   A   */    break; }
    case 0x12: { /*   B   */    break; }
    case 0x13: { /*   C   */    break; }
    case 0x14: { /*   D   */    break; }
    case 0x15: { /*   E   */    break; }
    case 0x16: { /*MATRIX */    break; }
    case 0x17: { /* FIX   */    break; }
    case 0x18: { /* SCI   */    break; }

    case 0x21: { /* LBL   */    break; }
    case 0x22: { /* HYP   */    break; }
    case 0x23: { /* DIM   */    break; }
    case 0x24: { /* (i)   */    break; }
    case 0x25: { /* I     */    break; }
    case 0x26: { /*RESULT */    break; }
    case 0x27: { /* X><   */    break; }
    case 0x28: { /* DSE   */    break; }

    case 0x31: { /* PSE   */    break; }
    case 0x32: { clearStats();  clearShiftState(); break; }
    case 0x33: { /* CL PGM*/    break; }
    case 0x34: { /* CL REG*/    break; }
    case 0x35: { /* CL PFX*/    break; }
    case 0x36: { doRandom();    clearShiftState(); break; }
    case 0x37: { /* => R  */    break; }
    case 0x38: { /* =>HMS */    break; }

    case 0x41: { /* ON    */    break; }
    case 0x42: { /* f     */    break; }
    case 0x43: { /* g     */    break; }
    case 0x44: { /* FRAC  */    break; }
    case 0x45: { /* USER  */    break; }
    case 0x46: { doRandom();    clearShiftState(); break; }
    case 0x47: { /* X!    */    break; }
    case 0x48: { /* Y,r   */    break; }

    case 0x51: { /* ENG  */     break; }
    case 0x52: { /* SOLVE*/     break; }
    case 0x53: { /* ISG  */     break; }
    case 0x54: { /* f XY */     break; }
    case 0x55: { toRAD();       clearShiftState();  break; }
    case 0x56: { /* Re Im*/     break; }
    case 0x57: { /* L.R. */     break; }
    case 0x58: { /* P x,y*/     break; }

    default: { break; }
  }
}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in shiftF state
 * ------------------------------------------------------------------------- */
void handleShiftG() {
  switch (key) {

    case 0x11: { SQRT();        clearShiftState(); break; }
    case 0x12: { LOG();         clearShiftState(); break; }
    case 0x13: { LOG10();       clearShiftState(); break; }
    case 0x14: { PERCENT();     clearShiftState(); break; }
    case 0x15: { /*  ^%   */    break; }
    case 0x16: { ABS();         clearShiftState(); break; }
    case 0x17: { DEG();         clearShiftState(); break; }
    case 0x18: { RAD();         clearShiftState(); break; }

    case 0x21: { /* BST   */    break; }
    case 0x22: { /* HYP-1 */    break; }
    case 0x23: { /* SIN-1 */    break; }
    case 0x24: { /* COS-1 */    break; }
    case 0x25: { /* TAN-1 */    break; }
    case 0x26: { push(PI);      clearShiftState(); break; }
    case 0x27: { /* SF    */    break; }
    case 0x28: { /* CF    */    break; }

    case 0x31: { /* P/R   */    break; }
    case 0x32: { /* RTN   */    break; }
    case 0x33: { rollUp();      clearShiftState(); break; }
    case 0x34: { /* RND   */    break; }
    case 0x35: { CLX();         clearShiftState(); break; }
    case 0x36: { /* LSTX  */    break; }
    case 0x37: { /* => P  */    break; }
    case 0x38: { /* => H  */    break; }

    case 0x41: { /* ON    */    break; }
    case 0x42: { /* f     */    break; }
    case 0x43: { /* g     */    break; }
    case 0x44: { doInt();       clearShiftState(); break; }
    case 0x45: { /* MEM   */    break; }
    case 0x46: { /* LSTX  */    break; }
    case 0x47: { meanValues();   break; }
    case 0x48: { /* S     */    break; }

    case 0x51: { GRD();         clearShiftState(); break; }
    case 0x52: { /* X<=Y  */    break; }
    case 0x53: { /* F?    */    break; }
    case 0x54: { /* x = 0 */    break; }
    case 0x55: { toDEG();       clearShiftState();  break; }
    case 0x56: { /* TEST  */    break; }
    case 0x57: { /* E -   */    break; }
    case 0x58: { /* C x,y */    break; }

    default: { break; }
  }
}


/* ------------------------------------------------------------------------- *
 *                                                          Handle Enter key
 * ------------------------------------------------------------------------- */
void doEnter() {
  /* to be coded */
}


/* ***************************************************************************
 * ***************************************************************************
 * *****
 * *****   Below are the various functions per domain:
 * *****   - Logaritmic
 * *****   - Algebraic
 * *****   - Goniometric
 * *****   - Statistic
 * *****   - Calculator general
 * *****
 * ***************************************************************************
 * ***************************************************************************/


/* ------------------------------------------------------------------------- *
 *                                                     Logarithmic functions
 * ------------------------------------------------------------------------- */
void LOG10()  { stack[X] = log10(stack[X]); }     // 10 log OF x
void TENtoX() { stack[X] = pow(10, stack[X]); }   // 10 TO THE POWER OF x 
void LOG()    { stack[X] = log(stack[X]); }       // natural log OF x
void EtoX()   { stack[X] = pow(E, stack[X]); }    // e TO THE POWER OF x


/* ------------------------------------------------------------------------- *
 *                                                      Algebraicic functions
 * ------------------------------------------------------------------------- */
void doRandom() {                                 // Random Number
//  double i;
  randomSeed(analogRead(A7));
  double i = (double)random(2147483647) / 2147483647;
  push( i );
}

void doInt() {                                    // x = int(x)
  stack[X] = int(stack[X]);
}

void SQRT() { stack[X] = sqrt(stack[X]); }        // Square root
void SQ()   { stack[X] = sq(stack[X]); }          // Square

void OneOverX()   { stack[X] = 1 / stack[X]; }    // 1 / X
void CHS()   { stack[X] = -1 * stack[X]; }        // Change Sign
void ABS()   { stack[X] = abs(stack[X]); }        // Absolute value

void POW()  {                                     // Y to the power of X
  double temp = pow(stack[Y], stack[X]);
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}

void PERCENT()  {                                 // Take X perecnt of Y
  double temp = stack[Y] * stack[X] / 100;
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}

void ADD()  {                                     // Add
  double temp = stack[Y] + stack[X];
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}

void SUBTRACT()  {                                // Subtract
  double temp = stack[Y] - stack[X];
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}

void MULTIPLY()  {                                // Multiply
  double temp = stack[Y] * stack[X];
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}

void DIVIDE()  {                                  // Divide
  double temp = stack[Y] / stack[X];
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}


/* ------------------------------------------------------------------------- *
 *                                                     Goniometric functions
 * ------------------------------------------------------------------------- */
void toRAD() {                                    // convert degrees to Radians
  stack[X] = stack[X]*2*PI/360;
}

void toDEG() {                                    // convert Radians to degrees
  stack[X] = stack[X]*360/(2*PI);
}

void SIN() {                                      // Sine
  switch (gonioStatus) {
    case statDEG:
      stack[X] = sin(stack[X]*2*PI/360);          // Degrees to radians
      break;
    case statGRD:
      stack[X] = sin(stack[X]*2*PI/400);          // Gradians to radians
      break;
    case statRAD:
      stack[X] = sin(stack[X]);
      break;
    default:
      break;
  }
}

void ASIN() {                                     // Arc Sine
  switch (gonioStatus) {
    case statDEG:
      stack[X] = asin(stack[X])*360/(2*PI);       // Radians to Degrees
      break;
    case statGRD:
      stack[X] = asin(stack[X])*400/(2*PI);       // Radians to Gradians
      break;
    case statRAD:
      stack[X] = asin(stack[X]);
      break;
    default:
      break;
  }
}

void COS() {                                      // Cosine
  switch (gonioStatus) {
    case statDEG:
      stack[X] = cos(stack[X]*2*PI/360);          // Degrees to radians
      break;
    case statGRD:
      stack[X] = cos(stack[X]*2*PI/400);          // Gradians to radians
      break;
    case statRAD:
      stack[X] = cos(stack[X]);
      break;
    default:
      break;
  }
}

void ACOS() {                                     // Arc Cosine
  switch (gonioStatus) {
    case statDEG:
      stack[X] = acos(stack[X])*360/(2*PI);       // Radians to Degrees
      break;
    case statGRD:
      stack[X] = acos(stack[X])*400/(2*PI);       // Radians to Gradians
      break;
    case statRAD:
      stack[X] = acos(stack[X]);
      break;
    default:
      break;
  }
}

void TAN() {                                      // Tangent
  switch (gonioStatus) {
    case statDEG:
      stack[X] = tan(stack[X]*2*PI/360);          // Degrees to radians
      break;
    case statGRD:
      stack[X] = tan(stack[X]*2*PI/400);          // Gradians to radians
      break;
    case statRAD:
      stack[X] = tan(stack[X]);
      break;
    default:
      break;
  }
}

void ATAN() {                                     // Arc Tangent
  switch (gonioStatus) {
    case statDEG:
      stack[X] = atan(stack[X])*360/(2*PI);       // Radians to Degrees
      break;
    case statGRD:
      stack[X] = atan(stack[X])*400/(2*PI);       // Radians to Gradians
      break;
    case statRAD:
      stack[X] = atan(stack[X]);
      break;
    default:
      break;
  }
}


/* ------------------------------------------------------------------------- *
 *                                                       Statistic functions
 * ------------------------------------------------------------------------- */
void clearStats() {                            // Clear statistic registters
  for (int i=statRegLo; i<=statRegHi; i++) {
    Reg[i] = 0;
  }
}

void sigmaPlus() {
  double x = stack[X];
  double y = stack[Y];

  Reg[statRegLo]   = Reg[statRegLo]   + 1;        // Number of data points
  Reg[statRegLo+1] = Reg[statRegLo+1] + x;        // Summation of X values
  Reg[statRegLo+2] = Reg[statRegLo+2] + (x*x);    // Summation of X squares
  Reg[statRegLo+3] = Reg[statRegLo+3] + y;        // Summation of Y values
  Reg[statRegLo+4] = Reg[statRegLo+4] + (y*y);    // Summation of Y squares
  Reg[statRegLo+5] = Reg[statRegLo+5] + (x*y);    // Summation of x*yY products

  stack[X] = (double)Reg[statRegLo];
}

void sigmaMinus() {
  double x = stack[X];
  double y = stack[Y];

  Reg[statRegLo]   = Reg[statRegLo]   - 1;        // Number of data points
  Reg[statRegLo+1] = Reg[statRegLo+1] - x;        // Summation of X values
  Reg[statRegLo+2] = Reg[statRegLo+2] - (x*x);    // Summation of X squares
  Reg[statRegLo+3] = Reg[statRegLo+3] - y;        // Summation of Y values
  Reg[statRegLo+4] = Reg[statRegLo+4] - (y*y);    // Summation of Y squares
  Reg[statRegLo+5] = Reg[statRegLo+5] - (x*y);    // Summation of x*yY products

  stack[X] = (double)Reg[statRegLo];
}

void meanValues() {
  push( Reg[statRegLo+3] / Reg[statRegLo] );
  push( Reg[statRegLo+1] / Reg[statRegLo] );
}


/* ------------------------------------------------------------------------- *
 *                                                      Calculator functions
 * ------------------------------------------------------------------------- */
void clearRegs() {                                 // Clear registters
  for (int i=0; i<30; i++) {
    Reg[i] = 0;
  }
}

void STO(int reg) {                               // Store X in register
  Reg[reg] = stack[X];
}

void RCL(int reg) {                               // Recall X from register
  rollUp();
  stack[X] = Reg[reg];
}

void CLX() {                                      // Recall X from register
  stack[X] = 0;
}

void FIX(int val) { precision = val; }

void DEG() {                                      // Status to Degrees
  gonioStatus = statDEG; 
  LCD_display(display, 3, 0, F("deg") );
}

void RAD() {                                      // Status to Radians
  gonioStatus = statRAD; 
  LCD_display(display, 3, 0, F("rad") );
}

void GRD() {                                      // Status to Gradians
  gonioStatus = statGRD; 
  LCD_display(display, 3, 0, F("grd") );
}

void swapXY() {                                   // Swap X and Y
  double f = stack[X];
  stack[X] = stack[Y];
  stack[Y] = f;
}

void push(double val) {                           // Push value on stack
  rollUp();
  stack[X] = val;
}

void rollDown() {                                 // Roll stack down
  double f = stack[X];
  stack[X] = stack[Y];
  stack[Y] = stack[Z];
  stack[Z] = stack[T];
  stack[T] = f;
}

void rollUp() {                                   // Roll stack up
  double f = stack[T];
  stack[T] = stack[Z];
  stack[Z] = stack[Y];
  stack[Y] = stack[X];
  stack[X] = f;
}


/* ------------------------------------------------------------------------- *
 *                                                     Shift state functions
 * ------------------------------------------------------------------------- */
void makeShiftF() {
  debugln("Making shift state F");
  stateShift = shiftF;
  LCD_display(display, 3, 4, F("f") );
}

void makeShiftG() {
  debugln("Making shift state G");
  stateShift = shiftG;
  LCD_display(display, 3, 4, F("g") );
}

void clearShiftState() {
  stateShift = noShift;
  LCD_display(display, 3, 4, F(" ") );
}


/* ***************************************************************************
 * ***************************************************************************
 * *****
 * *****   Below are the display functions:
 * *****
 * ***************************************************************************
 * ***************************************************************************/


/* ------------------------------------------------------------------------- *
 *                                                               showStack()
 * ------------------------------------------------------------------------- */
void showStack() {
  String myString = "Z: ";
  myString.concat(String(stack[Z], precision));
  myString.concat(F("                    "));
  LCD_display(display, 0, 0, myString.substring(0,20) );

  myString = "Y: ";
  myString.concat(String(stack[Y], precision));
  myString.concat(F("                    "));
  LCD_display(display, 1, 0, myString.substring(0,20) );

  myString = "X: ";
  myString.concat(String(stack[X], precision));
  myString.concat(F("                    "));
  LCD_display(display, 2, 0, myString.substring(0,20) );
}


/* ------------------------------------------------------------------------- *
 *         Routine to display stuff on the display of choice - LCD_display()
 * ------------------------------------------------------------------------- */
void LCD_display(LiquidCrystal_I2C screen, int row, int col, String text) {
  screen.setCursor(col, row);
  screen.print(text);
}
