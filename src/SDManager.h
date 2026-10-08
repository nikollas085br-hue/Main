#pragma once
#include <Arduino.h>
namespace SDManager {
bool begin(); bool ready(); String list(const String& path); String readText(const String& path); bool isDir(const String& path); String normalize(const String& base,const String& name);
}
