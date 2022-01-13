/*
 * Copyright (c) 2010-2011, Fabian Greif
 * Copyright (c) 2012-2015, 2017, Niklas Hauser
 * Copyright (c) 2014, 2017, Sascha Schade
 * Copyright (c) 2022, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/driver/can/mcp2515.hpp>

using namespace Board;

// mcp2515.hpp disables LOG_LEVEL :unamused:
#undef	MODM_LOG_LEVEL
#define	MODM_LOG_LEVEL modm::log::DEBUG

using SpiMaster = SpiMaster1;
using Mosi = GpioB5;
using Miso = GpioB4;
using Sck = GpioB3;
using Cs = GpioA4;
using Int = GpioB8;

modm::Mcp2515<SpiMaster, Cs, Int> mcp2515;

// Default filters to receive any extended CAN frame
FLASH_STORAGE(uint8_t canFilter[]) =
{
	MCP2515_FILTER(0),	// Filter 0
	MCP2515_FILTER(0),	// Filter 1

	MCP2515_FILTER_EXTENDED(0),	// Filter 2
	MCP2515_FILTER_EXTENDED(0),	// Filter 3
	MCP2515_FILTER_EXTENDED(0),	// Filter 4
	MCP2515_FILTER_EXTENDED(0),	// Filter 5

	MCP2515_FILTER(0),	// Mask 0
	MCP2515_FILTER_EXTENDED(0),	// Mask 1
};

int
main()
{
	Board::initialize();
	Leds::setOutput();

	MODM_LOG_INFO << "CAN MCP2515 Example on Nucleo-F429ZI\n";

	// Initialize SPI interface and the other pins
	// needed by the MCP2515
	SpiMaster::connect<Sck::Sck, Mosi::Mosi, Miso::Miso>();
	SpiMaster::initialize<SystemClock, 1.3_MHz>();
	Cs::setOutput();
	Int::setInput(Gpio::InputType::PullUp);

	// Configure MCP2515 and set the filters
	mcp2515.initialize<8_MHz, 125_kbps>();
	mcp2515.setFilter(modm::accessor::asFlash(canFilter));

	uint8_t counter{0};

	while (true)
	{
		MODM_LOG_INFO << "Loop " << counter << "\n";
		if (mcp2515.isMessageAvailable())
		{
			modm::can::Message msg;
			if (mcp2515.getMessage(msg))
			{
				MODM_LOG_INFO << msg << modm::endl;
				LedGreen::toggle();
			}
		}

		modm::can::Message msg;
		msg.setExtended(false);
		msg.identifier = counter;
		mcp2515.sendMessage(msg);

		modm::delay(1s);
		counter++;
	}
}
