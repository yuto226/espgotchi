/*
 * MCUGotchi - A Tamagotchi P1 emulator for microcontrollers
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdint.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "spi.h"
#include "board.h"

static spi_device_handle_t spi_device;

void spi_init(void)
{
	spi_bus_config_t buscfg = {
		.miso_io_num = -1,
		.mosi_io_num = BOARD_SCREEN_MOSI_PIN,
		.sclk_io_num = BOARD_SCREEN_SCLK_PIN,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4096
	};

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 10*1000*1000,
		.mode = 0,
		.spics_io_num = -1,
		.queue_size = 7,
	};

	ESP_ERROR_CHECK(spi_bus_initialize(BOARD_SCREEN_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
	ESP_ERROR_CHECK(spi_bus_add_device(BOARD_SCREEN_SPI_HOST, &devcfg, &spi_device));
}

void spi_write(uint8_t data)
{
	spi_transaction_t t = {
		.length = 8,
		.tx_buffer = &data,
	};
	
	ESP_ERROR_CHECK(spi_device_polling_transmit(spi_device, &t));
}