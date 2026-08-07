#include "main.h"
#include "adc.h"
#include "light.h"
volatile uint8_t voltage_light = 0;
float vol = 0;

void lighttrasit(void){
	vol = (HAL_ADC_GetValue(&hadc3)*3)/4095;
	if(vol>1.1)
		voltage_light = 1;
	else if(vol>2.2)
		voltage_light = 2;
	else 
		voltage_light = 0;
}