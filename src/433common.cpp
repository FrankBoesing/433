#include "433common.h"

static std::list<tempData433_t> rcv433Data;

//TODO: Add digitalReadFast for more boards
#ifdef ESP8266
#define GPIO_IN ((volatile uint32_t *)0x60000318)
#define _digitalReadFast(pin) ((*GPIO_IN & (1 << (pin))))
#define digitalReadFast(pin) (_digitalReadFast(pin) ? 1 : 0)
#endif

bool IRAM_ATTR parsePPM(const unsigned len, protocol_ppm_data_t *data, const protocol_ppm_t *protocol)
{

  data->data <<= 1;

  if (len <= protocol->LEN_BIT_0_MAX && len >= protocol->LEN_BIT_0_MIN)
  {
    ;
  }
  else if (len <= protocol->LEN_BIT_1_MAX && len >= protocol->LEN_BIT_1_MIN)
  {
    data->data |= 1;
  }
  else
  {
    data->data = data->count = 0;
    return false;
  }

  ++data->count;
  return (data->count == protocol->BITS);
}

static IRAM_ATTR void isr()
{
  static unsigned lastInt;
  const unsigned long t = micros();
  const unsigned len = t - lastInt;
  lastInt = t;
  //if (len < 100) //ignore too fast pulses
  //  return;

  auto bit = digitalReadFast(rcv433_pin);
  // printBitLen(len, bit);
  if (bit)
  {
    // these protocols parse the 1-bit only:
    rcv_infactory(len);
    rcv_teknihall(len);
  }
}

void addTempData(tempData433_t *td)
{
  rcv433Data.push_back(*td);
}

static inline void handle433()
{
  handle_infactory();
  handle_teknihall();
}

bool read433(tempData433_t *td)
{

  handle433();

  if(rcv433Data.empty())
    return false;

  *td = rcv433Data.front();
  rcv433Data.pop_front();
  return true;
}

void init433()
{
  pinMode(rcv433_pin, INPUT);
  attachInterrupt(rcv433_pin, isr, CHANGE);
}
