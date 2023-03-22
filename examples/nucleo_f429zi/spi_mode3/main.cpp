/*
 * Copyright (c) 2023, Henrik Hose
 * Copyright (c) 2023, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <array>
#include <modm/board.hpp>
#include <modm/processing.hpp>

using namespace Board;

using SpiMaster = modm::platform::SpiMaster2;
using SpiHal = modm::platform::SpiHal2;

using Mosi = GpioC3;
using Sck = GpioD3;
using Cs = GpioF2;
//using Miso = GpioB4;

int
main()
{
	Board::initialize();
	Cs::setOutput(modm::Gpio::High);

	SpiMaster::connect</*Miso::Miso,*/ Mosi::Mosi, Sck::Sck>();
	SpiMaster::initialize<Board::SystemClock, 328_kHz>();

	MODM_LOG_INFO << "Example: SPI Mode 3" << modm::endl;

	std::array<uint8_t, 2> buffer;

	Cs::reset();
	RF_CALL_BLOCKING(SpiMaster::transfer(0xaa));
	RF_CALL_BLOCKING(SpiMaster::transfer(nullptr, buffer.data(), 1));
	Cs::set();

	modm::delay(10us);
	SpiMaster::setDataMode(SpiMaster::DataMode::Mode3);
	modm::delay(10us);

	Cs::reset();
	RF_CALL_BLOCKING(SpiMaster::transfer(0xaa));
	RF_CALL_BLOCKING(SpiMaster::transfer(nullptr, buffer.data(), 1));
	Cs::set();

	while (true) { Board::Leds::toggle(); }

	return 0;
}
