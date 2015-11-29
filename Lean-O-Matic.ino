/******************************************************************************/
/*                       Lean-O-Matic for motorcycle                          */
/******************************************************************************/
/* Parts needed:                                                              */
/* - Breakout accelerometer MMA7361L                                          */
/* - OLED Display 0.96" SPI                                                   */
/* - Two Neopixels bars of eight leds                                         */
/******************************************************************************/

 #include <SPI.h>
 #include <Wire.h>
 #include <Adafruit_GFX.h>
 #include <Adafruit_SSD1306.h>
 #include <AcceleroMMA7361.h>
 #include <Adafruit_NeoPixel.h>

/******************************************************************************/
/*                                VARIABLES                                   */
/******************************************************************************/
/* AC_POS                                                                     */
/*     Default 0, 0 for the correct way or 1 for turned right                 */
/*     accelerometer position switch (X/Y).                                   */
/*                                                                            */
/* AC_SENSITIVITY                                                             */
/*      Default HIGH, HIGH for 6G or LOW for 1.5G                             */
/*      accelerometer sensitivity between 1.5G or 6G                          */
/*                                                                            */
/* USE_NEOPIXEL                                                               */
/*      Default 1, 0 for off or 1 of on                                       */
/*      Activate the use of Neopixels, more settings below                    */
/*                                                                            */
/* NPS_LENGTH                                                                 */
/*      Default 16 pixels                                                     */
/*      Length of the Neopixel array                                          */
/*                                                                            */
/* NPS_BRIGHTNESS                                                             */
/*      Default 50, from 0 which is off or 255 for full brightness            */
/*      Brightness value for the Neopixels bars                               */
/*                                                                            */
/* DZN_ANGLE                                                                  */
/*      Default 15, in degrees, from 0 to 90                                  */
/*      Deadzone of the lean angle                                            */
/*                                                                            */
/* DZN_ACC                                                                    */
/*       Default 10, in purcents, from 0 to 100                               */
/*       Deadzone of acceleration and breaking                                */
/*                                                                            */
/* SPL_TIME                                                                   */
/*      Default 5, in x time                                                  */
/*      Sample time between each reading to make an average value. For        */
/*      exemple with 5 spl_time, it will read 5 time the value of the         */
/*      accelerometer and divide it by 5 to get the average reading.          */
/* SPL_RATE                                                                   */
/*      Default 50, in milliseconds                                           */
/*      In addition of the sample time is the sample rate. It's the time      */
/*      between each sample to smooth out the reading further more.           */
/* DSP_RATE                                                                   */
/*      Default 50, in milliseconds                                           */
/*      Set the rate of refresh on the OLED and Neopixels array.              */
/*                                                                            */
/******************************************************************************/

#define AC_POS 1
#define AC_SENSITIVITY HIGH
#define USE_NEOPIXEL 1
#define NP_LENGTH 16
#define NPS_BRIGHTNESS 20
#define DZN_ANGLE 5
#define DZN_ACC 10
#define SPL_TIME 5
#define SPL_RATE 5
#define DSP_RATE 10


/******************************************************************************/
/*                                  SETTING                                   */
/******************************************************************************/
/* ACCELEROMETER                                                              */
/*      Command in setup as Sleep, SelfTest, ZeroG, GSelect, X, Y, Z          */
/*     10 - 0GS    SFT - 7                                                    */
/*     A0 -  X     GSL - 4                                                    */
/*     A1 -  Y     SLP - 12                                                   */
/*     A2 -  Z     GND -                                                      */
/*        - GND    VCC -                                                      */
/*                                                                            */
/* OLED DISPLAY                                                               */
/*     GND     VCC     D0     D1     RST     DC                               */
/*                     8      9      10      11                               */
/*                                                                            */
/* NEOPIXEL                                                                   */
/*    Command pin on 6                                                        */
/*                                                                            */
/******************************************************************************/

#define AC_SLP 12
#define AC_GSL 4
#define AC_SFT 7
#define AC_0G 10
#define AC_X A0
#define AC_Y A1
#define AC_Z A2

#define OD_CLK 8
#define OD_MOSI 9
#define OD_RESET 10
#define OD_DC 11


#define NP_PIN 6
#define NP_TYPE NEO_GRB + NEO_KHZ800

AcceleroMMA7361 acc;
Adafruit_SSD1306 oled(OD_MOSI, OD_CLK, OD_DC, OD_RESET, 0);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NP_LENGTH, NP_PIN, NP_TYPE);

int angle;
int l_max = 0;
int r_max = 0;

char *version = "VERSION 1.0 BY M600";

void setup(){
    acc.begin(AC_SLP, AC_SFT, AC_0G, AC_GSL, AC_X, AC_Y, AC_Z);
    acc.setARefVoltage(3.3);
    acc.setSensitivity(AC_SENSITIVITY);
    oled.begin(SSD1306_SWITCHCAPVCC);
    oled.clearDisplay();
    oled.display();
    splashscreen();
    if (USE_NEOPIXEL)
    {
        strip.begin();
        strip.setBrightness(NPS_BRIGHTNESS);
        neopixel_reset();
    }
}

void loop(){
    int sr_angle = 0;
    int sr_accbrk = 0;
    for (int i = 0; i < SPL_TIME; i++)
    {
        if (AC_POS)
        {
            sr_accbrk = sr_accbrk + acc.getYAccel();
            sr_angle = sr_angle + map(acc.getXAccel(), 0, 100, 0, 90);
        }
        else
        {
            sr_accbrk = sr_accbrk + acc.getXAccel();
            sr_angle = sr_angle + map(acc.getYAccel(), 0, 100, 0, 90);
        }
        delay(SPL_RATE);
    }
    sr_accbrk = sr_accbrk / SPL_TIME;
    sr_angle = sr_angle / SPL_TIME;
    oled_display(sr_angle, sr_accbrk);
    if (USE_NEOPIXEL == 1)
        neopixel(sr_angle);
    delay(DSP_RATE);
}

void splashscreen(){
    oled.setTextColor(WHITE);
    oled.setTextSize(2);
    oled.setCursor(0,0);
    oled.print("LEAN");
    oled.setCursor(52,0);
    oled.print("O");
    oled.setCursor(68,0);
    oled.print("MATIC");
    oled.fillRect(0, 53, 128, 11, WHITE);
    oled.setTextSize(1);
    oled.setTextColor(BLACK);
    oled.setCursor(7, 55);
    oled.print(version);
    oled.display();
    delay(1000);
    oled.setTextColor(WHITE);
    oled.setCursor(17,20);
    oled.setTextSize(2);
    oled.print("LEVELING");
    oled.setCursor(32, 40);
    oled.setTextSize(1);
    oled.print("PLEASE WAIT");
    oled.display();
    delay(2000);
    acc.calibrate();
    oled.fillRect(0,16,128,37,BLACK);
    oled.setCursor(16,25);
    oled.setTextSize(2);
    oled.print("ALL DONE");
    oled.display();
    delay(1000);
    oled.fillRect(0,16,128,37,BLACK);
    oled.setCursor(5,25);
    oled.setTextSize(2);
    oled.print("DRIVE SAFE");
    oled.display();
    delay(1000);

}

void neopixel_reset(void){
    for (int i = 0; i < NP_LENGTH; i++)
    {
        strip.setPixelColor(i, 0, 0, 0);
        strip.show();
    }
}

void neopixel_dsp(int id, int g, int r, int b){
    neopixel_reset();
    strip.setPixelColor(id, g, r, b);
    strip.show();
}

void neopixel(int angle){
    if (angle == 0)
       neopixel_reset();
    if (angle < 0)
    {
        angle = angle * -1;
        if (angle > 90)
            angle = 90;
        if (angle > DZN_ANGLE)
        {
            angle = map(angle, 0, 90, 0, 8);
            switch (angle)
            {
                case 7:
                    neopixel_dsp(0, 255, 0, 0);
                    break;
                case 6:
                    neopixel_dsp(1, 218, 36, 0);
                    break;
                case 5:
                    neopixel_dsp(2, 182, 72, 0);
                    break;
                case 4:
                    neopixel_dsp(3, 145, 109, 0);
                    break;
                case 3:
                    neopixel_dsp(4, 109, 145, 0);
                    break;
                case 2:
                    neopixel_dsp(5, 72, 182, 0);
                    break;
                case 1:
                    neopixel_dsp(6, 36, 218, 0);
                    break;
                case 0:
                    neopixel_dsp(7, 0, 255, 0);
                    break;
            }
        }
        else
            neopixel_reset();
    }
    else
    {
        if (angle > 90)
            angle = 90;
        if (angle > DZN_ANGLE)
        {
            angle = map(angle, 0, 90, 0, 8);
            switch (angle)
            {
                case 0:
                    neopixel_dsp(8, 0, 255, 0);
                    break;
                case 1:
                    neopixel_dsp(9, 36, 218, 0);
                    break;
                case 2:
                    neopixel_dsp(10, 72, 182, 0);
                    break;
                case 3:
                    neopixel_dsp(11, 109, 145, 0);
                    break;
                case 4:
                    neopixel_dsp(12, 145, 109, 0);
                    break;
                case 5:
                    neopixel_dsp(13, 182, 72, 0);
                    break;
                case 6:
                    neopixel_dsp(14, 218, 36, 0);
                    break;
                case 7:
                    neopixel_dsp(15, 255, 0, 0);
                    break;
            }
        }
        else
        {
            neopixel_reset();
        }
    }
}

void oled_display(int angle, int accbrk){
    oled.clearDisplay();
    dsp_gfx();
    dsp_angle(angle);
    dsp_accbrk(accbrk);
    oled.display();
}

void dsp_gfx(){
    oled.setTextSize(1);
    oled.drawRect(0,0,18,56,1);
    oled.fillRect(2,2,14,52,1);
    oled.setCursor(0,57);
    oled.println("ACC");
    oled.drawRect(110,0,18,56,1);
    oled.fillRect(112,2,14,52,1);
    oled.setCursor(110,57);
    oled.println("BRK");
}

void dsp_idle(){
    oled.setTextSize(1);
    oled.setCursor(49,0);
    oled.print("ANGLE");
    oled.setCursor(55,9);
    oled.print("MAX");
    oled.setTextSize(7);
    oled.setCursor(25,16);
    oled.print("--");
}

void dsp_angle(int angle){
    oled.setTextSize(2);
    if (AC_POS)
    {
        oled.setCursor(85,1);
        oled.print(l_max);
        oled.setCursor(20,1);
        oled.print(r_max);
    }
    else
    {
        oled.setCursor(20,1);
        oled.print(l_max);
        oled.setCursor(85,1);
        oled.print(r_max);
    }
    oled.setTextSize(7);
    oled.setCursor(25,16);
    if (angle == 0)
        dsp_idle();
    if (angle < 0)
    {
        angle = angle * -1;
        if (angle > 90)
            angle = 90;
        if (AC_POS)
        {
            if (angle > DZN_ANGLE)
            {
                if (angle > l_max)
                    l_max = angle;
                oled.fillTriangle(65, 0, 75, 7, 65, 14, WHITE);
                oled.print(angle);
            }
            else
                dsp_idle();
        }
        else
        {
            if (angle > DZN_ANGLE)
            {
                if (angle > r_max)
                    r_max = angle;
                oled.fillTriangle(62, 0, 52, 7, 62, 14, WHITE);
                oled.print(angle);
            }
            else
                dsp_idle();
        }
    }
    else
    {
        if (angle > 90)
            angle = 90;
        if (AC_POS)
        {
            if (angle > DZN_ANGLE)
            {
                if (angle > r_max)
                    r_max = angle;
                oled.fillTriangle(62, 0, 52, 7, 62, 14, WHITE);
                oled.print(angle);
            }
            else
                dsp_idle();
        }
        else
        {
            if (angle > DZN_ANGLE)
            {
                if (angle > l_max)
                    l_max = angle;
                oled.fillTriangle(65, 0, 75, 7, 65, 14, WHITE);
                oled.print(angle);
            }
            else
                dsp_idle();
        }
    }
}

void dsp_accbrk(int accbrk){
//    double g_value = sqrt(acc.getXAccel() * acc.getXAccel() + acc.getYAccel() * acc.getYAccel() + acc.getZAccel() * acc.getZAccel());
//    oled.setTextSize(1);
//    oled.setCursor(50,55);
//    oled.print(g_value);
    if (accbrk > 0 && accbrk > DZN_ACC)
    {
        accbrk = map(accbrk,0,100,0,52);
        oled.fillRect(2,2,14,52 - accbrk,0);
        oled.fillRect(112,2,14,52,0);
    }
    else if (accbrk < 0)
    {
        accbrk = accbrk * -1;
        accbrk = map(accbrk,0,100,0,52);
        oled.fillRect(2,2,14,52,0);
        oled.fillRect(112,2,14,52 - accbrk,0);
    }
    else
    {
        oled.fillRect(2,2,14,52,0);
        oled.fillRect(112,2,14,52,0);
    }
}
