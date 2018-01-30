void dispPos()
{
  // all these coordinates and sizes need to be scaled for different size of displays automagically
  //  should determine these values by querying tft and touch - maybe in setup
  // 320 wd X 240  ht display

  roundness = 4;   // orig 14
  spacing = 3;
  smwd = 0;   // wd / 20; //16;    //vert s meter width
  buttonht = ht / 7; // 37;   general height of buttons 26
  buttonwd = ((wd - smwd - 3 * spacing) / 4); // max 4 buttons to the right of s meter with spacing between  76

  // vert Smeter related other coordinates
  smx = 0;
  smy = 0;
  smht = 0;  //ht / 1.2; //200;   // sm height

  // First row of buttons
  firstrowy = 3;   //ht/48 ;   //5 y pos of 1st row
  // VFO button related
  vfox = smwd + spacing;   //wd /16 ;  // 20; top of screen
  vfoy = firstrowy;    //ht/48; // 5;
  vfowd = buttonwd ;  // wd/4.2; //  75;
  vfoht = buttonht;

  // MEM button related
  memx = vfox + vfowd + spacing;   //110; 5 px space
  memy = firstrowy;    //5;
  memwd = 2 * buttonwd;    //135; reduce spacing 5
  memht = buttonht;    //37;

  // Tx Rx box
  txrx = memx + memwd + spacing; // 260;
  txry = firstrowy;    //5;
  txrwd = buttonwd;    //50;
  txrht = buttonht;    //37;

  // frequeny box  Second Row
  frqx = smwd + 35;  //50;
  frqy = firstrowy + buttonht + 5; // 45;
  frqwd = 3.5 * buttonwd;   // 264;
  frqht = 1.3 * buttonht ;    //40;

  vfoABMx = smwd + spacing;  // 25 where A/B or M is displayed
  vfoABMy = frqy + 8;

  frq2x1 = vfoABMx + spacing; // next line to display other freqs
  frq2x2 = vfoABMx + wd / 2 - 5; // x pos for 2nd freq display
  frq2y = vfoABMy + frqht + 1;   // second display of VFO /mem

  // band button  Third Row  : below freq2 display row
  bandx = smwd + spacing;    //20;
  bandy = frq2y + 20;     // 113; 2 for freq2 displ
  bandwd = 1.4 * buttonwd;    //89;
  bandht = buttonht;  //37;

  //step button
  stpx = bandx + bandwd + spacing;  // 114;
  stpy = bandy;     //113
  stpwd = 1.6 *  buttonwd  ;   //124;
  stpht = buttonht;   //37;
  // sideband button
  sbx = stpx + stpwd + spacing ;   //243;
  sby = bandy;         //113;
  sbwd =  buttonwd;  //69;
  sbht = buttonht;   //37;

  // vfo < > mem button   Fourth Row
  vmx = smwd + spacing;  //20;
  vmy = bandy + bandht + 2;   //152;
  vmwd = 1.4 * buttonwd;     //89;  ??
  vmht = buttonht;    //37;

  // BFO button
  bfox = vmx + vmwd + spacing; //114,
  bfoy = vmy;  //152,
  bfowd = 1.6 * buttonwd ; //124,
  bfoht = buttonht;  //37;
  // save button
  svx = bfox + bfowd + spacing; // 243,
  svy = vmy;        //152,
  svwd =  buttonwd; // 69,
  svht = buttonht; // 37;

  // F1 button Tx timeout button
  f1x = smwd + spacing;   ;   //20,
  f1y = svy + svht + 2; //192,
  f1wd = 1.4 * buttonwd;   //70,
  f1ht = buttonht;  //28;

  // F2 button for bfo2
  f2x = f1x + f1wd + spacing; //95,
  f2y = f1y;         //192,
  f2wd = 1.6 * buttonwd; //70,long
  f2ht = buttonht; //28;

  // F3 button
  f3x = f2x + f2wd + spacing; // 165,
  f3y = f1y; // 192,
  f3wd = buttonwd; //70,
  f3ht = buttonht; //28;

 /* //F4 button not in ubitx
  f4x = f3x + f3wd + spacing; // 240,
  f4y = f1y;        //192,
  f4wd = buttonwd;  //70,
  f4ht = buttonht;  //28; */

  // BOT MESSAGE STRIP /S Meter
  botx = smwd + 5;
  boty = f3y + f3ht; //220,
  botwd = wd -botx-50;  //320, 30 for ver display
  botht = ht - (f3y + f3ht + 1); // 20

}

