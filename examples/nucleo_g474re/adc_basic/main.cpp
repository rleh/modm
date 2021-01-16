/*
 * Copyright (c) 2020, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <modm/board.hpp>
#include <modm/debug/logger.hpp>
#include <modm/platform/adc/adc_1.hpp>
#include <array>

using namespace modm::literals;

#undef	MODM_LOG_LEVEL
#define	MODM_LOG_LEVEL modm::log::INFO

int
main()
{
	Board::initialize();

	MODM_LOG_INFO << "STM32G4 ADC basic example" << modm::endl;
	MODM_LOG_INFO << " running on Nucleo-G474RE" << modm::endl << modm::endl;

	MODM_LOG_INFO << "Configuring ADC ...";
	Adc1::initialize(
		Adc1::ClockMode::SynchronousPrescaler1,
		Adc1::ClockSource::SystemClock,
		Adc1::Prescaler::Disabled,
		Adc1::CalibrationMode::SingleEndedInputsMode,
		true);

	Adc1::connect<GpioA0::In1>();

	Adc1::setPinChannel<GpioA0>(Adc1::SampleTime::Cycles13);

	while (true)
	{
		Adc1::startConversion();
		// wait for conversion to finish
		while(!Adc1::isConversionFinished);
		// print result
		int adcValue = Adc1::getValue();
		MODM_LOG_INFO << "adcValue=" << adcValue;
		float voltage = adcValue * 3.3 / 0xfff;
		MODM_LOG_INFO << " voltage=";
		MODM_LOG_INFO.printf("%.3f\n", voltage);
		modm::delay(500ms);
	}

	return 0;
}
