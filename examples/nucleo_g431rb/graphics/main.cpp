/*
 * Copyright (c) 2020, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/driver/display/ili9341_spi.hpp>
using namespace Board;

namespace display
{
	using Spi = SpiMaster1;
	using Cs = modm::platform::GpioA3;
	using Sck = modm::platform::GpioA5;
	using Miso = modm::platform::GpioA6;
	using Mosi = modm::platform::GpioA7;
	using DataCommands = modm::platform::GpioA10;
	using Reset = modm::platform::GpioA2;
	using Backlight = modm::platform::GpioB3;
}

modm::Ili9341Spi<
	display::Spi,
	display::Cs,
	display::DataCommands,
	display::Reset,
	display::Backlight
> tft;

/*
 * Blinks the green user LED with 1 Hz.
 * It is on for 90% of the time and off for 10% of the time.
 */

int
main()
{
	using namespace modm::literals;
	Board::initialize();
	display::Spi::connect<
		display::Sck::Sck, display::Miso::Miso, display::Mosi::Mosi>();
	display::Spi::initialize<SystemClock, 1328_kHz>();
	tft.initialize();
	LedD13::set();
	tft.setColor(modm::glcd::Color::black());
	int16_t w = tft.getWidth();
	int16_t h = tft.getHeight();
	tft.drawLine({0,0}, {w, h});
	tft.drawLine({w,0}, {0, h});

	while (true)
	{
		LedD13::set();
		modm::delay(900ms);

		LedD13::reset();
		modm::delay(100ms);
	}

	return 0;
}
