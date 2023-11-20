// Project: LuggageTag
// Company: JW Marriot
// By: Kang Liat Chuan (Nov 2023) with some help from cGPT
// Description: Receive luggage tag from app (LDS.aia) via Bluetooth
//              Search for the RFID tag. When found, light LED and send signal to app
// Ref: https://www.electronicwings.com/esp32/rfid-rc522-interfacing-with-esp32
// Software: modified initially from DumpInfo.ino
// Hardware: ESP32 - MFRC522
//           3.3v  - 3.3v
//           GND   - GND
//           G0    - RST
//           G19   - MISO
//           G23   - MOSI
//           G18   - SCK
//           G5    - SDA

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "BluetoothSerial.h"

#define RST_PIN    0         // Configurable, see pin layout above
#define SS_PIN     5         // Configurable, see pin layout above
#define LED_PIN    27

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

String tag = "";
String tag_tmp = "";
String uidString = "";

void setup() {
	Serial.begin(115200);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SerialBT.begin("LDS"); //Bluetooth device name
  Serial.println("The device 'LDS' started, now you can pair it with bluetooth!");

	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("MFRC522 init done..."));
  pinMode(LED_PIN, OUTPUT);
}

void loop() {

  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();

    // Check for the end of the string
    if (incomingChar != '\n') {
      // Concatenate the incoming character to the string
      tag_tmp += incomingChar;
    } else {
      // End of the string, process the complete string
      tag = tag_tmp;
      tag_tmp = ""; // Reset the tag string for the next input
      Serial.println("Received Tag: " + tag);
    }
  }

  // Check for a card and read the UID
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Convert uidByte array to a string
    uidString = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      // Concatenate each byte to the string
      uidString += String(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println("UID: " + uidString);

    if ((uidString == tag) && !((tag == "") && (uidString == ""))) {
      digitalWrite(LED_PIN, HIGH);
    }
    delay(2000);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Halt the PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

}
