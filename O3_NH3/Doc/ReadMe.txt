1. 主控 STM32F103RCT6   RET6
2. 主要连接
	///////////////////////TIM3原始的四通道 /////////////////////
	//PA6 --TIM3CH1
	//PA7 --TIM3CH2
	//PB0 --TIM3CH3
	//PB1 --TIM3CH4

	///////////////////////TIM3重映射的四通道 /////////////////////
	//PC6 --TIM3CH1
	//PC7 --TIM3CH2
	//PC8 --TIM3CH3
	//PC9 --TIM3CH4
	
	M5310 :
	PB10(U3_TX) -- M5310 RXD
	PB11(U3_RX) -- M5310 TXD
	PC13       --m5310 reset
	3.3V      ---m5310 vcc
	gnd       -- m5310 gnd
	
	tft SCREEN:(SDWA070C03N)
	PA2(U2_TX)   -- tftdin   --pin4 && pin3
	PA3(U2_RX)   -- tftdout  --pin5
	
	tft pin :
	PIN1    --GND
	PIN2 	--GND	
	PIN3    --TFTDIN
	PIN4    --TFTDIN
	PIN5    --TFTDOUT
	PIN6    --NC
	PIN7    --VCC5V 
	PIN8    --VCC5V
	
	MKEY :  
	//               |------------------|
	//PB15--key8-----|                  |
	//PB14--key7-----|                  |
	//PB13--key6-----|                  |
	//PB12--key5-----|                  |
	//PC5 --key4-----|                  |
	//PC4 --key3-----|                  |
	//PA7 --key2-----|                  |
	//PA6 --key1-----|                  |
	//               |------------------|
	
	
	eeprom ；AT24C02
	PB6
	PB7
	
	DHT22 
	PA12(any io pin)
	
	
	io口控制 5个
	
	IO1 --PA15
	IO2 --PB0
	IO3 --PB1
	IO4 --PB3
	IO5 --PB4
	IO6
	