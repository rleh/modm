/*
 * Copyright (c) 2023, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <chrono>
#include <modm/board.hpp>
#include <modm/debug/logger.hpp>
#include <modm/processing/protothread.hpp>

using namespace modm::literals;
using namespace modm::platform;
using namespace Board;

// Set the log level
#undef	MODM_LOG_LEVEL
#define	MODM_LOG_LEVEL modm::log::INFO

using Message = modm::can::Message;

class SendThread : public modm::pt::Protothread
{
public:
	bool
	update()
	{
		PT_BEGIN();
		for (i = 0; i < testMessages.size(); i++)
		{
			MODM_LOG_INFO << "Mcan1: Trying to send message (" << testMessages[i] << ") ...\n";
			while (not Mcan1::sendMessage(testMessages[i])) {
				MODM_LOG_INFO << "Mcan1: Unable to send message on Mcan1\n";
				PT_YIELD();
			}
			PT_YIELD();
			MODM_LOG_INFO << "Mcan1: ...success!\n";
			//modm::delay(delay);
			//PT_YIELD();
		}
		PT_END();
	}

private:
	static constexpr uint8_t messageLength = 64;
	const std::array<uint8_t, messageLength> messageData{0xaa};
	std::array<Message, 13> testMessages = {
		Message{0x10, messageLength, messageData.data(), false},
		Message{0x11, messageLength, messageData.data(), false},
		Message{0x12, messageLength, messageData.data(), false},
		Message{0x13, messageLength, messageData.data(), false},
		Message{0x14, messageLength, messageData.data(), false},
		Message{0x15, messageLength, messageData.data(), false},
		Message{0x16, messageLength, messageData.data(), false},
		Message{0x15, messageLength, messageData.data(), false},
		Message{0x14, messageLength, messageData.data(), false},
		Message{0x13, messageLength, messageData.data(), false},
		Message{0x12, messageLength, messageData.data(), false},
		Message{0x11, messageLength, messageData.data(), false},
		Message{0x10, messageLength, messageData.data(), false},
	};

	size_t i;

	std::chrono::nanoseconds delay = std::chrono::nanoseconds{1};
};

template<class Can, uint8_t id>
class ReceiveThread : public modm::pt::Protothread
{
public:
	bool
	update()
	{
		PT_BEGIN();
		while (true)
		{
			PT_WAIT_UNTIL(Can::isMessageAvailable());
			MODM_LOG_INFO << "Mcan" << id << ": Message is available... ";
			__DSB();
			if (Can::getMessage(rxMsg))
				MODM_LOG_INFO << rxMsg << modm::endl;
			else
				MODM_LOG_INFO << " but getting message FAILED" << modm::endl;
		}
		PT_END();
	}
private:
	modm::can::Message rxMsg;
};

SendThread sendThread;
ReceiveThread<Mcan0, 0> receiveThreadMcan0;

int
main()
{
	Board::initialize();

	MODM_LOG_INFO << "CAN Dual Test Program" << modm::endl;

	MODM_LOG_INFO << "Mcan1: Initializing with 125kbps / 1Mbps for boards CAN transceiver (PC12/PC14)." << modm::endl;
	// Mcan1 is connted in Board::initialize(); CAN transceiver on the dev board
	Mcan1::initialize<Board::SystemClock, 125_kbps, 1_pct, 1_Mbps>(12);

	Mcan1::setErrorInterruptCallback([](){
		Board::Led1::set();
	});

	MODM_LOG_INFO << "Mcan1: Setting up Filter to receive every message." << modm::endl;
	Mcan1::setExtendedFilter(0, Mcan1::FilterConfig::Fifo0,
			modm::can::ExtendedIdentifier(0),
			modm::can::ExtendedMask(0));
	Mcan1::setStandardFilter(0, Mcan1::FilterConfig::Fifo0,
			modm::can::StandardIdentifier(0),
			modm::can::StandardMask(0));

	MODM_LOG_INFO << "Mcan0: Initializing with 125kbps / 1Mbps for PB2/PB3." << modm::endl;
	Mcan0::connect<GpioB2::Tx, GpioB3::Rx>();
	Mcan0::initialize<Board::SystemClock, 125_kbps, 1_pct, 1_Mbps>(12);

	Mcan0::setErrorInterruptCallback([](){
		Board::Led0::set();
	});

	MODM_LOG_INFO << "Mcan0: Setting up Filter to receive every message." << modm::endl;
	Mcan0::setExtendedFilter(0, Mcan0::FilterConfig::Fifo0,
			modm::can::ExtendedIdentifier(0),
			modm::can::ExtendedMask(0));
	Mcan0::setStandardFilter(0, Mcan0::FilterConfig::Fifo0,
			modm::can::StandardIdentifier(0),
			modm::can::StandardMask(0));

	while (true)
	{
		receiveThreadMcan0.update();
		sendThread.update();
	}

	return 0;
}
