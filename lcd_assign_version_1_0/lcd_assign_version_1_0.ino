#include <avr/eeprom.h>
#include <IRremote.h>
#include <LiquidCrystal.h>

#define BAUD 9600

#define ZONE1PIN 9
#define ZONE2PIN 8
#define ZONE3PIN 2
#define ANALOG_ZONE_PIN 5

#define LED 7

#define POW 0xFFA25D
#define MODE 0xFF629D
#define MUTE 0xFFE21D
#define PREV 0xFF22DD
#define NEXT 0xFF02FD
#define PLAY_PAUSE 0xFFC23D
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

#define ALARM1 0
#define ALARM2 1
#define ALARM3 2
#define ALARM4 3

volatile byte seconds =0; 
volatile byte minutes =0;
volatile byte hours   =0;

LiquidCrystal lcd(12, 10, 6, 5, 4, 3);

//used to store the state of each alarm
//0 for off on otherwise
volatile int alarmStates[4] = {0,0,0,0};

//set up infrared remote pin and variable for which button was pressed
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

//settings structure
struct StoreStruct {
    int flag;
    int threshold;
    int alarmTime;
    int password[4];
    int activeDigital;
} storage = { 
    666,
    5,
    10,
    {0,0,0,0},
    HIGH
};

struct AlarmStruct{
  int flag;
  int zone;
  int hours;
  int minutes;
  int seconds;
  
} lastAlarm = {
  
  666,
  -1,
  -1,
  -1,
  -1,
  
};

int currentMenuItem = 0;

void setup(){
  
  StoreStruct tempSettings;
  eeprom_read_block((void*)&tempSettings, (void*)0, sizeof(tempSettings));
  if(tempSettings.flag != 555){
    eeprom_write_block((const void*)&storage, (void*)0, sizeof(storage));
  }
  else{
    storage = tempSettings;
  }
  
  AlarmStruct tempAlarmStruct;
  eeprom_read_block((void*)&tempAlarmStruct, (void*)sizeof(storage), sizeof(tempAlarmStruct));
  if(tempAlarmStruct.flag != 555){
    eeprom_write_block((const void*)&lastAlarm, (void*)sizeof(storage), sizeof(lastAlarm));
  }
  else{
    lastAlarm = tempAlarmStruct;
  }
  
  Serial.begin(BAUD);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print("Loading...");
  //enable the infrared receiver and different pins
  irrecv.enableIRIn();
  
  pinMode(ZONE1PIN, INPUT);
  pinMode(ZONE2PIN, INPUT);
  pinMode(LED, OUTPUT);

  
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
  
  //custom interrupt stufff
  pinMode(2, OUTPUT);
  
  attachInterrupt(0, alarm3Interrupt, CHANGE);


  lcd.clear();
}

void alarmOn(){ 
  digitalWrite(LED,HIGH);
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
  for(int i = 0;i<passSize;i++){
    storage.password[i] = pass[i];
  }
  eeprom_write_block((const void*)&storage, (void*)0, sizeof(storage));
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
      eeprom_write_block((const void*)&storage, (void*)0, sizeof(storage));
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
      eeprom_write_block((const void*)&storage, (void*)0, sizeof(storage));
    }
  }
}

void setDigitalPin(){
  if(checkPassword()){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1 for HIGH, 2 for LOW");
    lcd.setCursor(0, 1);

    int tempActiveDigital;
    int cancelled = 0;
    int isSet = 0;
    
    while ( !isSet  && !cancelled ) {
    
      if (irrecv.decode(&results)) {
          unsigned long tempResult = results.value;
          irrecv.resume();
          int newDigit;
          
          if( (newDigit = checkDigit(tempResult))>= 0){
              lcd.setCursor(0, 1);
              lcd.print(newDigit);
              if(newDigit == 1 || newDigit == 2){
                tempActiveDigital = newDigit;
                isSet = 1;
              }
          }
          else if(tempResult == RETURN){
            cancelled = 1;
          }
      }
    }
    lcd.clear();
    if(!cancelled){
      if(tempActiveDigital == 1){
        storage.activeDigital = HIGH;
      }else{
        storage.activeDigital = LOW;
      }
      Serial.println(storage.activeDigital);
      eeprom_write_block((const void*)&storage, (void*)0, sizeof(storage));
    }
  }
}



int alarmTripped(){
 return alarmStates[ALARM1] || alarmStates[ALARM2] || alarmStates[ALARM3] || alarmStates[ALARM4];
}

void saveAlarm(int alarmZone){
  lastAlarm.zone = alarmZone;
  lastAlarm.hours = hours;
  lastAlarm.minutes = minutes;
  lastAlarm.seconds = seconds;
  eeprom_write_block((const void*)&lastAlarm, (void*)sizeof(storage), sizeof(lastAlarm));
  
}

void checkAlarmZone1(){
  
  int buttonState = digitalRead(ZONE1PIN);
  
  if(buttonState == storage.activeDigital){
    alarmStates[ALARM1] = 1;
    saveAlarm(ALARM1);
  }
  
}

void checkAlarmZone2(){
  int sensorValue = analogRead(ANALOG_ZONE_PIN);
  if(sensorValue < storage.threshold){
      alarmStates[ALARM2] = 1;
      saveAlarm(ALARM2);
  }
}

void checkAlarmZone3(){
  int buttonState = digitalRead(ZONE2PIN);

  if(buttonState == HIGH){
      int tempPass[4];
      unsigned long startTime = millis();
      unsigned long timeDiff = 0;
      int timeOverrun = 0;
      
      while(!timeOverrun && !checkPassEqual(tempPass,storage.password,4)){
    
          int digitCount = 0;
          int passSize = 4;
          
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("RESET:Enter Pass");
          lcd.setCursor(0, 1);
          
          while ((digitCount < passSize) && !timeOverrun) {
            timeDiff = (millis() - startTime)/1000;
            timeOverrun = timeDiff > storage.alarmTime;
            if (irrecv.decode(&results)) {
                  unsigned long tempResult = results.value;
                  irrecv.resume();
                  int newDigit;
                  if( (newDigit = checkDigit(tempResult))>= 0){
                      tempPass[digitCount] = newDigit;
                      digitCount++;
                      lcd.print("*");
                  }
            }
          }
      }
      lcd.clear();
      if(timeOverrun){
        alarmStates[ALARM3] = 1;
        saveAlarm(ALARM3);
      }
      
      
  }
}


void checkAlarmZones(){
  checkAlarmZone1();
  checkAlarmZone2();
  checkAlarmZone3();
}

void resetAlarms(){
  for(int i=0;i<4;i++){
    alarmStates[i] = 0;
  } 
  digitalWrite(LED,LOW);
}

void printTime(){
  lcd.setCursor(0, 0);
  lcd.print(hours,DEC);
  lcd.print(":");
  lcd.print(minutes, DEC);
  lcd.print(":");
  lcd.print(seconds, DEC);      
  lcd.print("  ");
}

void printCurrentMenuItem(int item){
    lcd.setCursor(0,1);
    switch(item){
      case 0:
        lcd.print("                ");
        break;
      case 1:
        lcd.print("Set Time        ");
        break;
      case 2:
        lcd.print("Set Password    ");
        break;
      case 3:
        lcd.print("Set Alarmtime   ");
        break;
      case 4:
        lcd.print("Set Threshold   ");
        break;
      case 5:
        lcd.print("Set Pin LOW/HIGH");
        break;
      case 6:
        if(lastAlarm.zone < 0){
          lcd.print(" No Last Alarm  ");
        }
        else{
          lcd.print("Zone:");
          lcd.print(lastAlarm.zone);
          lcd.print(" @ ");
          lcd.print(lastAlarm.hours);
          lcd.print(":");
          lcd.print(lastAlarm.minutes);
          lcd.print(":");
          lcd.print(lastAlarm.seconds);
          lcd.print("  ");
        }
    }
}

void setTime(){
  int timeSet = 0;
  while(!timeSet){
    if (irrecv.decode(&results)) {
        unsigned long tempResult = results.value;
        irrecv.resume();
        if(tempResult == PLAY_PAUSE || tempResult == RETURN){
          timeSet = 1;
        }
        else if(tempResult == MINUS){
          hours++;
          if(hours > 23){
            hours = 0;
          }
        }
        else if(tempResult == PLUS){
          minutes++;
          if(minutes > 59){
            minutes = 0;
          }
        }
    }
    printTime();
    lcd.setCursor(0,1);
    lcd.print("- hrs / + mins");
  }
}


void enterMenu(int menuItem){
  switch(menuItem){
      case 0:
        break;
      case 1:
        setTime();
        break;
      case 2:
        setPassword();
        break;
      case 3:
        setAlarmTime();
        break;
      case 4:
        setThreshold();
        break;
      case 5:
        setDigitalPin();
        break;
    }
  
}


void loop(){
  
     checkAlarmZones();
     
     if(!alarmTripped()){
         //alarm is off
         printTime();
         if (irrecv.decode(&results)) {
            unsigned long tempResult = results.value;
            irrecv.resume();
            if (tempResult == NEXT){
               currentMenuItem++;
               currentMenuItem = currentMenuItem % 7;
            }
            else if(tempResult == PREV){
               if(currentMenuItem == 0){
                 currentMenuItem = 6;
               }
               else{
                 currentMenuItem--;
               }
            }
            else if(tempResult == PLAY_PAUSE){
               enterMenu(currentMenuItem);
            }
         }
         printCurrentMenuItem(currentMenuItem);
     }
     else{
      //alarm is on
      alarmOn();
      if(checkPassword()){
         resetAlarms();
      }
      
     }
}


//custom interrupt function
void alarm3Interrupt() {
  int buttonState = digitalRead(ZONE3PIN);
  if(buttonState==LOW){
    alarmStates[ALARM4] = 1;
    saveAlarm(ALARM4);
    alarmOn();
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
