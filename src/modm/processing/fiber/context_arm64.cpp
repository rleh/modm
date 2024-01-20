/*
 * Copyright (c) 2020, Erik Henriksson
 * Copyright (c) 2021, 2023, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include "context.h"
#include <modm/architecture/detect.hpp>

/* Stack layout (growing downwards):
 *
 * Permanent Storage:
 * Fiber Function
 * Fiber Function Argument
 *
 * Temporary Prepare:
 * Entry Function
 *
 * Register file:
 * LR
 * FP
 * x28
 * x27
 * x26
 * x25
 * x24
 * x23
 * x22
 * x21
 * x20
 * x19
 * d15
 * d14
 * d13
 * d12
 * d11
 * d10
 * d9
 * d8
 */

namespace
{

constexpr size_t StackWordsReset = 1;
constexpr size_t StackWordsStorage = 2;
constexpr size_t StackWordsRegisters = 20;
constexpr size_t StackWordsAll = StackWordsStorage + StackWordsRegisters;
constexpr size_t StackSizeWord = sizeof(uintptr_t);
constexpr uintptr_t StackWatermark = 0xc0ffee'f00d'facade;

}

extern "C" void modm_context_jump(modm_context_t* from, modm_context_t* to);

void
modm_context_init(modm_context_t *ctx,
				  uintptr_t *bottom, uintptr_t *top,
				  uintptr_t fn, uintptr_t fn_arg)
{
	ctx->bottom = bottom;
	ctx->top = top;

	ctx->sp = top;
	*--ctx->sp = fn;
	*--ctx->sp = fn_arg;
}

void
modm_context_entry()
{
	asm volatile
	(
	"ldr x0, [sp]	\n\t"		// Load closure data pointer
	"ldr x1, [sp, #8]	\n\t"	// Jump to closure function
	"br x1	\n\t"
	);
}

void
modm_context_reset(modm_context_t *ctx)
{
	*ctx->bottom = StackWatermark;

	ctx->sp = ctx->top - StackWordsStorage;
	*--ctx->sp = (uintptr_t) modm_context_entry;
	ctx->sp -= StackWordsRegisters - StackWordsReset;
}

void
modm_context_watermark(modm_context_t *ctx)
{
	// clear the register file on the stack
	for (auto *word = ctx->top - StackWordsAll;
		 word < ctx->top - StackWordsStorage - StackWordsReset; word++)
		*word = 0;

	// then color the whole stack *below* the register file
	for (auto *word = ctx->bottom; word < ctx->top - StackWordsAll; word++)
		*word = StackWatermark;
}

size_t
modm_context_stack_usage(const modm_context_t *ctx)
{
	for (auto *word = ctx->bottom; word < ctx->top; word++)
		if (StackWatermark != *word)
			return (ctx->top - word) * StackSizeWord;
	return 0;
}

bool
modm_context_stack_overflow(const modm_context_t *ctx)
{
	return *ctx->bottom != StackWatermark;
}

static modm_context_t main_context;

void
modm_context_start(modm_context_t *to)
{
	modm_context_jump(&main_context, to);
}

void
modm_context_end()
{
	modm_context_t dummy;
	modm_context_jump(&dummy, &main_context);
	__builtin_unreachable();
}
