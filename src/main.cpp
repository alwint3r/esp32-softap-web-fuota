#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Update.h>

const char *ssid = "ESPASYNCWEB";
const char *password = "ESPASYNCWEB";

const char *PARAM_MESSAGE = "message";

DNSServer dnsServer;
AsyncWebServer server(80);

const char* serverIndex = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

void setup()
{
    Serial.begin(115200);
    WiFi.softAP(ssid, password);

    Serial.println("Begin!");

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", serverIndex); });

    server.on(
        "/upload", HTTP_POST, [](AsyncWebServerRequest *request)
        {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError() ? "NOT OK" : "OK");
            response->addHeader("Connection", "closed");
            request->send(response);
            ESP.restart();
        },
        onUpload);
    server.begin();
}

void loop()
{
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index)
    {
        Serial.printf("Update start: %s\r\n", filename.c_str());
        if (!Update.begin())
        {
            Update.printError(Serial);
        }
    }
    if (!Update.hasError())
    {
        if (Update.write(data, len) != len)
        {
            Update.printError(Serial);
        }
    }

    if (final)
    {
        if (Update.end(true))
        {
            Serial.printf("Update success: %uB\r\n", index + len);
        }
        else
        {
            Update.printError(Serial);
        }
    }
}