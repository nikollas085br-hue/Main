#pragma once
#include <Arduino.h>
namespace SDManager {
    bool begin();
    bool ready();
    void listRoot();
    String readText(const String& path);
    bool writeText(const String& path, const String& content, bool append=false);
    bool saveUploaded(const String& name, const uint8_t* data, size_t len);
}
