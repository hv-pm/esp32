/*
   Simple data logger.
   * Adapted from sdfat library example "dataLogger"
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
int l = 0;
int cont = 0;
int prevmili = 0;
//==============================================================================
// User functions.  Edit writeHeader() and logData() for your requirements.

const uint8_t ANALOG_COUNT = 1; //número de colunas header após a primeira (CHANNELS)
//------------------------------------------------------------------------------
// Write data header.
void writeHeader() {
  file.print(F("micros")); //primeira coluna
  file.print(F(",label")); //primeira coluna
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

  //------------------------------------------------------------------------------
  pinMode(2, OUTPUT);
  delay(10000); //espera 10 segundos
  digitalWrite(2, HIGH); //liga onboard led para iniciar protocolo
  delay(7000); // espera 7 segundos antes de começar a gravar os dados
  digitalWrite(2, LOW); //desliga led
  prevmili = millis();
}

void loop() {
  //((ADC_VALUE * 3.3 ) / (4095)) / (10050)) -> 3.3V: output; 4095: 10 bits resolution; 10050: (G = 201 * Rgain / 1k), Rgain=50k ohm.
  // ( (3.3) / (4095) ) / (10050) = 8.018515481e-8.
  String linha = String(i) + "," + String(l) + ",";
  file.print(linha);
  file.println(double(analogRead(32) * 4.009257741e-8), 16);
  if (Serial.available()) {
    file.close();
    Serial.println(F("Done"));
    SysCall::halt();
  }
  if (millis() >= prevmili + 3000 & l == 0) {
    prevmili = millis();
    if (cont < 6) {
      l = 1;
    }
    else {
      l = 2;
    }
    cont++;
  }
  else if (millis() >= prevmili + 5000 & l == 1) {
    prevmili = millis();
    l = 0;
  }
  delayMicroseconds(773); //773 1000 samples por segundo, para 2kHz -> delayMicroseconds(293)
  i++;
}
