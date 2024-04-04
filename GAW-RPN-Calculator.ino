/* ------------------------------------------------------------------------- *
 * Name   : GAW-RPN-Calculator
 * Author : Gerard Wassink
 * Date   : March 2024
 * Purpose: Generate keyboard commands
 * Versions:
 *   0.1  : Initial code base
 *   0.2  : Most functions built in
 *   0.3  : Minor improvements
 *   0.4  : Optimization
 *          Built in number entry
 *   0.5  : Built in FRAC function
 *          Corrected PERCENT function
 *          Built in DIFPERC function
 *          Built in ASIN, ACOS and ATAN functions
 *          Added lst X function
 *   0.6  : Rearranged routines to prepare for programming
 *          Program storing and handling tested in setup() routine
 *          Built in FIX functionality using X register...
 *          Built in FIX() function
 *------------------------------------------------------------------------- */
#define progVersion "0.6"                     // Program version definition
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
 *                                		  Positioning of onscreen annunciators
 * ------------------------------------------------------------------------- */
// item		    startpos	  endpos    values
// ----------	--------    ------    --------------------------------
// shiftState     4          4      f, g
// gonioState    17         19      rad, grd (spaces when Degree mode)


/* ------------------------------------------------------------------------- *
 *                                                        DEBUGGING ON / OFF
 * Compiler directives to switch debugging on / off
 * Do not enable debug when not needed, Serial takes space and time!
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
int precision = 9;                      // default precision = 9

const unsigned int statDEG=0;           // Goniometric state
const unsigned int statRAD=1;
const unsigned int statGRD=2;
unsigned int gonioStatus = statDEG;     // Gonio default to degrees

const unsigned int noShift=0;           // Shift States
const unsigned int shiftF=1;
const unsigned int shiftG=2;
unsigned int stateShift=noShift;        // shift default to 0 (off)

String numString = "";                  // String to build number in
bool numEntry = false;                  // Number entry state

double E = 2.718281828459045;           // constant for the natural number

int statRegLo = 2;                      // Lo register for statistics
int statRegHi = 7;                      // Hi register for statistics

/* ------------------------------------------------------------------------- *
 *                                             Stack variables X, Y, Z and T
 * ------------------------------------------------------------------------- */
double stack[4] = {0, 0, 0, 0}; 
int X=0, Y=1, Z=2, T=3;

double lastX = 0;

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
 *                                                       Define keypad codes
 *  Conforming to a HP-15C calculator
 *  When a key is pressed it will generate the codes below
 *  Key values indicate row and column number
 * ------------------------------------------------------------------------- */
  char keys[ROWS][COLS] = {
  //   7     8     9    10    11    12    13    17     <== pins
  //   1     2     3     4     5     6     7     8     <== columns
    {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18}, // row 1, pin 2
    {0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28}, // row 2, pin 3
    {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38}, // row 3, pin 4
    {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48}, // row 4, pin 5
    {0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58}, // row 5, pin 6
  };
  byte rowPins[ROWS] = {2,3,4,5,6};               // row pins of the keypad
  byte colPins[COLS] = {7,8,9,10,11,12,13,17};    // column pins of the keypad
  
/* ------------------------------------------------------------------------- *
 *                                                  Create object for Keypad
 * ------------------------------------------------------------------------- */
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* ------------------------------------------------------------------------- *
 *                                                           Global keypress
 * ------------------------------------------------------------------------- */
  char key = 0x00;


/* ------------------------------------------------------------------------- *
 *                                                                   setup()
 * ------------------------------------------------------------------------- */
void setup() {
  debugstart(115200);

#if DEBUG == 0
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
#endif

  display.init();                       // Re-initialize display
  display.backlight();                  // Backlights on by default

  clearRegs();                          // Clear all registers
  DEG();                                // Make mode degrees by default
  showStack();                          // Show the stack

  // -----------------------------------
  // ------------ TEST AREA ------------
#if DEBUG == 1

//  #include "testStatistics.h"
//  #include "testGoniometrics.h"
//  #include "testLogaritmic.h"
//  #include "testAlgebraics.h"

  char keys[10] = {0x38, 0x37, 0x23};   // Prepare 
  for (int i=0; i<3; i++) {             //  for program
    key = keys[i];                      //   storing and
    handleKeys();                       //    handling
  }


#endif
  // ------------ TEST AREA ------------
  // -----------------------------------

}

/* ------------------------------------------------------------------------- *
 *                                                                    loop()
 * ------------------------------------------------------------------------- */
void loop() {

  key = keypad.getKey();                          // Read key from keypad
  
  if (key) {  
    LCD_display(display, 3, 13, "   ");           // Clear ovf if we had one
    handleKeys();                                 // Handle Keystrokes
  }
}


/* ------------------------------------------------------------------------- *
 *                                                              handleKeys()
 * ------------------------------------------------------------------------- */
void handleKeys() {
  #if DEBUG == 1
    String showchar="";
    showchar = String(key, HEX);
    debugln(showchar + " - key pressed");
  #endif

  switch (stateShift) {

    case noShift: { handleNoShift(); break; }

    case shiftF:  { handleShiftF();  break; }

    case shiftG:  { handleShiftG();  break; }

    default: { break; }
  }
  if (!numEntry) {
    showStack();
  }
}


/* ------------------------------------------------------------------------- *
 *                                                    Build up numeric entry
 * ------------------------------------------------------------------------- */
void bldNum(char c) {
  if (!numEntry) {
    numString = "";
    numEntry = true;
    rollUp();
    LCD_display(display, 2, 3, "                 " );
  }
  if (c == 'Z') {
    int l = numString.length();
    debugln(l);
    numString = numString.substring(0,l-1);
  } else {
    numString.concat(c);
    debugln(numString);
  }
  LCD_display(display, 2, 3, "                 " );
  LCD_display(display, 2, 3, numString );
}


/* ------------------------------------------------------------------------- *
 *                                    Stop numeric entry and assemble number
 * Convert string to double
 * Store number in X register
 * ------------------------------------------------------------------------- */
void endNum() {
  if (numEntry) {
    stack[X] = numString.toDouble();
    showStack();
    numString = "";
    numEntry = false;
  }
}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in noShift state
 * ------------------------------------------------------------------------- */
void handleNoShift() {
  switch (key) {
    case 0x11: { endNum(); SQRT();        break; }
    case 0x12: { endNum(); EtoX();        break; }
    case 0x13: { endNum(); TENtoX();      break; }
    case 0x14: { endNum(); POW();         break; }
    case 0x15: { endNum(); OneOverX();    break; }
    case 0x16: { endNum(); CHS();         break; }
    case 0x17: { bldNum('7');   break; }
    case 0x18: { bldNum('8');   break; }

    case 0x21: { /* SST  */     break; }
    case 0x22: { /* GTO  */     break; }
    case 0x23: { endNum(); SIN();         break; }
    case 0x24: { endNum(); COS();         break; }
    case 0x25: { endNum(); TAN();         break; }
    case 0x26: { /* EEX  */     break; }
    case 0x27: { bldNum('4');   break; }
    case 0x28: { bldNum('5');   break; }

    case 0x31: { /* R/S   */    break; }
    case 0x32: { /* GSB   */    break; }
    case 0x33: { endNum(); rollDown();    break; }
    case 0x34: { endNum(); swapXY();      break; }
    case 0x35: { bldNum('Z');   break; }
    case 0x36: { endNum(); doEnter();     break; }
    case 0x37: { bldNum('1');   break; }
    case 0x38: { bldNum('2');   break; }

    case 0x41: { /* ON   */     break; }
    case 0x42: { endNum(); makeShiftF();  break; }
    case 0x43: { endNum(); makeShiftG();  break; }
    case 0x44: { /* STO  */     break; }
    case 0x45: { /* RCL  */     break; }
    case 0x46: { endNum(); doEnter();     break; }
    case 0x47: { bldNum('0');   break; }
    case 0x48: { bldNum('.');   break; }

    case 0x51: { bldNum('9');   break; }
    case 0x52: { endNum(); DIVIDE();      break; }
    case 0x53: { bldNum('6');   break; }
    case 0x54: { endNum(); MULTIPLY();    break; }
    case 0x55: { bldNum('3');   break; }
    case 0x56: { endNum(); SUBTRACT();    break; }
    case 0x57: { endNum(); sigmaPlus();   break; }
    case 0x58: { endNum(); ADD();         break; }

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
    case 0x17: { endNum(); FIX();  clearShiftState(); break; }
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
    case 0x32: { endNum(); clearStats();  clearShiftState(); break; }
    case 0x33: { /* CL PGM*/    break; }
    case 0x34: { /* CL REG*/    break; }
    case 0x35: { /* CL PFX*/    break; }
    case 0x36: { endNum(); doRandom();    clearShiftState(); break; }
    case 0x37: { /* => R  */    break; }
    case 0x38: { /* =>HMS */    break; }

    case 0x41: { /* ON    */    break; }
    case 0x42: { /* f     */    break; }
    case 0x43: { /* g     */    break; }
    case 0x44: { endNum(); FRAC() ;         clearShiftState(); break; }
    case 0x45: { /* USER  */    break; }
    case 0x46: { endNum(); doRandom();    clearShiftState(); break; }
    case 0x47: { endNum(); FAC();         clearShiftState(); break; }
    case 0x48: { /* Y,r   */    break; }

    case 0x51: { /* ENG  */     break; }
    case 0x52: { /* SOLVE*/     break; }
    case 0x53: { /* ISG  */     break; }
    case 0x54: { /* f XY */     break; }
    case 0x55: { endNum(); toRAD();       clearShiftState();  break; }
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

    case 0x11: { endNum(); SQRT();        clearShiftState(); break; }
    case 0x12: { endNum(); LOG();         clearShiftState(); break; }
    case 0x13: { endNum(); LOG10();       clearShiftState(); break; }
    case 0x14: { endNum(); PERCENT();     clearShiftState(); break; }
    case 0x15: { endNum(); DIFPERC();     clearShiftState(); break; }
    case 0x16: { endNum(); ABS();         clearShiftState(); break; }
    case 0x17: { endNum(); DEG();         clearShiftState(); break; }
    case 0x18: { endNum(); RAD();         clearShiftState(); break; }

    case 0x21: { /* BST   */    break; }
    case 0x22: { /* HYP-1 */    break; }
    case 0x23: { endNum(); ASIN();        clearShiftState(); break;  }
    case 0x24: { endNum(); ACOS();        clearShiftState(); break;  }
    case 0x25: { endNum(); ATAN();        clearShiftState(); break;  }
    case 0x26: { endNum(); push(PI);      clearShiftState(); break; }
    case 0x27: { /* SF    */    break; }
    case 0x28: { /* CF    */    break; }

    case 0x31: { /* P/R   */    break; }
    case 0x32: { /* RTN   */    break; }
    case 0x33: { endNum(); rollUp();      clearShiftState(); break; }
    case 0x34: { /* RND   */    break; }
    case 0x35: { endNum(); CLX();         clearShiftState(); break; }
    case 0x36: { endNum(); lstX();        clearShiftState(); break; }
    case 0x37: { /* => P  */    break; }
    case 0x38: { /* => H  */    break; }

    case 0x41: { /* ON    */    break; }
    case 0x42: { /* f     */    break; }
    case 0x43: { /* g     */    break; }
    case 0x44: { endNum(); doInt();       clearShiftState(); break; }
    case 0x45: { /* MEM   */    break; }
    case 0x46: { /* LSTX  */    break; }
    case 0x47: { endNum(); meanValues();  clearShiftState(); break; }
    case 0x48: { /* StdDev*/    break; }

    case 0x51: { endNum(); GRD();         clearShiftState(); break; }
    case 0x52: { /* X<=Y  */    break; }
    case 0x53: { /* F?    */    break; }
    case 0x54: { /* x = 0 */    break; }
    case 0x55: { endNum(); toDEG();       clearShiftState();  break; }
    case 0x56: { /* TEST  */    break; }
    case 0x57: { endNum(); sigmaMinus();  clearShiftState();  break; }
    case 0x58: { /* C x,y */    break; }

    default: { break; }
  }
}


/* ------------------------------------------------------------------------- *
 *                                                          Handle Enter key
 * ------------------------------------------------------------------------- */
void doEnter() {
  rollUp();
  stack[X] = stack[Y];
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
void LOG10()  { saveLastX(); stack[X] = log10(stack[X]); }     // 10 log OF x
void TENtoX() { saveLastX(); stack[X] = pow(10, stack[X]); }   // 10 TO THE POWER OF x 
void LOG()    { saveLastX(); stack[X] = log(stack[X]); }       // natural log OF x
void EtoX()   { saveLastX(); stack[X] = pow(E, stack[X]); }    // e TO THE POWER OF x


/* ------------------------------------------------------------------------- *
 *                                                      Algebraicic functions
 * ------------------------------------------------------------------------- */
void doRandom() {                                 // Random Number
  saveLastX(); 
  randomSeed(analogRead(A7));
  push( (double)random(2147483647) / 2147483647 );
}

void FRAC() {                                    // x = int(x)
  saveLastX(); 
  stack[X] = stack[X] - int(stack[X]);
}

void doInt() {                                    // x = int(x)
  saveLastX(); 
  stack[X] = int(stack[X]);
}

void SQRT() { saveLastX(); stack[X] = sqrt(stack[X]); }        // Square root
void SQ()   { saveLastX(); stack[X] = sq(stack[X]); }          // Square

void OneOverX() { saveLastX(); stack[X] = 1 / stack[X]; }    // 1 / X
void CHS() { saveLastX(); stack[X] = -1 * stack[X]; }          // Change Sign
void ABS() { saveLastX(); stack[X] = abs(stack[X]); }          // Absolute value

void POW() {                                      // Y to the power of X
  saveLastX(); 
  twoNums( pow(stack[Y], stack[X]));
}

void PERCENT() {                                  // Take X percent of Y
  saveLastX(); 
  stack[X] = stack[Y] * stack[X] / 100;
}

void DIFPERC() {                                  // Take delta X percent of Y
  saveLastX(); 
  stack[X] = ( stack[X] - stack[Y] ) *100 / stack[Y];
}

void ADD()  {                                     // Add
  saveLastX(); 
  twoNums( stack[Y] + stack[X] );
}

void SUBTRACT()  {                                // Subtract
  saveLastX(); 
  twoNums( stack[Y] - stack[X] );
}

void MULTIPLY()  {                                // Multiply
  saveLastX(); 
  twoNums( stack[Y] * stack[X] );
}

void DIVIDE()  {                                  // Divide
  saveLastX(); 
  twoNums( stack[Y] / stack[X] );
}

void twoNums(double temp) {
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}

void FAC()  {                                     // Factorial
  saveLastX(); 
  double x = int(stack[X]);
  if (x <=170) {
    double r = 1;
    for (double i=2; i<=x; i++) {
      r = r * i; 
    }
    stack[X] = r;
  } else {
    LCD_display(display, 3, 13, "ovf");
  }
}


/* ------------------------------------------------------------------------- *
 *                                                     Goniometric functions
 * ------------------------------------------------------------------------- */
void toRAD() {                                    // convert degrees to Radians
  saveLastX(); 
  stack[X] = stack[X]*2*PI/360;
}

void toDEG() {                                    // convert Radians to degrees
  saveLastX(); 
  stack[X] = stack[X]*360/(2*PI);
}

void SIN() {                                      // Sine
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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
  saveLastX(); 
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

void saveLastX() {                                // Save X to lastX
  lastX = stack[X];
}

void lstX() {                                     // Get lastX to X
  stack[X] = lastX;
}

void STO(int reg) {                               // Store X in register
  Reg[reg] = stack[X];
}

void RCL(int reg) {                               // Recall X from register
  saveLastX(); 
  rollUp();
  stack[X] = Reg[reg];
}

void CLX() {                                      // Recall X from register
  saveLastX(); 
  stack[X] = 0;
}

int getOneNum() {
  key = 0;
  int val = 99;
  String numString = "";
  LCD_display(display, 2, 0, "                    " );
  do {
    key = keypad.getKey();                        // Read key from keypad

    switch (key) {
      case 0x47: val = 0; break;
      case 0x37: val = 1; break;
      case 0x38: val = 2; break;
      case 0x55: val = 3; break;
      case 0x27: val = 4; break;
      case 0x28: val = 5; break;
      case 0x53: val = 6; break;
      case 0x17: val = 7; break;
      case 0x18: val = 8; break;
      case 0x51: val = 9; break;

      default: break;

    }

  } while ( val == 99 );
  numString = String(val, DEC);
  LCD_display(display, 2, 0, numString );
  return val;
}

void FIX() {
  debugln("Entering FIX()");
  int val = 99;
  val = getOneNum();
  if (val >= 0 && val <= 9) { precision = val; }
}

void DEG() {                                      // Status to Degrees
  gonioStatus = statDEG; 
  LCD_display(display, 3,17, F("deg") );
}

void RAD() {                                      // Status to Radians
  gonioStatus = statRAD; 
  LCD_display(display, 3,17, F("rad") );
}

void GRD() {                                      // Status to Gradians
  gonioStatus = statGRD; 
  LCD_display(display, 3,17, F("grd") );
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
