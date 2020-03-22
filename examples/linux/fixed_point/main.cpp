/*
 * Copyright (c) 2016-2017, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/platform.hpp>
#include <modm/math/fixed_point.hpp>
#include <modm/debug.hpp>

#include <typeinfo>


int
main()
{
	using namespace modm;

	MODM_LOG_INFO << "\n";

	fix8_t<2>   a = -10.5;
	MODM_LOG_INFO << "fix8_t<2> a = " << a << "\n";
	ufix8_t<3>  b = 10.5;
	MODM_LOG_INFO << "ufix8_t<3> b = " << b << "\n";

	MODM_LOG_INFO << "a*b = " << a << " * " << b << " = " << a * b << "\n";
	MODM_LOG_INFO << "b*a = " << b << " * " << a << " = " << b * a << "\n";

	fix16_t<7>  c = 10.5;
	MODM_LOG_INFO << "fix16_t<7> c = " << c << "\n";
	ufix16_t<8> d = 10.5;
	MODM_LOG_INFO << "ufix16_t<8> d = " << d << "\n";

	MODM_LOG_INFO << "c*d = " << c << " * " << d << " = " << c*d << "\n";
	MODM_LOG_INFO << "d*c = " << d << " * " << c << " = " << d*c << "\n";

	fix16_t<8> e = -3.3333;
	MODM_LOG_INFO << "fix16_t<8> e = " << e << "\n";

	MODM_LOG_INFO << "d*e = " << d << " * " << e << " = " << d*e << "\n";
	MODM_LOG_INFO << "e*d = " << e << " * " << d << " = " << e*d << "\n";
	MODM_LOG_INFO << "c*e = " << c << " * " << e << " = " << c*e << "\n";
	MODM_LOG_INFO << "e*c = " << e << " * " << c << " = " << e*c << "\n";

	MODM_LOG_INFO << "a*e = " << a << " * " << e << " = " << a*e << "\n";
	MODM_LOG_INFO << "e*a = " << e << " * " << a << " = " << e*a << "\n";

	auto foo = (int8_t)10 * (int8_t)42;
	MODM_LOG_INFO << typeid(foo).name() << "\n" << sizeof(foo) << modm::endl;

	auto foo2 = (int16_t)10 * (int16_t)42;
	MODM_LOG_INFO << typeid(foo2).name() << "\n" << sizeof(foo2) << modm::endl;

	auto foo3 = (int32_t)10 * (int32_t)42;
	MODM_LOG_INFO << typeid(foo3).name() << "\n" << sizeof(foo3) << modm::endl;

	return 0;
}
