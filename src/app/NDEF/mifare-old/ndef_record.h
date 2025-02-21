#ifndef ndef_record_h
#define ndef_record_h

#include "stdint.h"
#include "stdbool.h"
#include <esp_log.h>
#include <esp_log_buffer.h>
#include <stdlib.h>
#include <string.h>

#define TNF_EMPTY 0x0
#define TNF_WELL_KNOWN 0x01
#define TNF_MIME_MEDIA 0x02
#define TNF_ABSOLUTE_URI 0x03
#define TNF_EXTERNAL_TYPE 0x04
#define TNF_UNKNOWN 0x05
#define TNF_UNCHANGED 0x06
#define TNF_RESERVED 0x07

typedef struct
{
    uint8_t tnf;
    uint8_t typeLength;
    int payloadLength;
    unsigned int idLength;
    uint8_t *type;
    uint8_t *payload;
    uint8_t *id;
} ndefRecord_t;

void create_ndef_record(ndefRecord_t *record);
int record_getEncodedSize(ndefRecord_t *record);
void record_encode(ndefRecord_t *record, uint8_t *data, bool firstRecord, bool lastRecord);
uint8_t getTnfByte(ndefRecord_t *record, bool firstRecord, bool lastRecord);
void setPayload(ndefRecord_t *record, uint8_t *payload, const int numBytes);
void setType(ndefRecord_t *record, uint8_t *type, const unsigned int numBytes);
void print_record(ndefRecord_t *record);

#endif