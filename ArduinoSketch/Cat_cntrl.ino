// some of the following are borrowed from various permitted sources
void check_CAT()
//void serialEvent()
{
  while (Serial.available())
  {
    CAT_buff[CAT_buff_ptr] = Serial.read();
    CAT_buff_ptr++;
    if (CAT_buff_ptr == 5)
    {
      CAT_buff_ptr = 0;
      CAT_ctrl = 1;
      exec_CAT(CAT_buff);
    }
  }
}

// The next 4 functions are needed to implement the CAT protocol, which
// uses 4-bit BCD formatting.
//
byte setHighNibble(byte b, byte v) {
  // Clear the high nibble
  b &= 0x0f;
  // Set the high nibble
  return b | ((v & 0x0f) << 4);
}

byte setLowNibble(byte b, byte v) {
  // Clear the low nibble
  b &= 0xf0;
  // Set the low nibble
  return b | (v & 0x0f);
}

byte getHighNibble(byte b) {
  return (b >> 4) & 0x0f;
}

byte getLowNibble(byte b) {
  return b & 0x0f;
}

// Takes a number and produces the requested number of decimal digits, starting
// from the least significant digit.
//
void getDecimalDigits(unsigned long number, byte* result, int digits) {
  for (int i = 0; i < digits; i++) {
    // "Mask off" (in a decimal sense) the LSD and return it
    result[i] = number % 10;
    // "Shift right" (in a decimal sense)
    number /= 10;
  }
}

// Takes a frequency and writes it into the CAT command buffer in BCD form.
//
void writeFreq(unsigned long freq, byte* cmd) {
  // Convert the frequency to a set of decimal digits. We are taking 9 digits
  // so that we can get up to 999 MHz. But the protocol doesn't care about the
  // LSD (1's place), so we ignore that digit.
  byte digits[9];
  getDecimalDigits(freq, digits, 9);
  // Start from the LSB and get each nibble
  cmd[3] = setLowNibble(cmd[3], digits[1]);
  cmd[3] = setHighNibble(cmd[3], digits[2]);
  cmd[2] = setLowNibble(cmd[2], digits[3]);
  cmd[2] = setHighNibble(cmd[2], digits[4]);
  cmd[1] = setLowNibble(cmd[1], digits[5]);
  cmd[1] = setHighNibble(cmd[1], digits[6]);
  cmd[0] = setLowNibble(cmd[0], digits[7]);
  cmd[0] = setHighNibble(cmd[0], digits[8]);
}

// This function takes a frquency that is encoded using 4 bytes of BCD
// representation and turns it into an long measured in Hz.
//
// [12][34][56][78] = 123.45678? Mhz
//
unsigned long readFreq(byte* cmd) {
  // Pull off each of the digits
  byte d7 = getHighNibble(cmd[0]);
  byte d6 = getLowNibble(cmd[0]);
  byte d5 = getHighNibble(cmd[1]);
  byte d4 = getLowNibble(cmd[1]);
  byte d3 = getHighNibble(cmd[2]);
  byte d2 = getLowNibble(cmd[2]);
  byte d1 = getHighNibble(cmd[3]);
  byte d0 = getLowNibble(cmd[3]);
  return
    (unsigned long)d7 * 100000000L +
    (unsigned long)d6 * 10000000L +
    (unsigned long)d5 * 1000000L +
    (unsigned long)d4 * 100000L +
    (unsigned long)d3 * 10000L +
    (unsigned long)d2 * 1000L +
    (unsigned long)d1 * 100L +
    (unsigned long)d0 * 10L;
}

void update_CAT()
{
  //CAT_get_freq();
 // CAT_set_mode();
}

void CAT_set_freq()   // first four bytes in buffer are freq in compressed bcd
{
  // This function sets  the VFO frequency
  Serial.write(0); // ACK
  vfo = readFreq(CAT_buff);
  set_vfo();
  // changed_f = 1; //update display gives prob with wsjtx reading current freq time out
  display_frequency();
  set_bfo1();
  set_band();
  display_band();
  CAT_ctrl = 0;
}

void CAT_SetSplit()
{
  Serial.write(0); // ACK
  CAT_ctrl = 0;

}

void CAT_get_freq()
{
  writeFreq(vfo, CAT_buff);
  if (sideband == LSB)
    CAT_buff[4] = 0x00;
  else
    CAT_buff[4] = 0x01;

  for (i = 0; i < 5; i++)
  {
    Serial.write(CAT_buff[i]);
  }

//  Serial.write(0);
  CAT_ctrl = 0;
}

void CAT_ptt_on()
{
  PTT_by_CAT=true;
  ptt_ON();
  Serial.write(0);
  CAT_ctrl = 0;
}

void CAT_ptt_off()
{
  PTT_by_CAT=false;
  ptt_OFF();
  Serial.write(0);
  CAT_ctrl = 0;
}

void CAT_set_mode()
{
  if (CAT_buff[0] == 00)
    sideband = LSB ;
  else
    sideband = USB;

  set_bfo1();
  display_sideband();
  Serial.write(0);
  CAT_ctrl = 0;
}

void CAT_toggle_VFO()   // only between VFO A & B
{
  //Serial.write(0x00);    // just send 1 bytes ACK
  if (vfo_A_sel)
    vfo_selB();
  else
    vfo_selA();
  // CAT_get_freq();
  Serial.write(0x00);    // just send 1 bytes ACK
  CAT_ctrl = 0;
}

void CAT_Eeprom_read()
{
  Serial.write(0x10);    // Mem 64 = 10 means 38400 baud
  Serial.write(0x00);    // Mem 65 = 00
  CAT_ctrl = 0;

  //  Serial.write(0x10);  // cat rate 38400
}

void CAT_Tx_status()
{
  Serial.write(0x88);    // just send a dummy byte
  CAT_ctrl = 0;

}


void exec_CAT(byte* cmd)
{
  switch (cmd[4])
  {
    case 0x01 :   //Set Frequency
      CAT_set_freq();
      break;

    case 0x02 : //Split On
    case 0x82:  //Split Off
      CAT_SetSplit();
      break;

    case 0x03 :   //Read Frequency and mode
      CAT_get_freq();   // retreive freq & mode
      break;

    case 0x07 :   //Set Operating  Mode
      CAT_set_mode();
      // modes 00 - LSB, 01 - USB, 02 - CW, 03 - CWR, 04 - AM, 08 - FM, 0A - DIG, 0C - PKT
      break;

    case 0x08 : //Set PTT_ON
      CAT_ptt_on();
      break;

    case 0x88:  //Set PTT Off
      CAT_ptt_off();
      break;

    case 0x81:  //Toggle VFO
      CAT_toggle_VFO(); // between A & B
      break;

    case 0xDB:  //Read uBITX EEPROM Data
      Serial.write(0x00);    // just send a dummy byte
      break;

    case 0xBB:  //Read FT-817 EEPROM Data  (for comfortable)
      CAT_Eeprom_read();
      break;

    case 0xDC:  //Write uBITX EEPROM Data
      Serial.write(0x00);    // just send a dummy byte
      break;

    case 0xBC:  //Write FT-817 EEPROM Data  (for comfirtable)
      Serial.write(0x00);    // just send a dummy byte
      break;

    case 0xE7 :       //Read RX Status
      Serial.write(0x00);    // just send a dummy byte
      break;

    case 0xF7:      //Read TX Status
      CAT_Tx_status();
      break;
    default:
      /*
        char buff[16];
        sprintf(buff, "DEFAULT : %x", CAT_BUFF[4]);
        printLine2(buff);
      */
      //     Serial.write(0x00);
    //  Serial.write(0x00);
      CAT_ctrl = 0;
      //   Serial.flush();

      break;
  } //end of switch
  //  CAT_ctrl = 0;
  checkingCAT = 0;
}



