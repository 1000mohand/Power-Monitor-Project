#include <Arduino.h>
#include <WString.h>
#include <WiFi.h>
#include <time.h>
#include <SPIFFS.h>
#include <freertos/task.h>
#include <map>
#include <tuple>

#include <style.hpp>
#include <devices.h>
#include "AsyncServer.h"


const IPAddress local_IP(192, 168,   1, 232);
const IPAddress gateway (192, 168,   1,   1);
const IPAddress subnet  (255, 255, 255,   0);
const IPAddress DNS1    (192, 168,   1,   1);
const IPAddress DNS2    (  8,   8,   8,   8);

String ssid     = "";
String password = "";
String getLine(bool echo = true, char delim = '\n');
std::map<int,int> as;

//time service
const char* ntp_server = "pool.ntp.org";
const long  gmt_offset_sec = 3600*2; //egypt time is GMT+2.
const int   daylight_offset_sec = 0; // day light hour is no 
                                     // longer standard in egypt.
void printLocalTime();
void keepPrintingLocalTime(void*);


////////////////////////////////////////////////////////////////////////

void setup()
{
    // Starting serial monitor

    Serial.begin(115200);


    // Starting SPIF File System (SPIFFS)

    SPIFFS.begin();                  // Start the SPI Flash Files System
    

    delay(5000);

    // load the devices
    Devices.begin();


#ifdef DEBUG
    Serial.println();
    Serial.println();
    Serial.println(__cplusplus);
#endif

    // getting user name and password

    //prompt to change ssid and password
    Serial.println("press any key to change wifi config");
    bool interacted = false;
    repeat(65){
        Serial.print('.');
        delay(50);
        if (Serial.available()){//user pressed a button
            Serial.read();      //remove user input from buffer
            interacted = true; 
            break;
        }
    }
    Serial.println();

    if (not interacted) //AND
    if (SPIFFS.exists("/ssid") && SPIFFS.exists("/password")){
        File f; size_t size; char* buff;

        f = SPIFFS.open("/ssid", "r");
        size = f.size();
        buff = new char[size];
        f.readBytes(buff,size);
        ssid = (char*) buff;
        delete[] buff;
        f.close();

        f = SPIFFS.open("/password", "r");
        size = f.size();
        buff = new char[size];
        f.readBytes(buff,size);
        password = (char*) buff;
        delete[] buff;
        f.close();

        goto wifi_confg_end;
    }

    
    code_block(/* start of user interactive wifi config */)

    Serial.print("please enter wifi ssid: ");Serial.flush();
    ssid = getLine();
    Serial.println("ssid is \"" + ssid + '"');
    what:
    Serial.print("show password ? y/n: ");Serial.flush();
    const auto ans = getLine();
    Serial.println("answer is \"" + ans + '"');
    if (ans == String("y")){
        Serial.print("please enter wifi password: ");Serial.flush();
        password = getLine();
    }
    else if (ans == String("n")){
        Serial.print("please enter wifi password: ");Serial.flush();
        password = getLine(false);
    }
    else{
        Serial.println("what ??" "!");Serial.flush();
        goto what;
    }

    // last thing save wifi data to ESP

    File f;

    f = SPIFFS.open("/ssid", "w");
    f.write( (uint8_t*) ssid.c_str(), ssid.length()+1);
    f.close();

    f = SPIFFS.open("/password", "w");
    f.write( (uint8_t*) password.c_str(), password.length()+1);
    f.close();

    end_block(/* end of interactive wifi config */)
    wifi_confg_end:


    // connecting to a WiFi network

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.config(local_IP,gateway,subnet,DNS1,DNS2);
    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


    // Time Configuration

    Serial.println("\nNow, configuring time");
    configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
    Serial.println("Time is:-\n");
    printLocalTime();
    auto ptr_tuple = new std::tuple<int,String> {15000, ""};   
    xTaskCreate(&keepPrintingLocalTime, "timeprint", 2048, ptr_tuple, 0,
                NULL);



    // Web server

    serverInit();
}


void loop(){

    digitalWrite(2,HIGH);

    Devices.devices_list.begin();

    repeat(10){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (!manually_changed) digitalWrite(2,!digitalRead(2));
    }

    repeat(10){
        vTaskDelay(50 / portTICK_PERIOD_MS);
        digitalWrite(2,!digitalRead(2));
    }
    
    while (!xSemaphoreTake(LED_mux,portMAX_DELAY));
    manually_changed = false;
    xSemaphoreGive(LED_mux);

}


/**********************************************************************/

void printLocalTime()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        //return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");
    

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour,3, "%H", &timeinfo);
    Serial.println(timeHour);

    char time_week_day[10];
    strftime(time_week_day,10, "%A", &timeinfo);
    Serial.println(time_week_day);
    Serial.println();
}

void keepPrintingLocalTime(void* param_ptr)
{
    const auto time_param_ptr =
                reinterpret_cast<std::tuple<int,String>*>(param_ptr);
    ref [delay_ms,format] = *time_param_ptr;
    Loop{
    if (format == ""){
        printLocalTime();
    }
    else{
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo)){
            Serial.println("Failed to obtain time");
        }
    Serial.println(&timeinfo, format.c_str());
    }

    vTaskDelay(delay_ms / portTICK_PERIOD_MS);
}}

String getLine(bool echo, char delim)
{
    String line = "";
    char ch = '\0';
    do{
        if (Serial.available()){
            ch = (char) Serial.read();
            if (echo)                                   
                then Serial.print(ch);
            if (ch != '\n' && ch != '\r' && ch != '\b') 
                then line+= ch;
            if (ch == '\b' && line.length() > 0)
                then line = line.substring(0,line.length()-1);
        }
    }while (ch != delim );
    return line;
}