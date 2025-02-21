#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include <esp_log_buffer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"
#include "card_reader.h"
#include "driver/gpio.h"
#include "pn532.h"

#define BLINK_GPIO 2

#define PN532_SCK 10
#define PN532_MOSI 7
#define PN532_SS 20
#define PN532_MISO 2
// #define PN532_SCK (2)
// #define PN532_MOSI (18)
// #define PN532_SS (4)
// #define PN532_MISO (5)

#define NUM_AUTH_BLOCKS 16
#define NTAG_215_PAGE_STOP 20
#define CARD_READER_TAG "card_reader"

#define NR_SHORTSECTOR (32)         // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR (8)           // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4) // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR (16) // Number of blocks in a long sector

// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector) < NR_SHORTSECTOR) ? ((sector) * NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR - 1) : (NR_SHORTSECTOR * NR_BLOCK_OF_SHORTSECTOR + (sector - NR_SHORTSECTOR) * NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR - 1))

// Determine the sector's first block based on the sector number
#define BLOCK_NUMBER_OF_SECTOR_1ST_BLOCK(sector) (((sector) < NR_SHORTSECTOR) ? ((sector) * NR_BLOCK_OF_SHORTSECTOR) : (NR_SHORTSECTOR * NR_BLOCK_OF_SHORTSECTOR + (sector - NR_SHORTSECTOR) * NR_BLOCK_OF_LONGSECTOR))

// The default Mifare Classic key
static const uint8_t KEY_DEFAULT_KEYAB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static const char *INIT_TAG = "PN532 Initialization";
static const char *READ_TAG = "PN532 Reading";
static const char *WRITE_TAG = "PN532 Writing";
static const char *FORMAT_TAG = "NDEF Formatting";
static const char *REFORMAT_TAG = "NDEF to Classic Formatting";
const char *url = "5387541";

extern pn532_t nfc;

void blink_task(void *pvParameters)
{
    esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (1)
    {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(900 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// void nfc_init()
// {
//     pn532_spi_init(&nfc, PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
//     pn532_begin(&nfc);

//     uint32_t versiondata = pn532_getFirmwareVersion(&nfc);
//     if (!versiondata)
//     {
//         ESP_LOGI(INIT_TAG, "Didn't find PN53x board");
//         while (1)
//         {
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     }
//     // Got ok data, print it out!
//     ESP_LOGI(INIT_TAG, "Found chip PN5 %lu", (versiondata >> 24) & 0xFF);
//     ESP_LOGI(INIT_TAG, "Firmware ver. %lu.%lu", (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);

//     // configure board to read RFID tags
//     bool success = pn532_SAMConfig(&nfc);

//     if (!success)
//     {
//         ESP_LOGI("INIT_TAG", "SAMConfig failed");
//         while (1)
//         {
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     }

//     ESP_LOGI(INIT_TAG, "Waiting for an ISO14443A Card ...");
// }

void build_tlv(char *userID)
{
    uint8_t tlv_tag = 0x03;
    uint8_t tlv_len = strlen(userID);
    uint8_t tlv_value[tlv_len];
}

void ndef_to_classic_task(void *pvParameters)
{
    uint8_t success;                       // Flag to check if there was an error with the PN532
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
    uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    uint8_t blockBuffer[16];               // Buffer to store block contents
    uint8_t blankAccessBits[3] = {0xff, 0x07, 0x80};
    uint8_t idx = 0;
    uint8_t numOfSector = 16; // Assume Mifare Classic 1K for now (16 4-block sectors)

    ESP_LOGI(REFORMAT_TAG, "Place your NDEF formatted Mifare Classic 1K card on the reader");
    ESP_LOGI(REFORMAT_TAG, "and press any key to continue ...");

    // Wait for user input before proceeding
    // while (!Serial.available())
    //     ;
    // while (Serial.available())
    //     Serial.read();

    // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)

    while (1)
    {
        success = pn532_readPassiveTargetID(&nfc, PN532_MIFARE_ISO14443A, uid, &uidLength, 0);

        if (success)
        {
            // We seem to have a tag ...
            // Display some basic information about it
            ESP_LOGI(REFORMAT_TAG, "Found an ISO14443A card/tag");
            ESP_LOGI(REFORMAT_TAG, "  UID Length: %d bytes", uidLength);
            ESP_LOGI(REFORMAT_TAG, "  UID Value: ");
            esp_log_buffer_hexdump_internal(REFORMAT_TAG, uid, uidLength, ESP_LOG_INFO);
            ESP_LOGI(REFORMAT_TAG, "");

            // Make sure this is a Mifare Classic card
            if (uidLength != 4)
            {
                ESP_LOGI(REFORMAT_TAG, "Ooops ... this doesn't seem to be a Mifare Classic card!");
            }
            else
            {
                ESP_LOGI(REFORMAT_TAG, "Seems to be a Mifare Classic card (4 byte UID)");
                ESP_LOGI(REFORMAT_TAG, "");
                ESP_LOGI(REFORMAT_TAG, "Reformatting card for Mifare Classic (please don't touch it!) ... ");

                // Now run through the card sector by sector
                for (idx = 0; idx < numOfSector; idx++)
                {
                    // Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
                    success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, (uint8_t *)KEY_DEFAULT_KEYAB);
                    if (!success)
                    {
                        ESP_LOGI(REFORMAT_TAG, "Authentication failed for sector %d", numOfSector);
                    }
                    else
                    {
                        // Step 2: Write to the other blocks
                        if (idx == 16)
                        {
                            memset(blockBuffer, 0, sizeof(blockBuffer));
                            if (!(pn532_mifareclassic_WriteDataBlock(&nfc, (BLOCK_NUMBER_OF_SECTOR_TRAILER(idx))-3, blockBuffer)))
                            {
                                ESP_LOGI(REFORMAT_TAG, "Unable to write to sector %d", numOfSector);
                                // Removed return statement
                            }
                        }
                        if ((idx == 0) || (idx == 16))
                        {
                            memset(blockBuffer, 0, sizeof(blockBuffer));
                            if (!(pn532_mifareclassic_WriteDataBlock(&nfc, (BLOCK_NUMBER_OF_SECTOR_TRAILER(idx))-2, blockBuffer)))
                            {
                                ESP_LOGI(REFORMAT_TAG, "Unable to write to sector %d", numOfSector);
                                // Removed return statement
                            }
                        }
                        else
                        {
                            memset(blockBuffer, 0, sizeof(blockBuffer));
                            if (!(pn532_mifareclassic_WriteDataBlock(&nfc, (BLOCK_NUMBER_OF_SECTOR_TRAILER(idx))-3, blockBuffer)))
                            {
                                ESP_LOGI(REFORMAT_TAG, "Unable to write to sector %d", numOfSector);
                                // Removed return statement
                            }
                            if (!(pn532_mifareclassic_WriteDataBlock(&nfc, (BLOCK_NUMBER_OF_SECTOR_TRAILER(idx))-2, blockBuffer)))
                            {
                                ESP_LOGI(REFORMAT_TAG, "Unable to write to sector %d", numOfSector);
                                // Removed return statement
                            }
                        }
                        memset(blockBuffer, 0, sizeof(blockBuffer));
                        if (!(pn532_mifareclassic_WriteDataBlock(&nfc, (BLOCK_NUMBER_OF_SECTOR_TRAILER(idx))-1, blockBuffer)))
                        {
                            ESP_LOGI(REFORMAT_TAG, "Unable to write to sector %d", numOfSector);
                            // Removed return statement
                        }

                        // Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
                        memcpy(blockBuffer, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));
                        memcpy(blockBuffer + 6, blankAccessBits, sizeof(blankAccessBits));
                        blockBuffer[9] = 0x69;
                        memcpy(blockBuffer + 10, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));

                        // Step 4: Write the trailer block
                        if (!(pn532_mifareclassic_WriteDataBlock(&nfc, (BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer)))
                        {
                            ESP_LOGI(REFORMAT_TAG, "Unable to write trailer block of sector %d", numOfSector);
                            // Removed return statement
                        }
                    }
                }
            }
        }

        // Wait a bit before trying again
        ESP_LOGI(REFORMAT_TAG, "\n\nDone!");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        // Serial.flush();
        // while (Serial.available())
        //     Serial.read();
    }
}

void ndef_format_task(void *pvParameters)
{
    uint8_t success;                       // Flag to check if there was an error with the PN532
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
    uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

    // Use the default key
    uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    while (1)
    {
        ESP_LOGI(FORMAT_TAG, "PLEASE NOTE: Formatting your card for NDEF records will change the");
        ESP_LOGI(FORMAT_TAG, "authentication keys.  To reformat your NDEF tag as a clean Mifare");
        ESP_LOGI(FORMAT_TAG, "Classic tag, use the mifareclassic_ndeftoclassic example!");
        ESP_LOGI(FORMAT_TAG, "\n");
        ESP_LOGI(FORMAT_TAG, "Place your Mifare Classic card on the reader to format with NDEF");
        ESP_LOGI(FORMAT_TAG, "and press any key to continue ...");

        // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
        // 'uid' will be populated with the UID, and uidLength will indicate
        // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
        success = pn532_readPassiveTargetID(&nfc, PN532_MIFARE_ISO14443A, uid, &uidLength, 0);

        if (success)
        {
            // Display some basic information about the card
            ESP_LOGI(FORMAT_TAG, "Found an ISO14443A card");
            ESP_LOGI(FORMAT_TAG, "UID Length: %d bytes", uidLength);
            ESP_LOGI(FORMAT_TAG, "UID Value:");
            esp_log_buffer_hexdump_internal(FORMAT_TAG, uid, uidLength, ESP_LOG_INFO);

            // Make sure this is a Mifare Classic card
            if (uidLength != 4)
                ESP_LOGI(FORMAT_TAG, "Ooops ... this doesn't seem to be a Mifare Classic card!");
            else
            {
                // We probably have a Mifare Classic card ...
                ESP_LOGI(FORMAT_TAG, "Seems to be a Mifare Classic card (4 byte UID)");

                // Try to format the card for NDEF data
                success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, 0, 0, keya);
                if (!success)
                    ESP_LOGI(FORMAT_TAG, "Unable to authenticate block 0 to enable card formatting!");
                else
                {
                    success = pn532_mifareclassic_FormatNDEF(&nfc);
                    if (!success)
                        ESP_LOGI(FORMAT_TAG, "Unable to format the card for NDEF");
                    else
                    {
                        ESP_LOGI(FORMAT_TAG, "Card has been formatted for NDEF data using MAD1");

                        // Try to authenticate block 4 (first block of sector 1) using our key
                        success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, 4, 0, keya);

                        // Make sure the authentification process didn't fail
                        if (!success)
                            ESP_LOGI(FORMAT_TAG, "Authentication failed.");
                    }
                }
            }
        }
        // Wait a bit before attempting to format again
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

// Tag: 0x03    (NDEF Message)
// Length: 0x08 (8 bytes)
// Value: User ID (8 bytes)
void pn532_write_user_id(char *userID)
{
    uint8_t len = strlen(userID);
    uint8_t sectorNumber = 2;
    uint8_t uriIdentifier = 0;
    // Make sure the URI payload is between 1 and 38 chars
    if ((len < 1) || (len > 38))
    {
        ESP_LOGI(WRITE_TAG, "UserID is too long ... must be less than 38 characters!");
        return;
    }
    // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, len + 5, 0xD1, 0x01, len + 1, 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (len <= 6)
    {
        // Unlikely we'll get a url this short, but why not ...
        memcpy(sectorbuffer1 + 9, url, len);
        sectorbuffer1[len + 9] = 0xFE;
    }
    else if (len == 7)
    {
        // 0xFE needs to be wrapped around to next block
        memcpy(sectorbuffer1 + 9, url, len);
        sectorbuffer2[0] = 0xFE;
    }
    else if ((len > 7) && (len <= 22))
    {
        // Url fits in two blocks
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, len - 7);
        sectorbuffer2[len - 7] = 0xFE;
    }
    else if (len == 23)
    {
        // 0xFE needs to be wrapped around to final block
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, len - 7);
        sectorbuffer3[0] = 0xFE;
    }
    else
    {
        // Url fits in three blocks
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, 16);
        memcpy(sectorbuffer3, url + 23, len - 24);
        sectorbuffer3[len - 22] = 0xFE;
    }
}
void nfc_write_task(void *pvParameters)
{
    uint8_t success;                       // Flag to check if there was an error with the PN532
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
    uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    const char *url = "adafruit.com/blog/";
    uint8_t ndefprefix = NDEF_URIPREFIX_HTTP_WWWDOT;

    // Use the default NDEF keys (these would have have set by mifareclassic_formatndef.pde!)
    uint8_t keyb[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};

    ESP_LOGI(WRITE_TAG, "Place your NDEF formatted Mifare Classic card on the reader to update the");
    ESP_LOGI(WRITE_TAG, "NDEF record and press any key to continue ...");

    // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
    // 'uid' will be populated with the UID, and uidLength will indicate
    // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
    while (1)
    {
        success = pn532_readPassiveTargetID(&nfc, PN532_MIFARE_ISO14443A, uid, &uidLength, 0);

        if (success)
        {
            // Display some basic information about the card
            ESP_LOGI(WRITE_TAG, "Found an ISO14443A card");
            ESP_LOGI(WRITE_TAG, "  UID Length: %d bytes", uidLength);
            ESP_LOGI(WRITE_TAG, "  UID Value:");
            esp_log_buffer_hexdump_internal(WRITE_TAG, uid, uidLength, ESP_LOG_INFO);
            ESP_LOGI(WRITE_TAG, "");

            // Make sure this is a Mifare Classic card
            if (uidLength != 4)
                ESP_LOGI(WRITE_TAG, "Ooops ... this doesn't seem to be a Mifare Classic card!");
            else
            {
                // We probably have a Mifare Classic card ...
                ESP_LOGI(WRITE_TAG, "Seems to be a Mifare Classic card (4 byte UID)");

                // Check if this is an NDEF card (using first block of sector 1 from mifareclassic_formatndef.pde)
                // Must authenticate on the first key using 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7
                success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, 4, 0, keyb);
                if (!success)
                    ESP_LOGI(WRITE_TAG, "Unable to authenticate block 4 ... is this card NDEF formatted?");
                else
                {
                    ESP_LOGI(WRITE_TAG, "Authentication succeeded (seems to be an NDEF/NFC Forum tag) ...");

                    // Authenticated seems to have worked
                    // Try to write an NDEF record to sector 1
                    // Use 0x01 for the URI Identifier Code to prepend "http://www."
                    // to the url (and save some space).  For information on URI ID Codes
                    // see http://www.ladyada.net/wiki/private/articlestaging/nfc/ndef
                    if (strlen(url) > 38)
                    {
                        // The length is also checked in the WriteNDEFURI function, but lets
                        // warn users here just in case they change the value and it's bigger
                        // than it should be
                        ESP_LOGI(WRITE_TAG, "URI is too long ... must be less than 38 characters!");
                    }
                    else
                    {
                        ESP_LOGI(WRITE_TAG, "Updating sector 1 with URI as NDEF Message");

                        // URI is within size limits ... write it to the card and report success/failure
                        success = pn532_mifareclassic_WriteNDEFURI(&nfc, 1, ndefprefix, url);
                        if (success)
                        {
                            ESP_LOGI(WRITE_TAG, "NDEF URI Record written to sector 1");
                            ESP_LOGI(WRITE_TAG, "");
                        }
                        else
                            ESP_LOGI(WRITE_TAG, "NDEF Record creation failed! :(");
                    }
                }
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void nfc_read_task(void *pvParameters)
{
    while (1)
    {
        uint8_t success;
        uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
        uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
        uint8_t keyuniversal[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        // uint8_t keyuniversal[6] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5};
        bool authenticated = false;
        uint8_t data[16];

        // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
        // 'uid' will be populated with the UID, and uidLength will indicate
        // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
        success = pn532_readPassiveTargetID(&nfc, PN532_MIFARE_ISO14443A, uid, &uidLength, 0);

        if (success)
        {
            // Display some basic information about the card
            ESP_LOGI(READ_TAG, "Found an ISO14443A card");
            ESP_LOGI(READ_TAG, "UID Length: %d bytes", uidLength);
            ESP_LOGI(READ_TAG, "UID Value:");
            esp_log_buffer_hexdump_internal(READ_TAG, uid, uidLength, ESP_LOG_INFO);

            if (uidLength == 4)
            {
                // We probably have a Mifare Classic card ...
                ESP_LOGI(READ_TAG, "Seems to be a Mifare Classic card (4 byte UID)");
                uint8_t currentblock;

                // Now we try to go through all 16 sectors (each having 4 blocks)
                // authenticating each sector, and then dumping the blocks
                for (currentblock = 1; currentblock < 16; currentblock++)
                {
                    // Check if this is a new block so that we can reauthenticate
                    if (pn532_mifareclassic_IsFirstBlock(&nfc, currentblock))
                        authenticated = false;

                    // If the sector hasn't been authenticated, do so first
                    if (!authenticated)
                    {
                        // Starting of a new sector ... try to to authenticate
                        ESP_LOGI(READ_TAG, "------------------------Sector %d-------------------------", currentblock / 4);

                        if (currentblock == 0)
                        {
                            // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
                            // or 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 for NDEF formatted cards using key a,
                            // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
                            success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, currentblock, 1, keyuniversal);
                        }
                        else
                        {
                            // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
                            // or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
                            // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
                            success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, currentblock, 1, keyuniversal);
                        }
                        // success = pn532_mifareclassic_AuthenticateBlock(&nfc, uid, uidLength, currentblock, 1, keyuniversal);

                        if (success)
                        {
                            authenticated = true;
                        }
                        else
                        {
                            authenticated = false;
                            ESP_LOGI(READ_TAG, "Authentication error");
                        }
                    }
                    // If we're still not authenticated just skip the block
                    if (!authenticated)
                    {
                        ESP_LOGI(READ_TAG, "Block %d unable to authenticate", currentblock);
                    }
                    else
                    {
                        // Authenticated ... we should be able to read the block now
                        // Dump the data into the 'data' array
                        success = pn532_mifareclassic_ReadDataBlock(&nfc, currentblock, data);
                        if (success)
                        {
                            // Read successful
                            ESP_LOGI(READ_TAG, "Block %d", currentblock);
                            // Dump the raw data
                            esp_log_buffer_hexdump_internal(READ_TAG, data, 16, ESP_LOG_INFO);
                        }
                        else
                        {
                            // Oops ... something happened
                            ESP_LOGI(READ_TAG, "Block %d unable to read", currentblock);
                        }
                    }
                }
            }
            else
            {
                ESP_LOGI(READ_TAG, "Ooops ... this doesn't seem to be a Mifare Classic card!");
            }
        }

        else
        {
            // PN532 probably timed out waiting for a card
            ESP_LOGI(READ_TAG, "Timed out waiting for a card");
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}