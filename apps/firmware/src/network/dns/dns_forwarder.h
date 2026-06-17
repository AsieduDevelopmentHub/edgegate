#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>

class DNSForwarder {

public:

using QueryCallback=
void (*)(const char*,const IPAddress&,bool*);

bool begin(
uint16_t port=53
){
return dns_.begin(
port
);
}

void setUpstream(
IPAddress ip
){
upstream_=ip;
}

void setQueryCallback(
QueryCallback cb
){
cb_=cb;
}

void processNext(){

int size=
dns_.parsePacket();

if(
size<=0
)return;

IPAddress client=
dns_.remoteIP();

uint16_t port=
dns_.remotePort();

int len=
dns_.read(
packet_,
512
);

if(
len<=0
)return;

bool block=
false;

char domain[64];

extractDomain(
domain,
len
);

if(cb_){

cb_(
domain,
client,
&block
);

}

if(block){

replyNX(
client,
port,
len
);

return;

}

forward(
client,
port,
len
);

}

private:

void forward(
IPAddress client,
uint16_t port,
int len
){

WiFiUDP up;

up.begin(30000);

up.beginPacket(
upstream_,
53
);

up.write(
packet_,
len
);

up.endPacket();

unsigned long t=
millis();

while(
millis()-t
<1200
){

if(
up.parsePacket()
){

int n=
up.read(
resp_,
512
);

dns_.beginPacket(
client,
port
);

dns_.write(
resp_,
n
);

dns_.endPacket();

break;

}

delay(1);

}

up.stop();

}

void replyNX(
IPAddress ip,
uint16_t port,
int len
){

packet_[2]|=0x80;

packet_[3]=3;

dns_.beginPacket(
ip,
port
);

dns_.write(
packet_,
len
);

dns_.endPacket();

}

void extractDomain(
char* out,
int len
){

int p=12;

int w=0;

while(
p<len
){

int l=
packet_[p++];

if(
l==0
)
break;

if(
w
)
out[w++]='.';

memcpy(
out+w,
packet_+p,
l
);

w+=l;

p+=l;

}

out[w]=0;

}

WiFiUDP dns_;

IPAddress upstream_;

QueryCallback cb_=nullptr;

uint8_t packet_[512];

uint8_t resp_[512];

};