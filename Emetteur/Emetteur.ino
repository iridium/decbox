#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
const uint64_t addr = 0xE8E8F0F0E1LL; 
float pot_val;
String InitMSG;
const int b_pin = 5;
bool trig = false;
int menu = 0; // 1 -> Test connection ; 2 -> Déclenchement instantané ; 3 -> Déclenchement retardé ; 4 ->  Temps de déclenchement
int menu_f = 0;
float t;
int t1;
float td = 1;
const int led_secu = 2;
const int btn_secu = 7;

RF24 radio(9, 10);

byte croix[] = {
  B00000,
  B10001,
  B11011,
  B01110,
  B01110,
  B11011,
  B10001,
  B00000
};

byte OK[] = {
  B00000,
  B00000,
  B00001,
  B00011,
  B00010,
  B10110,
  B11100,
  B01000
};

byte bomb[] = {
  B00010,
  B00010,
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110
};

byte bomb_ex[] = {
  B00100,
  B10001,
  B11000,
  B00010,
  B10011,
  B01001,
  B01000,
  B10000
};

byte bombexleft[] = {
  B00100,
  B00011,
  B00001,
  B00110,
  B00011,
  B00000,
  B00000,
  B00000
};

byte bombexright[] = {
  B01000,
  B11000,
  B00010,
  B01000,
  B00010,
  B10000,
  B01100,
  B00000
};

byte arrow[] = {
  B10000,
  B11000,
  B11100,
  B11110,
  B11100,
  B11000,
  B10000,
  B00000
};

byte ee[] = {
  B01110,
  B00000,
  B01110,
  B10001,
  B11111,
  B10000,
  B01110,
  B00000
};
void setup()
{ 
  
  pinMode(b_pin, INPUT_PULLUP);
  pinMode(btn_secu, INPUT_PULLUP);
  lcd.begin(16,2);
  lcd.createChar(1,ee);
  lcd.createChar(2,arrow);
  lcd.createChar(3,bomb);
  lcd.createChar(4,bomb_ex);
  lcd.createChar(5,bombexleft);
  lcd.createChar(6,bombexright);
  lcd.createChar(7,OK);
  
  lcd.backlight();
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel( RF24_PA_MAX ) ; 
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(addr);
}

void loop()
{
  if(menu == 0) {
    lcd.setCursor(0,0);
    pot_val = map(analogRead(A0), 0, 1024, 1,5);
    if(pot_val == 1) {
      lcd.print(String("\2")+"Test Connection    ");
      lcd.setCursor(0,1);
      lcd.print("                  "); 
      menu_f = 1;
    }else if(pot_val == 2) {
      lcd.print(String("\2")+"D"+String("\1")+"clenchement    ");
      lcd.setCursor(0,1);
      lcd.print("  Instantan"+String("\1")+"   ");
      menu_f = 2;
    }else if(pot_val == 3) {
      lcd.print(String("\2")+"D"+String("\1")+"clenchement     ");
      lcd.setCursor(0,1);
      lcd.print("   Retard"+String("\1")+"      ");
      menu_f = 3;
    }else if(pot_val == 4) {
      lcd.print(String("\2")+"Temps de          ");
      lcd.setCursor(0,1);
      lcd.print("  D"+String("\1")+"clenchement");
      menu_f = 4;
    }
    if(!digitalRead(b_pin)) {
      menu = menu_f;
      delay(500);
    }
  }else if (menu == 3){
    
     while(!trig) {
       lcd.setCursor(0,0);
       pot_val = map(analogRead(A0),0,1024,10,150);
       t = pot_val/10;
       lcd.print("   D"+String("\1")+"lai :         ");
       lcd.setCursor(0,1);
       lcd.print("    "+String(t) + "s          ");
       Serial.println(t);
       if(!digitalRead(b_pin)) trig = true;
    }

    if(!digitalRead(btn_secu)) { 
      lcd.clear();
      //A -> Delay "D;5.2;3" Delay de 5.2s pendant 3s
      //D -> Direct "D;0;3" déclenchement instantané pendant 3s
      //T -> test connection
      InitMSG = "A;"+String(t)+";"+String(td);
      char msg[] = "";
      InitMSG.toCharArray(msg, 32);
      if(radio.write(&msg, strlen(msg))) {
          for(float i = t; i > 0; i-=0.1) {
            lcd.setCursor(0,0);
            lcd.print("  "+String("\3")+"  "+String(i)+" ");
            lcd.setCursor(12,0);
            lcd.print(String("\3"));
            lcd.setCursor(0,1);
            lcd.print("  "+String("\3")+"  !!!!!  "+String("\3"));
            delay(70);
          }
  
  
          lcd.setCursor(0,0);
          lcd.print(" "+String("\5")+String("\4")+String("\6")+" 0.00  "+String("\5")+String("\4")+String("\6"));
          lcd.setCursor(0,1);
          lcd.print(" "+String("\5")+String("\4")+String("\6")+" !!!!! "+String("\5")+String("\4")+String("\6"));
          delay(2000);
          trig = false;
          menu = 0;
      }else {
        lcd.setCursor(0,0);
        lcd.print("Erreur : hote       ");
        lcd.setCursor(0,1);
        lcd.print("X injoignable X     ");
        delay(2000);
        menu = 0;
        trig = false;
      }
    }else {
      lcd.setCursor(0,0);
      lcd.print("Erreur: S"+String("\1")+"curit"+String("\1")+"       ");
      lcd.setCursor(0,1);
      lcd.print("X Enclench"+String("\1")+"e      ");
      delay(1500);
      trig = false;
    }
    // Pb millis() ?
    /*while(millis()-t1 < t*1000) {
          lcd.setCursor(0,0);
          lcd.print("  "+String("\3")+"  "+String(t-float((millis()-t1))/1000)+" ");
          lcd.setCursor(12,0);
          lcd.print(String("\3"));
          lcd.setCursor(0,1);
          lcd.print("  "+String("\3")+"  !!!!!  "+String("\3"));
    }*/
 
    
  }else if (menu == 1) {
    InitMSG = "T;0000;"+String(td);
    char msg[] = "";
    InitMSG.toCharArray(msg, 32);
      if(radio.write(&msg, strlen(msg))) {
        lcd.setCursor(0,0);
        lcd.print(String("\7")+" Boitier "+String("\7")+"           ");
        lcd.setCursor(0,1);
        lcd.print(String("\7")+" Connect"+String("\1")+" "+String("\7"));
        lcd.setCursor(0,0);
        delay(2000);
        menu = 0;
      }else {
        lcd.setCursor(0,0);
        lcd.print("    boitier       ");
        lcd.setCursor(0,1);
        lcd.print("X d"+String("\1")+"connect"+String("\1")+" X");
        delay(2000);
        menu = 0;
      }
    
  }else if (menu == 2) {
    InitMSG = "D;0000;"+String(td);
    char msg[] = "";
    InitMSG.toCharArray(msg, 32);
    if(!digitalRead(btn_secu)) {
      if(!radio.write(&msg, strlen(msg))) {
        lcd.setCursor(0,0);
        lcd.print("Erreur : hote         ");
        lcd.setCursor(0,1);
        lcd.print("X injoignable X       ");
        delay(2000);
        menu = 0;
      }else {
        Serial.println("Test");
        lcd.setCursor(0,0);
        lcd.print("D"+String("\1")+"clenchement      ");
        lcd.setCursor(0,1);
        lcd.print("En cours...        ");
        lcd.setCursor(0,0);
        delay(2000);
        menu = 0;
        
      }
    }else {
      lcd.setCursor(0,0);
      lcd.print("Erreur: S"+String("\1")+"curit"+String("\1")+"       ");
      lcd.setCursor(0,1);
      lcd.print("X Enclench"+String("\1")+"e      ");
      delay(1500);
      menu = 0;
    }
  }else if (menu == 4) {
     while(!trig) {
       lcd.setCursor(0,0);
       pot_val = map(analogRead(A0),0,1024,10,150);
       td = pot_val/10;
       lcd.print("Temps ferm"+String("\1")+" :      ");
       lcd.setCursor(0,1);
       lcd.print("    "+String(td) + "s          ");
       if(!digitalRead(b_pin)) trig = true;
    }
    trig = false;
    delay(500);
    menu = 0;
  }
}





