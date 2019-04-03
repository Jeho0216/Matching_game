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

uint8_t random_array[2][8] = {  };
uint8_t select_flag[2][8] = { 0, };		//각 숫자가 선택가능한 상태인지 표현.
int row = 0;							//LCD의 행위치 지정변수
int col = 0;							//LCD의 열위치 지정변수
volatile uint8_t select = 0;			//선택키 입력에 따라 값이 변경.

ISR(INT0_vect){		//좌측키
	if(col == 0 && row == 1){		//2행 1열에 있는 경우,
		row--;
		col = 7;		//1행 14열로 이동.
	}
	else if(col != 0)
	col--;
}

ISR(INT1_vect){		//중간키
	printf("INT1 exeuted\n");
	if(select == 0){		//첫번째 선택.
		if(select_flag[row][col] != 1){
			LCD_write_data(random_array[row][col] + 48);		//선택한 위치의 숫자 출력
			LCD_goto_XY(row, col);
			select_flag[row][col] = 1;
			select = 1;
		}
	}
	else if(select == 1){
		if(select_flag[row][col] != 1){
			LCD_write_data(random_array[row][col] + 48);
			LCD_goto_XY(row, col);
			select_flag[row][col] = 1;
			select = 2;
		}
	}
}

ISR(INT2_vect){		//우측키
	if(col == 7 && row == 0){
		row++;
		col = 0;
	}
	else if(col != 7)
		col++;
}

void PORT_init(){
	UART0_init();
	LCD_init();
	DDRD &= ~(1 << BTN_L | 1 << BTN_R | 1 << BTN_C);
	PORTD = (1 << BTN_L | 1 << BTN_R | 1 << BTN_C);		//내부 풀업저항 사용.
	DDRF = 0xFF;
	PORTF = 0x00;
}

void INT0_init(){
	EIMSK = (1 << INT0 | 1 << INT1 | 1 << INT2);
	EICRA = (1 << ISC01 | 1 << ISC11 | 1 << ISC21);
}

void game_initialize(int *select_1, int *select_2, uint8_t *flag, int *warning, int *success_count){
	int i, j;
	//게임에 필요한 각종 변수 초기화
	select = 0;
	*success_count = 0;
	*warning = 0;
	flag = 0;
	row = 0;
	col = 0;
	//게임에 필요한 배열 초기화
	for(i = 0; i < 2; i++){
		for(j = 0; j < 16; j++)
			select_flag[i][j] = 0;
	}
	for(i = 0; i < 3; i++){
		select_1[i] = 0;
		select_2[i] = 0;
	}
	//LCD 초기화면 출력.
	LCD_write_command(0x0C);		//커서 출력 금지
	LCD_clear();
	PORTF = 0x00;
	LCD_goto_XY(0, 0);
	LCD_write_string("**Memory game**");
	LCD_goto_XY(1, 0);
	LCD_write_string("Press 1st KEY");
}

void create_random_num(uint8_t array[][8]){
	uint8_t flag[8] = {0};		//생성된 랜덤 숫자가 몇번 발생했는지 확인하기 위한 배열.
	int rand_num;
	int i = 0, j = 0;
	
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

void print_random_num_UART(uint8_t array[][8]){		//시리얼 모니터에 정답화면 출력
	int i, j;
	for(i = 0; i < 2; i++){
		for(j = 0; j < 8; j++)
			printf("%d ", array[i][j]);
		printf("\n");
	}
}

void print_random_num_LCD(uint8_t array[][8]){		//CLCD에 정답 출력.
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

void print_game_board_LCD(){		//게임판 출력함수
	int i, j;
	LCD_goto_XY(0, 0);
	for(i = 0; i < 2; i++){
		for(j = 0; j < 8; j++){
			LCD_write_data(165);
			LCD_write_data(32);
		}
		LCD_goto_XY(1, 0);
	}
	LCD_goto_XY(0, 0);
}

void print_game_clear_LCD(){		//게임클리어 화면 출력함수
	int i, j;
	LCD_goto_XY(0, 0);
	for(i = 0; i < 2; i++){
		for(j = 0; j < 16; j++){
			LCD_write_data('*');
		}
		LCD_goto_XY(1, 0);
	}
	LCD_goto_XY(0, 4);
	LCD_write_string("GAME");
	LCD_goto_XY(1, 7);
	LCD_write_string("CLEAR");
}

void print_game_over_LCD(){		//게임오버 화면 출력함수.
	int i, j;
	LCD_goto_XY(0, 0);
	for(i = 0; i < 2; i++){
		for(j = 0; j < 16; j++){
			LCD_write_data(255);
		}
		LCD_goto_XY(1, 0);
	}
	LCD_goto_XY(0, 4);
	LCD_write_string("GAME");
	LCD_goto_XY(1, 8);
	LCD_write_string("OVER");
}

void check_is_matched(int *select_1, int *select_2, int *warning, int *success_count){		//선택한 숫자 조건 검사함수
	if(select_1[2] == select_2[2]){		//성공했을 경우,
		printf("success\n");
		(*success_count)++;
	}
	else{								//실패했을 경우,
		printf("fail\n");
		(*warning)++;
		//실패했을 경우, 선택헀던 숫자를 다시 가려줌.
		LCD_goto_XY(select_1[0], select_1[1] * 2);
		LCD_write_data(165);
		LCD_goto_XY(select_2[0], select_2[1] * 2);
		LCD_write_data(165);
		LCD_goto_XY(row, col);
		//선택 플래그변수도 초기화
		select_flag[select_1[0]][select_1[1]] = 0;
		select_flag[select_2[0]][select_2[1]] = 0;
	}
}

int main(void){
	int select_1[3] = {0,}, select_2[3] = {0,};		//선택한 숫자의 행, 열, 값을 저장하는 배열.
	uint8_t game_state = 0;			//게임 상태. 0 : 게임종료, 1 : 게임진행중
	uint8_t flag = 0;				//while문안의 if문을 한번씩만 실행하도록 하기위한 변수.
	int success_count = 0;
	int warning = 0;
	int rand_cnt = 0;
	
	PORT_init();
	INT0_init();
	
	stdout = &OUTPUT;
	stdin = &INPUT;
	
	while(1){
		if(game_state == 0){		//게임 진행을 위해서 초기 설정부분.
			game_initialize(select_1, select_2, &flag, &warning, &success_count);
			while((PIND & 0x01) != 0){};		//1번째 버튼 입력전까지 NOP
			srand(rand_cnt);		//같은 패턴의 랜덤수가 나오는 것을 방지
			create_random_num(random_array);
			rand_cnt++;
			print_random_num_UART(random_array);
			print_random_num_LCD(random_array);
			_delay_ms(2000);
			//정답을 보여주고 5초뒤 숫자를 가려줌.
			print_game_board_LCD();
			LCD_write_command(0x0F);
			game_state = 1;
		}
		else{						//게임 시작.
			EIFR = 0x00;
			sei();		//인터럽트 허용
			printf("select = %d, flag = %d\n", select, flag);
			while(game_state == 1){
				LCD_goto_XY(row, col * 2);		//row, col의 범위가 0 ~ 7까지이므로, LCD에 2를 곱해서 출력.
				if(select == 1 && flag == 0){				//첫 번째 숫자를 선택했을 경우,
					select_1[0] = row;
					select_1[1] = col;
					select_1[2] = random_array[row][col];
					flag = 1;
					printf("select = %d, flag = %d\n", select, flag);
					printf("select 1 : %d\n", select_1[2]);
				}
				else if(select == 2 && flag == 1){			//두 번째 숫자를 선택했을 경우,
					select_2[0] = row;
					select_2[1] = col;
					select_2[2] = random_array[row][col];
					printf("select = %d, flag = %d\n", select, flag);
					printf("selcet 2 : %d\n", select_2[2]);
					//숫자 선택 끝나고 정답 여부 확인.
					check_is_matched(select_1, select_2, &warning, &success_count);
					flag = 0;
					select = 0;
				}
				PORTF |= ((0x01) << warning) - 1;		//경고 LED 출력. 0x01 -> 0x03 -> 0x07 순서대로 출력함.
				if(success_count >= 8){
					print_game_clear_LCD();
					print_game_clear_LCD();
					cli();
					game_state = 0;
				}
				if(warning >= 3){
					print_game_over_LCD();
					print_game_over_LCD();
					cli();		//인터럽트 금지.
					game_state = 0;
				}
			}
		}
	}
	return 0;
}

