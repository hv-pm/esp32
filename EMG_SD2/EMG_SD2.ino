/*
   Simple data logger.
*/

//PLACA: ESP32 DEV MODULE

#include <SPI.h>
#include "SdFat.h"

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = SS;

// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"
//------------------------------------------------------------------------------
// File system object.
SdFat sd;

// Log file.
SdFile file;

int i = 0;
//==============================================================================
// User functions.  Edit writeHeader() and logData() for your requirements.

const uint8_t ANALOG_COUNT = 2; //número de colunas header após a primeira (CHANNELS)
//------------------------------------------------------------------------------
// Write data header.
void writeHeader() {
  file.print(F("micros")); //primeira coluna
  for (uint8_t i = 0; i < ANALOG_COUNT; i++) {
    file.print(F(",CHN"));
    file.print(i, DEC);
  }
  file.println();
}
//==============================================================================
// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))
//------------------------------------------------------------------------------
void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";

  Serial.begin(115200);

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  //delay(1000);

  Serial.println(F("Type any character to start"));
  while (!Serial.available()) {
    SysCall::yield();
  }

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(25))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    error("file.open");
  }
  // Read any Serial data.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  Serial.println(F("Type any character to stop"));

  // Write data header.
  writeHeader();
}

void loop() {
  String linha = String(i) + "," + String(float(analogRead(25))) + "," + String(float(analogRead(26)));
  file.println(linha);
  if (Serial.available()) {
    file.close();
    Serial.println(F("Done"));
    SysCall::halt();
  }
  delayMicroseconds(780); //1000 samples por segundo, para 2kHz -> delayMicroseconds(293)
  i++;
}
