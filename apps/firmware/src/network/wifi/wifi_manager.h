#pragma once

#include <WiFi.h>
#include <esp_netif.h>

#include "config.h"
#include "../dns/dns_forwarder.h"

#if EDGEGATE_AP_INTERNET
extern "C" {
#include "lwip/lwip_napt.h"
#include "lwip/netif.h"
}
#endif

class WiFiManager {
public:

using ClientCallback =
void ()(const char, bool);

using DnsQueryCallback =
void ()(const char, const IPAddress&, bool*);

void setClientCallback(ClientCallback cb){
client_cb_=cb;
}

void setDnsQueryCallback(
DnsQueryCallback cb
){
dns_.setQueryCallback(cb);
}

bool begin(){

WiFi.onEvent(
[this](
WiFiEvent_t e,
WiFiEventInfo_t i
){
onEvent(e,i);
}
);

WiFi.mode(
WIFI_AP
);

IPAddress ap(
192,
168,
4,
1
);

WiFi.softAPConfig(
ap,
ap,
IPAddress(
255,
255,
255,
0
)
);

if(
!WiFi.softAP(
WIFI_AP_SSID,
WIFI_AP_PASSWORD,
1,
0,
8
)
){
return false;
}

if(
!dns_.begin(
53
)
){
return false;
}

sta_started_=false;

internet_=false;

return true;

}

void tickSta(){

if(
strlen(
WIFI_STA_SSID
)==0
){
return;
}

if(
!sta_started_
){

WiFi.mode(
WIFI_AP_STA
);

WiFi.begin(
WIFI_STA_SSID,
WIFI_STA_PASSWORD
);

sta_started_=true;

sta_ms_=millis();

Serial.printf(
"[wifi] connecting %s\n",
WIFI_STA_SSID
);

}
else if(
!staConnected()
&&
millis()-sta_ms_

«»

30000
){

WiFi.disconnect(
true
);

delay(
500
);

WiFi.begin(
WIFI_STA_SSID,
WIFI_STA_PASSWORD
);

sta_ms_=
millis();

internet_=
false;

Serial.println(
"[wifi] retry"
);

}

}

void processDNS(){

dns_.processNext();

}

bool staConnected() const{

return
WiFi.status()

WL_CONNECTED;

}

bool internetSharingEnabled() const{

return internet_;

}

IPAddress apIP() const{

return WiFi.softAPIP();

}

private:

#if EDGEGATE_AP_INTERNET

bool enableNapt(){

#if defined(EDGEGATE_NAPT_LIB)

if(
WiFi.AP.enableNAPT(
true
)
){

Serial.println(
"[wifi] NAPT via API"
);

return true;

}

#endif

IPAddress ap=
WiFi.softAPIP();

ip_napt_enable(
(uint32_t)ap,
1
);

delay(
100
);

for(
struct netif* n=
netif_list;
n;
n=n->next
){

if(
n->name[0]=='a'
&&
n->name[1]=='p'
&&
n->napt
){

Serial.println(
"[wifi] NAPT active"
);

return true;

}

}

Serial.println(
"[wifi] NAPT failed"
);

return false;

}

void disableNapt(){

#if defined(
EDGEGATE_NAPT_LIB
)

WiFi.AP.enableNAPT(
false
);

#endif

IPAddress ap=
WiFi.softAPIP();

ip_napt_enable(
(uint32_t)ap,
0
);

}

#endif

void enableInternet(){

#if EDGEGATE_AP_INTERNET

if(
internet_
||
!staConnected()
){
return;
}

IPAddress dns=
WiFi.dnsIP();

if(
dns==
IPAddress(
0,
0,
0,
0
)
){

dns=
IPAddress(
8,
8,
8,
8
);

}

dns_.setUpstream(
dns
);

if(
enableNapt()
){

internet_=true;

Serial.printf(
"[wifi] internet ON dns=%s\n",
dns.toString().c_str()
);

}

#endif

}

void disableInternet(){

#if EDGEGATE_AP_INTERNET

disableNapt();

internet_=false;

#endif

}

void onEvent(
WiFiEvent_t e,
WiFiEventInfo_t info
){

if(
e==
ARDUINO_EVENT_WIFI_STA_GOT_IP
){

Serial.printf(
"[wifi] STA up %s\n",
WiFi.localIP().toString().c_str()
);

enableInternet();

}

else if(
e==
ARDUINO_EVENT_WIFI_STA_DISCONNECTED
){

disableInternet();

Serial.printf(
"[wifi] STA down %d\n",
info.wifi_sta_disconnected.reason
);

}

else if(
e==
ARDUINO_EVENT_WIFI_AP_STACONNECTED
){

char mac[18];

snprintf(
mac,
18,
"%02X:%02X:%02X:%02X:%02X:%02X",
info.wifi_ap_staconnected.mac[0],
info.wifi_ap_staconnected.mac[1],
info.wifi_ap_staconnected.mac[2],
info.wifi_ap_staconnected.mac[3],
info.wifi_ap_staconnected.mac[4],
info.wifi_ap_staconnected.mac[5]
);

if(
client_cb_
){
client_cb_(
mac,
true
);
}

}

}

DNSForwarder dns_;

ClientCallback client_cb_=nullptr;

bool sta_started_=false;

bool internet_=false;

unsigned long sta_ms_=0;

};