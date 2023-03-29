#include "ota.h"
#include <Update.h>
//./OTAUpdate -f /tmp/arduino_build_857937/pufferbot.ino.bin -i 192.168.137.155 -p 5000
uint32_t chunk_size = OTA_CHUNK_SIZE;

bool ota_read_data(WiFiClient &client, uint8_t *buffer, size_t buffer_size, bool wait){
  if (client) { 
    if (client.connected()){ 
      size_t i = 0;
      if (!wait) {
        if (client.peek()==-1) return false;
      }
      while (i < buffer_size){
        i += client.read( buffer + i, buffer_size - i); 
      }
      return (i == buffer_size);
    } 
  }
  return false;
}

void ota_Update (WiFiClient &ota_client){
  while(!ota_client.connected());
  Serial.println("OTA Update started");
  uint32_t wifi_update_size = 0;
  ota_read_data(ota_client, wifi_update_size, true);
  ota_client.write((uint8_t*) &chunk_size, sizeof(chunk_size));
  uint32_t chunks = wifi_update_size / chunk_size;
  Update.begin(wifi_update_size);
  uint8_t *b = (uint8_t *)malloc(chunk_size);
  if (b==NULL) { 
    Serial.println("FAILED TO ALLOCATE MEMORY");
    return;
  } 
  Update.begin(wifi_update_size);
  uint32_t i;
  for (i=0;i < chunks; i++){
    ota_read_data(ota_client, b, chunk_size, true);
    Serial.printf("Writing at 0x%08x...(%d%%)\n", i*chunk_size, ( i * 100 / chunks));
    ota_client.write((uint8_t*) &i, sizeof(i));
    Update.write(b, chunk_size);
  }
  Serial.printf("Writing at 0x%08x...(100%%)\n", i*chunk_size);
  ota_read_data(ota_client, b, wifi_update_size % chunk_size, true);
  Update.write(b,  wifi_update_size % chunk_size);
  Serial.printf("Wrote %d bytes\n", wifi_update_size);
  if (Update.end(true)) { 
    Serial.printf("Rebooting...\n", wifi_update_size); 
    ESP.restart();
  } else { 
    Update.printError(Serial); 
  } 
}
