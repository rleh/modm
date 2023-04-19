/*
 * Copyright (c) 2023, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include "mcan_test.hpp"

#include <algorithm>
#include <modm/platform.hpp>
#include <modm/board.hpp>

using namespace modm::platform;

void
FdcanTest::setUp()
{
	Mcan0::initialize<Board::SystemClock, 125_kbps, 1_pct, 1_Mbps>(12, Mcan0::Mode::TestInternalLoopback);

	// receive all standard & extended messages into Fifo0
	Mcan0::setExtendedFilter(0, Mcan0::FilterConfig::Fifo0,
			modm::can::ExtendedIdentifier(0),
			modm::can::ExtendedMask(0));
	Mcan0::setStandardFilter(0, Mcan0::FilterConfig::Fifo0,
			modm::can::StandardIdentifier(0),
			modm::can::StandardMask(0));
}

void
FdcanTest::testSendReceive()
{
	constexpr size_t length = 8;

	std::array<uint8_t, length> data;
	std::generate(data.begin(), data.end(), [n=0]() mutable { return n++; });

	modm::can::Message extendedMessage{0x4242u, length};
	std::copy(std::begin(data), std::begin(data) + length, extendedMessage.data);

	TEST_ASSERT_FALSE(Mcan0::isMessageAvailable());
	TEST_ASSERT_TRUE(Mcan0::sendMessage(extendedMessage));

	modm::delay_ms(10);

	TEST_ASSERT_TRUE(Mcan0::isMessageAvailable());

	modm::can::Message receivedMessage;
	TEST_ASSERT_TRUE(Mcan0::getMessage(receivedMessage));
	TEST_ASSERT_EQUALS(receivedMessage.getIdentifier(), 0x4242u);
	TEST_ASSERT_EQUALS(receivedMessage.getLength(), length);
	TEST_ASSERT_TRUE(receivedMessage.isExtended());
	TEST_ASSERT_FALSE(receivedMessage.isRemoteTransmitRequest());
	TEST_ASSERT_FALSE(receivedMessage.isFlexibleData());
	TEST_ASSERT_FALSE(receivedMessage.isBitRateSwitching());
	TEST_ASSERT_TRUE(std::equal(std::begin(data), std::begin(data) + length, receivedMessage.data));

	// Same procedure with standard identifier message
	modm::can::Message standardMessage{0x777u, length};
	std::generate(data.begin(), data.end(), [n=length]() mutable { return n--; });
	std::copy(std::begin(data), std::begin(data) + length, standardMessage.data);
	standardMessage.setExtended(false);

	TEST_ASSERT_FALSE(Mcan0::isMessageAvailable());
	TEST_ASSERT_TRUE(Mcan0::sendMessage(standardMessage));
	modm::delay_ms(1);
	TEST_ASSERT_TRUE(Mcan0::isMessageAvailable());

	TEST_ASSERT_TRUE(Mcan0::getMessage(receivedMessage));
	TEST_ASSERT_EQUALS(receivedMessage.getIdentifier(), 0x777u);
	TEST_ASSERT_EQUALS(receivedMessage.getLength(), length);
	TEST_ASSERT_FALSE(receivedMessage.isExtended());
	TEST_ASSERT_FALSE(receivedMessage.isRemoteTransmitRequest());
	TEST_ASSERT_FALSE(receivedMessage.isFlexibleData());
	TEST_ASSERT_FALSE(receivedMessage.isBitRateSwitching());
	TEST_ASSERT_TRUE(std::equal(std::begin(data), std::begin(data) + length, receivedMessage.data));
}

void
FdcanTest::testFilters()
{
	Mcan0::setStandardFilter(0, Mcan0::FilterConfig::Fifo0,
			modm::can::StandardIdentifier(0x108),
			modm::can::StandardMask(0x1F8));

	modm::can::Message message{0x188, 0};
	message.setExtended(false);
	Mcan0::sendMessage(message);
	modm::delay_ms(1);
	TEST_ASSERT_FALSE(Mcan0::isMessageAvailable());

	message.setIdentifier(0x10F);
	Mcan0::sendMessage(message);
	modm::delay_ms(1);
	TEST_ASSERT_TRUE(Mcan0::isMessageAvailable());
	modm::can::Message receivedMessage;
	TEST_ASSERT_TRUE(Mcan0::getMessage(receivedMessage));
	TEST_ASSERT_FALSE(receivedMessage.isExtended());
	TEST_ASSERT_EQUALS(receivedMessage.getIdentifier(), 0x10Fu);
}

void
FdcanTest::testBuffers()
{
	modm::can::Message message{0, 8};
	// send 30 messages, default message buffer holds 32 elements
	for (uint_fast8_t i = 0; i <= 30; ++i) {
		message.setIdentifier(i);
		for (uint_fast8_t dataIndex = 0; dataIndex < i; ++dataIndex) {
			message.data[dataIndex] = i;
		}
		Mcan0::sendMessage(message);
	}

	modm::delay_ms(10);

	// try to receive same messages
	modm::can::Message receivedMessage;
	for (uint_fast8_t i = 0; i <= 30; ++i) {
		TEST_ASSERT_TRUE(Mcan0::getMessage(receivedMessage));

		TEST_ASSERT_EQUALS(receivedMessage.getIdentifier(), i);
		TEST_ASSERT_EQUALS(receivedMessage.getLength(), 8);

		for (uint_fast8_t dataIndex = 0; dataIndex < i; ++dataIndex) {
			TEST_ASSERT_EQUALS(receivedMessage.data[dataIndex], i);
		}
	}
	TEST_ASSERT_FALSE(Mcan0::isMessageAvailable());
	TEST_ASSERT_FALSE(Mcan0::getMessage(message));
}
