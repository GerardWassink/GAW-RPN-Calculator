
  RAD();								// Mode to Radians

  push(PI);
  showStack(); delay(2000);

  push(4);
  showStack(); delay(2000);

  DIVIDE();
  showStack(); delay(2000);				// show 1/4 PI
  
  SIN();								// compute sine
  showStack(); delay(2000);				// show sine of 1/4 PI in radians
  
  ASIN();								// compute arc sine
  showStack(); delay(2000);				// show arc sine of the sine of 1/4 PI
  
  toDEG();								// convert to degrees (45)
  showStack(); delay(2000);				// show degrees

  DEG();								// Make mode degrees
  
  COS();								// compute cosine
  showStack(); delay(2000);				// show cosine of 1/4 PI in radians
  
  ACOS();								// compute arc cosine
  showStack(); delay(2000);				// show arc cosine of 1/4 PI
  
  TAN();								// compute tangent
  showStack(); delay(2000);				// show tangent of 1/4 PI in radians
  
  ATAN();								// compute arc tangent
  showStack(); delay(2000);				// show arc tangent of 1/4 PI
  

  
