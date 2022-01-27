/*
LCD_I2C - Arduino library to control a 16x2 LCD via an I2C adapter based on PCF8574

Copyright(C) 2020 Blackhack <davidaristi.0504@gmail.com>

This program is free software : you can redistribute it and /or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include <LCD_I2C.h>

// Define the area to exclude from sprite placement
#define  XMIN1       6 //  7
#define  XMAX1      13 // 11
#define  XMIN2       6 //  5
#define  XMAX2      13
#define  BADY1       1
#define  BADY2       2

// define the pin numbers used for PWM output
#define PWM1         3
#define PWM2         5
#define PWM3         6
#define PWM4         9
#define PWM5        10
#define PWM6        11

// the delta to apply to PWM signal
#define PWMDELTA1   -4
#define PWMDELTA2   10
#define PWMDELTA3   18
#define PWMDELTA4   -8
#define PWMDELTA5   -3
#define PWMDELTA6    7

#define BASEDELAY   50
#define SPRITEDELAY 20

#define MAXSHAPES    6

LCD_I2C lcd(0x27, 20, 4); // Default address of most PCF8574 modules, change according

uint8_t shapes[MAXSHAPES][8] =
{
  {
    0b00000,
    0b01010,
    0b10101,
    0b10001,
    0b01010,
    0b00100,
    0b00000,
    0b00000
  },
  {
    0b00100,
    0b01001,
    0b10010,
    0b01001,
    0b00100,
    0b00000,
    0b00000,
    0b00000
  },
  {
    0b00000,
    0b00000,
    0b00000,
    0b01010,
    0b10101,
    0b10001,
    0b01010,
    0b00100,
  },
  {
    0b00000,
    0b00000,
    0b01010,
    0b00000,
    0b11011,
    0b01110,
    0b00000,
    0b00000,
  },
  {
    0b00100,
    0b10010,
    0b01001,
    0b10010,
    0b00100,
    0b00000,
    0b00000,
    0b00000
  },
  {
    0b00000,
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b01110,
    0b00100,
    0b00000
  }
};

struct sprite {
  uint8_t x,       y;
  uint8_t oldx, oldy;
  uint8_t shape;
} _sprite;

sprite *spr[MAXSHAPES];

// A listing of the pins used
uint8_t pwmPin[] = {PWM1, PWM2, PWM3, PWM4, PWM5, PWM6 };

// A table to hold the PWM value of each pin
uint8_t pwmVal[6];

// A table of the delta to appy to each PWM value each iteration of the loop
int8_t pwmDelta[6] = {PWMDELTA1, PWMDELTA2, PWMDELTA3, PWMDELTA4, PWMDELTA5, PWMDELTA6 };

int newx;
int newy;
int doSprite = 0;

void drawShape(sprite *fk) {
  //Serial.print("x= "); Serial.println(fk->x);
  //Serial.print("y= "); Serial.println(fk->y);
  //Serial.print("s= "); Serial.println(char(48 + (fk->shape)));
  lcd.setCursor(fk->x, fk->y);
  lcd.write(fk->shape);
  //lcd.write(0); //do all shape 0
}

void hideShape(sprite *fk) {
   lcd.setCursor(fk->oldx, fk->oldy);
   lcd.print(" ");
}

void computeShape(sprite *fk) {
   // Save previous positions
   fk->oldx = fk->x;
   fk->oldy = fk->y;

   while(1) {
      newx = random(0, 20);
      newy = random(0,4);
      if ((newy ==0) || (newy == 3))
         break;
      if (((newx < XMIN1) || (newx > XMAX1)) && (newy == BADY1)) 
         break; 
      if (((newx < XMIN2) || (newx > XMAX2)) && (newy == BADY2)) 
         break; 
      }
   fk->x = newx;
   fk->y = newy;
}

void twinkle() {
   int i;

   for (i = 0; i < 6; i++) {
     pwmVal[i] += pwmDelta[i];
     if (pwmVal[i] <   0 ) { 
        pwmVal[i] = -pwmVal[i];
        pwmDelta[i] = -pwmDelta[i];
     }
     if (pwmVal[i] > 255 ) {
        pwmVal[i] = 255;
        pwmDelta[i] = -pwmDelta[i];
     }
     analogWrite(pwmPin[i], pwmVal[i]);
   }
}

void setup()
{
    int i;

    //Serial.begin(115200);
    lcd.begin();
    lcd.backlight();

    // Load custome characters
    for (i = 0; i < MAXSHAPES; i++)
       lcd.createChar(i, shapes[i]);
       
    // Allocate storage for the sprites
    for (i = 0; i < MAXSHAPES; i++)
       spr[i] = (sprite *)calloc(1, sizeof(sprite));
       
    // Initialize the sprites and their locations (outside corners)
    spr[0]->x =  0; spr[0]->oldx =  0; spr[0]->y = 0; spr[0]->oldy = 0; spr[0]->shape = 0; 
    spr[1]->x = 19; spr[1]->oldx = 19; spr[1]->y = 0; spr[1]->oldy = 0; spr[1]->shape = 1;
    spr[2]->x =  0; spr[2]->oldx =  0; spr[2]->y = 3; spr[2]->oldy = 3; spr[2]->shape = 2;
    spr[3]->x = 19; spr[3]->oldx = 19; spr[3]->y = 3; spr[3]->oldy = 3; spr[3]->shape = 3;
    spr[4]->x =  1; spr[4]->oldx =  1; spr[4]->y = 1; spr[4]->oldy = 1; spr[4]->shape = 4;
    spr[4]->x = 18; spr[5]->oldx = 18; spr[5]->y = 2; spr[5]->oldy = 2; spr[5]->shape = 5;

    lcd.clear();
    lcd.setCursor(6, 1);
    lcd.print("LOVE YOU");
    lcd.setCursor(6, 2);
    lcd.print("FOREVER!");

    // Establish initial values for the PWM outputs, setup the pins as PWM outputs
    for (i = 0; i < 6; i++) {
       pwmVal[i] = random(0, 128);
       pinMode(pwmPin[i], OUTPUT);
       analogWrite(pwmPin[i], pwmVal[i]);
    }
}

void loop()
{
  int i = 0;
  for (i=0; i< MAXSHAPES - 1; i++)
    drawShape(spr[i]);

//  while (1);

  while(1) {
    // Wait a moment
    delay(BASEDELAY);
      
    // Update the twinkling lights
    twinkle();

    if (doSprite++ > SPRITEDELAY) {
      doSprite = 0;     
      for (i=0; i<MAXSHAPES; i++) {   
        // Erase old sprite
        hideShape(spr[i]);
        // Compute new sprite positions
        computeShape(spr[i]);
        // Draw spries in new positions
        drawShape(spr[i]);
      } //for
   } //if
  } //while
} //loop
