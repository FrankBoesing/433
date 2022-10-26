#include "433common.h"

/*

https://github.com/merbanan/rtl_433/blob/master/src/devices/infactory.c

    0000 1111 | 0011 0000 | 0101 1100 | 1110 0111 | 0110 0001
    iiii iiii | cccc ub?? | tttt tttt | tttt hhhh | hhhh ??nn
- i: identification // changes on battery switch
- c: CRC-4 // CCITT checksum, see below for computation specifics
- u: unknown // (sometimes set at power-on, but not always)
- b: battery low // flag to indicate low battery voltage
- h: Humidity // BCD-encoded, each nibble is one digit, 'A0' means 100%rH
- t: Temperature // in °F as binary number with one decimal place + 90 °F offset
- n: Channel // Channel number 1 - 3

*/

static const protocol_ppm_t protocol_infactory = {
    "Infactory", // Name
    1250,        // 0 Bit min len
    2750,        // 0 Bit max len
    3250,        // 1 Bit min len
    4750,        // 1 Bit max len
    40,          // # of Bits
};

static std::list<bitbuf_t> rcvList_infactory;

// called from interrupt, processes the "1" Bits only
void RAMFUNC rcv_infactory(const unsigned len)
{
  static protocol_ppm_data_t data = {0, 0};

  if (parsePPM(len, &data, &protocol_infactory))
  {
    rcvList_infactory.push_back(data.data);
  }
}

// from https://github.com/merbanan/rtl_433/ :
static unsigned crc4(uint8_t const message[], unsigned nBytes, const uint8_t polynomial, const uint8_t init)
{
  unsigned remainder = init << 4; // LSBs are unused
  const unsigned poly = polynomial << 4;

  while (nBytes--)
  {
    remainder ^= *message++;
    for (auto bit = 0; bit < 8; bit++)
    {
      if (remainder & 0x80)
      {
        remainder = (remainder << 1) ^ poly;
      }
      else
      {
        remainder = (remainder << 1);
      }
    }
  }
  return (remainder >> 4) & 0x0f; // discard the LSBs
}

static bool infactory_crc_check(bitbuf_t data)
{
  bitbuf_t d = __builtin_bswap64(data);
  auto msg = (uint8_t *)&d + 3; //first 3 bytes of uint64 ignored
  unsigned msg_crc = msg[1] >> 4;
  unsigned crc;

  msg[1] = (msg[1] & 0x0F) | ((msg[4] & 0x0F) << 4); // for CRC computation, channel bits are at the CRC position(!)

  crc = crc4(msg, 4, 0x13, 0); // Koopmann 0x9, CCITT-4; FP-4; ITU-T G.704
  crc ^= msg[4] >> 4;          // last nibble is only XORed
  return (crc == msg_crc);
}

void handle_infactory()
{
  static bitbuf_t data;
  bitbuf_t datanew;
  tempData433_t tempdata;

  if (rcvList_infactory.empty())
    return;

  datanew = rcvList_infactory.front();
  rcvList_infactory.pop_front();

  //if (datanew == data)
  //  return; // ignore same data (sender sends 6 times the same)
  data = datanew;
  //printData(&data);

  // Plausibility checks:
  // 1. Check that channel (1 - 3) is not zero
  unsigned channel = data & 0x03;
  if (channel == 0)
  {
    // dbgPrintInfo("Channel invalid");
    return;
  }

  // 2. Check CRC:
  if (!infactory_crc_check(data))
  {
    // dbgPrintInfo("CRC invalid");
    return;
  }

  tempdata.channel = channel;
  tempdata.id = GETBITS(protocol_infactory, data, 0, 8);
  tempdata.battery = GETBITS(protocol_infactory, ~data, 12, 1); // 1=OK
  int temp_raw = GETBITS(protocol_infactory, data, 16, 12);
  // temp = (temp_raw - 900) * 0.1f; //Fahrenheit
  tempdata.temp = (temp_raw - 1220) * (5.0f / 90.0f); // Celsius
  tempdata.humidity = GETBITS(protocol_infactory, data, 28, 4) * 10 +
                      GETBITS(protocol_infactory, data, 32, 4); // BCD, 'A0'=100%rH
  tempdata.button = 0;
  tempdata.protocolname = protocol_infactory.NAME;

  addTempData(&tempdata);
  // dbgPrintTempData(&tempdata);

  return;
}
