/* ************ LDD4EP: listing9-5: LTC2422_app.c ************ */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

int8_t read_adc();

// Global Variables
float LTC2422_lsb = 4.7683761E-6;  // The LTC2422 least significant bit value with 5V full-scale

#define LTC2422_FILE_VOLTAGE		"/sys/bus/iio/devices/iio:device2/out_voltage0_raw"
#define SPI_DATA_CHANNEL_OFFSET		22
#define SPI_DATA_CHANNEL_MASK		(1 << SPI_DATA_CHANNEL_OFFSET)
#define LTC2422_CONVERSION_TIME		137 // ms

// Returns the Data and Channel Number(0- channel 0, 1-Channel 1)
// Returns the status of the SPI read. 0=successful, 1=unsuccessful.
int8_t LTC2422_read(uint8_t *adc_channel, int32_t *code);

// Returns the Calculated Voltage from the ADC Code
float LTC2422_voltage(uint32_t adc_code, float LTC2422_lsb);

int8_t LTC2422_read(uint8_t *adc_channel, int32_t *code)
{
	int a2dReading = 0;
	FILE *f = fopen(LTC2422_FILE_VOLTAGE, "r");
	int read = fscanf(f, "%d", &a2dReading);
	if (read <= 0) {
		printf("ERROR: Unable to read values from voltage input file.\n");
		exit(-1);
	}

	// Determine the channel number
	*adc_channel = (a2dReading & SPI_DATA_CHANNEL_MASK) ? 1 : 0;
	*code = a2dReading;
	fclose(f);

	return(0);
}

// Returns the Calculated Voltage from the ADC Code
float LTC2422_voltage(uint32_t adc_code, float LTC2422_lsb)
{
	float adc_voltage;

	if (adc_code & 0x200000) {
		adc_code &= 0xFFFFF;	// Clears Bits 20-23
		adc_voltage=((float)adc_code)*LTC2422_lsb;
	} else {
		adc_code &= 0xFFFFF;	// Clears Bits 20-23
		adc_voltage = -1*((float)adc_code)*LTC2422_lsb;
	}
	return(adc_voltage);
}

void delay(unsigned int ms)
{
	usleep(ms*1000);
}

int8_t read_adc()
{
	float adc_voltage;
	int32_t adc_code;
	uint8_t adc_channel;

	LTC2422_read(&adc_channel, &adc_code);
	delay(LTC2422_CONVERSION_TIME);

	// The DC934A board connects VOUTA to CH1 and VOUTB toc CH0
	//adc_voltage = LTC2422_voltage(adc_code, LTC2422_lsb);

	LTC2422_read(&adc_channel, &adc_code);
	adc_voltage = LTC2422_voltage(adc_code, LTC2422_lsb);
	printf("the value of ADC channel %d\n", adc_channel);
	printf("     is : %6.4f\n", adc_voltage);
	delay(LTC2422_CONVERSION_TIME);

	LTC2422_read(&adc_channel, &adc_code);
	adc_voltage = LTC2422_voltage(adc_code, LTC2422_lsb);
	printf("the value of ADC channel %d\n", adc_channel);
	printf("     is : %6.4f\n", adc_voltage);

	return(0);
}

int main(void)
{
	read_adc();
	printf("Application termined\n");
	return 0;
}
