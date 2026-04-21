#include "led.h"
#include "buzzer.h"
#include "button.h"
#include "delay.h"
#include "dht.h"
#include "ldt.h"
#include "eint.h"
#include "iwdg.h"
#include "usart1.h"
#include "stdio.h"
#include "adj_res.h"
#include "oled.h"
#include "sht.h"
#include "eeprom.h"
#include "rtc.h"
#include "esp8266.h"
#include "string.h"

void recv_handler(char *buf, int len)
{
	if(!strcmp(buf, "BUZZER_ON"))
		buzzer_on();
	if(!strcmp(buf, "BUZZER_OFF"))
		buzzer_off();
}

int main(void)
{
	int i = 500;//计数变量
	u8 dht_data[5] = {0};//存储DHT11采集的数据
	double sht_data[2] = {0};//存储SHT30采集的数据
	int adj_res_value = 0;//存储高精度可调电阻的数据量
	u8 oldvalue = 96;//存储要写入到EEPROM中的数据
	u8 newvalue = 0;//存储从EEPROM中读取到的数据
	struct time_st time = {2026, 3, 11, 3, 15, 40, 0};//存储设置的时间
	char data[] = "2026-03-11 3 15:40:00 DHT:25/60 SHT:25.50/60.20 ADJ:1234";
	
	led_init();//调用LED灯初始化的函数
	buzzer_init();//调用蜂鸣器初始化的函数
	button_init();//调用功能按键初始化的函数
	delay_init();//调用系统定时器初始化的函数
	dht_init();//调用DHT11传感器初始化的函数
	ldt_init();//调用数码管初始化的函数
	eint_init();//调用按键中断初始化的函数
	usart_1_init();//调用USART1初始化的函数
	adj_res_init();//调用高精度可调电阻初始化的函数
	OLED_Init();//调用OLED显示屏初始化的函数
	sht_init();//调用SHT30传感器初始化的函数
	eeprom_init();//调用EEPROM初始化的函数
	rtc_init(&time);//调用RTC初始化的函数
	esp8266_init();//调用ESP8266初始化的函数
	
	OLED_Clear();//清屏
	
	//OLED_ShowChar(0, 0, '#', 16);
	//OLED_ShowNum(0, 2, 9527, 4, 16);
	//OLED_ShowString(0, 4, "Hello World", 16);
	//OLED_ShowCHinese(0, 6, 5);
	picture_1();
	
	led_on(0);
	esp8266_link_wifi("wzh", "weizhihao");//连接手机热点
	delay_ms(1000);//延时2s
	led_off(0);
	
	led_on(1);
	esp8266_setup_udp("8888");//设置UDP模式,本地端口8888
	delay_ms(1000);
	led_off(1);
	
	sht_write_mode();//设置SHT30传感器的采样频率
	
	//eeprom_byte_write(0xA0, 10, oldvalue);//把oldvalue的数据写入到EEPROM的第10个字节
	
	set_wifi_recv_handler(recv_handler);//设置WiFi的回调函数
	
	while(1)
	{
		//发送UDP数据到服务器 (IP: 10.91.124.81, 端口: 9527)
		newvalue = eeprom_rand_read(0xA0, 10);//从EEPROM的第10个字节空间读取数据
		sht_write_read_cmd();//发送获取SHT30采集数据的指令
		get_dht_value(dht_data);//获取DHT11传感器采集到的数据
		adj_res_value = get_adj_res_value();//获取高精度可调电阻的数据量
		delay_ms(1000);
		sht_read_data(sht_data);//获取SHT30采集到的数据
		sprintf(data, "%04d-%02d-%02d %d %02d:%02d:%02d DHT:%02d/%02d SHT:%.2f/%.2f ADJ:%04d\n", time.Y, time.M, time.D, time.W, time.h, time.m, time.s, dht_data[0], dht_data[2], sht_data[0], sht_data[1], adj_res_value);

		//sprintf(data, "DHT:%02d/%02d SHT:%.2f/%.2f ADJ:%04d\n",dht_data[0], dht_data[2], sht_data[0], sht_data[1], adj_res_value);
		
		//使用UDP发送数据
		esp8266_send_udp(data, "56", "192.168.32.4", "9527");
		
		while(i--)
			digit_show_data(adj_res_value);
		i = 500;
		led_on(2);
		delay_ms(2000);
		led_off(2);
		delay_ms(2000);
	}
}
