#include <ps2.h>

#define P_DATA  2
#define P_CLOCK 3

// pins 8-12
#define S_DATA  8
#define S_LATCH 9
#define S_CLOCK 10

#define REVERSE_BUTTONS
#define LED



#define checkLatch (PINB&(1<<(S_LATCH-8)))
#define checkClock (PINB&(1<<(S_CLOCK-8)))
inline void writeData(byte x){ if(x) PORTB|=(1<<(S_DATA-8)); else PORTB&=~(1<<(S_DATA-8)); }

inline byte reverse(byte b) { // thanks "sth" from StackOverflow
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

byte mdata[3];
byte output[2];
byte bitout[32];
byte i;
signed char x,y;
#ifdef LED
byte led;
#endif

PS2 mouse(P_CLOCK, P_DATA);
void mouse_init()
{
  mouse.write(0xff);  // reset
  mouse.read();
  mouse.read();
  mouse.read();
  /* // wheel mouse mode
  
  mouse.write(0xf3);  // sample rate
  mouse.read();  // ack
  mouse.write(0xc8);  // sample rate
  mouse.read();  // ack
  
  mouse.write(0xf3);  // sample rate
  mouse.read();  // ack
  mouse.write(0x64);  // sample rate
  mouse.read();  // ack
  
  mouse.write(0xf3);  // sample rate
  mouse.read();  // ack
  mouse.write(0x50);  // sample rate
  mouse.read();  // ack
  
  mouse.write(0xf2);  // get mouse id
  mouse.read();  // ack
  mouse.read();  // mouse id (we would care about this if we actually scrolled)*/
  
  delayMicroseconds(100);
}
void mouse_update()
{
  mouse.write(0xeb);
  mouse.read();
  mdata[0] = mouse.read();
  mdata[1] = mouse.read();
  mdata[2] = mouse.read();
}

void setup() {
  mouse_init();
  pinMode(S_DATA,OUTPUT);
  pinMode(S_LATCH,INPUT_PULLUP);
  pinMode(S_CLOCK,INPUT_PULLUP);
  digitalWrite(S_DATA,LOW);
  
  #ifdef LED
  led=0;
  pinMode(13,OUTPUT);
  #endif
  
  for(i=0;i<16;i++){
    bitout[i]=1;
  }
}

void loop() {
  mouse_update();
  
  x=(signed char)mdata[1];
  y=(signed char)mdata[2];
  output[0]=reverse(~((min(abs(y),0x7f))|((y<0)?0x00:0x80))); //y
  output[1]=reverse(~((min(abs(x),0x7f))|((x<0)?0x80:0x00))); //x
  for(i=16;i<32;i++){
    bitout[i]=output[(i>>3)&1]&(1<<(i&7));
  }
  
  #ifdef REVERSE_BUTTONS
  bitout[8]=mdata[0]&0x01?0:1;
  bitout[9]=mdata[0]&0x02?0:1;
  #else
  bitout[8]=mdata[0]&0x02?0:1;
  bitout[9]=mdata[0]&0x01?0:1;
  #endif
  
  bitout[15]=0;
  i=0;
  
  #ifdef LED
  if (led) {
    led=0;PORTB&=~(1<<(13-8));
  } else {
    led=1;PORTB|=1<<(13-8);
  }
  #endif
  
  noInterrupts();
  while(!checkLatch);
  writeData(1);
  while(checkLatch);
  for(;i<32;i++){
    writeData(bitout[i]);
    while(checkClock);  //wait for falling
    while(!checkClock); //wait for rising
  }
  interrupts();
}
