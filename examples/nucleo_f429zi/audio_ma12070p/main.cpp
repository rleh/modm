/*
 * Copyright (c) 2022, Raphael Lehmann
 * Copyright (c) 2021, Christopher Durand
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <arm_math.h>
#include <modm/board.hpp>
#include <modm/processing.hpp>
#include <modm/processing/protothread.hpp>
#include <modm/driver/dac/ma12070p.hpp>
#include <modm/platform/gpio/inverted.hpp>
#include <modm/platform/i2c/i2c_master_1.hpp>
#include <modm/platform/i2s/i2s_master_2.hpp>
#include <numbers>
#include <optional>

using namespace Board;
using namespace modm::literals;

using Scl = GpioB8; // External pull-up necessary
using Sda = GpioB9; // External pull-up necessary
using MyI2cMaster = modm::platform::I2cMaster1;
using Ma12070p = modm::Ma12070p<MyI2cMaster>;

using I2sWs = GpioOutputB12;
using I2sMck = GpioOutputC6;
using I2sCk = GpioOutputB13;
using I2sData = GpioOutputB15;
using DmaTx = Dma1::Channel4;
using I2sMaster = modm::platform::I2sMaster2<DmaTx>;

using NotEnable	= GpioD15;
using Mute		= GpioInverted<GpioF12>;
using PowerEn	= GpioD14;

struct I2sSystemClock
{
	static constexpr uint32_t I2sPll = 172_MHz;

	static void
	enable()
	{
		const Rcc::PllI2sFactors pllI2sFactors{
			.pllN = 172,	// 2 MHz * N=172 -> 344 MHz
			.pllR = 2		// 344 MHz / R=2 -> 172 MHz (max 192 MHz)
		};
		Rcc::enablePllI2s(pllI2sFactors);
	}
};

template<typename T, std::size_t length>
constexpr auto computeSinTable(uint8_t cycles=1)
{
	cycles=3;
	std::array<T, length> data{};
	constexpr auto HalfOutput = std::numeric_limits<T>::max() / 2; // 16 bit full scale
	for (size_t i = 0; i < data.size(); i+=2) {
		constexpr auto pi = std::numbers::pi_v<float>;
		T value = float(HalfOutput) * (1 + arm_sin_f32(i * (2*pi / data.size() * cycles))) * 0.5;
		data[i] = value;
		data[i+1] = value;
	}
	return data;
}

constexpr std::size_t bufferSize = 960;
auto bufferA = computeSinTable<uint16_t, bufferSize>(1);
auto bufferB = computeSinTable<uint16_t, bufferSize>(2);
volatile bool bufferA_ready{true};
volatile bool bufferB_ready{true};

void
transferCompleteIrqHandler()
{
	LedGreen::reset();

	if (bufferA_ready) {
		I2sMaster::setTxBuffer(uintptr_t(bufferA.data()), bufferSize);
		bufferA_ready = false;
	}
	else if (bufferB_ready) {
		I2sMaster::setTxBuffer(uintptr_t(bufferB.data()), bufferSize);
		bufferB_ready = false;
	}
	else {
		LedRed::toggle();
		//MODM_LOG_ERROR << "No buffer ready for DMA :(" << modm::endl;
	}
	I2sMaster::startDma();

	LedGreen::set();
}

class Ma12070pThread : public modm::pt::Protothread
{
public:
	bool
	run()
	{
		PT_BEGIN();

		MODM_LOG_INFO << "Configuring I2S and VLP settings of MA12070P..." << modm::endl;
		while (!PT_CALL(ma12070p.configureI2sAndVlp(config))) {
			MODM_LOG_ERROR << "Unable to configure I2S and VLP settings of MA12070P" << modm::endl;
			timeout.restart();
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		MODM_LOG_INFO << "Enabling both amplifiers...\n";
		while (!PT_CALL(ma12070p.disableAmplifier(false, false))) {
			MODM_LOG_ERROR << "Unable to enable amplifiers of MA12070P" << modm::endl;
			timeout.restart();
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		MODM_LOG_INFO << "Setting MA12070P limiter thresholds to 0db...\n";
		while (!PT_CALL(ma12070p.setLimiterThreshold(0_q_db, 0_q_db))) {
			MODM_LOG_ERROR << "Unable to set channel volumes of MA12070P" << modm::endl;
			timeout.restart();
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		MODM_LOG_INFO << "Setting MA12070P channel volumes to 0db...\n";
		while (!PT_CALL(ma12070p.setChannelVolume(0_q_db, 0_q_db))) {
			MODM_LOG_ERROR << "Unable to set channel volumes of MA12070P" << modm::endl;
			timeout.restart();
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		MODM_LOG_INFO.printf("Setting MA12070P volume to %2.1fdb...\n", modm::ma12070p::quarterDecibelToFloat(volume));
		while (!PT_CALL(ma12070p.setMasterVolume(volume))) {
			MODM_LOG_ERROR << "Unable to set master volume of MA12070P" << modm::endl;
			timeout.restart();
			PT_WAIT_UNTIL(timeout.isExpired());
		}

		MODM_LOG_INFO << "Unmute..." << modm::endl;
		Mute::reset();

		while (true) {
			if (Board::Button::read()) {
				volume += 3_q_db;
				if (volume > modm::ma12070p::MaxVolume) {
					volume = modm::ma12070p::MinVolume;
				}
				MODM_LOG_INFO.printf("Volume: %2.1fdb\n", modm::ma12070p::quarterDecibelToFloat(volume));
				if (!PT_CALL(ma12070p.setMasterVolume(volume))) {
					MODM_LOG_ERROR << "Unable to set master volume of MA12070P" << modm::endl;
				}
				timeout.restart();
				PT_WAIT_UNTIL(timeout.isExpired());
			}

			if (timer.execute()) {
				vlpMonitor = PT_CALL(ma12070p.readVlpMonitor());
				if(!vlpMonitor) {
					MODM_LOG_ERROR << "Unable to read VLP monitor register" << modm::endl;
				}
				else {
					MODM_LOG_ERROR << *vlpMonitor << modm::endl;
				}
				errorRegister = PT_CALL(ma12070p.readAccumulatedErrors());
				if(!errorRegister) {
					MODM_LOG_ERROR << "Unable to read accumulated error register" << modm::endl;
				}
				else {
					if ((*errorRegister).value > 0) {
						MODM_LOG_ERROR << *errorRegister << modm::endl;
						MODM_LOG_ERROR << "Clearing errors..." << modm::endl;
						if(!PT_CALL(ma12070p.clearErrorHandler())) {
							MODM_LOG_ERROR << "Unable to clear errors" << modm::endl;
						}
					}
					else {
						MODM_LOG_ERROR << "No errors!" << modm::endl;
					}
				}
			}

			PT_YIELD();
		}
		PT_END();
	}

private:
	static constexpr modm::ma12070p::I2sAndVlpConfig config = {
		.pcmWordFormat		= modm::ma12070p::PcmWordFormat::RightJustifed16b,
		.clockPolarity		= modm::ma12070p::ClockPolarity::RisingEdge,
		.frameSize			= modm::ma12070p::FrameSize::Bits32,
		.wordSelectPolarity	= modm::ma12070p::WordSelectPolarity::Low,
		.rightLeftOrder		= modm::ma12070p::RightLeftOrder::RightFirst,
		.useVlp				= true,
		.useLimiter			= true,
	};
	static constexpr uint8_t ma12070pAddressI2c = 0x20;
	Ma12070p ma12070p{ma12070pAddressI2c};
	std::optional<modm::ma12070p::VlpMonitor_t> vlpMonitor;
	std::optional<modm::ma12070p::ErrorRegister_t> errorRegister;
	modm::ma12070p::quarter_decibel_t volume = modm::ma12070p::quarter_decibel(-30);
	modm::ShortTimeout timeout{250ms};
	modm::ShortPeriodicTimer timer{1s};
};

int
main()
{
	Board::initialize();
	I2sSystemClock::enable();
	Dma1::enable();
	Dma2::enable();

	Mute::set();
	Mute::setOutput();
	NotEnable::set();
	NotEnable::setOutput();
	PowerEn::reset();
	PowerEn::setOutput();

	modm::delay(1ms);

	MODM_LOG_INFO << "Audio MA12070P demo on Nucleo-F429ZI" << modm::endl;

	I2sMaster::connect<I2sMck::Mck, I2sCk::Ck, I2sWs::Ws, I2sData::Sd>();
	constexpr I2sMaster::I2sConfig config{
		.samplerate = 48_kHz,
		.tolerance = 1_pct,
		.bitDepth = I2sMaster::BitDepth::SixteenWithChannel16,
		.masterClockOutput = I2sMaster::MasterClockOutput::Enabled,
		.i2sStandard = I2sMaster::I2sStandard::LsbJustified,
	};
	I2sMaster::initialize<I2sSystemClock, config>();
	I2sMaster::setTransferCompleteIrqHandler(transferCompleteIrqHandler);
	I2sMaster::setTxBuffer(uintptr_t(bufferA.data()), bufferSize);
	I2sMaster::start();

	MyI2cMaster::connect<Scl::Scl, Sda::Sda>();
	MyI2cMaster::initialize<SystemClock, 50_kHz>();

	PowerEn::set();
	modm::delay(10ms);

	NotEnable::reset();
	modm::delay(2ms);

	Ma12070pThread ma12070pThread{};

	modm::PeriodicTimer tmr{1000ms};
	uint8_t counter{3};

	while (true)
	{
		if (!bufferA_ready) {
			//bufferA = computeSinTable<uint16_t, bufferSize>(counter++);
			bufferA_ready = true;
		}
		if (!bufferB_ready) {
			//bufferB = computeSinTable<uint16_t, bufferSize>(counter++);
			bufferB_ready = true;
		}
		if (counter > 100) {
			counter = 3;
		}

		ma12070pThread.run();

		if (I2sMaster::hasDmaError()) {
			MODM_LOG_ERROR << "I2S DMA Error :(" << modm::endl;
		}

		if (tmr.execute()) {
			LedBlue::toggle();
			Mute::toggle();
		}
	}

	return 0;
}
