#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <freertos/task.h>

#include <style.hpp>
#include "devices.h"

#include "AsyncServer.h"



AsyncWebServer server(80);

void turnOnRequest  (AsyncWebServerRequest *request);
void turnOffRequest (AsyncWebServerRequest *request);
void fileService    (AsyncWebServerRequest *request);
void read_LED       (AsyncWebServerRequest *request);
void GPIOoperate    (AsyncWebServerRequest *request);



void serverInit()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/Controls.html");
    });

    //turn on  LED in High
    server.on("/LED H", HTTP_GET, &turnOnRequest);

    //turn off LED in Low
    server.on("/LED L", HTTP_GET, &turnOffRequest);

    //read LED status
    server.on("/LED", HTTP_GET, &read_LED);

    //general pin operations service
    server.on("^\\/pin\\/(\\d+)(\\/|$)(.*)$", HTTP_ANY, &GPIOoperate);

    //file service
    server.on("^\\/(.*\\..*)$", HTTP_ANY , fileService);


    server.onNotFound([](AsyncWebServerRequest *request){
        Serial.println(request->url() +
                       " was requested but not found!");
        request->send(404, "text/html", "Not found");
    });


    server.begin();
}


// Request Functions ///////////////////////////////////////////////////

void turnOnRequest (AsyncWebServerRequest *request) {
    digitalWrite(LED, HIGH);
    request->send(SPIFFS, "/Controls.html");

    Serial.println("LED On");

    while (!xSemaphoreTake(LED_mux,portMAX_DELAY));
    manually_changed = true;
    xSemaphoreGive(LED_mux);
}


void turnOffRequest (AsyncWebServerRequest *request){
    digitalWrite(LED, LOW);
    request->send(SPIFFS, "/Controls.html");

    Serial.println("LED Off");

    while (!xSemaphoreTake(LED_mux,portMAX_DELAY));
    manually_changed = true;
    xSemaphoreGive(LED_mux);
}


void fileService    (AsyncWebServerRequest *request){
    Serial.print  ("new file request\t");
    Serial.print("methode: ");
    Serial.println(request->methodToString());

    if (SPIFFS.exists(request->url() ))
    then request->send(SPIFFS, request->url());
    else {
        Serial.println(request->url() +
                  " was requested through file service but not found!");
        request->send(404, "text/html", "Not found");
    }
}


void read_LED       (AsyncWebServerRequest *request){
    if (digitalRead(LED))
    then request->send(200, "text/plain", "On" );
    else request->send(200, "text/plain", "Off");
}


void GPIOoperate    (AsyncWebServerRequest *request){
    Serial.println("GPIO operation: " + request->url() );
    int pin = request->pathArg(0).toInt();

    switch (request->method()){
    case HTTP_GET:
        if (pin_reading.count(pin) != 0 ){
            Serial.println("GPIO unhandled reading fall-back to normal method");
            if   (request->pathArg(2).equalsIgnoreCase("digital") || request->pathArg(2) == "")
            then request->send(200, "text/plain", String(digitalRead(pin)));
            else if (request->pathArg(2).equalsIgnoreCase("analog"))
            then request->send(200, "text/plain", String(analogRead(pin)));
            else if (request->pathArg(2).equalsIgnoreCase("state"))
            then request->send(200, "text/plain", digitalRead(pin)==HIGH? "On" : "Off");
            else request->send(400, "text/plain", "bad reading mode");            
        }else{
            if   (request->pathArg(2).equalsIgnoreCase("digital") || request->pathArg(2) == "")
            then request->send(200, "text/plain", pin_reading.at(pin)>2457? "High":"Low");
            else if (request->pathArg(2).equalsIgnoreCase("analog"))
            then request->send(200, "text/plain", String(pin_reading.at(pin)));
            else if (request->pathArg(2).equalsIgnoreCase("state"))
            then request->send(200, "text/plain", pin_reading.at(pin)>2457? "On" : "Off");
            else request->send(400, "text/plain", "bad reading mode");
        }

        break;

    case HTTP_PUT:
        if (pin_reading.count(pin) > 0 ) {
            request->send(423, "text/plain", "Pin is read-only reserved, and Mode cannot be changed.");
            break;
        }

        if   (request->pathArg(2).equalsIgnoreCase("input"))
        then pinMode(pin,INPUT)         , request->send(200, "text/plain", "changed to input");
        else if (request->pathArg(2).equalsIgnoreCase("output"))
        then pinMode(pin,OUTPUT)        , request->send(200, "text/plain", "changed to output");
        else if (request->pathArg(2).equalsIgnoreCase("pullup"))
        then pinMode(pin,INPUT_PULLUP)  , request->send(200, "text/plain", "changed to pullup");
        else if (request->pathArg(2).equalsIgnoreCase("pulldown"))
        then pinMode(pin,INPUT_PULLDOWN), request->send(200, "text/plain", "changed to pulldown");
        else if (request->pathArg(2).equalsIgnoreCase("high"))
        then digitalWrite(pin, HIGH)    , request->send(200, "text/plain", "output to high");
        else if (request->pathArg(2).equalsIgnoreCase("low"))
        then digitalWrite(pin, LOW)     , request->send(200, "text/plain", "output to low");
        else request->send(400, "text/plain", "bad mode or value");
        break;
    
    default:
        request->send(400, "text/plain", "bad method"); 
        break;
    }
}

