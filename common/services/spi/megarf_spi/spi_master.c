/**
 * \file
 *
 * \brief SPI master driver functions.
 *
 * Copyright (c) 2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include "spi_master.h"
#include "sysclk.h"

/**
 * \brief Initializes the SPI in master mode.
 *
 * \param spi Base address of the SPI instance.
 *
 */
void spi_master_init(volatile void *spi)
{
	sysclk_enable_module(POWER_RED_REG0, PRSPI_bm);

	spi_enable_master_mode(spi);
}

/**
 * \brief Setup a SPI device.
 *
 * The returned device descriptor structure must be passed to the driver
 * whenever that device should be used as current slave device.
 *
 * \param spi Base address of the SPI instance.
 * \param device    Pointer to SPI device struct that should be initialized.
 * \param flags     SPI configuration flags. Common flags for all
 *                  implementations are the SPI modes SPI_MODE_0 ...
 *                  SPI_MODE_3.
 * \param baud_rate Divider for Baud rate setting.
 * \param sel_id    Board specific select id
 */
void spi_master_setup_device(volatile void *spi, struct spi_device *device,
		spi_flags_t flags, uint32_t baud_rate,
		board_spi_select_id_t sel_id)
{
	/* Set SPI Baud rate */
	if (spi_set_baud_div(spi, baud_rate, sysclk_get_cpu_hz()) < 0) {
		Assert(false);
		return;
	}

	/* Clear any set SPI mode flags and set them to the user-specified mode
	 **/
	SPCR = (SPCR & ~SPI_MODE_gm) |
			((flags << SPI_MODE_gp) & SPI_MODE_gm);
}

/**
 * \brief Send a sequence of bytes to a SPI device
 *
 * Received bytes on the SPI bus are discarded.
 *
 * \param spi Base address of the SPI instance.
 * \param data   data buffer to write
 * \param len    Length of data
 *
 * \pre SPI device must be selected with spi_select_device() first
 */
status_code_t spi_write_packet(volatile void *spi, const uint8_t *data,
		size_t len)
{
	while (len) {
		spi_write_single(spi, *data++);

		while (!spi_is_tx_empty(spi)) {
		}

		len--;
	}

	return STATUS_OK;
}

/**
 * \brief Receive a sequence of bytes from a SPI device
 *
 * All bytes sent out on SPI bus are sent as value 0.
 *
 * \param spi Base address of the SPI instance.
 * \param data   data buffer to read
 * \param len    Length of data
 *
 * \pre SPI device must be selected with spi_select_device() first
 */
status_code_t spi_read_packet(volatile void *spi, uint8_t *data, size_t len)
{
	while (len) {
		spi_write_single(spi, CONFIG_SPI_MASTER_DUMMY); /* Dummy write */

		while (!spi_is_rx_full(spi)) {
		}

		*data = SPDR;

		data++;
		len--;
	}

	return STATUS_OK;
}

/**
 * \brief Select given device on the SPI bus
 *
 * Set device specific setting and calls board chip select.
 *
 * \param spi Base address of the SPI instance.
 * \param device SPI device
 *
 */
void spi_select_device(volatile void *spi, struct spi_device *device)
{
	ioport_set_pin_low(device->id);
}

/**
 * \brief Deselect given device on the SPI bus
 *
 * Calls board chip deselect.
 *
 * \param spi Base address of the SPI instance.
 * \param device SPI device
 */
void spi_deselect_device(volatile void *spi, struct spi_device *device)
{
	ioport_set_pin_high(device->id);
}
