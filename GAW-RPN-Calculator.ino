/* ------------------------------------------------------------------------- *
 * Name   : GAW-RPN-Calculator
 * Author : Gerard Wassink
 * Date   : May 2024
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
 *          Improve startup screen
 *          Code cleanup
 *   0.7  : Added PSE() routine
 *          Added STO command / routine
 *          Added RCL command / routine
 *   0.8  : Display improvement for FIX and exponent display
 *          Implemented EEX() function
 *          Removed some dbugging
 *   0.9  : Corrected erroneous stack rollup while entering numbers
 *          Implemented f clearRegs()
 *          Simplified F and G shift handling
 *          Implemented f clear Prefix
 *          Repositioned shift state on display
 *          Improved number display for FIX mode
 *
 *   1.0  : Release
 *          debugging off
 *          Code cleanup
 *          Added pictures
 *          Changed README
 *
 *   1.0.1: Added standard deviation function
 *          Added linear regression function L.R.
 *          Added linear estimation function ^y,r
 *          Added Permutation function Py,x
 *          Added Combination function Cy,x
 *   1.0.2: Code cleanup
 *          Added Rectangular to Polar =>P
 *          Added Polar to Rectangular =>R
 *------------------------------------------------------------------------- */
#define progVersion "1.0.2"                   // Program version definition
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
 *       Copyright (C) May 2024 Gerard Wassink
 * ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- *
 *                   Positioning of onscreen annunciators on the bottom line
 * ------------------------------------------------------------------------- */
// item		    startpos	  endpos    values
// ----------	--------    ------    --------------------------------
// shiftState     7          7      f, g
// displError     9         11      ovf
// displState    13         15      fix, sci, eng
// gonioState    17         19      rad, grd, deg


/* ------------------------------------------------------------------------- *
 *                                                        DEBUGGING ON / OFF
 * Compiler directives to switch debugging on / off
 * Do not enable debug when not needed, Serial takes space and time!
 * ------------------------------------------------------------------------- */
#define DEBUG 0

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
#define ROWS 5                          // five rows for keyboard
#define COLS 8                          // eight columns for keyboard

#define statDEG 0                       // Goniometric states
#define statRAD 1
#define statGRD 2

#define noShift 0                       // Shift States
#define shiftF  1
#define shiftG  2

#define dispFix 0                       // Display states
#define dispSci 1
#define dispEng 2


/* ------------------------------------------------------------------------- *
 *                                                          Library includes
 * ------------------------------------------------------------------------- */
#include <Wire.h>                       // I2C comms library
#include <LiquidCrystal_I2C.h>          // LCD library

#include <Keypad.h>                     // Keypad library

#include <String.h>                     // String library

#include <math.h>                       // Arduino math library


/* ------------------------------------------------------------------------- *
 *                                                     Create Display object
 * ------------------------------------------------------------------------- */
LiquidCrystal_I2C display(0x25,20,4);   // Instantiate display object


/* ------------------------------------------------------------------------- *
 *                                                      Calculator variables
 * ------------------------------------------------------------------------- */
int precision = 9;                      // default precision = 9

unsigned int gonioState = statDEG;      // Gonio default to degrees
unsigned int shiftState = noShift;      // shift default to 0 (off)
unsigned int dispState  = dispFix;      // shift default to 0 (off)

String numString = "";                  // String to build number
bool numEntry = false;                  // Number entry state

double E = 2.718281828459045;           // constant for the natural number


/* ------------------------------------------------------------------------- *
 *                                                      Statistics registers
 * Containing:
 * statRegLo + 0 = n    number of values
 * statRegLo + 1 = Ex   sum of x values
 * statRegLo + 2 = Ex2  sum of squared x values
 * statRegLo + 3 = Ey   sum of y values
 * statRegLo + 4 = Ey2  sum of squared y values
 * statRegLo + 5 = Exy  sum of products of x and y values
 * ------------------------------------------------------------------------- */
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
void setup() 
{
  debugstart(115200);

  display.init();                                 // Initialize display
  display.backlight();                            // Backlight on
  
  //                                              Intro Screen
  // Assemble and show intro text and version
  //
  LCD_display(display, 0, 0, F("GAW RPN calculator  "));
  String myString = F("version ");
  myString.concat(progVersion);
  myString.concat("          ");
  LCD_display(display, 1,  0, myString.substring(0,20) );
  LCD_display(display, 2,  0, F("(c) Gerard Wassink  ") );
  LCD_display(display, 3,  0, F("GNU license v3      ") );

  delay(3000);

  myString = F("RPN calculator ");                // headline
  myString.concat(progVersion);
  LCD_display(display, 0, 0, myString);
                                                  // clear bottom line
  LCD_display(display, 3, 0, F("                    ") );
  LCD_display(display, 3,13, F("fix") );          // assuming fix as default


  clearRegs();                                    // Clear all registers
  DEG();                                          // Make mode degrees by default
  showStack();                                    // Show the stack

  // -----------------------------------
  // ------------ TEST AREA ------------
#if DEBUG == 1

//  #include "testStatistics.h"
//  #include "testGoniometrics.h"
//  #include "testLogaritmic.h"
//  #include "testAlgebraics.h"
//  #include "STO-RCL.h"

#endif
  // ------------ TEST AREA ------------
  // -----------------------------------

}

/* ------------------------------------------------------------------------- *
 *                                                                    loop()
 * ------------------------------------------------------------------------- */
void loop() {

  key = keypad.getKey();                          // Read key from keypad
  
  if (key)
  {
    LCD_display(display, 3,  9, "   ");           // Clear ovf if we had one

    switch (shiftState)                           // Determine shift state
    {                                             //  and handle keys accordingly
      case noShift: { handleNoShift(); break; }

      case shiftF:  { handleShiftF();  break; }

      case shiftG:  { handleShiftG();  break; }

      default: { break; }
    }

    if (!numEntry)                                // No number entry in progress
    {
      showStack();                                //  then show current stack
    }

  }

}



/* ------------------------------------------------------------------------- *
 *                                                    Build up numeric entry
 * ------------------------------------------------------------------------- */
void bldNum(char c) 
{
  if (!numEntry) 
  {                                // Starting number entry?
    numString = "";                               // Then clear buildup string
    numEntry = true;                              // Entering number = true
    LCD_display(display, 2, 3, "                 " );
  }

  if (c == 'Z') 
  {                                 // Backspace??
    int l = numString.length();                   // Determine lenght of string
    numString = numString.substring(0,l-1);       // Remove last character
  } 
    else 
  {
    numString.concat(c);                          // Concatenate number
  }
                                                  // Display number so far
  LCD_display(display, 2, 3, "                 " );
  LCD_display(display, 2, 3, numString );
}


/* ------------------------------------------------------------------------- *
 *                                    Stop numeric entry and assemble number
 * Convert string to double
 * Store number in X register
 * ------------------------------------------------------------------------- */
void endNum() 
{
  if (numEntry) 
  {                                 // Check num entry was going on
    stack[X] = numString.toDouble();              // If so, put num in X register
    showStack();                                  // Show new stack status
    numString = "";                               // clear num string for next time
    numEntry = false;                             // Stop num entry
  }
}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in noShift state
 * ------------------------------------------------------------------------- */
void handleNoShift() 
{
  switch (key) 
  {
    case 0x11: { endNum(); SQRT();        break; }
    case 0x12: { endNum(); EtoX();        break; }
    case 0x13: { endNum(); TENtoX();      break; }
    case 0x14: { endNum(); POW();         break; }
    case 0x15: { endNum(); OneOverX();    break; }
    case 0x16: { endNum(); CHS();         break; }
    case 0x17: { bldNum('7');             break; }
    case 0x18: { bldNum('8');             break; }

    case 0x21: { /* SST  */               break; }
    case 0x22: { /* GTO  */               break; }
    case 0x23: { endNum(); SIN();         break; }
    case 0x24: { endNum(); COS();         break; }
    case 0x25: { endNum(); TAN();         break; }
    case 0x26: { endNum(); EEX();         break; }
    case 0x27: { bldNum('4');             break; }
    case 0x28: { bldNum('5');             break; }

    case 0x31: { /* R/S   */              break; }
    case 0x32: { /* GSB   */              break; }
    case 0x33: { endNum(); rollDown();    break; }
    case 0x34: { endNum(); swapXY();      break; }
    case 0x35: { bldNum('Z');             break; }
    case 0x36: { endNum(); doEnter();     break; }
    case 0x37: { bldNum('1');             break; }
    case 0x38: { bldNum('2');             break; }

    case 0x41: { /* ON   */               break; }
    case 0x42: { endNum(); makeShiftF();  break; }
    case 0x43: { endNum(); makeShiftG();  break; }
    case 0x44: { endNum(); STO();         break; }
    case 0x45: { endNum(); RCL();         break; }
    case 0x46: { endNum(); doEnter();     break; }
    case 0x47: { bldNum('0');             break; }
    case 0x48: { bldNum('.');             break; }

    case 0x51: { bldNum('9');             break; }
    case 0x52: { endNum(); DIVIDE();      break; }
    case 0x53: { bldNum('6');             break; }
    case 0x54: { endNum(); MULTIPLY();    break; }
    case 0x55: { bldNum('3');             break; }
    case 0x56: { endNum(); SUBTRACT();    break; }
    case 0x57: { endNum(); sigmaPlus();   break; }
    case 0x58: { endNum(); ADD();         break; }

    default: { break; }
  }
}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in shiftF state
 * ------------------------------------------------------------------------- */
void handleShiftF() 
{
  switch (key) 
  {

    case 0x11: { /*   A   */              break; }
    case 0x12: { /*   B   */              break; }
    case 0x13: { /*   C   */              break; }
    case 0x14: { /*   D   */              break; }
    case 0x15: { /*   E   */              break; }
    case 0x16: { /*MATRIX */              break; }
    case 0x17: { endNum(); FIX();         break; }
    case 0x18: { /* SCI   */              break; }

    case 0x21: { /* LBL   */              break; }
    case 0x22: { /* HYP   */              break; }
    case 0x23: { /* DIM   */              break; }
    case 0x24: { /* (i)   */              break; }
    case 0x25: { /* I     */              break; }
    case 0x26: { /*RESULT */              break; }
    case 0x27: { /* X><   */              break; }
    case 0x28: { /* DSE   */              break; }

    case 0x31: { endNum(); PSE();         break; }
    case 0x32: { endNum(); clearStats();  break; }
    case 0x33: { /* CL PGM*/              break; }
    case 0x34: { endNum(); clearRegs();   break; }
    case 0x35: { endNum(); clearShiftState();  break; }
    case 0x36: { endNum(); doRandom();    break; }
    case 0x37: { endNum(); toRectlr();    break; }
    case 0x38: { /* =>HMS */              break; }

    case 0x41: { /* ON    */              break; }
    case 0x42: { endNum(); makeShiftF();  break; }
    case 0x43: { endNum(); makeShiftG();  break; }
    case 0x44: { endNum(); FRAC();        break; }
    case 0x45: { /* USER  */              break; }
    case 0x46: { endNum(); doRandom();    break; }
    case 0x47: { endNum(); FAC();         break; }
    case 0x48: { endNum(); linEstim();    break; }

    case 0x51: { /* ENG  */               break; }
    case 0x52: { /* SOLVE*/               break; }
    case 0x53: { /* ISG  */               break; }
    case 0x54: { /* f XY */               break; }
    case 0x55: { endNum(); toRAD();       break; }
    case 0x56: { /* Re Im*/               break; }
    case 0x57: { endNum(); linRegr();     break; }
    case 0x58: { endNum(); permu();       break; }

    default: { break; }
  }
  clearShiftState();
}


/* ------------------------------------------------------------------------- *
 *                                              Handle keys in shiftF state
 * ------------------------------------------------------------------------- */
void handleShiftG() 
{
  switch (key) 
  {

    case 0x11: { endNum(); SQRT();        break; }
    case 0x12: { endNum(); LOG();         break; }
    case 0x13: { endNum(); LOG10();       break; }
    case 0x14: { endNum(); PERCENT();     break; }
    case 0x15: { endNum(); DIFPERC();     break; }
    case 0x16: { endNum(); ABS();         break; }
    case 0x17: { endNum(); DEG();         break; }
    case 0x18: { endNum(); RAD();         break; }

    case 0x21: { /* BST   */              break; }
    case 0x22: { /* HYP-1 */              break; }
    case 0x23: { endNum(); ASIN();        break; }
    case 0x24: { endNum(); ACOS();        break; }
    case 0x25: { endNum(); ATAN();        break; }
    case 0x26: { endNum(); push(PI);      break; }
    case 0x27: { /* SF    */              break; }
    case 0x28: { /* CF    */              break; }

    case 0x31: { /* P/R   */              break; }
    case 0x32: { /* RTN   */              break; }
    case 0x33: { endNum(); rollUp();      break; }
    case 0x34: { /* RND   */              break; }
    case 0x35: { endNum(); CLX();         break; }
    case 0x36: { endNum(); lstX();        break; }
    case 0x37: { endNum(); toPolar();     break; }
    case 0x38: { /* => H  */              break; }

    case 0x41: { /* ON    */              break; }
    case 0x42: { endNum(); makeShiftF();  break; }
    case 0x43: { endNum(); makeShiftG();  break; }
    case 0x44: { endNum(); doInt();       break; }
    case 0x45: { /* MEM   */              break; }
    case 0x46: { endNum(); lstX();        break; }
    case 0x47: { endNum(); meanValues();  break; }
    case 0x48: { endNum(); stdDev();      break; }

    case 0x51: { endNum(); GRD();         break; }
    case 0x52: { /* X<=Y  */              break; }
    case 0x53: { /* F?    */              break; }
    case 0x54: { /* x = 0 */              break; }
    case 0x55: { endNum(); toDEG();       break; }
    case 0x56: { /* TEST  */              break; }
    case 0x57: { endNum(); sigmaMinus();  break; }
    case 0x58: { endNum(); combi();       break; }


    default: { break; }
  }
  clearShiftState();
}


/* ------------------------------------------------------------------------- *
 *                                                          Handle Enter key
 * ------------------------------------------------------------------------- */
void doEnter() 
{
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
void doRandom() 
{                                                 // Random Number
  saveLastX(); 
  randomSeed(analogRead(A7));
  push( (double)random(2147483647) / 2147483647 );
}

void FRAC() 
{                                                 // x = x - int(x)
  saveLastX(); 
  stack[X] = stack[X] - int(stack[X]);
}

void doInt() 
{                                                 // x = int(x)
  saveLastX(); 
  stack[X] = int(stack[X]);
}

                                                  // Square root
void SQRT() { saveLastX(); stack[X] = sqrt(stack[X]); }


                                                  // Square
void SQ()   { saveLastX(); stack[X] = sq(stack[X]); }


                                                  // 1 / X
void OneOverX() { saveLastX(); stack[X] = 1 / stack[X]; }


                                                  // Change Sign
void CHS() { saveLastX(); stack[X] = -1 * stack[X]; }


                                                  // Absolute value
void ABS() { saveLastX(); stack[X] = abs(stack[X]); }


void POW()                                        // Y to the power of X
{
  saveLastX(); 
  twoNums( pow(stack[Y], stack[X]));
}


void PERCENT()                                    // Take X percent of Y
{
  saveLastX(); 
  stack[X] = stack[Y] * stack[X] / 100;
}


void DIFPERC()                                    // Take delta X percent of Y
{
  saveLastX(); 
  stack[X] = ( stack[X] - stack[Y] ) *100 / stack[Y];
}


void ADD()                                        // Add
{
  saveLastX(); 
  twoNums( stack[Y] + stack[X] );
}


void SUBTRACT()                                   // Subtract
{
  saveLastX(); 
  twoNums( stack[Y] - stack[X] );
}


void MULTIPLY()                                   // Multiply
{
  saveLastX(); 
  twoNums( stack[Y] * stack[X] );
}


void DIVIDE()                                     // Divide
{
  saveLastX(); 
  twoNums( stack[Y] / stack[X] );
}


void twoNums(double temp) 
{
  stack[X] = stack[T];
  rollDown();
  stack[X] = temp;
}


void FAC()                                        // Factorial
{
  saveLastX(); 
  double x = int(stack[X]);
  double f = factorial(x);
  if (x <=170)                                    // Check overflow
  {
    stack[X] = f;
  }
}


double factorial(double f) 
{
  double r = 1;
  if (f <=170)                                    // Check overflow
  {
    for (double i=2; i<=f; i++) 
    {
      r = r * i; 
    }
  }
    else 
  {
    LCD_display(display, 3, 9, "ovf");
    r = 0;
  }
  return(r);
}

/* ------------------------------------------------------------------------- *
 *                                                     Goniometric functions
 * ------------------------------------------------------------------------- */
void toRAD()                                      // convert degrees to Radians
{
  saveLastX(); 
  stack[X] = stack[X]*2*PI/360;
}


void toDEG()                                      // convert Radians to degrees
{
  saveLastX(); 
  stack[X] = stack[X]*360/(2*PI);
}


void toRectlr()                                   // Polar to Rectangular
{
  double rho = stack[Y];
  double r   = stack[X];
  switch (gonioState) 
  {
    case statDEG:
      stack[Y] = r*sin(rho*2*PI/360);             // Degrees to radians
      stack[X] = r*cos(rho*2*PI/360);             // Degrees to radians
      break;
    case statGRD:
      stack[Y] = r*sin(rho*2*PI/400);             // Gradians to radians
      stack[X] = r*cos(rho*2*PI/400);             // Gradians to radians
      break;
    case statRAD:
      stack[X] = r*sin(rho);
      stack[X] = r*cos(rho);
      break;
    default:
      break;
  }
}


void toPolar()                                    // Rectangular to Polar
{
  double y = stack[Y];
  double x = stack[X];
  switch (gonioState) 
  {
    case statDEG:
      stack[Y] = atan(y/x) * 360 / (2*PI);        // rho in degrees
      stack[X] = sqrt(x*x + y*y);                 // r, Pythagoras
      break;
    case statGRD:
      stack[Y] = atan(y/x) * 400 / (2*PI);        // rho in gradians
      stack[X] = sqrt(x*x + y*y);                 // r, Pythagoras
      break;
    case statRAD:
      stack[Y] = atan(y/x);                       // rho in radians
      stack[X] = sqrt(x*x + y*y);                 // r, Pythagoras
      break;
    default:
      break;
  }
}


void SIN()                                        // Sine
{ saveLastX(); 
  switch (gonioState) 
  {
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


void ASIN()                                       // Arc Sine
{
  saveLastX(); 
  switch (gonioState) 
  {
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


void COS()                                        // Cosine
{
  saveLastX(); 
  switch (gonioState) 
  {
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


void ACOS()                                       // Arc Cosine
{
  saveLastX(); 
  switch (gonioState) 
  {
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


void TAN()                                        // Tangent
{
  saveLastX(); 
  switch (gonioState) 
  {
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


void ATAN()                                       // Arc Tangent
{
  saveLastX(); 
  switch (gonioState) 
  {
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
void clearStats()                                 // Clear statistic registters
{
  for (int i=statRegLo; i<=statRegHi; Reg[i++] = 0);
}


void sigmaPlus()                                  // Add number(s) to statistics
{
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


void sigmaMinus()                                 // Correct last statistic entry
{
  saveLastX(); 
  double x = stack[X];
  double y = stack[Y];

  Reg[statRegLo]   = Reg[statRegLo]   - 1;        // Number of data points
  Reg[statRegLo+1] = Reg[statRegLo+1] - x;        // Summation of X values
  Reg[statRegLo+2] = Reg[statRegLo+2] - (x*x);    // Summation of X squares
  Reg[statRegLo+3] = Reg[statRegLo+3] - y;        // Summation of Y values
  Reg[statRegLo+4] = Reg[statRegLo+4] - (y*y);    // Summation of Y squares
  Reg[statRegLo+5] = Reg[statRegLo+5] - (x*y);    // Summation of x*y products

  stack[X] = (double)Reg[statRegLo];
}


void meanValues()                                 // Calculate mean
{
  saveLastX(); 
  push( Reg[statRegLo+3] / Reg[statRegLo] );
  push( Reg[statRegLo+1] / Reg[statRegLo] );
}


void stdDev()                                     // Standard deviation
{
  double n = Reg[statRegLo];                      // n = number of entries

  double M = ( n * Reg[statRegLo+2] ) - ( Reg[statRegLo + 1] * Reg[statRegLo + 1] ); // M = nEx2 - (Ex)2
  double N = ( n * Reg[statRegLo+4] ) - ( Reg[statRegLo + 3] * Reg[statRegLo + 3] ); // N = nEy2 - (Ey)2

  double sX = sqrt( M / ( n * ( n - 1 ) ) );      // sX = SQR( M / n(n-1) )
  double sY = sqrt( N / ( n * ( n - 1 ) ) );      // sY = SQR( N / n(n-1) )

  push(sY);                                       // push them on the stack
  push(sX);
}


void linRegr()                                    // Linear Regression
{
  double n = Reg[statRegLo];                      // n = number of entries

  double M = ( n * Reg[statRegLo+2] ) - ( Reg[statRegLo + 1] * Reg[statRegLo + 1] ); // M = nEx2 - (Ex)2
  double N = ( n * Reg[statRegLo+4] ) - ( Reg[statRegLo + 3] * Reg[statRegLo + 3] ); // N = nEy2 - (Ey)2
  double P = ( n * Reg[statRegLo+5] ) - ( Reg[statRegLo + 1] * Reg[statRegLo + 3] ); // P = nExy - ExEy

  double A = P / M;
  double B = ( ( ( M * Reg[statRegLo+3] ) - ( P *  Reg[statRegLo+1] ) ) / ( n * M ) );

  push(A);                                       // push them on the stack
  push(B);
}


void linEstim()                                   // Linear Estimation of X
{
  double n = Reg[statRegLo];                      // n = number of entries

  double M = ( n * Reg[statRegLo+2] ) - ( Reg[statRegLo + 1] * Reg[statRegLo + 1] ); // M = nEx2 - (Ex)2
  double N = ( n * Reg[statRegLo+4] ) - ( Reg[statRegLo + 3] * Reg[statRegLo + 3] ); // N = nEy2 - (Ey)2
  double P = ( n * Reg[statRegLo+5] ) - ( Reg[statRegLo + 1] * Reg[statRegLo + 3] ); // P = nExy - ExEy

  double r = P / sqrt(M * N);
  double yVal = ( ( M * Reg[statRegLo+3] ) + (P * ( n * stack[X] - Reg[statRegLo+1] ) ) )
                / 
                (  n * M );

  push(r);                                        // push them on the stack
  push(yVal);
}


void permu()                                      // Permutions Py,x
{
  double x = stack[X];
  double y = stack[Y];
  double Pxy = factorial(y) / factorial(y - x);
  rollDown();
  stack[X] = Pxy;
}


void combi()                                      // Combinations Cy,x
{
  double x = stack[X];
  double y = stack[Y];
  double Cxy = factorial(y) / ( factorial(x) * factorial(y - x) );
  rollDown();
  stack[X] = Cxy;
}


/* ------------------------------------------------------------------------- *
 *                                             Programming related functions
 * ------------------------------------------------------------------------- */
void PSE()                                        // Pause program 1 second
{
  delay(1000);
}


/* ------------------------------------------------------------------------- *
 *                                                      Calculator functions
 * ------------------------------------------------------------------------- */
void clearRegs()                                  // Clear registters
{
  for (int i=0; i<30; Reg[i++] = 0);
}


void saveLastX()                                  // Save X to lastX
{
  lastX = stack[X];
}


void lstX()                                       // Get lastX to X
{
  rollUp();
  stack[X] = lastX;
}


void EEX()                                        // Specify exponent
{
  bool positive = true;
  bool goOn = true;
  String numString = "";
  int l = 0;
  int exp = 0;
                                                  // get max two digits
                                                  // display them at 3,18 and 3,19
                                                  // allow for BSP
                                                  // if CHS pressed change the sign at 3,17
                                                  // enter pressed?
                                                  //    build number
                                                  //      recalculate X
                                                  //        return

  do 
  {
    l = numString.length();
    key = keypad.getKey();                        // Read key from keypad

    if (key != 0) 
    {
      switch (key)
      {
        case 0x16:                                // CHS
        {
          positive = !positive;
          if (positive) 
          {
            LCD_display(display, 2,17, " " );
          } else{
            LCD_display(display, 2,17, "-" );
          }
          break;
        }

        case 0x35:                                // BSP
        {
          if (l > 0)
          {
            numString = numString.substring(0, l - 1 );
            LCD_display(display, 2,18, "  " );
            LCD_display(display, 2,18, numString );
          }
          break;
        }

        case 0x36:                                // ENTER
        case 0x46:                                // ENTER
        {
          exp = numString.toDouble();
          if ( !positive ) exp = -1 * exp;
          stack[X] = stack[X] * pow( 10, exp );
          goOn = false;
        }

        case 0x47: if (l<2) numString.concat("0"); break;
        case 0x37: if (l<2) numString.concat("1"); break;
        case 0x38: if (l<2) numString.concat("2"); break;
        case 0x55: if (l<2) numString.concat("3"); break;
        case 0x27: if (l<2) numString.concat("4"); break;
        case 0x28: if (l<2) numString.concat("5"); break;
        case 0x53: if (l<2) numString.concat("6"); break;
        case 0x17: if (l<2) numString.concat("7"); break;
        case 0x18: if (l<2) numString.concat("8"); break;
        case 0x51: if (l<2) numString.concat("9"); break;

        default:                                   break;
      }

      LCD_display(display, 2,18, "  " );
      LCD_display(display, 2,18, numString );
    }
  } while ( goOn );

  return;

}


void STO()                                        // Store X in register
{
  Reg[getReg()] = stack[X];
}



void RCL()                                        // Recall X from register
{
  saveLastX(); 
  rollUp();
  stack[X] = Reg[getReg()];
}


void CLX()                                        // Clear X register
{
  saveLastX(); 
  stack[X] = 0;
}


//                                                --------------------------
//                                                Display state settings
//                                                --------------------------


void FIX()                                        // set # of digits
{
  int val = 99;
  val = getOneNum();
  if (val >= 0 && val <= 9) { precision = val; }
}


int getOneNum()                                   // get one digit
{
  int val = 99;
  do 
  {
    key = keypad.getKey();                        // Read key from keypad

    switch (key) 
    {
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
      default:            break;
    }

  } while ( val == 99 );

  LCD_display(display, 3,13, F("fix") );

  return val;
}


int getReg()                                      // get Register
{
  int val = 0;
  bool getDone = false;
  do 
  {
    key = keypad.getKey();                        // Read key from keypad

    switch (key) {
      case 0x48: val += 10;                 break;  // decimal point: offset 10
      case 0x47: val +=  0; getDone = true; break;  // add values 0 - 9
      case 0x37: val +=  1; getDone = true; break;
      case 0x38: val +=  2; getDone = true; break;
      case 0x55: val +=  3; getDone = true; break;
      case 0x27: val +=  4; getDone = true; break;
      case 0x28: val +=  5; getDone = true; break;
      case 0x53: val +=  6; getDone = true; break;
      case 0x17: val +=  7; getDone = true; break;
      case 0x18: val +=  8; getDone = true; break;
      case 0x51: val +=  9; getDone = true; break;
      default: break;
    }

  } while ( getDone == false );

  return val;

}


//                                                --------------------------
//                                                Goniometric state settings
//                                                --------------------------

void DEG()                                        // Status to Degrees
{
  gonioState = statDEG; 
  LCD_display(display, 3,17, F("deg") );
}


void RAD()                                        // Status to Radians
{
  gonioState = statRAD; 
  LCD_display(display, 3,17, F("rad") );
}


void GRD()                                        // Status to Gradians
{
  gonioState = statGRD; 
  LCD_display(display, 3,17, F("grd") );
}


//                                                --------------------------
//                                                Stack manipulations
//                                                --------------------------

void swapXY()                                     // Swap X and Y
{
  double f = stack[X];
  stack[X] = stack[Y];
  stack[Y] = f;
}


void push(double val)                             // Push value on stack
{
  rollUp();
  stack[X] = val;
}


void rollDown()                                   // Roll stack down
{
  double f = stack[X];
  stack[X] = stack[Y];
  stack[Y] = stack[Z];
  stack[Z] = stack[T];
  stack[T] = f;
}


void rollUp()                                     // Roll stack up
{
  double f = stack[T];
  stack[T] = stack[Z];
  stack[Z] = stack[Y];
  stack[Y] = stack[X];
  stack[X] = f;
}


/* ------------------------------------------------------------------------- *
 *                                                     Shift state functions
 * ------------------------------------------------------------------------- */
void makeShiftF() 
{
debugln("Making shift state F");
  shiftState = shiftF;
  LCD_display(display, 3, 7, F("f") );
}


void makeShiftG() 
{
debugln("Making shift state G");
  shiftState = shiftG;
  LCD_display(display, 3, 7, F("g") );
}


void clearShiftState() 
{
  shiftState = noShift;
  LCD_display(display, 3, 7, F(" ") );
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
void showStack() 
{
  String myString;

  myString = "Y: ";
//  myString.concat(String(stack[Y], precision));
  myString.concat( numMakeup(stack[Y]) );
  myString.concat(F("                  "));
  LCD_display(display, 1, 0, myString.substring(0,20) );

  myString = "X: ";
//  myString.concat(String(stack[X], precision));
  myString.concat( numMakeup(stack[X]) );
  myString.concat(F("                  "));
  LCD_display(display, 2, 0, myString.substring(0,20) );
}


/* ------------------------------------------------------------------------- *
 *    routine to format number depending on mode and precision - numMakeup()
 * ------------------------------------------------------------------------- */
String numMakeup(double n) 
{
  String numString = "";
  double expn;
  double newNum = 0;

  if (n == 0.0) 
  {
    expn = 0.0;
  } 
    else 
  {
    expn = int( log10( abs(n) ) );
  }

debug("n    = "); debugln(n);
debug("expn = "); debugln(expn);

  switch (dispState) {

    case dispFix:                                 // FIX diaplay
      if (expn > 9.0)
      {
         newNum = (double)( n / pow(10.0, expn) );
debug(" newNum = "); debugln(String(newNum,9));
         numString.concat( String(newNum, precision) );
         numString.concat( F("             "));
         numString = numString.substring(0, 14);
         numString.concat( String( (int)expn) );

      } 
        else if (expn < (-9.0) ) 
      {

         newNum = (double)( n * pow(10.0, abs(expn) ) );
debug(" newNum = "); debugln(String(newNum,9));
         numString.concat( String(newNum, precision) );
         numString.concat( F("             "));
         numString = numString.substring(0, 14);
         numString.concat( String( (int)expn) );

      }
        else 
      {

debug("n = "); debugln(n);
         numString.concat( String(n, precision) );
         numString.concat( F("             "));
         numString = numString.substring(0, 17);
      }
      break;

    case dispSci:                                 // SCI diaplay
      break;
    
    case dispEng:                                 // ENG diaplay
      break;
    
    default:
      break;
    
  }

  return numString;
}


/* ------------------------------------------------------------------------- *
 *         Routine to display stuff on the display of choice - LCD_display()
 * ------------------------------------------------------------------------- */
void LCD_display(LiquidCrystal_I2C screen, int row, int col, String text) 
{
  screen.setCursor(col, row);
  screen.print(text);
}
