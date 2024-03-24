# GAW-RPN-Calculator
 HP like RPN calculator on Arduino, based on the HP-15C. No programming capabilities yet, perhaps in a future expansion.

 The project was built on an Arduino NANO ESP32, using a 20x4 I2C LCD display. 
 It needs a key matrix of 4 rows by 8 columns.
 

## Functions

### Logaritmic

- LOG10()         // 10 log OF X
- TENtoX()        // 10 TO THE POWER OF X
- LOG()           // natural log OF X
- EtoX()          // e TO THE POWER OF X

### Algebraic

- SQRT()          // Square root
- SQ()            // Square
- OneOverX()      // 1 / X
- CHS()           // Change Sign
- ABS()           // Absolute value
- POW()           // Y to the power of X
- INT()           // Make X = INT(X)
- ADD()           // Add X to Y
- SUBTRACT()      // Subtract Y from X
- MULTIPLY()      // Multiply Y by X
- DIVIDE()        // Divide Y by X
- PERCENT()       // Take X perecnt of Y
- RANDOM()        // Generate random number

### Goniometric

- toRAD()         // degrees to radians
- toDEG()         // radians to degrees
- DEG()           // switch mode to Degrees
- RAD()           // switch mode to Radians
- GRD()           // switch mode to Gradians
- SIN()           // Sine of X
- COS()           // Cosine of X
- TAN()           // Tangent of X
- ASIN()          // Arcsine of X
- ACOS()          // Arccosine of X
- ATAN()          // Arctangent of X

### Statistics

- clearStats()    // Clear statistic registers

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

