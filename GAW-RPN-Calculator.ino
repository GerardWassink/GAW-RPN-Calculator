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
 *                                                         Include libraries
 * ------------------------------------------------------------------------- */
#include <Wire.h>                       // I2C comms library
#include <LiquidCrystal_I2C.h>          // LCD library

#include <String.h>                     // String library

#include <math.h>                       // Arduino math library

/* ------------------------------------------------------------------------- *
 *                                                            Create objects
 * ------------------------------------------------------------------------- */
LiquidCrystal_I2C display(0x26,20,4);   // Instantiate display object


/* ------------------------------------------------------------------------- *
 *                                                     Goniometric variables
 * ------------------------------------------------------------------------- */
const unsigned int statDEG=0;
const unsigned int statRAD=1;
const unsigned int statGRD=2;
unsigned int gonioStatus = statDEG;     // default to degrees

double E = 2.718281828459045;

/* ------------------------------------------------------------------------- *
 *                                               Variables for X, Y, Z and T
 * ------------------------------------------------------------------------- */
double stack[4] = {0, 0, 0, 0};
int X=0, Y=1, Z=2, T=3;


/* ------------------------------------------------------------------------- *
 *                                                      Calculator variables
 * ------------------------------------------------------------------------- */
int precision = 4;


/* ------------------------------------------------------------------------- *
 *                                                                    loop()
 * ------------------------------------------------------------------------- */
void loop() {
  // put your main code here, to run repeatedly:

}


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
  LCD_display(display, 0, 0, F("GAW RPN calc "));
  String myString = "v";
  myString.concat(progVersion);
  myString.concat("          ");
  LCD_display(display, 0, 13, myString.substring(0,20) );

  //
  // Leave intro screen for 3 seconds
  //
  LCD_display(display, 2, 0, "Initializing " );
  for (int t=13; t<16; t++) {
    LCD_display(display, 2, t, "." );
    delay(1000);
  }


// -----------------------------------
// ------------ TEST AREA ------------
#if DEBUG == 1
  showStack();

  FIX(9);
  showStack();

  push(1);
  showStack();

  push(2);
  showStack();

  push(3);
  showStack();

  push(4);
  showStack();

  ADD();
  showStack();

  SUBTRACT();
  showStack();

  MULTIPLY();
  showStack();

  DIVIDE();
  showStack();

  push(3);
  showStack();

  push(2);
  showStack();

  POW();
  showStack();

  OneOverX();
  showStack();

  CHS();
  showStack();

  ABS();
  showStack();

#endif
// ------------ TEST AREA ------------
// -----------------------------------

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

void POW()  {                                     // X to the power of Y
  double temp = pow(stack[Y], stack[X]);
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
      stack[X] = sin(stack[X]*2*PI/360);
      break;
    case statGRD:
      stack[X] = sin(stack[X]*2*PI/400);
      break;
    case statRAD:
      stack[X] = sin(stack[X]);
      break;
    default:
      break;
  }
}

void COS() {                                      // Cosine
  double angle;
  switch (gonioStatus) {
    case statDEG:
      stack[X] = cos(stack[X]*2*PI/360);
      break;
    case statGRD:
      stack[X] = cos(stack[X]*2*PI/400);
      break;
    case statRAD:
      stack[X] = cos(stack[X]);
      break;
    default:
      break;
  }
}

void TAN() {                                      // Tangent
  double angle;
  switch (gonioStatus) {
    case statDEG:
      stack[X] = tan(stack[X]*2*PI/360);
      break;
    case statGRD:
      stack[X] = tan(stack[X]*2*PI/400);
      break;
    case statRAD:
      stack[X] = tan(stack[X]);
      break;
    default:
      break;
  }
}


/* ------------------------------------------------------------------------- *
 *                                                      Calculator functions
 * ------------------------------------------------------------------------- */
void FIX(int val) { precision = val; }

void DEG() { gonioStatus = statDEG; }             // Calc in degrees
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

void rollUp() {                                 // Roll stack up
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
  myString.concat("                    ");
  LCD_display(display, 1, 0, myString.substring(0,20) );

  myString = "X: ";
  myString.concat(String(stack[X], precision));
  myString.concat("                    ");
  LCD_display(display, 2, 0, myString.substring(0,20) );
}


/* ------------------------------------------------------------------------- *
 *         Routine to display stuff on the display of choice - LCD_display()
 * ------------------------------------------------------------------------- */
void LCD_display(LiquidCrystal_I2C screen, int row, int col, String text) {
  screen.setCursor(col, row);
  screen.print(text);
}
