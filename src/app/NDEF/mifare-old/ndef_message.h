#ifndef ndef_message_h
#define ndef_message_h

#include "ndef_record.h"
#include "stdint.h"
#include <esp_log.h>
#include <esp_log_buffer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_NDEF_RECORDS 4

typedef struct
{
    ndefRecord_t records[MAX_NDEF_RECORDS];
    uint8_t recordCount;
} ndefMessage_t;

int message_getEncodedSize(ndefMessage_t *message);
void message_encode(ndefMessage_t *message, uint8_t *data);
void create_ndef_message(ndefMessage_t *message, const uint8_t *data, const int numBytes);
void create_ndef_message_empty(ndefMessage_t *message);
bool addRecord(ndefMessage_t *message, ndefRecord_t *record);
void addTextRecord(ndefMessage_t *msg, char *text);
void addEmptyRecord(ndefMessage_t *msg);
void print_message(ndefMessage_t *message);

#endif