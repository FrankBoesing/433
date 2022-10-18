#include "433debug.h"

void dbgPrintBitLen(const unsigned long len, const int bit)
{
  Serial.print(bit);
  Serial.print(':');
  Serial.print(len);
  Serial.print(" ");
}

void dbgPrintBit(const int bit)
{
  Serial.print(bit);
}

void dbgPrintData(const protocol_ppm_data_t *data)
{
  bitbuf_t mask = (bitbuf_t)1 << (data->count - 1);
  int i = 0;
  while (mask)
  {
    if (i % 8 == 0)
      Serial.print(' ');
    i++;

    Serial.print((data->data & mask) ? '1' : '0');
    mask >>= 1;
  }
  Serial.println();
}

void dbgPrintInfo(const char *msg)
{
  Serial.println(msg);
}

void dbgPrintTempData(const tempData433_t *td)
{
  Serial.printf("[%s] ID:%3u Chan:%u Bat:%u Button:%u Hum:%3u%% Temp: %3.1f°C\n",
                td->protocolname, td->id, td->channel, td->battery, td->button, td->humidity, (double)td->temp);
}
