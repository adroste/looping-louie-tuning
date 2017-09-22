/*******************************************************************************

Based on the original "Looping Louie Tuning"-Sketch by Ronny Kersten (2013)

+------------------+-----------------------------------------------------------+
|  Project         | Kamikaze Louie Super Mod                                  |
|  Author          | flasher4401                                               |
|  Last Modified   | Dec 2015                                                  |
|  Game Language   | English, German                                           |
|  Version         | 2.0c                                                      |
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
#include <EEPROM.h>

// initialize the library with the numbers of the interface pins
// RS,Enable,D4,D5,D6,D7
LiquidCrystal lcd(13, 12, 1, 2, 3, 4);

// pin declaration
const byte poti_input = A0;  // Analog input pin for potentiometer
const byte motor_out = 6; // Analog output pin for motor driver
const byte motor_direction1 = 7; // motor output 1
const byte motor_direction2 = 8; // motor output 2
const byte button_onoff = 18;
const byte button_mode = 17;
const byte button_led = 19;
const byte button_player = 16;
const byte led_rgb_r = 11;
const byte led_rgb_g = 10;
const byte led_rgb_b = 9;
const byte led_ws = 5;

//eeprom addresses
const int eeprom_language=0;
const int eeprom_player_enabled=1;
const int eeprom_led_enabled=2;
const int eeprom_led_r_max=3;
const int eeprom_led_g_max=4;
const int eeprom_led_b_max=5;
const int eeprom_led_w_max=6;

//const_values
const byte language_count = 2;
const char gametitle[] = "Kamikaze Louie +";
const char version_info[] = "v2.0c by flasher";
const int player_button_cooldown = 1500;
const int pause_resumeTime = 500;


//game_general
byte gamemode=0; 
byte pause=1, pause_resume=0, pause_resume_c=0;
int output=0;             // value output to the PWM (analog out)
int outputValue=0;        
int outputValueMod=0;
int outputValue_percent=0;
long duration=0, lasttime=0, nowtime=0;
long durationMod=0, lasttimeMod=0, lasttimeLed=0, lasttimepause_resume=0;

//input_handler
byte onoff_pushed=0, mode_pushed=0, led_toggled=0, player_pushed=0;
byte onoff_old=0, mode_old=0, player_old=0;
int sensorValue=0;        // value read from the pot

//led
byte ledmode=1, ledmode_old=255, led_off=0, selected_game_ledmode=0;
byte led_fade_changestep=1, led_rgb_fade=0, led_blink=0, led_test=0;
byte out_led_r=0, out_led_g=0, out_led_b=0, out_led_w=0;
int c_led_r=0, c_led_g=0, c_led_b=0, c_led_w=0; //on=255 off=0 //needs to be int: overflow!!!

//settings
byte settings_page=0, settings_edit_mode=0, setting_temp_value=0;
byte player_enabled=0, led_enabled=0, language=0;
byte led_r_max=255, led_g_max=255, led_b_max=255, led_w_max=255;

//gamemode: random
byte gamemode_random_lvl=1, gamemode_random_fwd1=0, gamemode_random_fwd2=0;
byte gamemode_random_bwd1=0, gamemode_random_bwd2=0;
int gamemode_random_maxplayerchange=0; //needs to be int: negative values

//gamemode: speedup
int gamemode_speedup_factor=1; //needs to be int: negative values


//STRING_TRANSLATIONS
char *str_gm_manual, *str_gm_random, *str_gm_speedup;
char *str_pause, *str_forward, *str_backward;
char *str_set_setupplay;

void init_language() {
	if(language==0) {
		str_gm_manual = "Manual          ";
		str_gm_random = "Random - Level ";
		str_gm_speedup = "Speed-up      x";
		str_pause = "Pause!     ";
		str_forward = "Forward    ";
		str_backward = "Backward   ";
		str_set_setupplay = "\176Setup \176Play    ";
	}
	else if(language==1) {
		str_gm_manual = "Manuell         ";
		str_gm_random = "Zufall - Stufe ";
		str_gm_speedup = "Beschleunigen x";
		str_pause = "Pause!     ";
		str_forward = "Vorw\341rts   ";
		str_backward = "R\365ckw\341rts  ";
		str_set_setupplay = "\176Setup \176Spielen ";
	}
}


void setup() {
	pinMode(motor_direction1, OUTPUT);
	pinMode(motor_direction2, OUTPUT);
	pinMode(led_rgb_r, OUTPUT);
	pinMode(led_rgb_g, OUTPUT);
	pinMode(led_rgb_b, OUTPUT);
	pinMode(led_ws, OUTPUT);
  
	// set up the LCD's number of columns and rows: 
	lcd.begin(16, 2);
	reset();
}

void reset() {
	ledmode = 1;
	ledmode_old = 255;
	led_off = 0;
	selected_game_ledmode = 0;
	
	gamemode = 0;
	pause = 1;
	pause_resume = 0;
	
	onoff_old = 0;
	mode_old = 0;
	player_old = 0;
	
	settings_page = 0;
	eeprom_read_settings();
	init_language();
}



void loop() {
	input_handler();  
  
	if(gamemode==0) {
		settings_menu();
	}
	else {
		game_control();
		game_routine();
		lcd_game_update();
	}     
	
	led_control();
  
	// wait 5 milliseconds before the next loop
	// for the analog-to-digital converter to settle
	// after the last reading:
	delay(5);
}

void input_handler() {
	sensorValue = analogRead(poti_input);
	onoff_pushed = (byte)digitalRead(button_onoff);
	mode_pushed = (byte)digitalRead(button_mode);
	led_toggled = (byte)digitalRead(button_led);
	
	if(player_enabled==1){
		player_pushed = (byte)digitalRead(button_player);
	}
	else{
		player_pushed = 0;
	}
  
	if(onoff_old==1 && onoff_pushed==1) {
		onoff_pushed = 0;
	}
	else {
		onoff_old=onoff_pushed;
	}

	if(mode_old==1 && mode_pushed==1) {
		mode_pushed = 0;
	}
	else {
		mode_old=mode_pushed; 
	}
}

void game_control() {
	if(onoff_pushed==1) {
		if(pause==0 || pause_resume==1)	{
			pause_resume=0;
			ledmode=1;
			pause=1;        
		}
		else {
			pause_resume=1;
			pause_resume_c=0;
			ledmode=3;
		}
	}

	if(pause_resume==2)	{
		pause_resume=0;
		ledmode=selected_game_ledmode;
		pause=0;
		duration=0;
	}

	if(mode_pushed==1) {
		if(pause==1){
			gamemode++;     
			duration=0;
			if (gamemode>3) {
				gamemode=1; 
			}
		}
		else{
			selected_game_ledmode++;
			if(selected_game_ledmode>2){
				selected_game_ledmode=0;
			}
		}
	}
	
	if(pause==0) {
		if(output>200){
			ledmode=4;
		}
		else{
			ledmode=selected_game_ledmode;
		}
	}
	
	nowtime=millis();
	
	//resume counter
	if(pause_resume==1) {		
		if(nowtime>(lasttimepause_resume+pause_resumeTime)) {
			lcd.setCursor(0, 1);
          
			switch(pause_resume_c) {
				case 0:
					led_blink = 1;
					lcd.print("Start in 3 ");
					break;
				case 1:
					led_blink = 0;
					break;
				case 2:
					led_blink = 1;
					lcd.print("Start in 2 ");
					break;
				case 3:
					led_blink = 0;
					break;
				case 4:
					led_blink = 5;
					lcd.print("Start in 1 ");
					break;
				case 5:
					led_blink = 0;
					break;
				case 6:
					led_blink = 2;
					break;
				case 7:
					pause_resume = 2;
					break;
				default:
					break;
			}
			lasttimepause_resume=nowtime;
			pause_resume_c++;
		}
	}
}

void game_routine() {	
	switch(gamemode) {
		case 1: // manual speed selection
			// map it to the range of the analog out:
			outputValue = (int)map(sensorValue, 0, 1023, 255, -255);  
			if(outputValue>=0) {
				digitalWrite(motor_direction1, HIGH);
				digitalWrite(motor_direction2, LOW);
			}
			else {
				digitalWrite(motor_direction2, HIGH);
				digitalWrite(motor_direction1, LOW); 
				outputValue=0-outputValue;
			}
			

			if(player_pushed==1) {
				if((player_old==0 && nowtime>(lasttimeMod+player_button_cooldown)) 
				|| (nowtime>(lasttimeMod+durationMod)))	{
					lasttimeMod=nowtime;
					durationMod=random(5000,8000);
					outputValueMod=random(-60,60);
					if(outputValueMod<30 && outputValueMod>-29) {
						outputValue=30; 
					}
				}
				player_old = 1;
			}
			else if(player_old==1) {
				player_old = 0;
				outputValueMod=0;
			}
			
      
			output = outputValue + outputValueMod;
	
			if(output > 255) {
				output = 255;
			}
			else if(output < 0) {
				output = 0;
			}
			break;

			
		case 2: // random mode
			gamemode_random_lvl = (byte)map(sensorValue, 0, 1023, 5, 1);

			switch(gamemode_random_lvl)	{
				case 1:
					gamemode_random_fwd1 = 45;
					gamemode_random_fwd2 = 70;
					gamemode_random_bwd1 = 42;
					gamemode_random_bwd2 = 70;
					gamemode_random_maxplayerchange = 40;
					break;
				case 2:
					gamemode_random_fwd1 = 50;
					gamemode_random_fwd2 = 100;
					gamemode_random_bwd1 = 47;
					gamemode_random_bwd2 = 100;
					gamemode_random_maxplayerchange = 60;
					break;
				case 3:
					gamemode_random_fwd1 = 50;
					gamemode_random_fwd2 = 140;
					gamemode_random_bwd1 = 47;
					gamemode_random_bwd2 = 110;
					gamemode_random_maxplayerchange = 60;
					break;
				case 4:
					gamemode_random_fwd1 = 50;
					gamemode_random_fwd2 = 190;
					gamemode_random_bwd1 = 50;
					gamemode_random_bwd2 = 120;
					gamemode_random_maxplayerchange = 80;
					break;
				case 5:
					gamemode_random_fwd1 = 50;
					gamemode_random_fwd2 = 230;
					gamemode_random_bwd1 = 50;
					gamemode_random_bwd2 = 130;
					gamemode_random_maxplayerchange = 100;
					break;
				default:
					break;
			}

			if(nowtime>lasttime+duration) {
				lasttime=nowtime;        
				if(random(5)>0)	{
					duration=random(3000,15000);
					digitalWrite(motor_direction1, HIGH);
					digitalWrite(motor_direction2, LOW);
					outputValue=random(gamemode_random_fwd1,gamemode_random_fwd2);
				}
				else {
					duration=random(1500,5000);
					digitalWrite(motor_direction2, HIGH);
					digitalWrite(motor_direction1, LOW);
					outputValue=random(gamemode_random_bwd1,gamemode_random_bwd2);       
				}
			}


			if(player_pushed==1) {
				if((player_old==0 && nowtime>(lasttimeMod+player_button_cooldown)) 
				|| (nowtime>(lasttimeMod+durationMod)))	{
					lasttimeMod=nowtime;
					durationMod=random(5000,8000);
					outputValueMod=random((-1)*gamemode_random_maxplayerchange,gamemode_random_maxplayerchange);
					if(outputValueMod<(gamemode_random_maxplayerchange/2) 
					&& outputValueMod>((-1)*(gamemode_random_maxplayerchange/2))) { 
						outputValue=(gamemode_random_maxplayerchange/2); 
					}
				}
				player_old = 1;
			}
			else if(player_old==1) {
				player_old = 0;
				outputValueMod=0;
			}
		  
     
			output = outputValue + outputValueMod;
		
			if(output > 255) {
				output = 255;
			}
			else if(output < 0)	{
				output = 0;
			}         
			break;

			
		case 3: // speedup mode
			gamemode_speedup_factor = map(sensorValue, 0, 1023, 5, 1);
    
			if(nowtime>lasttime+(1000*((-1)*gamemode_speedup_factor+6)/5))	{
				lasttime=nowtime;        
				digitalWrite(motor_direction1, HIGH);
				digitalWrite(motor_direction2, LOW);
				output++;
        
				if(player_pushed==1) {
					output -= 6;
				}
			}
          
			if(output > 255) {
				output = 255;
			}
			else if(output < 45) {
				output = 45;
			}
			break;    
      
		default:
			break;
	}
	
	//outside: for all gamemodes
	if(pause==1){
		output=0;
	}
	analogWrite(motor_out, output);
}

void lcd_game_update() {
	lcd.setCursor(0, 0);
	switch(gamemode) {
		case 1:
			lcd.print(str_gm_manual);
			break;
			
		case 2:
			lcd.print(str_gm_random);
			lcd.setCursor(15, 0);
			lcd.print(gamemode_random_lvl);
			break;
			
		case 3:
			lcd.print(str_gm_speedup);
			lcd.setCursor(15, 0);
			lcd.print(gamemode_speedup_factor);
			break;
			
		default:
			break;
	}	 
  
	if(pause_resume!=0) { }
	else if(pause==1) {
		lcd.setCursor(0, 1);
		lcd.print(str_pause);
	}
	else if(digitalRead(motor_direction1)==HIGH && digitalRead(motor_direction2)==LOW) {
		lcd.setCursor(0, 1);
		lcd.print(str_forward);
	}
	else {
		lcd.setCursor(0, 1);
		lcd.print(str_backward);
	}

	if(player_pushed==1) {
		lcd.setCursor(11, 1);
		lcd.print("\52");
	}
	else {
		lcd.setCursor(11, 1);
		lcd.print(" ");
	}
  
	lcd.setCursor(12, 1);
	outputValue_percent = (int)map(output,0,255,0,100);
	if(outputValue_percent<100) {
		lcd.print(" ");
	}
	if(outputValue_percent<10) {
		lcd.print(" ");
	}
	lcd.print(outputValue_percent);
	lcd.print("%");
}

void led_control() {
	if(led_enabled==1 && led_toggled==1) {	
		led_off = 0;
	
		switch(ledmode)	{
			case 0:
				if(ledmode_old!=0) {
					c_led_r = 255;
					c_led_g = 0;
					c_led_b = 0;
					c_led_w = 255;
					led_rgb_fade = 0;
				}
				ledmode_old = 0;
		  
				led_fade_changestep = (byte)map(output,0,255,1,5);
	  
				switch(led_rgb_fade) {
					case 0: //rot zu gelb
						c_led_g = c_led_g + led_fade_changestep;
						if(c_led_g > 255) {
							c_led_g = 255;
							led_rgb_fade++;
						}        
						break;
			  
					case 1: //gelb zu gruen
						c_led_r = c_led_r - led_fade_changestep;
						if(c_led_r < 0) {
							c_led_r = 0;
							led_rgb_fade++;
						}
						break;
					  
					case 2: // gruen zu cyan
						c_led_b = c_led_b + led_fade_changestep;
						if(c_led_b > 255) {
							c_led_b = 255;
							led_rgb_fade++;
						}
						break;
					  
					case 3: // cyan zu blau
						c_led_g = c_led_g - led_fade_changestep;
						if(c_led_g < 0) {
							c_led_g = 0;
							led_rgb_fade++;
						}
						break;
					  
					case 4: // blau zu lila
						c_led_r = c_led_r + led_fade_changestep;
						if(c_led_r > 255) {
							c_led_r = 255;
							led_rgb_fade++;
						}
						break;
					  
					case 5: // lila zu rot
						c_led_b = c_led_b - led_fade_changestep;
						if(c_led_b < 0) {
							c_led_b = 0;
							led_rgb_fade = 0;
						}
						break;
					  
					default:
						break;
				}
				break;
		  
		  
			case 1:
				c_led_r = 0;
				c_led_g = 0;
				c_led_b = 255;
				c_led_w = 255;
				ledmode_old=1;
				break;
				
			case 2:
				c_led_r = 255;
				c_led_g = 0;
				c_led_b = 0;
				c_led_w = 255;
				ledmode_old=2;
				break;

				
			case 3:
				if(ledmode_old!=3) {
					c_led_r = 0;
					c_led_g = 0;
					c_led_b = 0;
					c_led_w = 0;
				}
				ledmode_old=3;
			
				switch(led_blink) {
					case 0:
						c_led_r = 0;
						c_led_g = 0;
						c_led_b = 0;
						c_led_w = 0;
						break;
					case 1:
						c_led_r = 255;
						break;
					case 2:
						c_led_g = 255;
						break;
					case 3:
						c_led_b = 255;
						break;
					case 4:
						c_led_w = 255;
						break;
					case 5:
						c_led_r = 255;
						c_led_g = 255;
					default:
						break;
				}
				break;
				
				
			case 4:
				if(ledmode_old!=4) {
					c_led_r = 0;
					c_led_g = 0;
					c_led_b = 0;
					c_led_w = 255;
				}
				ledmode_old=4;
			
				c_led_r += 5;
				if(c_led_r>255){
					c_led_r=0;
				}				
				break;
			
			case 250: //test mode inside settings menu
				ledmode_old=250;
				
				switch(led_test) {
					case 0:
						c_led_r = 255;
						c_led_g = 0;
						c_led_b = 0;
						c_led_w = 0;
						break;
					
					case 1:
						c_led_r = 0;
						c_led_g = 255;
						c_led_b = 0;
						c_led_w = 0;
						break;
					
					case 2:
						c_led_r = 0;
						c_led_g = 0;
						c_led_b = 255;
						c_led_w = 0;
						break;
						
					case 3:
						c_led_r = 0;
						c_led_g = 0;
						c_led_b = 0;
						c_led_w = 255;
						break;
					
					default:
						break;
				}
			
			default:
				break;
		}
		
		if(ledmode==250) {
			out_led_r = (byte)map(c_led_r, 0, 255, 255, (byte)map(setting_temp_value, 0, 255, 255, 0));
			out_led_g = (byte)map(c_led_g, 0, 255, 255, (byte)map(setting_temp_value, 0, 255, 255, 0));
			out_led_b = (byte)map(c_led_b, 0, 255, 255, (byte)map(setting_temp_value, 0, 255, 255, 0));
			out_led_w = (byte)map(c_led_w, 0, 255, 0, setting_temp_value);
		}
		else {
			out_led_r = (byte)map(c_led_r, 0, 255, 255, (byte)map(led_r_max, 0, 255, 255, 0));
			out_led_g = (byte)map(c_led_g, 0, 255, 255, (byte)map(led_g_max, 0, 255, 255, 0));
			out_led_b = (byte)map(c_led_b, 0, 255, 255, (byte)map(led_b_max, 0, 255, 255, 0));
			out_led_w = (byte)map(c_led_w, 0, 255, 0, led_w_max);
		}

		//ws:0=off 255=on, rgb:0=on 255=off
		analogWrite(led_rgb_r, out_led_r);
		analogWrite(led_rgb_g, out_led_g);
		analogWrite(led_rgb_b, out_led_b);
		analogWrite(led_ws, out_led_w); 
	}
	else if(led_off==0)	{
		led_off = 1;
		ledmode_old = 255;
		digitalWrite(led_rgb_r, HIGH);
		digitalWrite(led_rgb_g, HIGH);
		digitalWrite(led_rgb_b, HIGH);
		digitalWrite(led_ws, LOW);
	}
}

void settings_menu() {
	ledmode = 1;
	
	if(settings_page==0) { //start (power on)
		lcd.setCursor(0, 0);
		lcd.print(gametitle);
		lcd.setCursor(0, 1);
		lcd.print(str_set_setupplay);
		
		if(mode_pushed) {
			gamemode++;
		}
		else if(onoff_pushed) {
			settings_page++;
			settings_edit_mode=0;
		}
	}
	else {		
		if(settings_edit_mode==0) {
			settings_page = (byte)map(sensorValue, 0, 1023, 9, 1);
			lcd.setCursor(13, 1);
			lcd.print(settings_page);
			lcd.setCursor(14, 1);
			lcd.print("/9");
			lcd.setCursor(0, 1);
			if(settings_page==1) {
				lcd.print("\176X           ");
				if(onoff_pushed) {
					settings_page=0;
				}
			}
			else {
				lcd.print("\176X     \176Sel  ");				
				if(mode_pushed) {
					settings_edit_mode=1;
				}
				else if(onoff_pushed) {
					settings_page=0;
				}
			}
					
			switch(settings_page) {
				case 1: 
					lcd.setCursor(0, 0);
					lcd.print(version_info);
					break;
				
				case 2:
					lcd.setCursor(0, 0);
					lcd.print("Reset EEPROM    ");
					break;
				
				case 3:
					lcd.setCursor(0, 0);
					lcd.print("Language: ");
					lcd.setCursor(10, 0);
					if(language==0) {
						lcd.print("   EN ");
					}
					else if(language==1) {
						lcd.print("   DE ");
					}
					break;
					
				case 4:
					lcd.setCursor(0, 0);
					lcd.print("Player Button: ");
					lcd.setCursor(15, 0);
					if(player_enabled==1) {
						lcd.print("+");
					}
					else {
						lcd.print("-");
					}
					break;
					
				case 5:
					lcd.setCursor(0, 0);
					lcd.print("LEDs Enabled:  ");
					lcd.setCursor(15, 0);
					if(led_enabled==1) {
						lcd.print("+");
					}
					else {
						lcd.print("-");
					}
					break;
				
				case 6:
					lcd.setCursor(0, 0);
					lcd.print("LED RED max: ");
					lcd.setCursor(13, 0);
					if(led_r_max<100) {
						lcd.print(" ");
					}
					if(led_r_max<10) {
						lcd.print(" ");
					}
					lcd.print(led_r_max);
					break;
					
				case 7:
					lcd.setCursor(0, 0);
					lcd.print("LED GRE max: ");
					lcd.setCursor(13, 0);
					if(led_g_max<100) {
						lcd.print(" ");
					}
					if(led_g_max<10) {
						lcd.print(" ");
					}
					lcd.print(led_g_max);
					break;
					
				case 8:
					lcd.setCursor(0, 0);
					lcd.print("LED BLU max: ");
					lcd.setCursor(13, 0);
					if(led_b_max<100) {
						lcd.print(" ");
					}
					if(led_b_max<10) {
						lcd.print(" ");
					}
					lcd.print(led_b_max);
					break;
					
				case 9:
					lcd.setCursor(0, 0);
					lcd.print("LED WHI max: ");
					lcd.setCursor(13, 0);
					if(led_w_max<100) {
						lcd.print(" ");
					}
					if(led_w_max<10) {
						lcd.print(" ");
					}
					lcd.print(led_w_max);
					break;
					
				default:
					break;
			}

		}
		else {
			switch(settings_page) {
				case 2:
					lcd.setCursor(0, 0);
					lcd.print("Clear EEPROM ???");
					break;
				
				case 3:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 1, 0);
					lcd.setCursor(0, 0);
					lcd.print("Language: ");
					lcd.setCursor(10, 0);
					if(setting_temp_value==0) {
						lcd.print("   EN ");
					}
					else if(setting_temp_value==1) {
						lcd.print("   DE ");
					}
					break;
				
				case 4:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 1, 0);
					lcd.setCursor(0, 0);
					lcd.print("Player Button: ");
					lcd.setCursor(15, 0);
					if(setting_temp_value==1) {
						lcd.print("+");
					}
					else {
						lcd.print("-");
					}
					break;
				
				case 5:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 1, 0);
					lcd.setCursor(0, 0);
					lcd.print("LEDs Enabled:  ");
					lcd.setCursor(15, 0);
					if(setting_temp_value==1) {
						lcd.print("+");
					}
					else {
						lcd.print("-");
					}
					break;
				
				case 6:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 255, 0);
					lcd.setCursor(0, 0);
					lcd.print("LED RED max: ");
					lcd.setCursor(13, 0);
					if(setting_temp_value<100) {
						lcd.print(" ");
					}
					if(setting_temp_value<10) {
						lcd.print(" ");
					}
					lcd.print(setting_temp_value);
					ledmode = 250;
					led_test = 0;
					break;
					
				case 7:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 255, 0);
					lcd.setCursor(0, 0);
					lcd.print("LED GRE max: ");
					lcd.setCursor(13, 0);
					if(setting_temp_value<100) {
						lcd.print(" ");
					}
					if(setting_temp_value<10) {
						lcd.print(" ");
					}
					lcd.print(setting_temp_value);
					ledmode = 250;
					led_test = 1;
					break;
					
				case 8:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 255, 0);
					lcd.setCursor(0, 0);
					lcd.print("LED BLU max: ");
					lcd.setCursor(13, 0);
					if(setting_temp_value<100) {
						lcd.print(" ");
					}
					if(setting_temp_value<10) {
						lcd.print(" ");
					}
					lcd.print(setting_temp_value);
					ledmode = 250;
					led_test = 2;
					break;
					
				case 9:
					setting_temp_value = (byte)map(sensorValue, 0, 1023, 255, 0);
					lcd.setCursor(0, 0);
					lcd.print("LED WHI max: ");
					lcd.setCursor(13, 0);
					if(setting_temp_value<100) {
						lcd.print(" ");
					}
					if(setting_temp_value<10) {
						lcd.print(" ");
					}
					lcd.print(setting_temp_value);
					ledmode = 250;
					led_test = 3;
					break;

					
				default:
					break;
			}
			
			lcd.setCursor(0, 1);
			lcd.print("\176Save  \176X    ");
			if(mode_pushed) {
				settings_edit_mode=0;
			}
			else if(onoff_pushed) {
				eeprom_write_settings();
				settings_edit_mode=0;
			}
		}
	}
}

void eeprom_write_settings() {
	switch(settings_page) {
		case 2:
			for (int i = 0 ; i < EEPROM.length() ; i++) {
				EEPROM.update(i, 0);
			}
			break;
			reset();
			
		case 3:
			EEPROM.update(eeprom_language, setting_temp_value);
			language=setting_temp_value;
			init_language();
			break;		
			
		case 4:
			EEPROM.update(eeprom_player_enabled, setting_temp_value);
			player_enabled=setting_temp_value;
			break;
			
		case 5:
			EEPROM.update(eeprom_led_enabled, setting_temp_value);
			led_enabled=setting_temp_value;
			led_off=0;
			break;
			
		case 6:
			EEPROM.update(eeprom_led_r_max, (byte)map(setting_temp_value, 0, 255, 255, 0));
			led_r_max=setting_temp_value;
			break;
			
		case 7:
			EEPROM.update(eeprom_led_g_max, (byte)map(setting_temp_value, 0, 255, 255, 0));
			led_g_max=setting_temp_value;
			break;
			
		case 8:
			EEPROM.update(eeprom_led_b_max, (byte)map(setting_temp_value, 0, 255, 255, 0));
			led_b_max=setting_temp_value;
			break;
			
		case 9:
			EEPROM.update(eeprom_led_w_max, (byte)map(setting_temp_value, 0, 255, 255, 0));
			led_w_max=setting_temp_value;
			break;
			
		default:
			break;
	}
}

void eeprom_read_settings() {
	language = EEPROM.read(eeprom_language);
	player_enabled = EEPROM.read(eeprom_player_enabled);
	led_enabled = EEPROM.read(eeprom_led_enabled);
	led_r_max = (byte)map(EEPROM.read(eeprom_led_r_max), 0, 255, 255, 0);
	led_g_max = (byte)map(EEPROM.read(eeprom_led_g_max), 0, 255, 255, 0);
	led_b_max = (byte)map(EEPROM.read(eeprom_led_b_max), 0, 255, 255, 0);
	led_w_max = (byte)map(EEPROM.read(eeprom_led_w_max), 0, 255, 255, 0);
	
	if(language >= language_count){
		language = 0;
	}
	if(player_enabled > 1){
		player_enabled = 1;
	}
	if(led_enabled > 1){
		led_enabled = 1;
	}
}
