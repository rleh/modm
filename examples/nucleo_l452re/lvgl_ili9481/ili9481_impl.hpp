/*
 * Copyright (c) 2021, Raphael Lehmann
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MODM_ILI9481_HPP
#	error	"Don't include this file directly, use 'ili9481.hpp' instead!"
#endif

namespace modm
{

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::initialize()
{
	// See https://d1.amobbs.com/bbs_upload782111/files_50/ourdev_711030GFAB18.pdf
	constexpr uint8_t powerSetting[] { 0x07, 0x41, 0x1D };
	constexpr uint8_t vcomControl[] { 0x00, 0x1c, 0x1f };
	constexpr uint8_t powerSettingNormalMode[] { 0x01, 0x11 };
	constexpr uint8_t panelDrivingSetting[] { 0x10 /* 0x00? */, 0x3b, 0x00, 0x02, 0x11 };
	constexpr uint8_t frameRateAndInversionControl[] { 0x03 }; // frame rate 72Hz (default)
	constexpr uint8_t interfaceCtrl[] { 0x83 };
	constexpr uint8_t gammaSetting[] { 0x00, 0x36, 0x21, 0x00, 0x00, 0x1f, 0x65, 0x23, 0x77, 0x00, 0x0f, 0x00 };
	constexpr uint8_t unknowCommandB0[] = { 0x00 };
	constexpr uint8_t unknowCommandE4[] = { 0xa0 };
	constexpr uint8_t unknowCommandF0[] = { 0x01 };
	constexpr uint8_t unknowCommandF3[] = { 0x40, 0xa4 };
	constexpr uint8_t setAddressMode[] { 0x0a };
	constexpr uint8_t setPixelFormat[] { 0x66 }; // 16bit/pixel
	//constexpr uint8_t columnAddressSet[] { 0x00, 0x00, 0x01, 0x3f };
	//constexpr uint8_t pageAddressSet[] { 0x00, 0x00, 0x01, 0xdf };

	reset();

	{
		BatchHandle h(*this);

		this->writeCommand(Command::SwReset);
		//this->writeCommand(Command::DisplayOff);

		this->writeCommand(Command::LeaveSleep);
		modm::delay_ms(20);
		this->writeCommand(Command::PowerSetting, powerSetting, sizeof(powerSetting));
		this->writeCommand(Command::VcomControl, vcomControl, sizeof(vcomControl));
		this->writeCommand(Command::PowerSettingNormalMode, powerSettingNormalMode, sizeof(powerSettingNormalMode));
		this->writeCommand(Command::PanelDrivingSetting, panelDrivingSetting, sizeof(panelDrivingSetting));
		this->writeCommand(Command::FrameRateAndInversionControl, frameRateAndInversionControl, sizeof(frameRateAndInversionControl));
		//this->writeCommand(Command::InterfaceCtrl, interfaceCtrl, sizeof(interfaceCtrl));
		this->writeCommand(Command::GammaSetting, gammaSetting, sizeof(gammaSetting));

		// unknown commands from application note
		this->writeCommand(Command::UnknowCommandB0, unknowCommandB0, sizeof(unknowCommandB0));
		this->writeCommand(Command::UnknowCommandE4, unknowCommandE4, sizeof(unknowCommandE4));
		this->writeCommand(Command::UnknowCommandF0, unknowCommandF0, sizeof(unknowCommandF0));
		this->writeCommand(Command::UnknowCommandF3, unknowCommandF3, sizeof(unknowCommandF3)); // maybe?

		this->writeCommand(Command::SetAddressMode, setAddressMode, sizeof(setAddressMode));
		this->writeCommand(Command::SetPixelFormat, setPixelFormat, sizeof(setPixelFormat));
		modm::delay_ms(120);
		this->writeCommand(Command::DisplayOn);
		modm::delay_ms(25);

		setOrientation(orientation);
	}
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::reset(bool hardReset /* = false */)
{
	if (hardReset)
	{
		Reset::set();
		modm::delay_ms(5);
		Reset::reset();
		modm::delay_ms(5);
		Reset::set();
		modm::delay_ms(5);
	}
	else {
		BatchHandle h(*this);
		this->writeCommand(Command::SwReset);
		modm::delay_ms(5);
	}
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
uint16_t
Ili9481<Interface, Reset, Backlight, BufferSize>::getIcModel()
{
	BatchHandle h(*this);

	//uint8_t buffer[4] { 0 };
	//this->readData(Command::ReadId4, buffer, 4);
	//return (buffer[2] << 8) | buffer[3];
	return 0x9481;
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
inline void
Ili9481<Interface, Reset, Backlight, BufferSize>::setOrientation(glcd::Orientation orientation)
{
	using MemoryAccessCtrl_t = ili9481::MemoryAccessCtrl_t;
	using MemoryAccessCtrl = ili9481::MemoryAccessCtrl;
	MemoryAccessCtrl_t madCtrl { MemoryAccessCtrl::BGR };

	switch (orientation)
	{
		case glcd::Orientation::Portrait90:
			madCtrl |= MemoryAccessCtrl::MV | MemoryAccessCtrl::MX;
			break;
		case glcd::Orientation::Landscape180:
			madCtrl |= MemoryAccessCtrl::MX | MemoryAccessCtrl::MY;
			break;
		case glcd::Orientation::Portrait270:
			madCtrl |= MemoryAccessCtrl::MV | MemoryAccessCtrl::MY;
			break;
		default:
//			madCtrl |= MemoryAccessCtrl::ML;
			break;
	}

	this->orientation = orientation;

	BatchHandle h(*this);
	this->writeCommandValue8(Command::MemoryAccessCtrl, madCtrl.value);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::turnOn()
{
	BatchHandle h(*this);
	this->writeCommand(Command::DisplayOn);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::turnOff()
{
	BatchHandle h(*this);
	this->writeCommand(Command::DisplayOff);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::setIdle(bool enable)
{
	BatchHandle h(*this);
	this->writeCommand(enable ? Command::IdleModeOn : Command::IdleModeOff);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::enableSleep(bool enable)
{
	BatchHandle h(*this);
	this->writeCommand(enable ? Command::EnterSleep : Command::LeaveSleep);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::setBrightness(uint8_t level)
{
	//BatchHandle h(*this);
	//this->writeCommand(Command::WriteBrightness, &level, 1);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::setInvert(bool invert)
{
	BatchHandle h(*this);
	this->writeCommand(invert ? Command::InversionOn : Command::InversionOff);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::clear()
{
	auto const saveForegroundColor { foregroundColor };
	foregroundColor = backgroundColor;
	fillRectangle(glcd::Point(0, 0), Width, Height);
	foregroundColor = saveForegroundColor;
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::drawHorizontalLine(
		glcd::Point start, uint16_t length)
{
	uint16_t const pixelValue { modm::toBigEndian(foregroundColor.color) };
	auto minLength { std::min(std::size_t(length), BufferSize) };
	uint16_t *buffer16 { reinterpret_cast<uint16_t *>(buffer) };
	std::fill(buffer16, buffer16+minLength, pixelValue);

	BatchHandle h(*this);

	setClipping(start.getX(), start.getY(), length, 1);
	while (length > BufferSize)
	{
		this->writeData(buffer, BufferSize * 2);
		length -= BufferSize;
	}
	this->writeData(buffer, length * 2);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::drawVerticalLine(
		glcd::Point start, uint16_t length)
{
	uint16_t const pixelValue { modm::toBigEndian(foregroundColor.color) };
	auto minLength { std::min(std::size_t(length), BufferSize) };
	uint16_t *buffer16 { reinterpret_cast<uint16_t *>(buffer) };
	std::fill(buffer16, buffer16+minLength, pixelValue);

	BatchHandle h(*this);

	setClipping(start.getX(), start.getY(), 1, length);
	while (length > BufferSize)
	{
		this->writeData(buffer, BufferSize * 2);
		length -= BufferSize;
	}
	this->writeData(buffer, length * 2);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::fillRectangle(
		glcd::Point upperLeft, uint16_t width, uint16_t height)
{
	auto const x { upperLeft.getX() };
	auto const y { upperLeft.getY() };
	std::size_t pixelCount { std::size_t(width) * std::size_t(height) };

	uint16_t const pixelValue { modm::toBigEndian(foregroundColor.color) };
	auto minLength { std::min(std::size_t(pixelCount), BufferSize) };
	uint16_t *buffer16 { reinterpret_cast<uint16_t *>(buffer) };
	std::fill(buffer16, buffer16+minLength, pixelValue);

	BatchHandle h(*this);

	setClipping(x, y, width, height);
	while (pixelCount > BufferSize)
	{
		this->writeData(buffer, BufferSize * 2);
		pixelCount -= BufferSize;
	}
	if (pixelCount)
		this->writeData(buffer, pixelCount * 2);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::fillCircle(
		glcd::Point center, uint16_t radius)
{
	uint8_t const setColor[] { uint8_t((foregroundColor.color >> 8) & 0xff),
			uint8_t(foregroundColor.color & 0xff) };

	int16_t f = 1 - radius;
	int16_t ddF_x = 0;
	int16_t ddF_y = -2 * radius;
	uint16_t x = 0;
	uint16_t y = radius;

	BatchHandle h(*this);

	setClipping(center.getX() - radius, center.getY(), 2 * radius, 1);
	for (std::size_t i = 0; i < 2 * radius; ++i)
		this->writeData(setColor, 2);

	while(x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		setClipping(center.getX() - x, center.getY() - y, 2 * x, 1);
		for (std::size_t i = 0; i < 2 * x; ++i)
			this->writeData(setColor, 2);
		setClipping(center.getX() - y, center.getY() - x, 2 * y, 1);
		for (std::size_t i = 0; i < 2 * y; ++i)
			this->writeData(setColor, 2);
		setClipping(center.getX() - x, center.getY() + y, 2 * x, 1);
		for (std::size_t i = 0; i < 2 * x; ++i)
			this->writeData(setColor, 2);
		setClipping(center.getX() - y, center.getY() + x, 2 * y, 1);
		for (std::size_t i = 0; i < 2 * y; ++i)
			this->writeData(setColor, 2);
	}
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::drawImageRaw(glcd::Point upperLeft,
		uint16_t width, uint16_t height, modm::accessor::Flash<uint8_t> data)
{
	uint8_t const setColor[] { uint8_t((foregroundColor.color >> 8) & 0xff),
			uint8_t(foregroundColor.color & 0xff) };
	uint8_t const clearColor[] { uint8_t((backgroundColor.color >> 8) & 0xff),
			uint8_t(backgroundColor.color & 0xff) };

	BatchHandle h(*this);

	setClipping(upperLeft.getX(), upperLeft.getY(), width, height);

	uint8_t bit = 0x01;
	for (uint16_t r = 0; r < height; ++r)
	{
		for (uint16_t w = 0; w < width; ++w)
		{
			uint8_t byte = data[(r / 8) * width + w];
			if (byte & bit)
				this->writeData(setColor, 2);
			else
				this->writeData(clearColor, 2);
		}
		// TODO: optimize, use ROL (rotate left)
		bit <<= 1;
		if (bit == 0)
			bit = 0x01;
	}
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::drawRaw(glcd::Point upperLeft,
		uint16_t width, uint16_t height, uint8_t* data)
{
	BatchHandle h(*this);

	setClipping(upperLeft.getX(), upperLeft.getY(), width, height);
	this->writeData(data, width * height * 2);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::setScrollArea(
		uint16_t topFixedRows, uint16_t bottomFixedRows, uint16_t firstRow)
{
	BatchHandle h(*this);

	uint8_t arg[]
	{
		uint8_t((topFixedRows >> 8) & 0xff), uint8_t(topFixedRows & 0xff),
		uint8_t((firstRow >> 8) & 0xff), uint8_t(firstRow & 0xff),
		uint8_t((bottomFixedRows >> 8) & 0xff), uint8_t(bottomFixedRows & 0xff)
	};
	this->writeCommand(Command::VerticalScrollDefinition, arg, 6);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::scrollTo(uint16_t row)
{
	BatchHandle h(*this);

	uint8_t arg[] { uint8_t((row >> 8) & 0xff), uint8_t(row & 0xff) };
	this->writeCommand(Command::VerticalScrollStartAddr, arg, 2);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::setColoredPixel(
		int16_t x, int16_t y, color::Rgb565 const &color)
{
	auto const pixelColor { color };
	uint8_t const setColor[] { uint8_t((pixelColor.color >> 8) & 0xff), uint8_t(pixelColor.color & 0xff) };

	BatchHandle h(*this);

	this->setClipping(x, y, 1, 1);
	this->writeData(setColor, 2);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::setClipping(
		uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	uint8_t buffer[4];

	buffer[0] = uint8_t((x >> 8) & 0xff);
	buffer[1] = uint8_t(x & 0xff);
	buffer[2] = uint8_t(((x + width - 1) >> 8) & 0xff);
	buffer[3] = uint8_t((x + width - 1) & 0xff);
	this->writeCommand(Command::ColumnAddressSet, buffer, 4);

	buffer[0] = uint8_t((y >> 8) & 0xff);
	buffer[1] = uint8_t(y & 0xff);
	buffer[2] = uint8_t(((y + height - 1) >> 8) & 0xff);
	buffer[3] = uint8_t((y + height - 1) & 0xff);
	this->writeCommand(Command::PageAddressSet, buffer, 4);
	this->writeCommand(Command::MemoryWrite);
}

template <class Interface, class Reset, class Backlight, std::size_t BufferSize>
void
Ili9481<Interface, Reset, Backlight, BufferSize>::drawBitmap(glcd::Point upperLeft,
		uint16_t width, uint16_t height, modm::accessor::Flash<uint8_t> data)
{
	BatchHandle h(*this);

	setClipping(upperLeft.getX(), upperLeft.getY(), width, height);
	for (int i = 0; i < width * height; ++i) {
		buffer[0] = data[i*2+1];
		buffer[1] = data[i*2];
		this->writeData(buffer, 2);
	}
//	this->writeData(data.getPointer(), width * height * 2);
}

} // namespace modm
