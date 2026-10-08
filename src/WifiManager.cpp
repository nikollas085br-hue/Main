#include "WifiManager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>

namespace {
WebServer server(80);
bool started = false;

String html() {
    return "<!doctype html><meta charset='utf-8'><title>Cardputer</title>"
           "<h2>Central de Estudos</h2>"
           "<p>Envie anotacoes, imagens, PDF ou PPT/PPTX para o SD.</p>"
           "<form method='POST' action='/upload' enctype='multipart/form-data'>"
           "<input type='file' name='file'><button>Enviar</button></form>";
}

void handleRoot() { server.send(200, "text/html", html()); }

void handleUploadData() {
    HTTPUpload& u = server.upload();
    if (u.status == UPLOAD_FILE_START) {
        String path = "/anotacoes/" + u.filename;
        if (!SD.exists("/anotacoes")) SD.mkdir("/anotacoes");
        File f = SD.open(path, FILE_WRITE);
        if (f) f.close();
    } else if (u.status == UPLOAD_FILE_WRITE) {
        File f = SD.open("/anotacoes/" + u.filename, FILE_APPEND);
        if (f) { f.write(u.buf, u.currentSize); f.close(); }
    } else if (u.status == UPLOAD_FILE_END) {
        server.sendHeader("Location", "/");
    }
}
}

namespace WifiManager {
void begin() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Cardputer-Estudos");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, [](){ server.send(200, "text/plain", "Arquivo recebido."); }, handleUploadData);
    server.begin();
    started = true;
}
void update() { if (started) server.handleClient(); }
}
