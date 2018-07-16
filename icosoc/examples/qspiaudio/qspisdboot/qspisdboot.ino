/*
 * Configure ICE40 with bitstream from offset 0x0801F000 in STM32 flash
 * (if present), and then repeatedly from Serial (USB CDC-ACM) port
 * If the first character read id not 0xFF then the input is a filename
 * to read from the SD card and send via QSPI to the Ice40.
 */

#include <MyStorm.h>
#include <QSPI.h>
#include <FS.h>

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  myStorm.FPGAConfigure((const byte*)0x0801F000, 135100);
  digitalWrite(LED_BUILTIN, 0);
    
  while (!Serial) ; // wait for USB serial connection to be established
  
  DOSFS.begin();

  Serial.println("SDCard files: ");
  Dir dir = DOSFS.openDir("/");
  do {
    Serial.println(dir.fileName());
  } while (dir.next());
  
  DOSFS.end();

  QSPI.begin(20000000, QSPI.Mode3);
}

void loop()
{
  char fileName[64];
  byte tx_data = 0xa5, rx_data;

  if (!Serial.available())
    return;

  int c = Serial.read();

  if (c == 0xFF) {
    // Configure from USB1
    digitalWrite(LED_BUILTIN, 1);
    if (myStorm.FPGAConfigure(Serial)) {
      while (Serial.available())
        Serial.read();
    }
    digitalWrite(LED_BUILTIN, 0);
  } else {
    if (readLine(c, Serial, fileName, sizeof fileName) == 0)
      return;

    // open the file on the SD card file system
    DOSFS.begin();
    File file = DOSFS.open(fileName, "r");
    if (file) {
      // Send the file to the Ice40
      Serial.print("Sending: ");
      Serial.println(fileName);
      digitalWrite(LED_BUILTIN, 1);
      QSPI.beginTransaction();
      while(file.available()) {
        byte data = file.read();
        if (!QSPI.write(&data, 1)) {
          Serial.println("QPSI transmit failed");
          break;
        }
        delayMicroseconds(100); // Send fairly slowly
      }
      digitalWrite(LED_BUILTIN, 0);
      file.close();
      QSPI.endTransaction();
    } else
      Serial.println("file not found");
    DOSFS.end();
  }
}

/*
 * Read and echo a line from given input stream until terminated
 * by '\n', '\r' or '\0' or until the buffer is full.
 * Discard the terminating character and append a null character.
 * 
 * Returns the number of input characters in the buffer (excluding the null).
 */
int readLine(char first, Stream &str, char *buf, int bufLen)
{
  int c, nread;

  // discard any extra CR or NL left from previous readLine
  do {
    c = str.read();
  } while (c == -1 || c == '\n' || c == '\r');
  // read until buffer until termination character seen or buffer full
  buf[0] = first;
  nread = 1;
  while (c != '\0' && c != '\n' && c != '\r') {
    str.write(c);
    buf[nread] = c;
    ++nread;
    if (nread == bufLen - 1)
      break;
    do {
      c = str.read();
    } while (c == -1);
  }
  // mark end of line and return
  buf[nread] = '\0';
  return nread;
}


