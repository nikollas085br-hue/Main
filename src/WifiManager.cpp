#include "WifiManager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
static WebServer server(80); static bool active=false; static File uploadFile;
static void page(){server.send(200,"text/html","<html><meta name='viewport' content='width=device-width'><body><h2>Cardputer - Central de Estudos</h2><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='file'><button>Enviar</button></form><p>Arquivos enviados para /ANOTACOES</p></body></html>");}
static void upload(){HTTPUpload& u=server.upload(); if(u.status==UPLOAD_FILE_START){String n=u.filename; if(!n.startsWith("/"))n="/"+n; int slash=n.lastIndexOf('/'); String base="/ANOTACOES";String fn=n.substring(slash+1);SD.mkdir(base);uploadFile=SD.open(base+"/"+fn,"w");} else if(u.status==UPLOAD_FILE_WRITE){if(uploadFile)uploadFile.write(u.buf,u.currentSize);} else if(u.status==UPLOAD_FILE_END){if(uploadFile)uploadFile.close();}}
namespace WifiManager {void begin(){WiFi.mode(WIFI_AP);WiFi.softAP("Cardputer-Estudos");server.on("/",HTTP_GET,page);server.on("/upload",HTTP_POST,[]{server.send(200,"text/html","<p>Upload concluido.</p><a href='/'>Voltar</a>");},upload);server.begin();active=true;}void update(){if(active)server.handleClient();}bool running(){return active;}}
