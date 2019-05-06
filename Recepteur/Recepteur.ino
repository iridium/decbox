#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

const uint64_t pipeIn = 0xE8E8F0F0E1LL;
RF24 radio(9, 10);  //CSN and CE pins

char msg[100]="";
int len = 0;
char mo[] = "ADTS"; // différents modes
char mode;
String delaiS;
int pin_buz = 7;
int pin_rel = 4;
float time_d;
int tm;
int td = 20;
int tbuz = 1000;

void setup()
{
  radio.begin();
  radio.setPALevel( RF24_PA_MAX ) ; 
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);  
  radio.openReadingPipe(0,pipeIn);
  
  radio.startListening();
  radio.printDetails();

  pinMode(pin_rel, OUTPUT);
  pinMode(pin_buz, OUTPUT);
}

unsigned long last_Time = 0;

void receive_data()
{
  if (radio.available()) {
    len = radio.getDynamicPayloadSize();
    radio.read(&msg, len);
    mode = msg[0];
    time_d = getValue(String(msg), ';', 2).toFloat()*1000;
  }
}

void loop()
{
  //Receive data
  receive_data();

  if(mode == mo[1]) {
    trig(time_d);
    mode = mo[3];
  }else if (mode == mo[0]) {
    delaiS = getValue(String(msg), ';', 1);
    float delai = delaiS.toFloat()*1000; //delai en ms
    //delay(delai);
    tm = millis();
    while(millis()-tm<delai) {
      digitalWrite(pin_buz, HIGH);
      delay(20);
      digitalWrite(pin_buz, LOW);
      delay(tbuz-tbuz*(millis()-tm)/(delai));
    }
    trig(time_d);
    mode = mo[3];
  }else if (mode == mo[2]) {
    digitalWrite(pin_buz, HIGH);
    delay(500);
    digitalWrite(pin_buz, LOW);
    mode = mo[3];
  }
}

void trig(float t) {
  digitalWrite(pin_rel, HIGH);
  if( t >= 1000) {
    delay(int(t));
  }else{
    delay(1500);
  }
  digitalWrite(pin_rel, LOW);
  mode = "S"; // STOP
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

