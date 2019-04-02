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
#include <time.h>
#include "UART0.h"
#include "CLCD.h"

#define BTN_L	0
#define BTN_R	1
#define BTN_C	2		//좌측키, 우측키, 확인버튼.

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);
FILE INPUT = FDEV_SETUP_STREAM(NULL, UART0_receive, _FDEV_SETUP_READ);

void PORT_init(){
	UART0_init();
	LCD_init();
	DDRD &= ~(1 << BTN_L | 1 << BTN_R | 1 << BTN_C);
	PORTD = (1 << BTN_L | 1 << BTN_R | 1 << BTN_C);		//내부 풀업저항 사용.
	DDRF = 0xFF;
	PORTF = 0x00;
}

void create_random_num(uint8_t array[][8]){
	uint8_t flag[8] = {0};		//생성된 랜덤 숫자가 몇번 발생했는지 확인하기 위한 배열.
	int rand_num;
	int i = 0, j = 0;
	srand(time(NULL));		//같은 패턴의 랜덤수가 나오는 이유. MCU에서는 시간을 읽어줄 OS가 존재하지 않음.
	
	for(i = 0; i < 2; i++){
		for(j = 0; j < 8; j++){
			rand_num = rand() % 8;		//0 ~ 7 사이의 값을 가짐.
			if(flag[rand_num] == 0 | flag[rand_num] == 1){
				array[i][j] = rand_num;		//2개이하로 생성되었을 경우, 2차원 배열에 저장.
				flag[rand_num]++;			//생성된 숫자 플래그 1 증가.
			}
			else if(flag[rand_num] >= 2){
				j--;
				continue;
			}
		}
	}
}

void print_random_num_UART(uint8_t array[][8]){
	int i, j;
	for(i = 0; i < 2; i++){
		for(j = 0; j < 8; j++)
			printf("%d ", array[i][j]);
		printf("\n");
	}
}

void print_random_num_LCD(uint8_t array[][8]){
	int i, j;
	LCD_clear();
	LCD_goto_XY(0, 0);
	for(i = 0; i < 2; i++){
		for(j = 0; j < 8; j++){
			LCD_write_data(array[i][j] + 48);
			LCD_write_data(32);
			_delay_ms(20);
		}
		LCD_goto_XY(1, 0);
	}
}
int main(void){
	uint8_t random_array[2][8] = {  };
	
	PORT_init();
	
	stdout = &OUTPUT;
	stdin = &INPUT;
	
	create_random_num(random_array);
	printf("create_rand_num executed\n");
	print_random_num_UART(random_array);
	print_random_num_LCD(random_array);
	while (1){
		
	}
}

