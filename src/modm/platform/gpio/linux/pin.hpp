/*
 * Copyright (c) 2021, Niklas Hauser
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
#include <filesystem>

#include "base.hpp"


namespace modm::platform
{

/// @ingroup modm_platform_gpio
template< int Pin >
class GpioPin : public Gpio, public modm::GpioIO
{
	//static inline bool output{false};
	static constexpr ::gpiod::line::offset pinOffset{Pin};

	static constexpr ::std::filesystem::path chip_path("/dev/gpiochip0"); // TODO: gpiochip path from template argument?

public:
	using Output = GpioPin<Pin>;
	using Input = GpioPin<Pin>;
	using IO = GpioPin<Pin>;
	using Type = GpioPin<Pin>;

public:
	inline static void setOutput() { setOutput(OutputType::PushPull); }
	inline static void setOutput(OutputType drive)
	{
		::gpiod::chip(chip_path)
			.prepare_request()
			.add_line_settings(pinOffset,
				::gpiod::line_settings()
				.set_direction(::gpiod::line::direction::OUTPUT)
				.set_drive(static_cast<::gpiod::line::drive>(drive)))
			.do_request();
	}
	inline static void setOutput(bool status)
	{
		setOutput();
		set(status);
	}

	inline static void set() { set(true); }
	inline static void reset() { set(false); }
	inline static bool isSet()
	{
		// TODO
		::gpiod::chip(chip_path)
			.prepare_request()
			.get_line_config()
			.get_line_settings()
	}

	inline static void set(bool status)
	{
		// TODO
	}

	inline static void toggle()
	{
		if (isSet()) { set(); }
		else { reset(); }
	}

	inline static void setInput()
	{
		// TODO
	}

	inline static void setInput(Gpio::InputType type)
	{
		auto request = ::gpiod::chip(chip_path)
						   .prepare_request()
						   .add_line_settings(pinOffset, ::gpiod::line_settings()
						   		.set_direction(::gpiod::line::direction::INPUT)
								.set_bias(static_cast<::gpiod::line::bias>(type))
							)
						   .do_request();
	}

	//inline static void configure(Gpio::InputType type) {}

	inline static bool read()
	{
		return ::gpiod::chip(chip_path).prepare_request().set_consumer().do_request().get_value(pinOffset) == ::gpiod::line::value::ACTIVE;
	}

	inline static modm::Gpio::Direction getDirection()
	{
		// TODO
	}

public:
	struct BitBang {}
};

}   // namespace modm::platform

