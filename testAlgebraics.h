  FIX(15);

  push(7);
  push(7);
  showStack(); delay(200);

  SQRT();
  showStack(); delay(200);				// Show square root

  SQ();
  showStack(); delay(200);				// Show square again (7)
  
  SUBTRACT();
  showStack(); delay(3000);				// Should be zero
  
  push(10);
  push(PI);
  showStack(); delay(200);

  POW();
  showStack(); delay(3000);				// Show 100 to power PI

  push(50);
  push(50);
  push(PI);
  showStack(); delay(200);

  PERCENT();
  showStack(); delay(200);				// PI percent of 50

  push(100);
  MULTIPLY();
  push(PI);
  DIVIDE();
  showStack(); delay(200);				// Should be zero

  SUBTRACT();
  showStack(); delay(3000);				// Should be zero

  push(PI);
  push(PI);
  showStack(); delay(200);				// show one over PI

  OneOverX();
  showStack(); delay(200);				// show one over PI

  OneOverX();
  showStack(); delay(200);				// show PI again
  
  SUBTRACT();
  showStack(); delay(3000);				// Should be zero

  push(PI);
  push(PI);
  showStack(); delay(200);				// show PI
  
  CHS();
  showStack(); delay(200);				// show -1 * PI
  
  CHS();
  showStack(); delay(200);				// show PI again

  SUBTRACT();
  showStack(); delay(3000);				// Should be zero
