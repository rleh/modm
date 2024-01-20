/*
 * Copyright (c) 2020, Erik Henriksson
 * Copyright (c) 2024, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#pragma once

#include <modm/architecture/interface/gpio.hpp>

#include <gpiod.hpp>

namespace modm::platform
{

/// @ingroup modm_platform_gpio
enum class
Peripheral
{
	BitBang,
	// ...
};

/// @ingroup	modm_platform_gpio
struct Gpio
{
	/// Each Input Pin can be configured in one of these states.
	enum class
	InputType : uint8_t
	{
		Floating = static_cast<uint8_t>(::gpiod::line::bias::DISABLED),
		PullUp = static_cast<uint8_t>(::gpiod::line::bias::PULL_UP),
		PullDown = static_cast<uint8_t>(::gpiod::line::bias::PULL_UP),
	};

	enum class
	OutputType : uint8_t
	{
		PushPull = static_cast<uint8_t>(::gpiod::line::drive::PUSH_PULL),
		OpenDrain = static_cast<uint8_t>(::gpiod::line::drive::OPEN_DRAIN),
	};

	enum class
	Signal
	{
		BitBang,
	};
};

}	// namespace modm::platform
