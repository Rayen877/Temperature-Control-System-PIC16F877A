#include <xc.h>
#define _XTAL_FREQ 20000000
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define RS PORTCbits.RC0
#define RW PORTCbits.RC1
#define EN PORTCbits.RC2

#define C1 PORTBbits.RB5
#define C2 PORTBbits.RB6
#define C3 PORTBbits.RB7

#define R1 PORTBbits.RB0
#define R2 PORTBbits.RB1
#define R3 PORTBbits.RB2
#define R4 PORTBbits.RB3

#define HEATER PORTAbits.RA1
#define FAN PORTAbits.RA2

//char key,old_key;
unsigned short temp_ref;
unsigned char key[10],keyActual[10],ch[10];
unsigned char keypress=' ';


unsigned char digit_count = 0;
unsigned char first_digit = 0;
unsigned char second_digit = 0;
unsigned char target_temp = 0;
unsigned int N[10]= {0,1,2,3,4,5,6,7,8,9};

unsigned int a,b,c,d,e,f; 
unsigned int temp,adc=0;  
float temperature;  


void lcd_cmd(unsigned char cmd){
    PORTC = (cmd & 0xF0);
    EN=1;
    RW=0;
    RS=0;
    __delay_ms(2);
    EN=0;
    PORTC = ((cmd <<4) & 0xF0);
    EN=1;
    RW=0;
    RS=0;
    __delay_ms(2);
    EN=0;
}

void lcd_data(unsigned char data){
    PORTC = (data & 0xF0);
    EN=1;
    RW=0;
    RS=1;
    __delay_ms(2);
    EN=0;
    PORTC = ((data <<4) & 0xF0);
    EN=1;
    RW=0;
    RS=1;
    __delay_ms(2);
    EN=0;
}
void lcd_init(){
    lcd_cmd(0x02);
    lcd_cmd(0x28); //4 bits mode 2 lines 16 columns
    lcd_cmd(0x0C); // cursor off & lcd on
    lcd_cmd(0x06); //auto increment
    lcd_cmd(0x01); // clear screen
    
}

unsigned char longueur(const unsigned char *str){
    unsigned char c;
    unsigned char i=0;
    if(str !=NULL){
     c= *str;
    }
    while(c !='\0'){
        i++;
        c= *(str+i);
    }
    return i;
    
}
void lcd_string(const unsigned char *str){
    unsigned char i;
    unsigned char num= longueur(str);
    for(i=0; i< num; i++){
        lcd_data(str[i]);
    }
}

void keypad(){
    C1=1,C2=0,C3=0;
    if(R1==1){
        lcd_data('1');
        keypress='1';
        digit_count++;
        while(R1==1);
    }
    if(R2==1){
        lcd_data('4');
        keypress='4';
        digit_count++;
        while(R2==1);
    }
    if(R3==1){
        lcd_data('7');
        keypress='7';
        digit_count++;
        while(R3==1);
    }
    if(R4==1){
       
        keypress='*';
        while(R4==1);
    }
    C1=0,C2=1,C3=0;
    if(R1==1){
        lcd_data('2');
        keypress='2';
        digit_count++;
        while(R1==1);
    }
    if(R2==1){
        lcd_data('5');
        keypress='5';
        digit_count++;
        while(R2==1);
    }
    if(R3==1){
        lcd_data('8');
        keypress='8';
        digit_count++;
        while(R3==1);
    }
    if(R4==1){
        lcd_data('0');
        keypress='0';
        digit_count++;
        while(R4==1);
    }
    C1=0,C2=0,C3=1;
    if(R1==1){
        lcd_data('3');
        keypress='3';
        digit_count++;
        while(R1==1);
    }
    if(R2==1){
        lcd_data('6');
        keypress='6';
        digit_count++;
        while(R2==1);
    }
    if(R3==1){
        lcd_data('9');
        keypress='9';
        digit_count++;
        while(R3==1);
    }
    if(R4==1){
        //lcd_data('#');
        keypress='#';
        while(R4==1);
    }
}
void read_temp(){
    ADCON0bits.CHS0=0,ADCON0bits.CHS1=0,ADCON0bits.CHS2=0; // choose channel AN0
        ADCON0bits.GO_DONE= 1; //start adc conversion
        while(PIR1bits.ADIF== 0); //adc conversion complete
        
        adc=ADRESH << 8;
        adc= adc+ADRESL;
        temperature =adc/2.046; // temperature value
}
void regulate(){
    read_temp();
    if(temperature <target_temp){
        HEATER=1;
        FAN=0;
    }else{
        HEATER=0;
        FAN=1;
    }
}
void main(void) {
    TRISC=0;
    int i=0;
    TRISB= 0x0F;
    PORTB=0;
    TRISA=0x01;
    
    temp_ref=0;
    lcd_init();
    //ADC
    ADCON0bits.ADCS0= 1,ADCON0bits.ADCS1= 0,ADCON1bits.ADCS2=0; //fosc/8
    ADCON0bits.ADON= 1; //powerup ADC
    
    ADCON1bits.PCFG0=0,ADCON1bits.PCFG1=1,ADCON1bits.PCFG2=1,ADCON1bits.PCFG3=1;//configue pins as analog or digital(lehne ken->AN0,wl be9y digital)
    ADCON1bits.ADFM=1; //right justifier
    
    lcd_cmd(0x80); // first row and column
    lcd_string("Automatic");
    lcd_cmd(0xC0);
    lcd_string("Temp Control");
    __delay_ms(2500);
    lcd_cmd(0x01);
    lcd_init();
    lcd_cmd(0x80);
    HEATER=0;
    FAN= 0;   
 MB:
     digit_count=0;
     keypress=' ';
     i=0;
     strcpy(key,"");
     lcd_cmd(0x01);
     lcd_cmd(0x80); // first row and column
     lcd_string("Enter temp Ref");
     
     lcd_cmd(0xC0);
     lcd_string("Temp ref: ");
     lcd_cmd(0xCA);
   
     while(1){
        keypad();
        if(keypress== '#'){
            goto affich;
            
        }
        if(keypress== '*'){
            goto MB;
        }
        if (keypress >= '0' && keypress <= '9') {
          if (digit_count == 1) {
            first_digit = keypress - '0';
         } else if (digit_count == 2) {
            second_digit = keypress - '0';
            target_temp = first_digit * 10 + second_digit;
            digit_count++;  // optionally block further input or reset later
           
            if(keypress== '#'){
            goto affich;}  
        } 
         }
       }
             
     
  affich:
   lcd_cmd(0x01);
   sprintf(ch,"%d",target_temp);
   lcd_cmd(0x80);
   lcd_string("Temp target:");
   lcd_cmd(0xC1);
   lcd_string(ch);
   lcd_data(0xDF);
   lcd_data('C');
   while(1){
       regulate();
       keypad();
       if(keypress== '*'){
            goto MB;
        }
   }
   
 
 
   
}
