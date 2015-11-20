#include <IRremote.h>
#include <LiquidCrystal.h>

#define BAUD 9600
#define B1 8
#define B2 9
#define B3
#define B4
#define BUZZER 10
#define LED 7

#define POW 0xFFA25D
#define MODE 0xFF629D
#define MUTE 0xFFE21D
#define PREV 0xFF22DD
#define NEXT 0xFF02FD
#define PLAY/PAUSE 0xFFC23D
#define MINUS 0xFFE01F
#define PLUS 0xFFA857
#define EQ 0xFF906F
#define HUNDRED_PLUS 0xFF9867
#define RETURN 0xFFB04F
#define ZERO 0xFF6897
#define ONE 0xFF30CF
#define TWO 0xFF18E7
#define THREE 0xFF7A85
#define FOUR 0xFF10EF
#define FIVE 0xFF38C7
#define SIX 0xFF5AA5
#define SEVEN 0xFF42BD
#define EIGHT 0xFF4AB5
#define NINE 0xFF52AD

#define SET 0
#define ENTER 1

volatile byte seconds =0; 
volatile byte minutes =0;
volatile byte hours   =0;

LiquidCrystal lcd(12, 10, 5, 4, 3, 2);

//used to store the state of each alarm
//0 for off on otherwise
int alarmStates[4] = {0,0,0,0};

//set up infrared remote pin and variable for which button was pressed
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

//settings structure
struct StoreStruct {
    int threshold;
    int alarmTime;
    int password[4];
} storage = { 
    5,
    10,
    {0,0,0,0}
};

void setup()
{
  Serial.begin(BAUD);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print("Loading...");
  //enable the infrared receiver and different pins
  irrecv.enableIRIn();
  pinMode(B1, INPUT);
  pinMode(B2, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  //**************************
  digitalWrite(BUZZER, LOW);
  //***************************
  
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

  for(int i=0;i<4;i++){
    Serial.print(storage.password[i]);
  }
  Serial.println();
  Serial.println(storage.alarmTime);
  Serial.println(storage.threshold);

  lcd.clear();
}

int checkDigit(unsigned long resultValue){
    int digit;
    if (resultValue == ZERO) {
      digit = 0;
    }
    else if (resultValue == ONE) {
      digit = 1;
    }
    else if(resultValue == TWO) {
      digit = 2;
    }
    else if (resultValue == THREE) {
      digit = 3;
    }
    else if(resultValue == FOUR) {
      digit = 4;
    }
    else if (resultValue == FIVE) {
      digit = 5;
    }
    else if(resultValue == SIX) {
      digit = 6;
    }
    else if (resultValue == SEVEN) {
      digit = 7;
    }
    else if(resultValue == EIGHT) {
      digit = 8;
    }
    else if(resultValue == NINE) {
      digit = 9;
    }
    else{
      digit = -1;
    }
    return digit;
}

void savePassword(int *pass, int passSize){
  //********************************
  //change this to actually save password
  
  for(int i = 0;i<passSize;i++){
    storage.password[i] = pass[i];
  }
  //********************************
}


int setTempPassword(int passType, int *newPass) {
  int digitCount = 0;
  int passSize = 4;
  int cancelled = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  if(passType == SET){
    lcd.print("Set New Password");
  }
  else if(passType == ENTER){
    lcd.print("Enter Password");
  }
  lcd.setCursor(0, 1);
  
  while ( (digitCount < passSize)  && !cancelled ) {
    
    if (irrecv.decode(&results)) {
          unsigned long tempResult = results.value;
          irrecv.resume();
          int newDigit;
          
          if( (newDigit = checkDigit(tempResult))>= 0){
              newPass[digitCount] = newDigit;
              digitCount++;
              lcd.print("*");
          }
          else if(tempResult == RETURN){
            cancelled = 1;
          }
    }
  
  }
  lcd.clear();
  if(!cancelled){
    return 1;
  }
  return 0;
   
}

int checkPassEqual(int* arr1,int* arr2,int size){
  for(int i =0;i<size;i++){
    if(arr1[i] != arr2[i]){
      return 0;
    }
  }
  return 1;
}

int checkPassword(){
  int enteredPass[4];
  if(setTempPassword(ENTER,enteredPass)){
   if(checkPassEqual(enteredPass,storage.password,4)){
     return 1;
    }
  }
  return 0;
}

void setPassword(){
   if(checkPassword()){
     int newPass[4];
     if(setTempPassword(SET,newPass)){
        savePassword(newPass,4);
     }
   }
}

void setThreshold(){
  if(checkPassword()){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Threshold(secs)");
    lcd.setCursor(0, 1);

    int digitCount = 3;
    int multiplier = 100;
    int count = 0;
    int tempThreshold = 0;
    int cancelled = 0;
    
    while ( (count < digitCount)  && !cancelled ) {
    
      if (irrecv.decode(&results)) {
          unsigned long tempResult = results.value;
          irrecv.resume();
          int newDigit;
          
          if( (newDigit = checkDigit(tempResult))>= 0){
              tempThreshold += newDigit * multiplier;
              count++;
              multiplier /= 10;
              lcd.print(newDigit);
          }
          else if(tempResult == RETURN){
            cancelled = 1;
          }
      }
    }
     lcd.clear();
    if(!cancelled){
      storage.threshold = tempThreshold;
    }
  } 
}

void setAlarmTime(){
  if(checkPassword()){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarmtime(secs)");
    lcd.setCursor(0, 1);

    int digitCount = 3;
    int multiplier = 100;
    int count = 0;
    int tempAlarmTime = 0;
    int cancelled = 0;
    
    while ( (count < digitCount)  && !cancelled ) {
    
      if (irrecv.decode(&results)) {
          unsigned long tempResult = results.value;
          irrecv.resume();
          int newDigit;
          
          if( (newDigit = checkDigit(tempResult))>= 0){
              tempAlarmTime += newDigit * multiplier;
              count++;
              multiplier /= 10;
              lcd.print(newDigit);
          }
          else if(tempResult == RETURN){
            cancelled = 1;
          }
      }
    }
    lcd.clear();
    if(!cancelled){
      storage.alarmTime = tempAlarmTime;
    }
  }
}


void loop() {

     int b1State = digitalRead(B1);
     int b2State = digitalRead(B2);
     
     
       
     lcd.print(" ");
     lcd.setCursor(0, 1);
     lcd.print(hours,DEC);
     lcd.print(":");
     lcd.print(minutes, DEC);
     lcd.print(":");
     lcd.print(seconds, DEC);
     
     if (irrecv.decode(&results)) {
        unsigned long tempResult = results.value;
        irrecv.resume();
        if (tempResult == ONE){
           setPassword();
        }
        else if(tempResult == TWO){
           setThreshold();
        }
        else if(tempResult == THREE){
           setAlarmTime();
        }
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
