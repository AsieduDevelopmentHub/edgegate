#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>

class DNSForwarder {

public:

using QueryCallback=
void ()(const char,const IPAddress&,bool*);

void setQueryCallback(
QueryCallback cb
){
cb_=cb;
}

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

void processNext(){

int size=
dns_.parsePacket();

if(
size<=0
||
size>512
){
return;
}

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
len<12
){
return;
}

char domain[128]={0};

parse(
domain,
len);

bool block=
false;

if(
cb_
){

cb_(
domain,
client,
&block
);

}

if(
block
){

nxdomain(
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

void parse(
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
l);

w+=l;

p+=l;

}

out[w]=0;

}

void forward(
IPAddress client,
uint16_t port,
int len
){

WiFiUDP up;

up.begin(
0
);

IPAddress dns=
upstream_;

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
WiFi.dnsIP();

}

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

up.beginPacket(
dns,
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
<
1500
){

int s=
up.parsePacket();

if(
s>0
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

delay(
2
);

}

up.stop();

}

void nxdomain(
IPAddress client,
uint16_t port,
int len
){

memcpy(
resp_,
packet_,
len
);

resp_[2]|=
0x80;

resp_[3]=
(resp_[3]&0xF0)
|
3;

dns_.beginPacket(
client,
port
);

dns_.write(
resp_,
len
);

dns_.endPacket();

}

WiFiUDP dns_;

IPAddress upstream_;

QueryCallback cb_=nullptr;

uint8_t packet_[512];

uint8_t resp_[512];

};