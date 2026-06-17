#pragma once

#include <WiFi.h>
#include <esp_netif.h>

#include "config.h"
#include "../dns/dns_forwarder.h"

extern "C" {
#include "lwip/lwip_napt.h"
}

class WiFiManager {
public:

using ClientCallback =
void (*)(const char*, bool);

using DnsQueryCallback =
void (*)(const char*, const IPAddress&, bool*);

void setClientCallback(ClientCallback cb){
client_cb_=cb;
}

void setDnsQueryCallback(DnsQueryCallback cb){
dns_.setQueryCallback(cb);
}

bool begin(){

WiFi.mode(WIFI_AP);

IPAddress ap(192,168,4,1);

WiFi.softAPConfig(
ap,
ap,
IPAddress(255,255,255,0)
);

if(
!WiFi.softAP(
WIFI_AP_SSID,
WIFI_AP_PASSWORD
)
){
return false;
}

dns_.begin();

return true;
}

void tickSta(){

if(
strlen(
WIFI_STA_SSID
)==0
)return;

if(
WiFi.status()!=WL_CONNECTED
){

static uint32_t last=0;

if(
millis()-last
>10000
){

last=millis();

Serial.println(
"[wifi] reconnect"
);

WiFi.mode(
WIFI_AP_STA
);

WiFi.begin(
WIFI_STA_SSID,
WIFI_STA_PASSWORD
);

}

}
else{

if(
!internet_
){
enableInternet();
}

}

dns_.processNext();

}

bool staConnected() const{
return WiFi.status()
==
WL_CONNECTED;
}

bool internetSharingEnabled() const{
return internet_;
}

IPAddress apIP() const{
return WiFi.softAPIP();
}

private:

void enableInternet(){

IPAddress dns=
WiFi.dnsIP();

if(
dns==
IPAddress(0,0,0,0)
){
dns=
IPAddress(
8,8,8,8
);
}

dns_.setUpstream(dns);

ip_napt_enable(
(uint32_t)
WiFi.softAPIP(),
1
);

internet_=true;

Serial.println(
"[wifi] internet ON"
);

}

DNSForwarder dns_;

ClientCallback client_cb_=nullptr;

bool internet_=false;

};