/* ************ LDD4EP: listing9-3: LTC2422_spidev.c ************ */
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
#include <linux/spi/spidev.h>

int8_t read_adc();

// Demo Board Name
char demo_name[] = "DC934";

// Global Variables

float LTC2422_lsb = 4.7683761E-6;  // The LTC2422 least significant bit value with 5V full-scale

// Global Constants
const uint16_t LTC2422_TIMEOUT= 1000;  //Set 1 second LTC2422 SPI timeout

#define SPI_CLOCK_RATE  200000 // SPI Clock in Hz

#define SPI_DATA_CHANNEL_OFFSET 22
#define SPI_DATA_CHANNEL_MASK   (1 << SPI_DATA_CHANNEL_OFFSET)

#define LTC2422_CONVERSION_TIME     137 // ms

// MISO timeout in ms
#define MISO_TIMEOUT 1000

// Returns the Data and Channel Number(0- channel 0, 1-Channel 1)
// Returns the status of the SPI read. 0=successful, 1=unsuccessful.
int8_t LTC2422_read(uint8_t *adc_channel, int32_t *code, uint16_t timeout);

// Returns the Calculated Voltage from the ADC Code
float LTC2422_voltage(uint32_t adc_code, float LTC2422_lsb);

// Bits/word or transaction size.
//#define SPI_DATA_BPW    16
//#define SPI_DATA_BPW    24
#define SPI_DATA_BPW    32

// Returns the Data and Channel Number(0- channel 0, 1-Channel 1)
// Returns the status of the SPI read. 0=successful, 1=unsuccessful.
// Timeout value is ignored.
int8_t LTC2422_read(uint8_t *adc_channel, int32_t *code, uint16_t timeout)
{
	int fd;
	int ret;
	int32_t value;
	uint8_t buffer[4];

	struct spi_ioc_transfer tr = {
		.tx_buf = 0,                      // No data to send
		.rx_buf = (unsigned long) buffer, // Where to store the received data
		.delay_usecs = 0,                 // No delay
		.speed_hz = SPI_CLOCK_RATE,       // SPI clock speed (in Hz)
		.bits_per_word = SPI_DATA_BPW,    // Word/transaction size.
		.len = (SPI_DATA_BPW / 8)         // Number of bytes to transfer.
	};

	// Open the device
	fd = open("/dev/spidev0.0", O_RDWR);
	if (fd < 0) {
		close(fd);
		return (1);
	}

	// Perform the transfer
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1) {
		close(fd);
		return (1);
	}

	// Close the device
	close(fd);

	// Assemble the returned code
#if SPI_DATA_BPW == 16
	value  = buffer[1] << 16;
	value |= buffer[0] << 8;
	// No lower 8-bits due to 16 bit restriction.
#elif SPI_DATA_BPW == 24
	value  = buffer[2] << 16;
	value |= buffer[1] << 8;
	value |= buffer[0];
#elif SPI_DATA_BPW == 32
	value  = buffer[3] << 16;
	value |= buffer[2] << 8;
	value |= buffer[1];
#else
#error Unsupported size for SPI_DATA_BPW.
#endif

	// Determine the channel number
	*adc_channel = (value & SPI_DATA_CHANNEL_MASK) ? 1 : 0;

	printf("the value is %x\n", value);

	// Return the code
	*code = value;

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
	int32_t  adc_code_array[2]; // Array for ADC data. Useful because you don't know which channel until the LTC2422 tells you.
	int8_t return_code;

	// Read ADC
	LTC2422_read(&adc_channel, &adc_code, LTC2422_TIMEOUT);   // Throw out the stale data
	delay(LTC2422_CONVERSION_TIME);

	return_code = LTC2422_read(&adc_channel, &adc_code, LTC2422_TIMEOUT);   // Get current data for both channels
	adc_code_array[adc_channel] = adc_code;                                 // Note that channels may return in any order,
	delay(LTC2422_CONVERSION_TIME);

	return_code = LTC2422_read(&adc_channel, &adc_code, LTC2422_TIMEOUT);   // that is, adc_channel will toggle each reading
	adc_code_array[adc_channel] = adc_code;

	// The DC934A board connects VOUTA to CH1
	adc_voltage = LTC2422_voltage(adc_code_array[1], LTC2422_lsb);
	printf("     ADC A : %6.4f\n", adc_voltage);

	// The DC934A board connects VOUTB to CH0
	adc_voltage = LTC2422_voltage(adc_code_array[0], LTC2422_lsb);
	printf("     ADC B : %6.4f\n", adc_voltage);

	return(return_code);
}

int main(void)
{
	read_adc();
	printf("Application termined\n");
	return 0;
}
