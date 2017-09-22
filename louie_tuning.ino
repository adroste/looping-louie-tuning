/*******************************************************************************

Based on the original "Looping Louie Tuning"-Sketch by Ronny Kersten (2013)

+------------------+-----------------------------------------------------------+
|  Project         | Looping Louie Tuning                                      |
|  Author          | flasher4401                                               |
|  Last Modified   | October 2015                                              |
|  Game Language   | German                                                    |
|  Version         | 2.0                                                       |
|  Microcontroller | Atmel ATmega328P                                          |
|  File            | louie_tuning.ino                                          |
+------------------+-----------------------------------------------------------+
|  Arduino Sketch for a Looping Louie Tuning Box with LCD (2x16 characters),   |
|  three buttons (2x toggle, 1x switch) and a potentiometer. The Sketch was    |
|  used for a stand-alone ATmega328P with Arduino Bootloader in combination    |
|  with a motor driver (L293D) and four mosfets for LED control. Additionally  |
|  there are four more buttons mounted at each catapult (common input).        |
+------------------------------------------------------------------------------+

*******************************************************************************/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
// RS,Enable,D4,D5,D6,D7
LiquidCrystal lcd(13, 12, 1, 2, 3, 4);

// pin declaration
const int poti_input = A0;  // Analog input pin for potentiometer
const int motor_out = 6; // Analog output pin for motor driver
const int motor_direction1 = 7; // motor output 1
const int motor_direction2 = 8; // motor output 2
const int button_onoff = 18;
const int button_mode = 17;
const int button_led = 19;
const int button_player = 16;
const int led_rgb_r = 11;
const int led_rgb_g = 10;
const int led_rgb_b = 9;
const int led_ws = 5;
const int louie_button_cooldown = 1500;

// variable declaration
int led_changestep = 5, led_rgb_fade = 0;
int c_led_r=0, c_led_g=255, c_led_b=255, c_led_ws=255; //rgb: 0=on, 255=off - ws: 0=off, 255=on
int gamemode=1, pause=1;
int onoff_pushed=0, mode_pushed=0, player_pushed=0;
int onoff_old=0, mode_old=0, led_old=-1, player_old=0;
int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
int outputValueMod = 0;
int outputValue_percent = 0;
long duration=0, lasttime=0, nowtime=0;
long durationMod=0, lasttimeMod=0;



void setup() {
  pinMode(motor_direction1, OUTPUT);
  pinMode(motor_direction2, OUTPUT);
  pinMode(led_rgb_r, OUTPUT);
  pinMode(led_rgb_g, OUTPUT);
  pinMode(led_rgb_b, OUTPUT);
  pinMode(led_ws, OUTPUT);
  
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  
}



void loop() {
  onoff_pushed = digitalRead(button_onoff);
  mode_pushed = digitalRead(button_mode);
  player_pushed = digitalRead(button_player);
  
  if(onoff_old ==0 && onoff_pushed==1)
  {
     pause = !pause;
  }
  onoff_old=onoff_pushed;

  if(mode_old ==0 && mode_pushed==1)
  {
     gamemode+=1;
     duration=0;
     if (gamemode>8) gamemode=1;
  }
  mode_old=mode_pushed;  

// manuell, random, speedup (+brake, +go faster), hold
  
  switch(gamemode){
    case 1: // manual speed selection
      sensorValue = analogRead(poti_input);
      // map it to the range of the analog out:
      outputValue = map(sensorValue, 0, 1023, 255, -255);  
      if(outputValue>=0){
       digitalWrite(motor_direction1, HIGH);
       digitalWrite(motor_direction2, LOW);
      }
      else{
       digitalWrite(motor_direction2, HIGH);
       digitalWrite(motor_direction1, LOW); 
       outputValue=0-outputValue;
      }
      //set output:
      if(pause==0){
         lcd.setCursor(0, 0);
         lcd.print("Manuell         ");
         analogWrite(motor_out, outputValue);           
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("Manuell - Pause!");
      }
      break;

    case 2: // manual speed selection + louie button
      sensorValue = analogRead(poti_input);
      // map it to the range of the analog out:
      outputValue = map(sensorValue, 0, 1023, 255, -255);  
      if(outputValue>=0){
       digitalWrite(motor_direction1, HIGH);
       digitalWrite(motor_direction2, LOW);
      }
      else{
       digitalWrite(motor_direction2, HIGH);
       digitalWrite(motor_direction1, LOW); 
       outputValue=0-outputValue;
      }

      if(player_pushed==1){
          if((player_old==0 && nowtime>(lasttimeMod+louie_button_cooldown)) || (nowtime>(lasttimeMod+durationMod))){
            lasttimeMod=nowtime;
            durationMod=random(5000,8000);
            outputValueMod=random(-60,60);
          }
          player_old = 1;
      }
      else if(player_old==1){
        player_old = 0;
        outputValueMod=0;
      }
      
      //set output:
      if(pause==0){
         lcd.setCursor(0, 0);
         lcd.print("Manuell\52        ");

        int output = outputValue + outputValueMod;
        
        if(output > 255){
          output = 255;
        }
        else if(output < 0){
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("Manuell\52- Pause!");
      }
      break;
      
    case 3: // random mode easy
      nowtime=millis();
      if(nowtime>lasttime+duration){
        lasttime=nowtime;
        if(random(5)>0){
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(37,65);
        }
        else{
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(42,65);       
        }
      }
      
      if(pause==0){
        lcd.setCursor(0, 0);
        lcd.print("Einfach         ");
        analogWrite(motor_out, outputValue);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("Einfach - Pause!");
      }  
      break;

    case 4: // random mode easy + louie button
      nowtime=millis();
      if(nowtime>lasttime+duration){
        lasttime=nowtime;
        if(random(5)>0){
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(37,65);
        }
        else{
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(42,65);       
        }
      }

      if(player_pushed==1){
          if((player_old==0 && nowtime>(lasttimeMod+louie_button_cooldown)) || (nowtime>(lasttimeMod+durationMod))){
            lasttimeMod=nowtime;
            durationMod=random(5000,8000);
            outputValueMod=random(-30,30);
          }
          player_old = 1;
      }
      else if(player_old==1){
        player_old = 0;
        outputValueMod=0;
      }
      
      if(pause==0){
        lcd.setCursor(0, 0);
        lcd.print("Einfach\52        ");

        int output = outputValue + outputValueMod;
        
        if(output > 255){
          output = 255;
        }
        else if(output < 0){
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("Einfach\52- Pause!");
      }  
      break;
      
    case 5: // random mode medium
      nowtime=millis();
      if(nowtime>lasttime+duration){
        lasttime=nowtime;        
        if(random(5)>0){
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(40,100);
        }
        else{
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(47,100);       
        }
      }
      
      if(pause==0){
        lcd.setCursor(0, 0);
        lcd.print("Mittel          ");
        analogWrite(motor_out, outputValue);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("Mittel  - Pause!");
      }
      break;

    case 6: // random mode medium + louie button
      nowtime=millis();
      if(nowtime>lasttime+duration){
        lasttime=nowtime;        
        if(random(5)>0){
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(40,100);
        }
        else{
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(47,100);       
        }
      }

      if(player_pushed==1){
          if((player_old==0 && nowtime>(lasttimeMod+louie_button_cooldown)) || (nowtime>(lasttimeMod+durationMod))){
            lasttimeMod=nowtime;
            durationMod=random(5000,8000);
            outputValueMod=random(-60,60);
          }
          player_old = 1;
      }
      else if(player_old==1){
        player_old = 0;
        outputValueMod=0;
      }
      
      if(pause==0){
        lcd.setCursor(0, 0);
        lcd.print("Mittel\52         ");

        int output = outputValue + outputValueMod;
        
        if(output > 255){
          output = 255;
        }
        else if(output < 0){
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("Mittel\52 - Pause!");
      }
      break;
      
    case 7: // random mode hard
      nowtime=millis();
      if(nowtime>lasttime+duration){
        lasttime=nowtime;        
        if(random(5)>0){
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(40,200);
        }
        else{
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(50,130);       
        }
      }
      
      if(pause==0){
        lcd.setCursor(0, 0);
        lcd.print("SCHWER          ");
        analogWrite(motor_out, outputValue);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("SCHWER  - Pause!");
      }
      break;

    case 8: // random mode hard + louie button
      nowtime=millis();
      if(nowtime>lasttime+duration){
        lasttime=nowtime;        
        if(random(5)>0){
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(40,200);
        }
        else{
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(50,130);       
        }
      }

      if(player_pushed==1){
          if((player_old==0 && nowtime>(lasttimeMod+louie_button_cooldown)) || (nowtime>(lasttimeMod+durationMod))){
            lasttimeMod=nowtime;
            durationMod=random(5000,8000);
            outputValueMod=random(-80,80);
          }
          player_old = 1;
      }
      else if(player_old==1){
        player_old = 0;
        outputValueMod=0;
      }
      
      if(pause==0){
        lcd.setCursor(0, 0);
        lcd.print("SCHWER\52         ");

        int output = outputValue + outputValueMod;
        
        if(output > 255){
          output = 255;
        }
        else if(output < 0){
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else{
         analogWrite(motor_out, 0);
         lcd.setCursor(0, 0);
         lcd.print("SCHWER\52 - Pause!");
      }          
      break;
      
    default:
      gamemode=1;
  }
  

  
  if(digitalRead(motor_direction1)==HIGH && digitalRead(motor_direction2)==LOW){
    lcd.setCursor(0, 1);
    lcd.print("Vorw\341rts   ");
  }
  else{
    lcd.setCursor(0, 1);
    lcd.print("R\365ckw\341rts  ");
  }

  if(player_pushed==1){
    lcd.setCursor(11, 1);
    lcd.print("\52");
  }
  else{
    lcd.setCursor(11, 1);
    lcd.print(" ");
  }
  
  lcd.setCursor(12, 1);
  outputValue_percent = map(outputValue,0,255,0,100);
  if(outputValue_percent<100){
    lcd.print(" ");
  }
  if(outputValue_percent<10){
    lcd.print(" ");
  }
  lcd.print(outputValue_percent);
  lcd.print("%");







  //led control
  
  if(digitalRead(button_led) == 1)
  {
    if (led_old == 0)
    {
      led_old = 1;

      //rgb: 0=on, 255=off - ws: 0=off, 255=on
      analogWrite(led_rgb_r, c_led_r);
      analogWrite(led_rgb_g, c_led_g);
      analogWrite(led_rgb_b, c_led_b);
      analogWrite(led_ws, c_led_ws);      
//      digitalWrite(led_ws, HIGH);
//      digitalWrite(led_rgb_r, LOW);
//      digitalWrite(led_rgb_g, LOW);
//      digitalWrite(led_rgb_b, LOW);

    }    

    switch(led_rgb_fade)
    {
      case 0: //rot zu gelb
        c_led_g = c_led_g - led_changestep;
        if(c_led_g < 0) 
        {
          c_led_g = 0;
          led_rgb_fade++;
        }        
        break;
        
      case 1: //gelb zu gruen
        c_led_r = c_led_r + led_changestep;
        if(c_led_r > 255) 
        {
          c_led_r = 255;
          led_rgb_fade++;
        }
        break;
        
      case 2: // gruen zu cyan
        c_led_b = c_led_b - led_changestep;
        if(c_led_b < 0) 
        {
          c_led_b = 0;
          led_rgb_fade = 0;
        }
        break;
        
      case 3: // cyan zu blau
        c_led_g = c_led_g + led_changestep;
        if(c_led_g > 255) 
        {
          c_led_g = 255;
          led_rgb_fade++;
        }
        break;
        
      case 4: // blau zu lila
        c_led_r = c_led_r - led_changestep;
        if(c_led_r < 0) 
        {
          c_led_r = 0;
          led_rgb_fade++;
        }
        break;
        
      case 5: // lila zu rot
        c_led_b = c_led_b + led_changestep;
        if(c_led_b > 255) 
        {
          c_led_b = 255;
          led_rgb_fade = 0;
        }
        break;
        
      default:
        c_led_r = 0;
        c_led_g = 255;
        c_led_b = 255;
        led_rgb_fade = 0;
    }

    //write values
    analogWrite(led_rgb_r, c_led_r);
    analogWrite(led_rgb_g, c_led_g);
    analogWrite(led_rgb_b, c_led_b);
    //analogWrite(led_ws, c_led_ws);
  }
  else if(led_old == 1)
  {
    led_old = 0;
    digitalWrite(led_rgb_r, HIGH);
    digitalWrite(led_rgb_g, HIGH);
    digitalWrite(led_rgb_b, HIGH);
    digitalWrite(led_ws, LOW);
  }

  // wait 5 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(5);
}
