#pragma once
#include "433common.h"

void dbgPrintBitLen(const unsigned long len, const int bit);
void dbgPrintBit(const int bit);
void dbgPrintData(const protocol_ppm_data_t *data);
void dbgPrintInfo(const char *msg);
void dbgPrintTempData(const tempData433_t *tempdata);
