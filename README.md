# GAW-RPN-Calculator
 HP like RPN calculator on Arduino, based on the HP-15C. No programming capabilities yet, perhaps in a future expansion.

 The project was built on an Arduino NANO ESP32, using a 20x4 I2C LCD display. 
 It needs a keyboard of 4 rows by 10 columns. 
 
 We use a matrix of 5 rows and 8 columns. 
 The fifth row handles the eigth buttons in the two columns on the far right, thus saving one pin.
 

## Functions

### Logaritmic

- LOG10()         // 10 log OF X
- TENtoX()        // 10 TO THE POWER OF X
- LOG()           // natural log OF X
- EtoX()          // e TO THE POWER OF X

### Algebraic

- SQRT()          // Square root
- SQ()            // Square
- POW()           // Y to the power of X
- PERCENT()       // Take X perecnt of Y
- OneOverX()      // 1 / X
- CHS()           // Change Sign
- ABS()           // Absolute value
- INT()           // Make X = INT(X)
- RANDOM()        // Generate random number
- ADD()           // Add X to Y
- SUBTRACT()      // Subtract Y from X
- MULTIPLY()      // Multiply Y by X
- DIVIDE()        // Divide Y by X

### Goniometric

- DEG()           // switch mode to Degrees
- RAD()           // switch mode to Radians
- GRD()           // switch mode to Gradians
- SIN()           // Sine of X
- ASIN()          // Arcsine of X
- COS()           // Cosine of X
- ACOS()          // Arccosine of X
- TAN()           // Tangent of X
- ATAN()          // Arctangent of X
- toRAD()         // degrees to radians
- toDEG()         // radians to degrees

### Statistics

- clearStats()    // Clear statistic registers
- sigmaPlus()     // Accumulate datapoints
- sigmaMinus()    // Correct datapoints


### Calculator

- clearReg()      // Clear all 30 registers
- STO()           // Store value in register
- RCL()           // Recall value from register
- CLX()           // Clear X register
- FIX()           // Set precision (default 4)
- swapXY()        // Exchange values of X and Y
- push()          // Push number on stack
- rollDown()      // Roll stack down
- rollUp()        // Roll stack up

