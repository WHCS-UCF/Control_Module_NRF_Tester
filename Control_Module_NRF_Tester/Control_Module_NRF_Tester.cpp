/*
 * Control_Module_NRF_Tester.cpp
 *
 * Created: 6/10/2015 3:19:53 PM
 *  Author: Jimmy
 */ 

#define F_CPU 16000000L
#include "RF24.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "MEGA88A_UART_LIBRARY.h"
#include "MEGA88_SPI_LIBRARY.h"

#ifdef __AVR_ATmega88A__
#endif

volatile uint8_t receiveBuffer[32];
//ce, cs
RF24 radio(1,2);

//Activated on falling edge.
void enableInt1( void ){
	//Set Int1 to activate on falling edge.
	EICRA |= (1<<ISC11);
	EICRA &= (~(1<<ISC10));
	
	DDRD &= (~(1<<DDD3)); // Set PD3(Int1) pin as input.
	
	PORTD |= (1<<DDD3); //Enable the pull-up resistor on PD3(Int1) pin.
	
	EIMSK |= (1<<INT1);
}

ISR(INT1_vect){
	USART_sendString("int\n\r");
	if(radio.available()){
		USART_sendString("Data available!\n\r");
		radio.read((uint8_t*)receiveBuffer, 1);
		USART_sendString("Received: ");
		USART_SendByte((uint8_t)receiveBuffer[0]);
		USART_sendString("\n\r");
	}
}

int main(void)
{
	DDRB |= (1<<DDB0);
	PORTB |= (1<<DDB0);
	initUart();
	InitSPI();
	enableInt1();
	sei();
	
	
	int i;
	for(i=0;i<3;i++){
		PORTB &= ~(1<<DDB0);
		_delay_ms(1000);
		PORTB |= (1<<DDB0);
		_delay_ms(1000);
	}
	
	// Single radio pipe address for the 2 nodes to communicate.
	const uint64_t pipe = 0xE8E8F0F0E1LL;
	

	PORTB |= (1<<DDB0);
	
	char NRFStatus = 0xFF;
	uint8_t command;
	uint8_t buff[5] = {0x0F, 0XA1, 0X33, 0X4C, 0X79};
	while(1)
	{
		command = USART_ReceiveByte();
		if(command=='S'){
			USART_sendString("Checking NRF status\n\r");
			NRFStatus = radio.read_register(STATUS);
			USART_sendString("NRF STATUS: ");
			USART_SendHexByte(NRFStatus);
			USART_sendString("\r\n");
		}
		else if(command=='B'){
			USART_sendString("Beginning the RF\n\r");
			radio.begin();
			USART_sendString("Radio has begun.\n\r");
		}
		else if(command=='A'){
			USART_sendString("Address R0 before setting: ");
			radio.read_register(RX_ADDR_P0, buff, 5);
			USART_sendHexArray(buff,5);
			USART_sendString("\n\r");
			radio.openWritingPipe(pipe);
			USART_sendString("After setting: ");
			radio.read_register(RX_ADDR_P0, buff, 5);
			USART_sendHexArray(buff,5);
			USART_sendString("\n\r");
		}
		else if(command=='O'){
			USART_sendString("Opening writing pipe.\n\r");
			radio.openWritingPipe(pipe);
			USART_sendString("After setting: ");
			radio.read_register(RX_ADDR_P0, buff, 5);
			USART_sendHexArray(buff,5);
			USART_sendString("\n\r");
		}
		else if(command=='W'){
			USART_sendString("Writing...\n\r");
			bool result = radio.write(buff, 5);
			if(result){
				USART_sendString("write() returned true\n\r");
			}
			else{
				USART_sendString("write() returned false\n\r");
			}
		}
		else if(command=='L'){
			radio.openReadingPipe(1, pipe);
			radio.startListening();
			USART_sendString("Beginning to listen...\n\r");
			USART_sendString("After setting: ");
			radio.read_register(RX_ADDR_P1, buff, 5);
			USART_sendHexArray(buff,5);
			USART_sendString("\n\r");
		}
		else if(command=='R'){
			USART_sendString("Read which register? ");
			uint8_t reg = USART_ReceiveHexi();
			USART_SendHexByte(reg);
			USART_sendString("\n\r");
			NRFStatus = radio.read_register(reg);
			USART_sendString("Register value: ");
			USART_SendHexByte(NRFStatus);
			USART_sendString("\n\r");
		}
		else{
			USART_sendString("command not recognized.\n\r");
		}
	}
}