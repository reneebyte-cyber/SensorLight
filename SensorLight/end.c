#include"STC89C52RC.h"
#include <string.h>
#include <stdio.h>
#include <intrins.h>
#define uint unsigned int
#define uchar unsigned char

struct DHTResult{
	int errornum;
	uchar rec_buffer[5];
};

//write
sbit led0=P1^0;
sbit led1=P1^1;
sbit led2=P1^2;
sbit led3=P1^3;
sbit led4=P1^4;
//sbit led5=P1^5;
//sbit led6=P1^6;
//sbit led7=P1^7;
sbit trig=P1^5;
sbit echo=P1^6;
sbit DHT11_IO=P1^7;

sbit lcden=P3^4;
sbit lcdrs=P3^5;
sbit rw=P3^6;

sbit xinghao=P2^0;
sbit S5=P2^5;
sbit dula=P2^6;
sbit wela=P2^7;


int trigNum;
volatile uint t;
uint mult;
struct DHTResult rst = {0,{0,0,0,0,0}};
volatile uchar DHT11_read_byte();
int time;
uint time1;
uint clock;
uint LEDSwitch;
uint moshi;
uint DCA;
//uint timekeeping;
//delay
void delay_ms(uint z){
	volatile uint x,y;
    for(x=z;x>0;x--)
        for(y=120;y>0;y--);
}
void delay_us(uint z){
	while(z--);
}
//display
void write_com(uchar com){
  lcdrs=0;
	rw=0;
	P0=com;
	delay_ms(5);
	lcden=1;
	delay_ms(5);
	lcden=0;
}

void write_data(uchar date){
  lcdrs=1;
	rw=0;
	P0=date;
	delay_ms(5);
	lcden=1;
	delay_ms(5);
	lcden=0;
}
void show(uchar *str){
	while(*str)
		write_data(*str++);
}
//init
void init(){
	led0=0;
	led1=0;
	led2=0;
	led3=0;
//	led4=0;
	mult=0;
	//LCD
	delay_ms(1000);
	dula=0;
	wela=0;
	lcden=0;
	delay_ms(20);
	write_com(0x38);
	write_com(0x0C);
	write_com(0x06);
	write_com(0x80);
	write_com(0x80);
		write_data('O');
		write_data('K');
	//DHT11
	t=0;
	
	//PWM
	trigNum=0;
	LEDSwitch=1;
	time=0;
	time1=0;
	clock=0;
	TMOD&=0x00;
	TMOD|=0x11;
	//1ms/time
	TL0 = 0x48;
	TH0 = 0xFF;		
	//50ms/time
	TL1 = 0x00;
	TH1 = 0x4C;
	EA=1;
	ET0=1;
	ET1=1;
}
//DHT11
int DHT11_start() {
	t=0;
	DHT11_IO = 1;
	//start
	DHT11_IO = 0;
	while(t<1700)
		t+=1;
	DHT11_IO=1;
	//respond
	//wait start
	while(DHT11_IO!=0);
	//wait 83us
	while(DHT11_IO==0);
	//wait 87us
	while(DHT11_IO==1);
	DHT11_read_byte();
	return 0;
}
volatile uchar DHT11_read_byte() {
	uchar i,j,k,dat=0;
	char buf[4];
	for(k=0;k<5;k++){
		for(j=0;j<8;j++){
			//skip 50us
			while(0==DHT11_IO);
			//delay 20us
			for(i=0;i<4;i++);
			if(DHT11_IO){
				dat=dat<<1;
				dat|=1;
				while(DHT11_IO);
			}
			else
				dat=dat<<1;
		}
		rst.rec_buffer[k]=dat;
		dat=0;
	}
	//panduan
	if(rst.rec_buffer[0]+rst.rec_buffer[1]+rst.rec_buffer[2]+rst.rec_buffer[3]==rst.rec_buffer[4]){
		write_com(0x80);
		write_data('H');
		write_data(':');
		buf[0]=rst.rec_buffer[0]/100+'0';
		buf[1]=(rst.rec_buffer[0]/10)%10+'0';
		buf[2]=rst.rec_buffer[0]%10+'0';
		buf[3]='\0';
		show(buf);
		write_data('.');
		buf[0]=rst.rec_buffer[1]/100+'0';
		buf[1]=(rst.rec_buffer[1]/10)%10+'0';
		buf[2]=rst.rec_buffer[1]%10+'0';
		buf[3]='\0';
		show(buf);
		write_com(0x80+0x40);
		write_data('T');
		write_data(':');
		buf[0]=rst.rec_buffer[2]/100+'0';
		buf[1]=(rst.rec_buffer[2]/10)%10+'0';
		buf[2]=rst.rec_buffer[2]%10+'0';
		buf[3]='\0';
		show(buf);
		write_data('.');
		buf[0]=rst.rec_buffer[3]/100+'0';
		buf[1]=(rst.rec_buffer[3]/10)%10+'0';
		buf[2]=rst.rec_buffer[3]%10+'0';
		buf[3]='\0';
		show(buf);
		delay_ms(1000);
	}
//	else
//		led2=0;
	while(DHT11_IO==0);
	return dat;
}
//PWM
void ledpwm(){
	if(LEDSwitch==0){
		TR0=1;
		if(clock<100){
			if(clock<=time)
				led0=0;
			else
				led0=1;
		}
		else
			clock=0;
		//duty ratio
		if(DCA>=200){
			DCA=0;
			if(time>=95){
				time-=1;
				moshi=2;
			}
			else if(time<40){
				time+=1;
				moshi=1;
			}
			if(moshi==1/*&&mult>8*/){
				time+=1;
				mult=0;
			}
			else if(moshi==2/*&&mult>8*/){
				time-=1;
				mult=0;
			}
//			mult++;
		}
	}
	else{
		TR1=1;
		led0=1;
		if(time1>=6){
			time1=0;
			TR1=0;
			led0=0;
		}
	}
}
//ultrasonicSensor
int ultrasonicSensor(){
	int i,k;
	trigNum=0;
	for(i=0;i<1;i++){
		trig=1;
		_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();
		trig=0;
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
		delay_us(500);
		if(echo==0){
			trigNum+=1;
		}
		delay_ms(200);
	}
	if(trigNum==1)
		return 1;
	else
		return 0;
}
//keyscan
int keyscan(){
	int k,cishu;
	if(S5==0){
		led3=1;
		cishu=0;
		delay_ms(20);
		if(S5==0){
			//0:short,1:long
			while(S5==0){
				for(k=0;k<9000;k++){
					if(S5==1&&cishu<6){
						DHT11_start();
						return 1;
					}
				}
				cishu+=1;
				led3=!led3;
			}
			if(LEDSwitch==0){
				LEDSwitch=1;
				return 2;
			}
			else{
				LEDSwitch=0;
				return 2;
			}
		}
		else
			return 0;
	}
	else{
		return 0;
	}
}
//main
void main() {
	init();
	while(1){
		if(LEDSwitch==0){
			ledpwm();
		}
		else{
			trigNum=ultrasonicSensor();
			if(trigNum==1){
				trigNum=0;
						time1=0;
				ledpwm();
			}
			else
				if(led0==1){
					if(time1>=60){
						time1=0;
						TR1=0;
						led0=0;
					}
				}
		}
		keyscan();
	}
}
//interrupt
void timer() interrupt 1{
	TL0 = 0x48;
	TH0 = 0xFF;		
	clock+=1;
	DCA+=1;
}
void timer1() interrupt 3{
	TL1 = 0x00;
	TH1 = 0x4C;
	time1+=1;
}