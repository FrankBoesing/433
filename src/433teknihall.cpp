#include "433common.h"

/*

https://github.com/merbanan/rtl_433/blob/master/src/devices/gt_wt_02.c

GT-WT-02 sensor on 433.92MHz.
Example and frame description provided by https://github.com/ludwich66
   [01] {37} 34 00 ed 47 60 : 00110100 00000000 11101101 01000111 01100000
   code, BatOK,not-man-send, Channel1, +23,7°C, 35%
   [01] {37} 34 8f 87 15 90 : 00110100 10001111 10000111 00010101 10010000
   code, BatOK,not-man-send, Channel1,-12,1°C, 10%
Humidity:
- the working range is 20-90 %
- if "LL" in display view it sends 10 %
- if "HH" in display view it sends 110%
SENSOR: GT-WT-02 (ALDI Globaltronics..)
   TYP IIIIIIII BMCCTTTT TTTTTTTT HHHHHHHX XXXXX
TYPE Description:
- I = Random Device Code, changes with battery reset
- B = Battery 0=OK 1=LOW
- M = Manual Send Button Pressed 0=not pressed 1=pressed
- C = Channel 00=CH1, 01=CH2, 10=CH3
- T = Temperature, 12 Bit 2's complement, scaled by 10
- H = Humidity = 7 Bit bin2dez 00-99, Display LL=10%, Display HH=110% (Range 20-90%)
- X = Checksum, sum modulo 64
A Lidl AURIO (from 12/2018) with PCB marking YJ-T12 V02 has two extra bits in front.

*/

static const protocol_ppm_t protocol_teknihall = {
    "Teknihall", // Name
    1700,        // 0 Bit min len
    2600,        // 0 Bit max len
    3900,        // 1 Bit min len
    5000,        // 1 Bit max len
    37,          // # of Bits
};

static std::list<bitbuf_t> rcvList_teknihall;

// called from interrupt, processes the "1" Bits only
void IRAM_ATTR rcv_teknihall(const unsigned len)
{
  static protocol_ppm_data_t data = {0, 0};

  if (parsePPM(len, &data, &protocol_teknihall))
  {
    rcvList_teknihall.push_back(data.data);
  }
}

void handle_teknihall()
{
  static bitbuf_t data;
  bitbuf_t datanew;
  tempData433_t tempdata;

  if (rcvList_teknihall.empty())
    return;

  datanew = rcvList_teknihall.front();
  rcvList_teknihall.pop_front();

  if (datanew == data)
    return; // ignore same data (sender sends 6 times the same)
  data = datanew;
  // dbgPrintData(&data);

  // decoding & plausibilitly tests:
  tempdata.channel = GETBITS(protocol_teknihall, data, 10, 2) + 1; // bit 10+11
  if (tempdata.channel > 3)
  {
    // dbgPrintInfo("Channel invalid");
    return;
  }
  // Checksum:
  int sum_nibbles = (GETBITS(protocol_teknihall, data, 0, 4) + GETBITS(protocol_teknihall, data, 4, 4) +
                     GETBITS(protocol_teknihall, data, 8, 4) + GETBITS(protocol_teknihall, data, 12, 4) +
                     GETBITS(protocol_teknihall, data, 16, 4) + GETBITS(protocol_teknihall, data, 20, 4) +
                     GETBITS(protocol_teknihall, data, 24, 4) + (GETBITS(protocol_teknihall, data, 28, 4) & 0x0e)) &
                    0x3f;
  int checksum = GETBITS(protocol_teknihall, data, 31, 6);
  // Serial.printf("sum:%d chksum:%d\n", sum_nibbles, checksum);
  if (sum_nibbles != checksum)
  {
    // dbgPrintInfo("Checksum invalid");
    return;
  }
  tempdata.id = GETBITS(protocol_teknihall, data, 0, 8);       // bit 0..7
  tempdata.battery = GETBITS(protocol_teknihall, ~data, 8, 1); // bit 8 (1=OK, 0=Empty)
  tempdata.button = GETBITS(protocol_teknihall, data, 9, 1);   // bit 9
  int hum = GETBITS(protocol_teknihall, data, 24, 7); // bit 24..31 : humidity range 20..90, but 10 is LL, 110 is HH
  if ((hum < 10) || (hum > 10 && hum < 0) || (hum > 90 && hum < 110) || (hum > 110))
  {
    // dbgPrintInfo("Hum invalid");
    return;
  }
  if (hum <= 10)
    hum = 0;
  else if (hum >= 90)
    hum = 100;
  tempdata.humidity = hum;

  int16_t temp_raw = (int16_t)(GETBITS(protocol_teknihall, data, 12, 12) << 4); // uses sign extend
  tempdata.temp = (temp_raw >> 4) * 0.1f;
  tempdata.protocolname = protocol_teknihall.NAME;

  addTempData(&tempdata);
  // dbgPrintTempData(&tempdata);

  return;
}
