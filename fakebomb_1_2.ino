/* Fake Bomb
 * Auteur : Julien SCHRIVE (aka etwratel)
 * Date 23/07/2019
 * Version du code : 2
 * Version Arduino : 1.8.8
 * Description : Code source d'un C4 FACTICE disposant d'un affichage LCD, d'une LED de signalisation, d'un HP et de 2 boutons poussoir pour amorcage et sabotage.
 * (Version prototypée)
 */
 
 //Décommenter pour avoir du debugage USB
//#define DEBUG_USB


//Definition des états
#define PRET 0
#define AMORCAGE 1
#define SABOTAGE 2
#define HS 3
#define EXPLOSION 4
#define AMORCE 5

//Definition des timer (Amorcage, sabotage, explosion)
#define T_DEFUSE 15
#define T_PLANT 10
#define T_BLOW 30

//Definition de GPIO 
#define GPIO_D7 2
#define GPIO_D6 3
#define GPIO_D5 4
#define GPIO_D4 5
#define GPIO_EN 11 //6
#define GPIO_RS 12 //7
#define GPIO_HPLED 8
#define GPIO_PLANT 6
#define GPIO_DEFUSE 7
#define GPIO_LIGHT 9

//D'autres parametre utiles (dimension LCD, temps d'allumage LED et HP...)
#define WIDTH_LCD 16
#define HEIGHT_LCD 2
#define T_PULSE 100
#define CONTRAST 90


#include <LiquidCrystal.h>

LiquidCrystal lcd(GPIO_RS,GPIO_EN,GPIO_D4,GPIO_D5,GPIO_D6,GPIO_D7);
int state, buttonState;
unsigned long c_time,p_time, timer_pulse, pushed_time, elapsed_time;

void setup() {
#ifdef DEBUG_USB
  Serial.begin(9600);
#endif
  analogWrite(GPIO_LIGHT,CONTRAST);
  lcd.begin(WIDTH_LCD,HEIGHT_LCD);
  pinMode(GPIO_HPLED, OUTPUT);
  pinMode(GPIO_DEFUSE, INPUT);
  pinMode(GPIO_PLANT, INPUT);
  buttonState = LOW;
  state = PRET;
}

void loop() {
  switch(state){
    case PRET:
      //On affiche l'etat du C4 pret a etre amorce
      lcd.blink();
      lcd.setCursor(7,0);
      lcd.print("C4");
      lcd.setCursor(6,1);
      lcd.print("PRET");
      #ifdef DEBUG_USB
        Serial.println("C4 - PRET");
      #endif
      //On attend que le bouton d'amorcage soit enclenché
      do{
        buttonState = digitalRead(GPIO_PLANT);
      }while(buttonState != HIGH);
      //On recupere l'instant ou le bouton a été enclenché
      lcd.clear();
      p_time = millis();
      //On passe en etat d'amorcage
      state = AMORCAGE;
    break;
    case AMORCAGE:
      //On affiche
      lcd.setCursor(4,0);
      lcd.print("AMORCAGE");
      //On recupere le temps depuis lequel le bouton est enclenché
      pushed_time = millis() - p_time;
      //On l'affiche car c'est beau
      #ifdef DEBUG_USB
        Serial.println("Amorcage");
        Serial.println(T_PLANT-pushed_time/1000);
      #endif
      lcd.setCursor(7,1);
      if(T_PLANT-pushed_time/1000 < 10){
        lcd.print("0");
        lcd.setCursor(8,1);
      }
      lcd.print(T_PLANT-pushed_time/1000);
      //On récupère létat du bouton
      buttonState = digitalRead(GPIO_PLANT);
      //Si le bouton est enclenché le temps necessaire
      if((pushed_time > T_PLANT*1000) && (buttonState == HIGH)){
        //C4 amorcé
        state = AMORCE;
        //On recupere le temps courant
        c_time = millis();
        timer_pulse = c_time;
        lcd.clear();
      }else if((pushed_time <= T_PLANT*1000) && (buttonState != HIGH)){
      //Si bouton relaché -> on recommence tout
        lcd.clear();
        state = PRET;
      }
    break;
    case AMORCE:
      elapsed_time = millis() - c_time;
      buttonState = digitalRead(GPIO_DEFUSE);
      lcd.setCursor(5,0);
      lcd.print("AMORCE");
      lcd.setCursor(7,1);
      if(T_BLOW - elapsed_time/1000 < 10){
        lcd.print("0");
        lcd.setCursor(8,1);
      }
      lcd.print(T_BLOW - elapsed_time /1000);
      #ifdef DEBUG_USB
        Serial.println("Amorce");
        Serial.println(T_BLOW - elapsed_time /1000);
      #endif
      if(millis() - timer_pulse > 1000)
        digitalWrite(GPIO_HPLED,HIGH);
      if(millis() - timer_pulse > 1000+T_PULSE){
        digitalWrite(GPIO_HPLED,LOW);
        timer_pulse = millis();
      }
      if(elapsed_time > T_BLOW*1000){
        lcd.clear();
        state = EXPLOSION;
      }
      if((elapsed_time <= T_BLOW*1000) && (buttonState==HIGH)){
        state = SABOTAGE;
        p_time = millis();
        lcd.clear();
      }
    break;
    case SABOTAGE:
      pushed_time = millis() - p_time;
      elapsed_time = millis() - c_time;
      buttonState = digitalRead(GPIO_DEFUSE);
      lcd.setCursor(4,0);
      lcd.print("SABOTAGE");
      lcd.setCursor(7,1);
      if(millis() - timer_pulse > 1000)
        digitalWrite(GPIO_HPLED,HIGH);
      if(millis() - timer_pulse > 1000+T_PULSE){
        digitalWrite(GPIO_HPLED,LOW);
        timer_pulse = millis();
      }
      if(T_DEFUSE - pushed_time/1000 < 10){
        lcd.print("0");
        lcd.setCursor(8,1);
      }
      lcd.print(T_DEFUSE - pushed_time /1000);
      #ifdef DEBUG_USB
        Serial.println("Sabotage");
        Serial.println(T_DEFUSE - pushed_time /1000);
      #endif
      if(buttonState == LOW){
        state = AMORCE;
        lcd.clear();
      }
      else if((buttonState == HIGH) && (pushed_time > T_DEFUSE*1000)){
        state = HS;
        lcd.clear();
      }
      else if (elapsed_time > T_BLOW*1000){
        state = EXPLOSION;
        lcd.clear();
      }
    break;
    case EXPLOSION:
      lcd.setCursor(0,0);
      lcd.print("BOOOOOOOOOOOOOOM");
      digitalWrite(GPIO_HPLED,HIGH);
      #ifdef DEBUG_USB
        Serial.println("BOOM");
      #endif
      delay(5000);
      digitalWrite(GPIO_HPLED,LOW);
      exit(0);
    break;
    case HS:
      lcd.setCursor(7,0);
      lcd.print("C4");
      lcd.setCursor(7,1);
      lcd.print("HS");
      #ifdef DEBUG_USB
        Serial.println("C4 - HS");
      #endif
      exit(0);
    break;
  }
}
