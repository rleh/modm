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

#include <unittest/testsuite.hpp>

/// @ingroup modm_test_test_platform_sam-mcan
class FdcanTest : public unittest::TestSuite
{
public:
	void
	setUp() override;

	void
	testSendReceive();

	void
	testFilters();

	void
	testBuffers();
};
