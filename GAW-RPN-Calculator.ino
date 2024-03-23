/* ------------------------------------------------------------------------- *
 * Name   : GAW-RPN-Calculator
 * Author : Gerard Wassink
 * Date   : March 2024
 * Purpose: Generate keyboard commands
 * Versions:
 *   0.1  : Initial code base
 *------------------------------------------------------------------------- */
#define progVersion "0.1"                     // Program version definition
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
 *       Definitions for the keyboard
 * ------------------------------------------------------------------------- */
#define ROWS 5                              // five rows for keyboard
#define COLS 8                              // eight columns for keyboard


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
    {0x31, 0x32, 0x33, 0x34, 0x35, 0xFF, 0x37, 0x38}, // row 3, pin 4
    {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48}, // row 4, pin 5
    {0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58}, // row 5, pin 6
  };
  byte rowPins[ROWS] = {2,3,4,5,6};               // row pins of the keypad
  byte colPins[COLS] = {7,8,9,10,11,12,13,17};    // column pins of the keypad
  
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
  
  //
  // Assemble and show intro text and versionb
  //
  LCD_display(display, 0, 0, "GAW RPN calc ");
  String myString = "v";
  myString.concat(progVersion);
  myString.concat("          ");
  LCD_display(display, 0, 13, myString.substring(0,20) );

#if DEBUG == 0
  //
  // Leave intro screen for 3 seconds
  //
  LCD_display(display, 2, 0, "Initializing " );

  for (int t=13; t<16; t++) {
    LCD_display(display, 2, t, "." );
    delay(1000);
  }
#endif

  // -----------------------------------
  // ------------ TEST AREA ------------
#if DEBUG == 1

  FIX(9);
  showStack();

  clearReg();
  push(1);
  push(2);
  showStack();

delay(1000);

  STO(0);
  showStack();

delay(1000);

  push(3);
  push(4);
  showStack();

  RCL(0);
  showStack();

/*
  String test1 = "-123.456789";
  double TEST1 = test1.toDouble();

  push(TEST1);
  showStack();
 */
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
  char key = keypad.getKey();               // get key press
  
//  if (key != NO_KEY){                       // did we receive one?
  if (key) {  
    showchar = String(key, HEX);
    debugln(showchar + " - key pressed");
    switch (stateShift) {
      case noShift: {
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
          case 0x36: { /* ENTER */    break; }
          case 0x37: { /*   1   */    break; }
          case 0x38: { /*   2   */    break; }

          case 0x41: { /* ON   */     break; }
          case 0x42: { /* f    */     break; }
          case 0x43: { /* g    */     break; }
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
          case 0x57: { /* SIGMA*/     break; }
          case 0x58: { ADD();         break; }

          default: { break; }
        }
      }

      case shiftF: {
        switch (key) {

          case 0x11: { /*   A  */     break; }
          case 0x12: { /*   B  */     break; }
          case 0x13: { /*   C  */     break; }
          case 0x14: { /*   D  */     break; }
          case 0x15: { /*   E  */     break; }
          case 0x16: { /*MATRIX*/     break; }
          case 0x17: { /* FIX  */     break; }
          case 0x18: { /* SCI  */     break; }

          case 0x21: { /* LBL  */     break; }
          case 0x22: { /* HYP  */     break; }
          case 0x23: { /* DIM  */     break; }
          case 0x24: { /* (i)  */     break; }
          case 0x25: { /* I    */     break; }
          case 0x26: { /*RESULT*/     break; }
          case 0x27: { /* X><  */     break; }
          case 0x28: { /* DSE  */     break; }

          case 0x31: { /* PSE   */    break; }
          case 0x32: { /* CL E  */    break; }
          case 0x33: { /* CL PGM*/    break; }
          case 0x34: { /* CL REG*/    break; }
          case 0x35: { /* CL PFX*/    break; }
          case 0x36: { /* RAN#  */    break; }
          case 0x37: { /* => R  */    break; }
          case 0x38: { /* =>HMS */    break; }

          case 0x41: { /* ON   */     break; }
          case 0x42: { /* f    */     break; }
          case 0x43: { /* g    */     break; }
          case 0x44: { /* FRAC */     break; }
          case 0x45: { /* USER */     break; }
          case 0x46: { /* ENTER*/     break; }
          case 0x47: { /* X!   */     break; }
          case 0x48: { /* Y,r  */     break; }

          case 0x51: { /* ENG  */     break; }
          case 0x52: { /* SOLVE*/     break; }
          case 0x53: { /* ISG  */     break; }
          case 0x54: { /* f XY */     break; }
          case 0x55: { /* =>RAD*/     break; }
          case 0x56: { /* Re Im*/     break; }
          case 0x57: { /* L.R. */     break; }
          case 0x58: { /* P x,y*/     break; }

          default: { break; }
        }
      }
      case shiftG: {
        switch (key) {
          default: { break; }
        }
      }
      default: { break; }
    }
    stateShift = noShift;
    showStack();
  }
}


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
void SIN() {                                      // Sine
  double angle;
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
 *                                                      Calculator functions
 * ------------------------------------------------------------------------- */
void clearReg() {                                 // Clear registters
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

void FIX(int val) { precision = val; }

void DEG() { gonioStatus = statDEG; }             // Calc in Degrees
void RAD() { gonioStatus = statRAD; }             // Calc in Radians
void GRD() { gonioStatus = statGRD; }             // Calc in Gradians

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
 *                                                               showStack()
 * ------------------------------------------------------------------------- */
void showStack() {
  String myString = "Y: ";
  myString.concat(String(stack[Y], precision));
  myString.concat(F("                    "));
  LCD_display(display, 1, 0, myString.substring(0,20) );

  myString = "X: ";
  myString.concat(String(stack[X], precision));
  myString.concat(F("                    "));
  LCD_display(display, 2, 0, myString.substring(0,20) );

  switch (gonioStatus) {
    case statDEG:
      LCD_display(display, 3, 4, F("    ") );
      break;
    case statGRD:
      LCD_display(display, 3, 4, F("grad") );
      break;
    case statRAD:
      LCD_display(display, 3, 4, F("rad ") );
      break;
    default:
      break;
  }
}


/* ------------------------------------------------------------------------- *
 *         Routine to display stuff on the display of choice - LCD_display()
 * ------------------------------------------------------------------------- */
void LCD_display(LiquidCrystal_I2C screen, int row, int col, String text) {
  screen.setCursor(col, row);
  screen.print(text);
}
