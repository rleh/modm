/*
 * Copyright (c) 2014, Sascha Schade
 * Copyright (c) 2014-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>

#include <modm/processing/timer.hpp>
#include <modm/processing/protothread.hpp>

using Led = Board::LedBlue;

class BlinkingLight : public modm::pt::Protothread
{
public:
	bool
	run()
	{
		PT_BEGIN();

		// set everything up
		Led::setOutput();
		Led::set();

		while (true)
		{
			timeout.restart(100ms);
			Led::set();
			PT_WAIT_UNTIL(timeout.isExpired());

			timeout.restart(200ms);
			Led::reset();
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		PT_END();
	}

private:
	modm::ShortTimeout timeout;
};

BlinkingLight light;


int
main()
{
	Board::initialize();

	while (true)
	{
		light.run();
	}

	return 0;
}
