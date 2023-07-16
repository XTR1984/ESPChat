/*
  ESPChat creates a WiFi access point and provides a simple webchat on it with save chat on flash.

*/
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "FS.h"
#include <LittleFS.h> // LittleFS is declared
#include <esp_task_wdt.h> //watchdog

#define WDT_TIMEOUT 120  //watchdog timeout

// Set these to your desired credentials.
const char *ssid = "Secret";
const char *password = "radio123";

//WiFiServer server(80);
WebServer server(80);
// DNS server 
const byte DNS_PORT = 53; 
DNSServer dnsServer;
IPAddress myIP;
int last_id = -1;

String buffer[100];

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}


void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}


void setup() {
  Serial.begin(115200);
  Serial.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
 
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid,password);
  myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError); 
  dnsServer.start(DNS_PORT, "*", myIP);
  server.serveStatic("/chat.html", LittleFS, "/chat.html");
  server.serveStatic("/js/jquery-3.6.0.min.js", LittleFS, "/js/jquery-3.6.0.min.js");
  server.on("/generate_204", handleRoot);
  server.on("/get_msgs", handle_getmsgs);
  server.on("/add_msg", HTTP_POST, handle_addmsg);
  server.on("/clear434343", HTTP_GET, handle_clear);
  server.onNotFound(handleRoot);
// init messages
  File mdir = LittleFS.open("/messages/");
  if(!mdir){
        Serial.println("failed to open messages directory");
        createDir(LittleFS, "/messages");       
        File mdir = LittleFS.open("/messages/");
        if(!mdir){
          Serial.println("failed to create messages directory");
          return;
        }
  }

  // filename in messages dir is message id
  File file = mdir.openNextFile();
  int n = 0;
  while(file){
        String fname = String(file.name());
        n = fname.toInt();
        if(n > last_id)
           last_id = n;
        file = mdir.openNextFile();
  }
  Serial.println("Last_id = " + String(last_id));
  server.begin();
  Serial.println("Server started");

 
}

boolean captivePortal() {
  Serial.println("Hostheader: "+ server.hostHeader()); 
  if (server.hostHeader()!=IpAddress2String(myIP)) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", "http://"+ server.client().localIP().toString()+"/chat.html", true);
    server.send(302, "text/plain", "");   
    server.client().stop(); 
    return true;
  }
  return false;
}

void handle_getmsgs() {
  uint8_t buf[512];
  Serial.println("Handle get_msgs");
  //String args = server.arg(0);
  //Serial.println(args);
  //int from_id = args.substring(args.indexOf(":")+1,args.indexOf("}")).toInt();
  int from_id = server.arg(0).toInt();
  if (from_id < last_id-200){
    from_id = last_id-200;
  }
  Serial.println(String("from_id:") + String(from_id));
  Serial.println(String("last_id:") + String(last_id));
  File mdir = LittleFS.open("/messages/");
  if(!mdir){
        Serial.println("failed to open messages directory");
        return;
  }
  if(!mdir.isDirectory()){
        Serial.println("Messages is not directory");
        return;
  }
  mdir.close();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");
  //server.sendContent("[");
  String content = "[";
  boolean first = true;
  File file;
  size_t toRead = 0;
  int counter = 0;
  int maxcount = 10;
  while(from_id <= last_id and counter<maxcount){
      counter++;
      file = LittleFS.open("/messages/" + String(from_id));
      size_t len = 0;
      if(file.available()){
          if(!first) content+=","; //server.sendContent(",");
          if (first) first = false;
          len = file.size();
          toRead = len;
          if(toRead > 512){
              toRead = 512;
          }
          file.read(buf, toRead);
          buf[toRead]='\0';
          content += String((char*)buf);
          if (content.length()>10000){
             server.sendContent(content);
             content = "";
          }
          //Serial.println("send file " + String(file.name()));
          file.close();
      }
      from_id +=1;
  }
  content +="]";
  server.sendContent(content);
  server.client().stop();
}



void handle_clear() {
  File mdir = LittleFS.open("/messages/");
  if(!mdir){
        Serial.println("failed to open messages directory");
        return;
  }
  File file = mdir.openNextFile();
  while(file){
        String path = file.path();
        file.close();
        LittleFS.remove(path);
        file = mdir.openNextFile();
  }      
  server.send(200, "text/html", "");
  server.client().stop();
  last_id = -1;
}


void handle_addmsg() {
  Serial.println("Handle add_msg");
  //String message = "{";
  //for (uint8_t i = 0; i < server.args(); i++) {
  //   message += "{" + server.argName(i) + ": " + server.arg(i) + ",";
  //}
  String message = server.arg(0);
  last_id += 1;
  message.replace("}",String(",\"id\":") + String(last_id) + String("}"));
  Serial.println(message);
  writeFile(LittleFS, (String("/messages/")+String(last_id)).c_str(), message.c_str());
  server.send(200, "text/html", "[]");
  server.client().stop();
}


void handleRoot() {
  Serial.println("Handle root");
  if (captivePortal()) { 
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  SendStatic("/chat.html");
  server.client().stop();
}

void SendStatic(String filepath) {
  static uint8_t buf[512];
  Serial.println("sendstatic:" + filepath);
  File file = LittleFS.open(filepath);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  int fsize = file.size();
  Serial.println("filesize: " + String(fsize));
  server.setContentLength(fsize);
  int len = fsize;
  server.send(200, "text/html", "");
  while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            server.sendContent((char *) buf,toRead);
            len -= toRead;
        }
  server.client().stop();
  file.close();
}

int last = millis();

void loop() {
  server.handleClient();
  dnsServer.processNextRequest();
  //watchdog reset
  if (millis() - last >= 20000) {
      Serial.println("Resetting WDT...");
      esp_task_wdt_reset();
      last = millis();
  }
  //WiFiClient client = server.available();   // listen for incoming clients
  
}
