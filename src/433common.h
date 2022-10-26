#pragma once
#include <list>
#include "433.h"
#include "hw.h"

struct protocol_ppm_t
{
  const char *NAME;
  const unsigned LEN_BIT_0_MIN;
  const unsigned LEN_BIT_0_MAX;
  const unsigned LEN_BIT_1_MIN;
  const unsigned LEN_BIT_1_MAX;
  const unsigned BITS;
};

typedef uint64_t bitbuf_t; //ppm raw data type
struct protocol_ppm_data_t
{
  bitbuf_t data;
  unsigned count;
};

#define GETBITS(protocol, data, pos, count) ((((data) >> (protocol.BITS - (count) - (pos)))) & (((bitbuf_t)1 << (count)) - 1))

bool RAMFUNC parsePPM(const unsigned len, protocol_ppm_data_t *data, const protocol_ppm_t *protocol);
void addTempData(tempData433_t *td);

void RAMFUNC rcv_infactory(const unsigned len);
void handle_infactory();

void RAMFUNC rcv_teknihall(const unsigned len);
void handle_teknihall();
