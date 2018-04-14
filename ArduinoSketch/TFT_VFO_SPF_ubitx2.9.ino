/* 
Arduino Sketch to control Si5351 for generating VFO and BFO signals for uBitx or any multiband transceiver
Uses MCUFRIEND 2.4 inch display with touch screen for controlling all functions. The display and a extension board sit piggy back on the Arduino Mega board.
SP Bhatnagar VU2SPF ,  vu2spf@gmail.com
with
Joe Basque VE1BWV, joeman2116@gmail.com
Released under GNU GPL3.0
The Author VU2SPF respectfully and humbly acknowledes the fine contributions of authors of various libraries and other similar projects for publishing their
code for the benefit of HAMS and other electronic enthusiasts. Ideas from many of these may reflect in this work. There is inspiration from OM Rob Lae g8vlq in initial screen
layout. Heartfelt Thanks to all users and testers for encouragement.
There are lot of possible additions / upgrades and improvements. Please suggest or correct and publish your changes for the benefit of all HAMS.
*/

#define Ver "2,9U"      // Split Freq mode added VFO A & B used for Rx and Tx respectively
//#define Ver "2.8U"    // CAT functionality
//#define Ver "2.7U"    // Band select synced with sidebands, increased drive levels to 6 mA for all clocks
//#define Ver "2.6U"    // Added Active PTT functionality
//#define Ver "2.5U"    // uBitx version.  Uses all 3 clocks of Si5351
// Needs 3 clocks from Si5351 - Clock0 = Fixed 12MHz (BFO2), Clock1 = 33 (USB) or 57 (LSB) MHz (BFO1) and Clock2 = 45-75 MHz (VFO)

#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <Wire.h>
#include <avr/io.h>
//--------------------Installable Libraries-----------------------------------------------------------------------
#include <Rotary.h>          // https://github.com/brianlow/Rotary 
#include <si5351.h>          // https://github.com/etherkit/Si5351Arduino
#include <Adafruit_GFX.h>    // Core graphics library located at adafuit website  https://github.com/adafruit/Adafruit-GFX-Library
#include <MCUFRIEND_kbv.h>   // https://github.com/prenticedavid/MCUFRIEND_kbv
#include "TouchScreen.h"     // https://github.com/adafruit/Touch-Screen-Library

//-----------------------USER SELECTABLE DEFINITIONS---------------------------------------------------------------
#define debug

// Various displays
//#define elegoo923     // Joe's 923elegoo display
//#define MCUF0x154     // VU2SPF's test display
//#define PL0x9341      // VU2SPF's Potential Lab display
#define VE0x7783        // Ventor Tech display
//#define REJI5408      // Rejimon's display from Robodo detected as 5408 but works as 9320
//#define IL9325        // Robodo 1/18
//#define Sa35_9486

// Si5351 related
#define si5351correction 0   // IF THERE IS ANY (check using calibrate program in the etherkit Si5351 library examples)

// pin allocations for ubitx
#define TX_RX  14
#define CW_TONE 15
#define TX_LPF_A 16
#define TX_LPF_B 17
#define TX_LPF_C 18
#define CW_KEY 19     // 20,21 are SDA/SCL (rev order on ubitx interface board) so rest pins in sequence on Arduino Mega 

//pin allocations for OPTIONAL input buttons (User selectable )
#define BandSelect      53
#define SideBandSelect  52
#define MEM             51
#define SAVE            50
#define VFO             49
#define STEP            48
#define BF              47
#define VtoMEM          46
#define F1              45
#define F2              44
#define F3              43
#define F4              42
#define SENSOR          66    // Analog Pin A12 for the S Meter function
#define ENCODER_A       67    // Encoder pin A is A13 on Mega
#define ENCODER_B       68    // Encoder pin B is A14 on Mega
#define ENCODER_BTN     37
#define PTT_Input       26

// **** Only Toggle PTT in normal mode / in active_ptt mode PTT remains HIGH during Tx e.g. 5v in PA
// Normal is Toggle PTT (press briefly to go from Rx to Tx & vice-versa)
bool active_PTT_in = false;           // if PTT remains continuously low during QSO make it true else
                                      // false means toggle PTT on active low

int ts_delay = 80;                    // delay between touch button to reduce sensitivity
unsigned long Tx_timeout = 20;        // time in sec upto which Tx works continuosly then goes to Rx
bool PTT_by_touch = false;            // flag for PTT from touch screen

int offset = 0;                       // offsets determined experimentally
volatile uint32_t if_offset = -1500;  // 1500;

bool splitON = false;                 // Is split freq operation between VFO A and B on?

//-------------------------------------------------------------------------------------
// Function prototypes
void dispPos();
void setup_vfo_screen();
void display_msg(int xposn, String msg);
void init_eprom();
void read_eprom();
void set_vfo();
void set_bfo1();
void display_frequency();
void display_vfo();
void display_band();
void display_step();
void display_sideband();
void display_mem();
void display_bfo();
void set_band();
void band_incr();
void band_decr();
int get_button(int x);
void step_decr();
void step_incr();
void save_frequency();
void displ_rx();
void vfo_sel();
void ptt_ON();
void ptt_OFF();
void toggle_ptt();
void sideband_chg();
void vfo_to_mem();
void mem_decr();
void mem_incr();
void mem_to_vfo();
void bfo_decr();
void bfo_incr();
void save();
void bfo2_decr();
void bfo2_incr();
void read_ch();
void set_bfo2();
void write_ch();
void write_vfo_A();
void write_vfo_B();
//void write_vfo_M();

// TFT display colors
// Assign human-readable names to some common 16-bit color values
// EXTRA Color definitions thanks Joe Basque
//                16-bit HEX         Red Green  Blue
#define BLACK       0x0000      //     0     0     0
#define LIGHTGREY   0xC618      //   192   192   192
#define GREY        0x7BEF      //   128   128   128
#define DARKGREY    0x7BEF      //   128   128   128
#define BLUE        0x001F      //     0     0   255
#define NAVY        0x000F      //     0     0   128
#define RED         0xF800      //   255     0     0
#define MAROON      0x7800      //   128     0     0
#define PURPLE      0x780F      //   128     0   128
#define YELLOW      0xFFE0      //   255   255     0
#define WHITE       0xFFFF      //   255   255   255
#define PINK        0xF81F      //   255     0   255
#define ORANGE      0xFD20      //   255   165     0
#define GREEN       0x07E0      //     0   255     0
#define DARKGREEN   0x03E0      //     0   128     0
#define OLIVE       0x7BE0      //   128   128     0
#define GREENYELLOW 0xAFE5      //   173   255    47
#define CYAN        0x07FF      //     0   255   255
#define DARKCYAN    0x03EF      //     0   128   128
#define MAGENTA     0xF81F      //   255     0   255

#define LCD_RD      A0    // LCD Read goes to Analog 0
#define LCD_WR      A1    // LCD Write goes to Analog 1
#define LCD_CD      A2    // Command/Data goes to Analog 2
#define LCD_CS      A3    // Chip Select goes to Analog 3    //**? Are these fixed
#define LCD_RESET   A4    // Can alternately just connect to Arduino's reset pin

// most mcufriend shields use these pins and Portrait mode:      
// **? can we auto define these pins
uint8_t YP;         // must be an analog pin, use "An" notation!
uint8_t XM;         // must be an analog pin, use "An" notation!
uint8_t YM;         // can be a digital pin
uint8_t XP;         // can be a digital pin

// Touch coordinates determined by one of the sample programs provided with touch screen library
uint16_t TS_LEFT;   // Touch Screen Left edge
uint16_t TS_RT;     // Touch Screen right edge
uint16_t TS_TOP;    // Touch Screen Top edge
uint16_t TS_BOT;    // Touch Screen Bottom edge

//--------------------------------------------------------------------------------------
Si5351 si5351;
Rotary r = Rotary(ENCODER_A, ENCODER_B);

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;
#define MINPRESSURE   20
#define MAXPRESSURE   1000
//--------------------------------------------------------------------------------------
boolean txstatus = false;                                         // Rx = False, Tx = True
uint32_t bfo_A, bfo_B, bfo_M;                                     // The bfo frequency is added to or subtracted from the vfo frequency in the "Loop()"
uint32_t bfo2 = 11993900L;                                        // Fixed 12 MHz BFO2 Farhans 11996500 this value found by test
uint32_t bfo1, bfo1_USB = 56995000L , bfo1_LSB = 32994000L;       // Initial Values of BFO1 for USB or LSB
uint32_t vfo , vfo_tx;                                            // vfo is displayed freq, vfo_tx is actual vfo on clock2 of Si5351
uint32_t vfo_A = 7050000L, vfo_B = 7130000L, vfo_M = 14000000L ;  // temp values // either vfo A or B or mem channel is selected for output at any time
boolean vfo_A_sel = true, vfo_B_sel = false, vfo_M_sel = false;   // true for vfo selected
boolean changed_f = false;                                        // indicating need for updating display
boolean xch_M = false;                                            // flag for xchged mem in V > M
uint16_t sideband, sb_A, sb_B, sb_M;
uint16_t LSB = 1, USB = 2;
// display step size and radix
String step_sz_txt[] = {"   1Hz ", "   10Hz ", "  100Hz ", "   1kHz ", "  10kHz ", "  100kHz", "   1MHz "};
uint32_t step_sz[] = {    1,          10,        100,        1000,     10000,      100000,     1000000};
int step_index = 3;
uint32_t radix = 1000;                                            // Starting step size - change to suit

//------------EPROM memory related -----------------------------------------------------------
uint16_t max_memory_ch = 100;  // each ch takes 10 bytes
struct allinfo 
{ 
  uint32_t s_vfo;
  uint32_t s_bfo;
  uint16_t s_sb;
} Allinfo;  // complete description of channel saved in mem

uint16_t eprom_base_addr = 0;
// Eprom content sequence: allinfo for vfoA, vfoB, mem1,2,3...
uint16_t address;         // temp address used by fns
allinfo ch_info;          // local copy
unsigned int memCh = 1;   // memory  channel  number (1,2,3...100) now.  Try names..??
//----------------------------------------------------------------------------------------------
bool Tx_timeout_mode = false;
unsigned long Tx_start_time = 0, rem_time = 0;
int diff;
int bnd_count, old_band;
int MAX_BANDS = 9;
// Band Limits and frequencies and their corresponding display on band button
volatile uint32_t F_MIN_T[9] = {100000UL,  3500000UL, 7000000UL, 10100000UL, 14000000UL, 18068000UL, 21000000UL, 24890000UL, 28000000UL};
volatile uint32_t  F_MAX_T[9] = {75000000UL,  3800000UL, 7200000UL, 10150000UL, 14350000UL, 18168000UL, 21450000UL, 24990000UL, 29700000UL};
String  B_NAME_T[] = {"  VFO", "  80m", "  40m", "  30m", "  20m", "  17m", "  15m", "  12m", "  10m" };
volatile uint32_t  VFO_T[9] = {9500000UL, 3670000UL, 7100000UL, 10120000UL, 14200000UL, 18105000UL, 21200000UL, 24925000UL, 28500000UL};
unsigned long F_MIN, F_MAX;

uint8_t magic_no = 01;      // used for checking the initialization of eprom or to reinitialize change this no.
                            // If mem not initialized there may be garbled display. In that case simply change this number and reload the prog
uint16_t xpos, ypos;        // screen coordinates

uint16_t identifier;        // TFT identifier : can be found from the example programs in the TFT library
int wd;                     //= tft.width();
int ht;                     //= tft.height();
int displ_rotn = 1;         // 0 - normal, 1 - 90deg rotn (landscape), 2 - reverse, 3-rev LS  ** if the touch buttons do not match try changing here

// individual button x,y coordinates, height and width, some common params,
uint16_t buttonht, buttonwd, smwd, smx, smy, smht, firstrowy, vfox, vfoy, vfowd, vfoht;
uint16_t memx, memy, memwd, memht, txrx, txry, txrwd, txrht, frqx, frqy, frqwd, frqht;
uint16_t vfoABMx, vfoABMy, frq2x1, frq2x2, frq2y, bandx, bandy, bandwd, bandht;
uint16_t stpx, stpy, stpwd, stpht, sbx, sby, sbwd, sbht, vmx, vmy, vmwd, vmht, bfox, bfoy, bfowd, bfoht;
uint16_t svx, svy, svwd, svht, f1x, f1y, f1wd, f1ht, f2x, f2y, f2wd, f2ht, f3x, f3y, f3wd, f3ht; //F2 made longer for bfo2
uint16_t f4x, f4y, f4wd, f4ht;        // F4 no longer displayed
uint16_t botx, boty, botwd, botht;
uint16_t roundness;                   // box edge roundness
uint16_t spacing;                     // between buttons on same row
//-------------------- S Meter -------------------------------------------------------------------------
unsigned int Sval, Sens, Ssamp = 0;   // s meter related
unsigned long Savg;
//----------------------CAT relted-------------
long prev_CAT_rd = 0l;                // time when last command was received
byte CAT_buff[5];
int CAT_buff_ptr = 0;
byte i;
bool checkingCAT = 0;                 // when CAT is still being received
bool CAT_ctrl = 0;                    // Under CAT control or not

/**************************************/
/* Interrupt service routine for      */
/* encoder frequency change           */
/**************************************/
ISR(PCINT2_vect)
{
  unsigned char result = r.process();
  if (result && !txstatus)            // lock freq during transmit
  {
    if (result == DIR_CW)
      vfo += radix;
    else if (result == DIR_CCW)
      vfo -= radix;
    changed_f = 1;
  }
}

void setup()
{
  //#ifdef debug
  Serial.begin(38400, SERIAL_8N2);  // 2 stop bits hamlib need - needed for CAT
  CAT_get_freq();
  // Serial.flush();
  //#endif

  #ifdef Sa35_9486      // spf Ventor's display ST7783
  identifier = 0x9486;  // identify display driver chip (known from mcufriend test progs)
  TS_LEFT = 50;
  TS_RT  =  890;
  TS_TOP = 880;         // TOP / BOT values exchanged
  TS_BOT = 100;
  YP = A2;              // must be an analog pin, use "An" notation!
  XM = A1;              // must be an analog pin, use "An" notation!
  YM = 6;               // can be a digital pin
  XP = 7;               // can be a digital pin
  #endif

  #ifdef VE0x7783       // spf Ventor's display ST7783
  identifier = 0x7783;  // identify display driver chip (known from mcufriend test progs)
  TS_LEFT = 50;
  TS_RT  =  890;
  TS_TOP = 880;         // TOP / BOT values exchanged
  TS_BOT = 100;
  YP = A2;              // must be an analog pin, use "An" notation!
  XM = A1;              // must be an analog pin, use "An" notation!
  YM = 6;               // can be a digital pin
  XP = 7;               // can be a digital pin
  #endif

  #if defined(MCUF0x154)// spf robodo display
  identifier = 0x154;   // identify display driver chip (known from mcufriend test progs)
  TS_LEFT = 950;        //**? how to find these points automagically
  TS_RT  = 120;
  TS_TOP = 920;
  TS_BOT = 120;
  YP = A1;              // must be an analog pin, use "An" notation!
  XM = A2;              // must be an analog pin, use "An" notation!
  YM = 7;               // can be a digital pin
  XP = 6;               // can be a digital pin
  #endif

  #if defined (IL9325)
  identifier = 0x9325;
  TS_LEFT = 950;        //**? how to find these points automagically
  TS_RT  = 120;
  TS_TOP = 920;
  TS_BOT = 120;
  YP = A1;              // must be an analog pin, use "An" notation!
  XM = A2;              // must be an analog pin, use "An" notation!
  YM = 7;               // can be a digital pin
  XP = 6;               // can be a digital pin
  #endif

  #if defined(PL0x9341) // spf's Pot Lab display
  identifier = 0x9341;  // identify display driver chip (known from mcufriend test progs)
  TS_LEFT = 950;        //**? how to find these points automagically
  TS_RT  = 120;
  TS_TOP = 120;         // 920;
  TS_BOT = 920;         // 120;
  YP = A1;              // must be an analog pin, use "An" notation!
  XM = A2;              // must be an analog pin, use "An" notation!
  YM = 7;               // can be a digital pin
  XP = 6;               // can be a digital pin
  #endif

#ifdef elegoo923        // Joes display
  identifier = 0x9341;  // Joe's 2.8 DISPLAY blueg5/5elegoo
  TS_LEFT = 74;         // 923elegoo testing------74---- Tested by Joe Basque
  TS_RT  = 906;         // 117-------------------906 --------
  TS_TOP = 117 ;        // 117
  TS_BOT = 923;         // 923
  YP = A3;              // must be an analog pin, use "An" notation!
  XM = A2;              // must be an analog pin, use "An" notation!
  YM = 9;               // can be a digital pin
  XP = 8;               // can be a digital pin
  #endif

  #if defined(REJI5408) // Regimons display
  identifier = 0x9320;  // Rejimon's 2.4 DISPLAY from Robodo detected as 5408 (0x5408) but works as 9320
  TS_LEFT = 74;
  TS_RT  = 906;
  TS_TOP = 117 ;
  TS_BOT = 923;
  YP = A3;              // must be an analog pin, use "An" notation!
  XM = A2;              // must be an analog pin, use "An" notation!
  YM = 9;               // can be a digital pin
  XP = 8;               // can be a digital pin
  #endif

  ts = TouchScreen(XP, YP, XM, YM, 300);
  tft.begin(identifier);        // setup to use driver
  wd = tft.width();
  ht = tft.height();
  tft.setRotation(displ_rotn);  // LS

  if (displ_rotn == 1 || displ_rotn == 3) // exchange the width and height
  {
    int temp = wd;
    wd = ht;
    ht = temp;
  }
  dispPos();

  Wire.begin();       // for si5351
  // welcome_screen();
  setup_vfo_screen(); // create main VFO screen

  if (EEPROM.read(eprom_base_addr) != magic_no)
    init_eprom();     // if blank eeprom or changed/new magic_no for reinit

  read_eprom();       // get infos of VFO A & B (and 1st memory channel only??)
  vfo = vfo_A;        // then display and use VFO A
  vfo_A_sel = 1;
  bfo1 = bfo_A;
  sideband = sb_A;

  // Ports Init
  pinMode(TX_RX, OUTPUT);       // Rx mode D14
  digitalWrite(TX_RX, 0);
  pinMode(CW_TONE, OUTPUT);     // d15
  digitalWrite(CW_TONE, 0);
  pinMode(CW_KEY, OUTPUT);      // d19
  digitalWrite(CW_KEY, 0);

  // Filters
  pinMode(TX_LPF_A, OUTPUT);    // d16
  pinMode(TX_LPF_B , OUTPUT);   // d17
  pinMode(TX_LPF_C, OUTPUT);    // d18
  digitalWrite(TX_LPF_A, 0);    // All filters off only 30MHz LPF for Rx
  digitalWrite(TX_LPF_B, 0);
  digitalWrite(TX_LPF_C, 0);

  pinMode(ENCODER_BTN, INPUT_PULLUP);     // ? pushbutton setup
  pinMode(BandSelect, INPUT_PULLUP);      // band pushbutton setup
  pinMode(SideBandSelect, INPUT_PULLUP);  // sideband pushbutton setup
  pinMode(VFO, INPUT_PULLUP);             // VFO pushbutton setup
  pinMode(VtoMEM, INPUT_PULLUP);          // VtoM pushbutton setup
  pinMode(STEP, INPUT_PULLUP);            // Step pushbutton setup
  pinMode(PTT_Input, INPUT_PULLUP);       // PTT Button toggle type  d26
  digitalWrite(PTT_Input, HIGH);          // temporary for test

  // Initialise si5351
  //si5351.set_correction(00);    // There is a calibration sketch in File/Examples/si5351Arduino-Jason; was 140
                                  // where one can determine the correction by using the serial monitor.
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, si5351correction);  // If using a 27Mhz crystal, put in 27000000 instead of 0
                                                              // 0 is the default crystal frequency of 25Mhz.
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA);       // this is 11dBm
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_6MA);       // you can set this to 2MA, 4MA, 6MA or 8MA
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_6MA);       // be careful though - measure into 50ohms
  si5351.set_freq(bfo2 * SI5351_FREQ_MULT, SI5351_CLK0);      // 12 MHZ which remains fixed
  set_bfo1();                                                 // adjust bfo1 and vfo_tx
  //si5351.set_freq(((vfo_tx + if_offset)* SI5351_FREQ_MULT), SI5351_CLK2); // 45 to 75 MHz

  Tx_timeout = Tx_timeout * 1000; // in ms

  // Enable interrupts
  PCICR |= (1 << PCIE2);                      // Enable pin change interrupt for the encoder
  PCMSK2 |= (1 << PCINT21) | (1 << PCINT22);  // MEGA interrupt pins mapped to A14 A13
  sei();                                      // start interrupts

  display_frequency();  // Update the local display at power on
  display_vfo();
  display_band();       // with values
  display_step();
  display_sideband();
  display_mem();
  display_bfo();
} // end of setup() //


void loop()
{
  if (CAT_ctrl) return;   // if in cat control go back

  // Update the display if the frequency changed
  if (changed_f)
  {
    // set_vfo();
    display_vfo();
    set_bfo1();
    display_frequency();
    set_band();
    display_band();
    display_sideband();
    save_frequency();
    changed_f = 0;
    return;
  }

  if (Tx_timeout_mode)
  {
    rem_time = millis() - Tx_start_time;   // remaining time
    if  (txstatus)
    {
      if ( rem_time >= Tx_timeout)
      {
        digitalWrite(TX_RX, LOW);
        displ_rx();
        txstatus = false;
      }
      else
      {
        tft.setCursor(txrx + 45, txry + 5);
        tft.setTextSize(2);
        tft.setTextColor(WHITE, RED);
        diff = (Tx_timeout - rem_time) / 1000 ;
        if (diff <= 9)
          tft.print(" ");
        tft.print(diff);
      }
    }
  }

  //-----------------------------------
  // External button controls

  //#### Toggle PTT button
  // for uBitx PTT   - touch ptt is always toggle - first touch on second off
  if (active_PTT_in && !PTT_by_touch)     // i.e. Normal type PTT we call active PTT
  {
    if (digitalRead(PTT_Input) == LOW )
      ptt_ON();
    else
      ptt_OFF();
  }
  else      // Toggle type PTT
  {
    if (digitalRead(PTT_Input) == LOW )
    {
      if (get_button(PTT_Input))  // get_button returns 1 if pressed for 100 ms+
        toggle_ptt();
    }
  }

  if (!txstatus)    // only if not in Tx
  {
    //##### Cycle through VFOs
    { if (digitalRead(VFO) == LOW )
        if (get_button(VFO))
          vfo_sel();
    }
    //##### Band Select
    { if (digitalRead(BandSelect) == LOW )
        if (get_button(BandSelect))
          band_incr();
    }

    //##### Step Size
    { if (digitalRead(STEP) == LOW )
        if (get_button(STEP))
          step_decr();
    }
    //#### Side Band Select
    { if (digitalRead(SideBandSelect) == LOW )
        if (get_button(SideBandSelect))
          sideband_chg();
    }

    //#### VFO to Memory
    { if (digitalRead(VtoMEM) == LOW )
        if (get_button(VtoMEM))
          vfo_to_mem();
    }
  }
  //----------------------------------------------
  //$$$$  for Touch Screen input control

  tp = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(XP, OUTPUT);
  pinMode(YM, OUTPUT);
  delay(ts_delay);     // delay between two touches to reduce sensitivity

  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE)
  {
    #if defined (MCUF0x154) || defined(PL0x9341) || defined (VE0x7783)
    xpos = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
    ypos = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());
    #endif

    #if defined elegoo923 || defined (IL9325)
    xpos = map(tp.y, TS_LEFT, TS_RT, 0, tft.width());
    ypos = map(tp.x, TS_TOP, TS_BOT, 0, tft.height());
    #endif

    // Rx/Tx PTT touch button or PTT_Input
    if (ypos > firstrowy && ypos < firstrowy + buttonht)  // first row of buttons
    {
      if (xpos > txrx  && xpos < txrx + txrwd - 2 ) // toggle between Rx & Tx, TX_RX (D14) goes Hi on Tx
      {
        toggle_ptt();
        PTT_by_touch = !PTT_by_touch;
        delay(50);
      }
    }

    if (!txstatus)    // only if not in Tx should any button be recognized for change
    {
      if (ypos > firstrowy && ypos < firstrowy + buttonht)  // first row of buttons (orig 5,42)
      {
        // VFO Button:  cycle VFO A/B/M in sequence when VFO button is touched
        if (xpos > vfox && xpos < vfox + vfowd ) // change VFO (orig 20,95)
          vfo_sel();

        // MEM Ch change Button
        // Left half button decreases channel no
        else if (xpos > memx && xpos < memx + buttonwd - 2 ) // decrease channel ,(110,175)
          mem_decr();   // decrease memory channel number

        // right half buttton increases ch no
        else if (xpos > memx + buttonwd + 2 && xpos < memx + 2 * buttonwd ) // increase channel  (176,245)
          mem_incr();   // increase mem ch no
        return;
      }
      // First row end

      //$$$$$ Freq Change Second Row touch  Button
      if (ypos > frqy && ypos < frqy + frqht && !txstatus) // (45,85)  lock freq change during transmit
      {
        if (xpos > frqx && xpos < (frqx + frqwd / 2) - 2 ) // Left half button decreases frq by step size (50,180)
        {
          vfo = vfo - radix;
          changed_f = 1;
        }

        else if (xpos > (frqx + frqwd / 2) + 2 && xpos < frqx + frqwd && !txstatus) // Right half button increases freq by step size (185,310), lock freq change during transmit
        {
          vfo = vfo + radix;
          changed_f = 1;
        }
        save_frequency();   // added 7/7/17
        return;
      }  // Freq Button/ Second Row end

      //$$$$  Third Row  Band Change Button
      if (ypos > bandy && ypos < bandy + bandht)   // (113,150)
      {
        if (xpos > bandx && xpos < (bandx + bandwd / 2) - 2 ) // Left half button decreases band(20,65)
          band_decr();

        else if (xpos > (bandx + bandwd / 2) + 2 && xpos < bandx + bandwd ) // Right half button increases band (67,115)
          band_incr();

        // $$$$ Third Row Step Size change  Button
        //  left half of step button decreases step size
        else if (xpos > stpx && xpos < (stpx + stpwd / 2) - 2 ) //(114,175)
          step_decr();

        //  right half of step button increases step size
        else if (xpos > (stpx + stpwd / 2) + 2 && xpos < stpx + stpwd ) // (177,238)
          step_incr();

        ///$$$$$ Third Row side band flip flop between LSB & USB   (others may be added if hardware permits)
        else if (xpos > sbx && xpos < sbx + sbwd ) // (245,310)
          sideband_chg();
        return;
      } // Third row end

      if (ypos > vmy && ypos < vmy + vmht) // (152,189) fourth row
      {
        //$$$$$ Fourth Row VFO < > Mem switch nothing saved on EEPROM unless SAVE button pressed
        if (xpos > vmx && xpos < (vmx + vmwd / 2) - 2 ) //left half VFO -> MEM (20,65)
          // currently selected VFO stored on currently selected mem ch (not in EEPROM which is by SAVE button)
          vfo_to_mem();

        else if (xpos > (vmx + vmwd / 2) + 2 && xpos < vmx + vmwd ) //right half VFO <- MEM    (66,109)
          mem_to_vfo();

        //#ifndef FIXED_BFO
        // $$$$$ Fourth Row bfo freq adjust
        // left half button decreases bfo freq
        else if (xpos > bfox && xpos < (bfox + bfowd / 2) - 2 ) // decrease freq
          bfo_decr();
        // right half button increases bfo freq
        else if (xpos > (bfox + bfowd / 2) + 2 && xpos < bfox + bfowd ) // increase freq  (175,235)
          bfo_incr();
        //#endif

        // Fourth Row SAVE Button
        else if (xpos > svx && xpos < svx + svwd ) // Save "current" Vfo/Mem on eeprom  (245,310)
          save();
      } // Fourth Row end
      //--------------Fifth Row
      if (ypos > f1y && ypos < f1y + f1ht) //  fifth row
      {
        if (xpos > f1x && xpos < f1x + f1wd) // Tx time out button RED / Green, Mode activated, timer starts on PTT
        {
          Tx_timeout_mode = !Tx_timeout_mode;    // toggle mode on rpeated touch
          if (Tx_timeout_mode)   // button red
          {
            tft.drawRoundRect(f1x, f1y, f1wd, f1ht, roundness, WHITE); // TxTmO button outline TxTimeOut
            tft.fillRoundRect(f1x + 2, f1y + 2, f1wd - 4, f1ht - 4, roundness - 4, RED); //F1
            tft.setTextSize(2);
            tft.setTextColor(WHITE);
            tft.setCursor(f1x + 5, f1y + 8);
            tft.print("TxTmO");
            Tx_start_time = 0;    // timer acually starts by PTT in rx_tx_ptt() function
          }
          else     // button green when not in Tx timeout mode
          {
            tft.drawRoundRect(f1x, f1y, f1wd, f1ht, roundness, WHITE); // TxTmO button outline TxTimeOut
            tft.fillRoundRect(f1x + 2, f1y + 2, f1wd - 4, f1ht - 4, roundness - 4, GREEN); //F1
            tft.setTextSize(2);
            tft.setTextColor(BLUE);
            tft.setCursor(f1x + 5, f1y + 8);
            tft.print("TxTmO");
          }
        }

        else if (xpos > f2x && xpos < (f2x + f2wd / 2) - 2 ) // decrease freq bfo2
          bfo2_decr();
        // right half button increases bfo2 freq
        else if (xpos > (f2x + f2wd / 2) + 2 && xpos < f2x + f2wd ) // increase freq  (175,235)
          bfo2_incr();

        else if (xpos > f3x && xpos < (f3x + f3wd)) // Split Control
        {
          splitON = !splitON;
          displ_split_button();
        }
      }
    }
  }

  //=====================
  //$$$$ S Meter display Take average and display after Ssamp no of samples
  // Logic not perfect as of v2.4
  Ssamp++; // sample no
  Sval = analogRead(SENSOR);
  Savg = (Savg + Sval );
  if (Ssamp >= 10)     // calc & display every 10 samples
  {
    Savg = Savg / Ssamp;
    // Serial.println(Savg);
    {
      Sens = map(Savg, 0, 1023, 0, botwd - 50);
      tft.fillRect(botx + 3, boty + 3, botwd - 8, botht - 5, BLACK);

      if (Savg > 500) // upper scale
      {
        tft.fillRect(botx + 3, boty + 3, Sens - 4, botht - 5, RED);

      }
      else if (Savg > 180) // middle scale
      {
        tft.fillRect(botx + 3, boty + 3, Sens - 4, botht - 5, MAGENTA);
      }
      else // lower scale   upside down  ///180 is random now should dc test
      {
        tft.fillRect(botx + 3, boty + 3, Sens - 4, botht - 5, GREEN);
      }
    }
    Ssamp = 0;
    Savg = 0;
  }
}    // end of loop()