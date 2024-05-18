  clearStats();

  push(4.63);
  push(0);
  sigmaPlus();
  showStack();
  debugln();
  debugln("statistic registers:"); 
  debug("reg 2: "); debugln(Reg[2]);
  debug("reg 3: "); debugln(Reg[3]);
  debug("reg 4: "); debugln(Reg[4]);
  debug("reg 5: "); debugln(Reg[5]);
  debug("reg 6: "); debugln(Reg[6]);
  debug("reg 7: "); debugln(Reg[7]);

  push(4.78);
  push(20);
  sigmaPlus();
  showStack();
  debugln();
  debugln("statistic registers:"); 
  debug("reg 2: "); debugln(Reg[2]);
  debug("reg 3: "); debugln(Reg[3]);
  debug("reg 4: "); debugln(Reg[4]);
  debug("reg 5: "); debugln(Reg[5]);
  debug("reg 6: "); debugln(Reg[6]);
  debug("reg 7: "); debugln(Reg[7]);
  
  push(6.61);
  push(40);
  sigmaPlus();
  showStack();
  debugln();
  debugln("statistic registers:"); 
  debug("reg 2: "); debugln(Reg[2]);
  debug("reg 3: "); debugln(Reg[3]);
  debug("reg 4: "); debugln(Reg[4]);
  debug("reg 5: "); debugln(Reg[5]);
  debug("reg 6: "); debugln(Reg[6]);
  debug("reg 7: "); debugln(Reg[7]);
  
  push(7.21);
  push(60);
  sigmaPlus();
  showStack();
  debugln();
  debugln("statistic registers:"); 
  debug("reg 2: "); debugln(Reg[2]);
  debug("reg 3: "); debugln(Reg[3]);
  debug("reg 4: "); debugln(Reg[4]);
  debug("reg 5: "); debugln(Reg[5]);
  debug("reg 6: "); debugln(Reg[6]);
  debug("reg 7: "); debugln(Reg[7]);

  push(7.78);
  push(80);
  sigmaPlus();
  showStack();
  debugln();
  debugln("statistic registers:"); 
  debug("reg 2: "); debugln(Reg[2]);
  debug("reg 3: "); debugln(Reg[3]);
  debug("reg 4: "); debugln(Reg[4]);
  debug("reg 5: "); debugln(Reg[5]);
  debug("reg 6: "); debugln(Reg[6]);
  debug("reg 7: "); debugln(Reg[7]);

  debugln("mean values:");
  meanValues();
  showStack();

  debugln("standard deviation:");
  stdDev();
  showStack();

  debugln("linear regression:");
  linRegr();
  showStack();

  push(70);
  debugln("linear estimation:");
  linEstim();
  showStack();

  push(5);
  push(3);
  debugln("permutations Py,x:");
  permu();
  showStack();

  push(52);
  push(4);
  debugln("combinations Cy,x:");
  combi();
  showStack();

  
