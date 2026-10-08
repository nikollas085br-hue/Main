#include "SDManager.h"
#include <SPI.h>
#include <SD.h>
#include <vector>

#define SD_SPI_SCK_PIN 40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN 12

namespace {
bool ok = false;
}

namespace SDManager {
bool begin() {
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    ok = SD.begin(SD_SPI_CS_PIN, SPI, 25000000);
    return ok;
}
bool ready() { return ok; }

void listRoot() {
    if (!ok) return;
    File root = SD.open("/");
    if (!root) return;
    File f = root.openNextFile();
    while (f) { f.close(); f = root.openNextFile(); }
    root.close();
}

String readText(const String& path) {
    if (!ok) return "";
    File f = SD.open(path, FILE_READ);
    if (!f) return "";
    String out;
    while (f.available()) out += char(f.read());
    f.close();
    return out;
}

bool writeText(const String& path, const String& content, bool append) {
    if (!ok) return false;
    File f = SD.open(path, append ? FILE_APPEND : FILE_WRITE);
    if (!f) return false;
    size_t n = f.print(content);
    f.close();
    return n == content.length();
}

bool saveUploaded(const String& name, const uint8_t* data, size_t len) {
    if (!ok) return false;
    String path = "/anotacoes/" + name;
    File f = SD.open(path, FILE_WRITE);
    if (!f) return false;
    size_t n = f.write(data, len);
    f.close();
    return n == len;
}
}
