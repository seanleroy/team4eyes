#include <IRremote.h>


#define BAUD 9600
#include <LiquidCrystal.h>
#define b1 9
#define b2 8
#define buzzer 10
#define LED 7

#define POW FFA25D
#define MODE FF629D
#define MUTE FFE21D
#define PREV FF22DD
#define NEXT FF02FD
#define PLAY/PAUSE FFC23D
#define MINUS FFE01F
#define PLUS FFA857
#define EQ FF906F
#define HUNDRED_PLUS FF9867
#define RETURN FFB04F
#define ZERO 0xFF6897
#define ONE 0xFF30CF
#define TWO 0xFF18E7
#define THREE 0xFFFFFF
#define FOUR 0xFF10EF
#define FIVE 0xFF38C7
#define SIX 0xFF5AA5
#define SEVEN 0xFF42BD
#define EIGHT 0xFF4AB5
#define NINE 0xFF52AD

volatile byte seconds =0; 
volatile byte minutes =0;
volatile byte hours   =0;

LiquidCrystal lcd(12, 10, 5, 4, 3, 2);

int b1State = 0;
int b2State = 0;
int modeState = 0;
int threshold = 8;

int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  Serial.begin(BAUD);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
  pinMode(b1, INPUT);
  pinMode(b2, INPUT);
  
  irrecv.enableIRIn();
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  
  //Setup Timer 1 to interrupt every second
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 15625;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1<< OCIE1A);
  sei();
}

void setPassword() {
  int digits = 4;
  int count = 0;
  int newDigit;
  int newPass[4];
  
   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Pasword");
  
  lcd.setCursor(0, 1);
  
  while (count < digits) {
    
     if (irrecv.decode(&results)) {  
          irrecv.resume(); // Receive the next value
          if (results.value == ZERO) {
            newDigit = 0;
            count++;
            lcd.print(newDigit);
          }
          if (results.value == ONE) {
            newDigit = 1;
            count++;
            lcd.print(newDigit);
          }
          if(results.value == TWO) {
            newDigit = 2;
            count++;
          }
          if (results.value == THREE) {
            newDigit = 3;
            count++;
            lcd.print(newDigit);
          }
          if(results.value == FOUR) {
            newDigit = 4;
            count++;
          }
          if (results.value == FIVE) {
            newDigit = 5;
            count++;
          }
          if(results.value == SIX) {
            newDigit = 6;
            count++;
          }
          if (results.value == SEVEN) {
            newDigit = 7;
            count++;
          }
          if(results.value == EIGHT) {
            newDigit = 8;
            count++;
          }
          if(results.value == NINE) {
            newDigit =9;
            count++;
          }
          newPass[count] = newDigit;
          
          irrecv.resume(); // Receive the next value
      }
  
   }
   
   
}

void setThreshold() {
  

  lcd.print("Please enter a new threshold");
}

void loop() {

     b1State = digitalRead(b1);
     b2State = digitalRead(b2);
     
     if (b1State == HIGH) {
       hours++; 
       if (hours == 24) {
         hours = 0; 
       }
       delay(350);
     } 
   
     if (b2State == HIGH) {
       minutes++;
       if (minutes == 60) {
         minutes = 0; 
       }
       delay(350);
     }  
     lcd.print(" ");
     lcd.setCursor(0, 1);
     lcd.print(hours,DEC);
     lcd.print(":");
     lcd.print(minutes, DEC);
     lcd.print(":");
     lcd.print(seconds, DEC);
     
     
     
     if (irrecv.decode(&results)) {
        if (results.value == ONE)
           setPassword();
        if(results.value == TWO)
           setThreshold();
        irrecv.resume(); // Receive the next value
     }
}





ISR (TIMER1_COMPA_vect){
  if(seconds == 59){
    seconds=0;
    minutes++;
    if (minutes == 60){
      minutes=0;
      hours++;
    }
    if (hours == 24)
       hours = 0;
  }
  else
     seconds++;
}
