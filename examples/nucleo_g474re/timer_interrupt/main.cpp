/*
 * Copyright (c) 2019, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <modm/board.hpp>
#include <modm/platform/timer/timer_1.hpp>

using namespace Board;

int
main()
{
	Board::initialize();
	LedD13::setOutput();

	// Use the logging streams to print some messages.
	// Change MODM_LOG_LEVEL above to enable or disable these messages
	MODM_LOG_DEBUG   << "debug"   << modm::endl;
	MODM_LOG_INFO    << "info"    << modm::endl;
	MODM_LOG_WARNING << "warning" << modm::endl;
	MODM_LOG_ERROR   << "error"   << modm::endl;

	uint32_t counter(0);

	using namespace modm::platform;

	Timer1::enable();
	Timer1::setMode(Timer1::Mode::UpCounter);

	Timer1::setPeriod<Board::SystemClock>(1'500'000);
	Timer1::enableInterruptVector(Timer1::Interrupt::Update, true, 10);
	Timer1::enableInterrupt(Timer1::Interrupt::Update);

	Timer1::applyAndReset();
	Timer1::start();

	while (true)
	{
		LedD13::toggle();
		modm::delay(Button::read() ? 100ms : 500ms);

		MODM_LOG_INFO << "loop: " << counter++ << modm::endl;
	}

	return 0;
}

MODM_ISR(TIM1_UP_TIM16)
{
	Timer1::acknowledgeInterruptFlags(Timer1::InterruptFlag::Update);
	MODM_LOG_DEBUG << "Timer1 update interrupt\n";
}
