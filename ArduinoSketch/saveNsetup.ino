void update_display()
{
  save_frequency();
  display_frequency();
  set_band();
  display_band();
  display_sideband();
}

void set_vfo()
{
  si5351.set_freq(((vfo_tx + if_offset)* SI5351_FREQ_MULT), SI5351_CLK2); // 45 to 75 MHz
}


void save_frequency()    // for temporarily saving in variables not in EEPROM
{
  if (vfo_M_sel)
  {
    vfo_M = vfo;
    bfo_M = bfo1;
    sb_M = sideband;
  }
  else if (vfo_A_sel)
  {
    vfo_A = vfo;
    bfo_A = bfo1;
    sb_A = sideband;
  }
  else
  {
    vfo_B = vfo;
    bfo_B = bfo1;
    sb_B = sideband;
  }
}


void set_bfo1()   // if sb changes readjust bfo and vfo
{
  if (sideband == LSB)
  {
    bfo1 = bfo1_LSB;
    vfo_tx = bfo1 + bfo2 + vfo;  // vfo is the displayed freq
  }
  else
  {
    bfo1 = bfo1_USB;
    vfo_tx =  bfo1 - bfo2 + vfo;
  }

  // si5351.set_freq(bfo2, SI5351_CLK0); // 12 MHZ
  si5351.set_freq(bfo1 * SI5351_FREQ_MULT, SI5351_CLK1);  // 33 MHz for USB , 57 MHz for LSB
  si5351.set_freq(((vfo_tx + if_offset)* SI5351_FREQ_MULT), SI5351_CLK2); // 45 to 75 MHz
}

void set_bfo2()
{
  si5351.set_freq(bfo2 * SI5351_FREQ_MULT, SI5351_CLK0);  // 12 MHZ
  set_bfo1();  // for setting up other clocks
}

void setup_vfo_screen() // sets up main screen for VFO etc display
{
  tft.fillScreen(BLACK); // setup blank screen LIGHTGREY
  // tft.fillRoundRect(0, 0, 320, 121, BLUE); // top segment
  tft.drawRect(0, 0, wd, ht, WHITE);  // outer full border
  tft.setTextSize(2);
  tft.setTextColor(WHITE);  //

  /* tft.drawRoundRect(smx, smy, smwd, smht, RED); // vert S meter
    tft.fillRoundRect(smx + 3, smy + 3, smwd - 4, smht - 5, YELLOW);
    tft.setTextSize(2);
    tft.setTextColor(RED);
    tft.setCursor(smx + 5, smy + smht + 5); // print below s meter
    tft.println("S");
  */
  tft.drawRoundRect(vfox, vfoy, vfowd, vfoht, roundness, RED);  // VFO A/B box outline
  tft.fillRoundRect(vfox + 2, vfoy + 2, vfowd - 4, vfoht - 4, roundness - 4, GREEN); //VFO A/B box
  tft.setCursor(vfox + 10, vfoy + 5);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.print("VFO");

  tft.drawRoundRect(memx, memy, memwd, memht, roundness, RED);  // Mem box outline
  tft.fillRoundRect(memx + 2, memy + 2, memwd - 4, memht - 4, roundness - 4, GREY); //Mem box
  tft.setCursor(memx + 20, memy + 5);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("MEM ");
  display_mem();

  tft.drawRoundRect(txrx, txry, txrwd, txrht, roundness, RED);  // TxRx box outline
  tft.fillRoundRect(txrx + 2, txry + 2, txrwd - 4, txrht - 4, roundness - 4, GREEN); //TxRx box
  tft.setCursor(txrx + 10, txry + 5);
  tft.setTextSize(3);
  tft.setTextColor(BLUE);
  tft.print("Rx");

  tft.drawRoundRect(frqx, frqy, frqwd, frqht, roundness, WHITE);  // freq box outline
  // tft.fillRoundRect(frqx+2, frqy+2, frqwd-4, frqht-4, roundness-4, ORANGE);   //freq box

  tft.fillRoundRect(bandx, bandy, bandwd, bandht, roundness, WHITE); //band button outline
  tft.fillRoundRect(bandx + 2, bandy + 2, bandwd - 4, bandht - 4, roundness - 4, GREY); //band

  tft.fillRoundRect(stpx, stpy, stpwd, stpht, roundness, WHITE); // step button outline
  tft.fillRoundRect(stpx + 2, stpy + 2, stpwd - 4, stpht - 4, roundness - 4, GREY); //step

  tft.fillRoundRect(sbx, sby, sbwd, sbht, roundness, WHITE); // sideband button outline
  tft.fillRoundRect(sbx + 2, sby + 2, sbwd - 4, sbht - 4, roundness - 4, GREEN); //sideband

  tft.drawRoundRect(vmx, vmy, vmwd, vmht, roundness, RED); //  VFO <> MEM  button outline
  tft.fillRoundRect(vmx + 2, vmy + 2, vmwd - 4, vmht - 4, roundness - 4, GREY); //  V/M
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(vmx + 10, vmy + 10);
  tft.print("V>  <M");

  tft.drawRoundRect(bfox, bfoy, bfowd, bfoht, roundness, RED); //bfo button outline
  tft.fillRoundRect(bfox + 2, bfoy + 2, bfowd - 4, bfoht - 4, roundness - 4, GREY); //bfo
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(bfox + 16, bfoy + 10);
  tft.print(bfo1);

  tft.drawRoundRect(svx, svy, svwd, svht, roundness, MAGENTA); // Save button outline
  tft.fillRoundRect(svx + 2, svy + 2, svwd - 4, svht - 4, roundness - 4, RED); //Save
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(svx + 10, svy + 10);
  tft.print("SAVE");

  // 5th row of buttons
  tft.drawRoundRect(f1x, f1y, f1wd, f1ht, roundness, WHITE); // F1 button outline TxTimeOut
  tft.fillRoundRect(f1x + 2, f1y + 2, f1wd - 4, f1ht - 4, roundness - 4, GREEN); //F1
  tft.setTextSize(2);
  tft.setTextColor(BLUE);
  tft.setCursor(f1x + 5, f1y + 8);
  tft.print("TxTmO");

  tft.drawRoundRect(f2x, f2y, f2wd, f2ht, roundness, RED); // F2 button outline long for bfo2
  tft.fillRoundRect(f2x + 2, f2y + 2, f2wd - 4, f2ht - 4, roundness - 4, GREY); //F2 or bfo2 in ubitx
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(f2x + 15, f2y + 10);
  //tft.print("F2");
  tft.print(bfo2);

  tft.drawRoundRect(f3x, f3y, f3wd, f3ht, roundness, GREEN); // F3 button outline
  tft.fillRoundRect(f3x + 2, f3y + 2, f3wd - 4, f3ht - 4, roundness - 4, PURPLE); //F3
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(f3x + 25, f3y + 6);
  tft.print("F3");

/*  tft.drawRoundRect(f4x, f4y, f4wd, f4ht, roundness, GREEN); // F4 button outline
  tft.fillRoundRect(f4x + 2, f4y + 2, f4wd - 4, f4ht - 4, roundness - 4, GREENYELLOW); //F4
  tft.setTextSize(2);
  tft.setTextColor(MAROON);
  tft.setCursor(f4x + 25, f4y + 6);
  tft.print("F4"); */

  tft.drawRect(botx, boty, botwd, botht, WHITE);  // surrounding RECT
  tft.fillRect(botx + 2, boty + 2, botwd - 4, botht - 4, BLACK); // bot strip
  tft.setTextColor(WHITE, BLUE);
  tft.setCursor(botwd, boty + 2);
  tft.print(Ver);
}  // end of setup_vfo_screen()
//---------------------------


/////// $$$ EEPROM related
void init_eprom()      // write some info on EEPROM when initially loaded or when magic number changes
{
  EEPROM.write(eprom_base_addr, magic_no);  // check byte may be same as ver no at 0 address
  display_mem_msg("InitEPrm");
  //display_msg(60, "Init EEPROM");
  ch_info = {vfo_A, bfo1_LSB, LSB};
  address = eprom_base_addr + 1 + sizeof(ch_info) * 1;  // initial infos for VFO A
  EEPROM_writeAnything(address, ch_info);

  //  ch_info={vfo_B, bfo1_usb, USB};  //or
  ch_info.s_vfo = vfo_B;   // initial values of VFO B
  ch_info.s_bfo = bfo1_USB;
  ch_info.s_sb = 2 ;


  address = eprom_base_addr + 1 + sizeof(ch_info) * 2;  // initial infos for VFO B
  EEPROM_writeAnything(address, ch_info);

  // Now store all 13 channels

  for (int i = 1; i <= MAX_BANDS; i++)  // starting from 1st entry in table of freq
  {
    ch_info.s_vfo = VFO_T[i];
    if (VFO_T[i] < 10000000)
    {
      ch_info.s_bfo = bfo1_LSB;
      ch_info.s_sb = LSB;
    }
    else
    {
      ch_info.s_bfo = bfo1_USB;
      ch_info.s_sb = USB;
    }
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }

  vfo = 7100000;
  for (int i = MAX_BANDS + 1; i <= 24; i++) // starting from
  {
    ch_info.s_vfo = vfo;
    ch_info.s_bfo = bfo1_LSB;
    ch_info.s_sb = LSB;
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }
  vfo = 14000000;
  for (int i = 25; i <= 50; i++)  // starting from 160m
  {
    ch_info.s_vfo = vfo;
    ch_info.s_bfo = bfo1_USB;
    ch_info.s_sb = USB;
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }
  vfo = 14200000;
  for (int i = 51; i <= max_memory_ch; i++)  // starting from 160m
  {
    ch_info.s_vfo = vfo;
    ch_info.s_bfo = bfo1_USB;
    ch_info.s_sb = USB;
    address = eprom_base_addr + 1 + sizeof(ch_info) * (i + 2); // first byte for magic no and first 2 infos for VFO A & B
    EEPROM_writeAnything(address, ch_info);
  }
  //display_msg(60, "           ");
  // clear area
  tft.drawRoundRect(memx, memy, memwd, memht, roundness, RED);  // Mem box outline
  tft.fillRoundRect(memx + 2, memy + 2, memwd - 4, memht - 4, roundness - 4, GREY); //Mem box
  tft.setCursor(memx + 20, memy + 5);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("MEM ");
  display_mem();
}

void read_eprom()     // should be called at powerup to retrieve stored values for vfos A and B
{
  //display_msg(60, "Read EEPROM");
  display_mem_msg("RdEPr");

  address = eprom_base_addr + 1 + sizeof(ch_info) * 1;  // first infos for VFO A
  EEPROM_readAnything(address, ch_info);
  vfo_A = ch_info.s_vfo ;   // initial values of VFO A
  bfo_A = ch_info.s_bfo;
  sb_A = ch_info.s_sb;

  address = eprom_base_addr + 1 + sizeof(ch_info) * 2;  // second infos for VFO B
  EEPROM_readAnything(address, ch_info);
  vfo_B = ch_info.s_vfo ;   // initial values of VFO B
  bfo_B = ch_info.s_bfo;
  sb_B = ch_info.s_sb;

  memCh = 1;
  read_ch();   // for 1st mem channel also
  //display_msg(60, "           ");
  // clear area
  tft.drawRoundRect(memx, memy, memwd, memht, roundness, RED);  // Mem box outline
  tft.fillRoundRect(memx + 2, memy + 2, memwd - 4, memht - 4, roundness - 4, GREY); //Mem box
  tft.setCursor(memx + 20, memy + 5);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.print("MEM ");
  display_mem();

}

void read_ch()    // read channel info from eeprom when ever mem ch no changed
{
  address = eprom_base_addr + 1 + sizeof(ch_info) * (memCh + 2); // info for mem channel displayed
  EEPROM_readAnything(address, ch_info);
  vfo_M = ch_info.s_vfo ;
  bfo_M = ch_info.s_bfo ;
  sb_M = ch_info.s_sb;
}

void write_ch()   // write memory channel into eeprom
{
  ch_info = {vfo_M, bfo_M, sb_M};
  address = eprom_base_addr + 1 + sizeof(ch_info) * (memCh + 2); // initial infos for VFO A
  EEPROM_writeAnything(address, ch_info);
}

void write_vfo_A()
{
  ch_info = {vfo_A, bfo_A, sb_A};
  address = eprom_base_addr + 1 + sizeof(ch_info) * 1;  // initial infos for VFO A
  EEPROM_writeAnything(address, ch_info);

}
void write_vfo_B()
{
  ch_info = {vfo_B, bfo_B, sb_B};
  address = eprom_base_addr + 1 + sizeof(ch_info) * 2;  // initial infos for VFO B
  EEPROM_writeAnything(address, ch_info);

}

