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
const int player_button_cooldown = 1500;
const int ledmode_pause_blinktime = 500;


// variable declaration
int ledmode = 1, ledmode_old = -1;
int led_fade_changestep = 1, led_rgb_fade = 0, led_pause_blink=0;
int c_led_r=255, c_led_g=255, c_led_b=255, c_led_ws=0; //rgb: 0=on, 255=off - ws: 0=off, 255=on
int gamemode=0, gamemode_old=0, pause=1, pause_trigger=0, player_enabled=1;
int onoff_pushed=0, mode_pushed=0, player_pushed=0;
int onoff_old=0, mode_old=0, led_old=1, player_old=0;
int sensorValue = 0;        // value read from the pot
int output = 0;             // value output to the PWM (analog out)
int outputValue = 0;        
int outputValueMod = 0;
int outputValue_percent = 0;
long duration=0, lasttime=0, nowtime=0;
long durationMod=0, lasttimeMod=0, lasttimeLed=0;

//gamemode_random
int gamemode_random_lvl=1, gamemode_random_fwd1 = 0, gamemode_random_fwd2 = 0;
int gamemode_random_bwd1 = 0, gamemode_random_bwd2 = 0, gamemode_random_maxplayerchange = 0;

//gamemode_speedup
int gamemode_speedup_factor=1;




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
  
  if(onoff_old==0 && onoff_pushed==1 && gamemode!=0)
  {
     if(pause==0)
     {
        ledmode=1;
        pause=1;        
     }
     else
     {
        ledmode=2; //sets pause_trigger=1
     }
  }
  onoff_old=onoff_pushed;

  if(pause_trigger==1)
  {
    pause_trigger=0;
    ledmode=0;
    pause=0;
  }

  if(mode_old==0 && mode_pushed==1)
  {
     if(player_enabled==1)
     { 
        player_enabled=0;
        gamemode++;
     }
     else
     {
        player_enabled=1;
     }
     
     duration=0;
     if (gamemode>3) { gamemode=1; }
  }
  mode_old=mode_pushed;  

  
  switch(gamemode){
    case 0: //start (power on)
      if(gamemode_old == 0)
      {
         lcd.setCursor(0, 0);
         lcd.print("Kamikaze Louie  ");
         lcd.setCursor(0, 1);
         lcd.print("v2.0   \176Spielen ");
      }
      break;

    case 1: // manual speed selection
      nowtime=millis();
      sensorValue = analogRead(poti_input);
      // map it to the range of the analog out:
      outputValue = map(sensorValue, 0, 1023, 255, -255);  
      if(outputValue>=0)
      {
       digitalWrite(motor_direction1, HIGH);
       digitalWrite(motor_direction2, LOW);
      }
      else
      {
       digitalWrite(motor_direction2, HIGH);
       digitalWrite(motor_direction1, LOW); 
       outputValue=0-outputValue;
      }

      if(player_enabled==1)
      {
        if(player_pushed==1)
        {
          if((player_old==0 && nowtime>(lasttimeMod+player_button_cooldown)) 
              || (nowtime>(lasttimeMod+durationMod)))
          {
            lasttimeMod=nowtime;
            durationMod=random(5000,8000);
            outputValueMod=random(-60,60);
            if(outputValueMod<30 && outputValueMod>-29) { outputValue=30; }
          }
          player_old = 1;
        }
        else if(player_old==1)
        {
          player_old = 0;
          outputValueMod=0;
        }
      }
      
      if(pause==0)
      {         
        output = outputValue + outputValueMod;
        
        if(output > 255)
        {
          output = 255;
        }
        else if(output < 0)
        {
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else
      {
         analogWrite(motor_out, 0);
      }
      
      lcd.setCursor(0, 0);
      if(player_enabled==0) { lcd.print("Manuell         "); }
      else { lcd.print("Manuell\52        "); }
      break;

    case 2: // random mode
      sensorValue = analogRead(poti_input);
      gamemode_random_lvl = map(sensorValue, 0, 1023, 5, 1);

      switch(gamemode_random_lvl)
      {
        case 1:
          gamemode_random_fwd1 = 37;
          gamemode_random_fwd2 = 70;
          gamemode_random_bwd1 = 42;
          gamemode_random_bwd2 = 70;
          gamemode_random_maxplayerchange = 40;
          break;
        case 2:
          gamemode_random_fwd1 = 40;
          gamemode_random_fwd2 = 100;
          gamemode_random_bwd1 = 47;
          gamemode_random_bwd2 = 100;
          gamemode_random_maxplayerchange = 60;
          break;
        case 3:
          gamemode_random_fwd1 = 45;
          gamemode_random_fwd2 = 140;
          gamemode_random_bwd1 = 47;
          gamemode_random_bwd2 = 110;
          gamemode_random_maxplayerchange = 60;
          break;
        case 4:
          gamemode_random_fwd1 = 45;
          gamemode_random_fwd2 = 190;
          gamemode_random_bwd1 = 50;
          gamemode_random_bwd2 = 120;
          gamemode_random_maxplayerchange = 80;
          break;
        case 5:
          gamemode_random_fwd1 = 45;
          gamemode_random_fwd2 = 230;
          gamemode_random_bwd1 = 50;
          gamemode_random_bwd2 = 130;
          gamemode_random_maxplayerchange = 100;
          break;
        default:
          break;
      }

    
      nowtime=millis();
      if(nowtime>lasttime+duration)
      {
        lasttime=nowtime;        
        if(random(5)>0)
        {
          duration=random(3000,15000);
          digitalWrite(motor_direction1, HIGH);
          digitalWrite(motor_direction2, LOW);
          outputValue=random(gamemode_random_fwd1,gamemode_random_fwd2);
        }
        else
        {
          duration=random(1500,5000);
          digitalWrite(motor_direction2, HIGH);
          digitalWrite(motor_direction1, LOW);
          outputValue=random(gamemode_random_bwd1,gamemode_random_bwd2);       
        }
      }

      if(player_enabled==1)
      {
        if(player_pushed==1)
        {
          if((player_old==0 && nowtime>(lasttimeMod+player_button_cooldown)) 
              || (nowtime>(lasttimeMod+durationMod)))
          {
            lasttimeMod=nowtime;
            durationMod=random(5000,8000);
            outputValueMod=random((-1)*gamemode_random_maxplayerchange,gamemode_random_maxplayerchange);
            if(outputValueMod<(gamemode_random_maxplayerchange/2) 
                && outputValueMod>((-1)*(gamemode_random_maxplayerchange/2))) 
            { 
              outputValue=(gamemode_random_maxplayerchange/2); 
            }
          }
          player_old = 1;
        }
        else if(player_old==1)
        {
          player_old = 0;
          outputValueMod=0;
        }
      }
      
      if(pause==0)
      {       
        output = outputValue + outputValueMod;
        
        if(output > 255)
        {
          output = 255;
        }
        else if(output < 0)
        {
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else
      {
         analogWrite(motor_out, 0);
      }
      
      lcd.setCursor(0, 0);
      if(player_enabled==0) { lcd.print("Zufall - Stufe "); }
      else { lcd.print("Zufall\52- Stufe "); }
      lcd.setCursor(15, 0);
      lcd.print(gamemode_random_lvl);          
      break;

    case 3: // speedup mode
      sensorValue = analogRead(poti_input);
      gamemode_speedup_factor = map(sensorValue, 0, 1023, 4, 1);
    
      nowtime=millis();
      if(nowtime>lasttime+(2000/gamemode_speedup_factor))
      {
        lasttime=nowtime;        
        digitalWrite(motor_direction1, HIGH);
        digitalWrite(motor_direction2, LOW);
        outputValue++;
      }

      if(player_enabled==1)
      {
        if(player_pushed==1 && nowtime>lasttime+(1000/gamemode_speedup_factor))
        {
          outputValue-=6;
        }
      }
      
      if(pause==0)
      {
        output = outputValue + outputValueMod;
        
        if(output > 255)
        {
          output = 255;
        }
        else if(output < 0)
        {
          output = 0;
        }
        analogWrite(motor_out, output);
      }
      else
      {
         output = 0;
         analogWrite(motor_out, 0);
      }

      lcd.setCursor(0, 0);
      if(player_enabled==0) { lcd.print("Beschleunigen x"); }
      else { lcd.print("Beschleunigen\52x"); }
      lcd.setCursor(15, 0);
      lcd.print(gamemode_speedup_factor);
      break;    
      
    default:
      break;
  }
  

  
  if(pause==1)
  {
    lcd.setCursor(0, 1);
    lcd.print("Pause!     ");
  }
  else if(digitalRead(motor_direction1)==HIGH && digitalRead(motor_direction2)==LOW)
  {
    lcd.setCursor(0, 1);
    lcd.print("Vorw\341rts   ");
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("R\365ckw\341rts  ");
  }

  if(player_pushed==1)
  {
    lcd.setCursor(11, 1);
    lcd.print("\52");
  }
  else
  {
    lcd.setCursor(11, 1);
    lcd.print(" ");
  }
  
  lcd.setCursor(12, 1);
  outputValue_percent = map(output,0,255,0,100);
  if(outputValue_percent<100)
  {
    lcd.print(" ");
  }
  if(outputValue_percent<10)
  {
    lcd.print(" ");
  }
  lcd.print(outputValue_percent);
  lcd.print("%");




  //led control
  
  if(digitalRead(button_led) == 1)
  {
    led_old = 1;
    
    switch(ledmode)
    {
      case 0:
          if(ledmode_old!=0)
          {
            c_led_r = 0;
            c_led_g = 255;
            c_led_b = 255;
            c_led_ws = 255;
            led_rgb_fade = 0;
          }
          ledmode_old=0;
          
          led_fade_changestep = map(output,0,255,1,5);
      
          switch(led_rgb_fade)
          {
            case 0: //rot zu gelb
              c_led_g = c_led_g - led_fade_changestep;
              if(c_led_g < 0) 
              {
                c_led_g = 0;
                led_rgb_fade++;
              }        
              break;
              
            case 1: //gelb zu gruen
              c_led_r = c_led_r + led_fade_changestep;
              if(c_led_r > 255) 
              {
                c_led_r = 255;
                led_rgb_fade++;
              }
              break;
              
            case 2: // gruen zu cyan
              c_led_b = c_led_b - led_fade_changestep;
              if(c_led_b < 0) 
              {
                c_led_b = 0;
                led_rgb_fade = 0;
              }
              break;
              
            case 3: // cyan zu blau
              c_led_g = c_led_g + led_fade_changestep;
              if(c_led_g > 255) 
              {
                c_led_g = 255;
                led_rgb_fade++;
              }
              break;
              
            case 4: // blau zu lila
              c_led_r = c_led_r - led_fade_changestep;
              if(c_led_r < 0) 
              {
                c_led_r = 0;
                led_rgb_fade++;
              }
              break;
              
            case 5: // lila zu rot
              c_led_b = c_led_b + led_fade_changestep;
              if(c_led_b > 255) 
              {
                c_led_b = 255;
                led_rgb_fade = 0;
              }
              break;
              
            default:
              break;
          }
          break;
          
      case 1:
        if(ledmode_old!=1)
        {
          c_led_r = 255;
          c_led_g = 255;
          c_led_b = 0;
          c_led_ws = 255;
        }
        ledmode_old=1;
        break;

      case 2:
        if(ledmode_old!=2)
        {
          c_led_r = 255;
          c_led_g = 255;
          c_led_b = 255;
          c_led_ws = 255;
          led_pause_blink = 0;
        }
        ledmode_old=2;
          
        nowtime=millis();
        if(nowtime>(lasttimeLed+ledmode_pause_blinktime))
        {
          switch(led_pause_blink)
          {
            case 0:
              c_led_r = 0;
              break;
            case 1:
              c_led_r = 255;
              break;
            case 2:
              c_led_r = 0;
              break;
            case 3:
              c_led_r = 255;
              break;
            case 4:
              c_led_r = 0;
              c_led_g = 0;
              break;
            case 5:
              c_led_r = 255;
              c_led_g = 255;
              break;
            case 6:
              c_led_g = 0;
              break;
            case 7:
              pause_trigger = 1;
              break;
            default:
              break;
          }
          lasttimeLed=nowtime;
          led_pause_blink++;
        }
        break;
        
    }

    //rgb: 0=on, 255=off - ws: 0=off, 255=on
    analogWrite(led_rgb_r, c_led_r);
    analogWrite(led_rgb_g, c_led_g);
    analogWrite(led_rgb_b, c_led_b);
    analogWrite(led_ws, c_led_ws); 
  }
  else if(led_old == 1)
  {
    led_old = 0;
    ledmode_old=-1;
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
