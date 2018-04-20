void display_mem()
{
  tft.setCursor(memx + 75, memy + 5); //(185, 12);
  tft.setTextSize(3);
  tft.setTextColor(GREEN, GREY);
  if (memCh < 10)
    tft.print("0");
  tft.print(memCh);
  if (memCh < 100)
    tft.print(" ");
  if (!xch_M)
    read_ch();
  else
    xch_M = 0;

  if (vfo_M_sel)
  {
    vfo = vfo_M;
    bfo1 = bfo_M;
    display_bfo();
    set_bfo1();
    sideband = sb_M;
    display_sideband();
    display_frequency();
  }
  else
    display_frequency2();
}

void display_mem_msg(String msg)
{
  tft.setCursor(memx + 10, memy + 5); //(185, 12);
  tft.setTextSize(3);
  tft.setTextColor(RED, GREY);
  tft.print(msg);
}

void displ_rx()
{
  tft.drawRoundRect(txrx, txry, txrwd, txrht, roundness, RED);  // TxRx box outline
  tft.fillRoundRect(txrx + 2, txry + 2, txrwd - 4, txrht - 4, roundness - 4, GREEN); //TxRx box
  tft.setCursor(txrx + 10, txry + 5);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.print("Rx");
}
void displ_tx()
{
  tft.drawRoundRect(txrx, txry, txrwd, txrht, roundness, RED);  // TxRx box outline
  tft.fillRoundRect(txrx + 2, txry + 2, txrwd - 4, txrht - 4, roundness - 4, RED); //TxRx box
  tft.setCursor(txrx + 10, txry + 5);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.print("Tx");
}

void display_vfo()
{
  tft.setCursor(vfoABMx, vfoABMy);    //(25, 50);
  tft.setTextSize(4);
  tft.setTextColor(WHITE, BLACK);
  old_band = bnd_count;

  if (vfo_M_sel)
    tft.print("M");  // Mem   ....
  else if (vfo_A_sel)
    tft.print("A");  // VFO A or B  ....
  else
    tft.print("B");  // VFO A or B  ....

  display_frequency2(); // 2nd line of display only when vfos changed
  set_band();  // select and display band according to frequency displayed
  display_band();
}


void display_frequency()
{
  tft.setTextSize(4);
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(frqx + 2, frqy + 8); //(70, 50);
  if (vfo < 10000000)
    tft.print(" ");
  tft.print(vfo / 1000.0, 3);
}

void display_frequency2()
{
  //other 2 vfo's displayed below
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(frq2x1, frq2y);   //(25, 93);

  if (vfo_A_sel)
  {
    tft.print("B ");
    tft.print(vfo_B / 1000.0, 3);
    tft.setCursor(frq2x2, frq2y);   // (170, 93);
    tft.print("M ");
    tft.print(vfo_M / 1000.0, 3);
    tft.print(" "); // takes care of previous leftover digit
  }
  if (vfo_B_sel)
  { tft.print("A ");
    tft.print(vfo_A / 1000.0, 3);
    tft.setCursor( frq2x2, frq2y);  //(170, 93);
    tft.print("M ");
    tft.print(vfo_M / 1000.0, 3);
    tft.print(" ");
  }
  if (vfo_M_sel)
  {
    tft.print("A ");
    tft.print(vfo_A / 1000.0, 3);
    tft.setCursor(frq2x2, frq2y);  //(170, 93);
    tft.print("B ");
    tft.print(vfo_B / 1000.0, 3);
    tft.print(" ");
  }
} // end of display_frequency2()

void set_band()       // from frequecy determine band and activate corresponding relay TBD
{
  for (int i = MAX_BANDS; i >= 0; i--)
  {
    if ((vfo >= F_MIN_T[i]) && (vfo <= F_MAX_T[i]))
    {
      bnd_count = i ;
      break;
    }
  }
  //  digitalWrite(band_cntrl[old_band], LOW);   // deactivate old band relay
  //  digitalWrite(band_cntrl[bnd_count], HIGH); // activate new selected band
}

void display_band()
{
  tft.setCursor(bandx + 2, bandy + 10); //22, 125);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  //  changed_f = 1;           // ???? why here
  tft.print(B_NAME_T[bnd_count]);
}  // end of Display-band()

void change_band() {
  display_band();
  F_MIN = F_MIN_T[bnd_count];
  F_MAX = F_MAX_T[bnd_count];
  vfo = VFO_T[bnd_count];
  //  set_band();
  changed_f = 1;
}  // end of change_band()

// Displays the frequency change step
void display_step()
{ tft.setCursor(stpx + 3, stpy + 10); // (117, 125);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  tft.print(step_sz_txt[step_index]);
}

void display_sideband() {
  tft.setCursor(sbx + 18, sby + 10); //(261, 125);
  tft.setTextSize(2);
  tft.setTextColor(RED, GREEN);
  if (sideband == LSB)
  {
    tft.print("LSB");
  }
  else if (sideband == USB)
  {
    tft.print("USB");
  }
}

void display_bfo()    // bfo1
{
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  tft.setCursor(bfox + 16, bfoy + 10); //(130, 164);  // also in setup_vfo_screen
  if (bfo1 < 10000000)
    tft.print(" ");
  tft.print(bfo1);
}

void display_bfo2()
{
  tft.setTextSize(2);
  tft.setTextColor(WHITE, GREY);
  tft.setCursor(f2x + 15, f2y + 10); // bfo2 t F2 button
  if (bfo2 < 10000000)
    tft.print(" ");
  tft.print(bfo2);
}

void display_msg(int xposn, String msg)
{ tft.setTextSize(2); // may setp some soft buttons here
  tft.setCursor(xposn, boty);    //223);
  tft.setTextColor(WHITE, BLUE);
  tft.println(msg);
}
void debug_msg(int xposn, int msg)
{ tft.setTextSize(2); // may setp some soft buttons here
  tft.setCursor(xposn, boty);    //223);
  tft.setTextColor(WHITE, BLUE);
  tft.write(msg);
}

void displ_split_button()
{
  if (!splitON)
  {
    tft.drawRoundRect(f3x, f3y, f3wd, f3ht, roundness, GREEN); // F3 button outline
    tft.fillRoundRect(f3x + 2, f3y + 2, f3wd - 4, f3ht - 4, roundness - 4, PURPLE); //F3
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(f3x + 10, f3y + 6);
    tft.print("SPLIT");
  }
  else   // under Split mode control
  {
    tft.drawRoundRect(f3x, f3y, f3wd, f3ht, roundness, GREEN); // F3 button outline
    tft.fillRoundRect(f3x + 2, f3y + 2, f3wd - 4, f3ht - 4, roundness - 4, YELLOW); //F3
    tft.setTextSize(2);
    tft.setTextColor(RED);
    tft.setCursor(f3x + 10, f3y + 6);
    tft.print("SPLIT");
  }
}
