/*
 * Configure ICE40 with bitstream from offset 0x0801F000 in STM32 flash
 * (if present), and then repeatedly from Serial (USB CDC-ACM) port
 */

#include <MyStorm.h>
#include <QSPI.h>

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  myStorm.FPGAConfigure((const byte*)0x0801F000, 135100);
  digitalWrite(LED_BUILTIN, 0);

  QSPI.begin(20000000, QSPI.Mode3);
  QSPI.beginTransaction();
}

void loop()
{
  byte tx_data = 0xa5, rx_data;
  
  if (!QSPI.write(&tx_data, 1))
    Serial.println("QSPI.transmit failed");
  if (!QSPI.read(&rx_data, 1))
    Serial.println("QSPI.receive failed");
  
  if (!Serial.available())
    return;

  // Configure from USB1
  digitalWrite(LED_BUILTIN, 1);
  if (myStorm.FPGAConfigure(Serial)) {
    while (Serial.available())
      Serial.read();
  }
  digitalWrite(LED_BUILTIN, 0);
}

