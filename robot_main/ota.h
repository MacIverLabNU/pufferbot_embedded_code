#pragma once
#include <WiFi.h>

#define OTA_CHUNK_SIZE 40960

#define OTA(PORT) \
  WiFiServer ota_Server(PORT);\
  WiFiClient ota_Client;

#define OTA_INIT() \
  ota_Server.begin(); 

bool ota_read_data(WiFiClient &, uint8_t *, size_t, bool);

template<typename T>
bool ota_read_data(WiFiClient &client, T &v, bool wait){
  return ota_read_data(client, (uint8_t *) &v, sizeof(T), wait);
}


void ota_Update (WiFiClient &);

#define OTA_CHECK_UPDATES() \
  if (ota_Client = ota_Server.available()){ \
    ota_Update(ota_Client);\
  }
