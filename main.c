/*
 * Matching_game.c
 *
 * Created: 2019-04-02 오후 4:54:18
 * Author : kccistc
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "UART0.h"
#include "CLCD.h"

#define BTN_L	0
#define BTN_R	1
#define BTN_C	2		//좌측키, 우측키, 확인버튼.

void PORT_init(){
	UART0_init();
	LCD_init();
	DDRD &= ~(1 << BTN_L | 1 << BTN_R | 1 << BTN_C);
	PORTD = (1 << BTN_L | 1 << BTN_R | 1 << BTN_C);		//내부 풀업저항 사용.
	DDRF = 0xFF;
	PORTF = 0x00;
}

int main(void){
    
    while (1){
    }
}

