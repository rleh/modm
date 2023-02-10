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
		for (Message tm : testMessages)
		{
			Mcan1::sendMessage(tm);
			modm::delay(delay);
		}
		PT_END();
	}

private:
	uint8_t messageLength = 8;
	std::array<Message, 13> testMessages = {
		Message{0x10, messageLength, 0x0000000000000010, false},
		Message{0x11, messageLength, 0x1111111111110011, false},
		Message{0x12, messageLength, 0x2222222222220012, false},
		Message{0x13, messageLength, 0x3333333333330013, false},
		Message{0x14, messageLength, 0x4444444444440014, false},
		Message{0x15, messageLength, 0x5555555555550015, false},
		Message{0x16, messageLength, 0x6666666666660016, false},
		Message{0x15, messageLength, 0x7777777777770015, false},
		Message{0x14, messageLength, 0x8888888888880014, false},
		Message{0x13, messageLength, 0x9999999999990013, false},
		Message{0x12, messageLength, 0xaaaaaaaaaaaa0012, false},
		Message{0x11, messageLength, 0xbbbbbbbbbbbb0011, false},
		Message{0x10, messageLength, 0xcccccccccccc0010, false},
	};

	std::chrono::microseconds delay = std::chrono::microseconds{1};
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

	MODM_LOG_INFO << "Mcan1: Initializing with 125kbps for boards CAN transceiver (PC12/PC14)." << modm::endl;
	// Mcan1 is connted in Board::initialize(); CAN transceiver on the dev board
	Mcan1::initialize<Board::SystemClock, 125_kbps, 1_pct>(12);

	MODM_LOG_INFO << "Mcan1: Setting up Filter to receive every message." << modm::endl;
	Mcan1::setExtendedFilter(0, Mcan1::FilterConfig::Fifo0,
			modm::can::ExtendedIdentifier(0),
			modm::can::ExtendedMask(0));
	Mcan1::setStandardFilter(0, Mcan1::FilterConfig::Fifo0,
			modm::can::StandardIdentifier(0),
			modm::can::StandardMask(0));

	MODM_LOG_INFO << "Mcan0: Initializing with 125kbps for PB2/PB3." << modm::endl;
	Mcan0::connect<GpioB2::Tx, GpioB3::Rx>();
	Mcan0::initialize<Board::SystemClock, 125_kbps, 1_pct>(12);

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
