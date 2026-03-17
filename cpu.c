/*
 * TamaLIB - A hardware agnostic first-gen Tamagotchi emulation library
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "cpu.h"
#include "hw.h"
#include "hal.h"
#include "opcodes.h"
#include <stdcountof.h>

#define TICK_FREQUENCY				32768 // Hz

#define OSC1_FREQUENCY				TICK_FREQUENCY // Hz
#define OSC3_FREQUENCY				1000000 // Hz

#define TIMER_2HZ_PERIOD			(TICK_FREQUENCY/2) // in ticks
#define TIMER_4HZ_PERIOD			(TICK_FREQUENCY/4) // in ticks
#define TIMER_8HZ_PERIOD			(TICK_FREQUENCY/8) // in ticks
#define TIMER_16HZ_PERIOD			(TICK_FREQUENCY/16) // in ticks
#define TIMER_32HZ_PERIOD			(TICK_FREQUENCY/32) // in ticks
#define TIMER_64HZ_PERIOD			(TICK_FREQUENCY/64) // in ticks
#define TIMER_128HZ_PERIOD			(TICK_FREQUENCY/128) // in ticks
#define TIMER_256HZ_PERIOD			(TICK_FREQUENCY/256) // in ticks

#define MASK_4B					0xF00
#define MASK_6B					0xFC0
#define MASK_7B					0xFE0
#define MASK_8B					0xFF0
#define MASK_10B				0xFFC
#define MASK_12B				0xFFF

#define PCS					(pc & 0xFF)
#define PCSL					(pc & 0xF)
#define PCSH					((pc >> 4) & 0xF)
#define PCP					((pc >> 8) & 0xF)
#define PCB					((pc >> 12) & 0x1)
#define TO_PC(bank, page, step)			((step & 0xFF) | ((page & 0xF) << 8) | (bank & 0x1) << 12)
#define NBP					((np >> 4) & 0x1)
#define NPP					(np & 0xF)
#define TO_NP(bank, page)			((page & 0xF) | (bank & 0x1) << 4)
#define XHL					(x & 0xFF)
#define XL					(x & 0xF)
#define XH					((x >> 4) & 0xF)
#define XP					((x >> 8) & 0xF)
#define YHL					(y & 0xFF)
#define YL					(y & 0xF)
#define YH					((y >> 4) & 0xF)
#define YP					((y >> 8) & 0xF)
#define M(n)					get_memory(n)
#define SET_M(n, v)				set_memory(n, v)
#define RQ(i)					get_rq(i)
#define SET_RQ(i, v)				set_rq(i, v)
#define SPL					(sp & 0xF)
#define SPH					((sp >> 4) & 0xF)

#define FLAG_C					(0x1 << 0)
#define FLAG_Z					(0x1 << 1)
#define FLAG_D					(0x1 << 2)
#define FLAG_I					(0x1 << 3)

#define C					!!(flags & FLAG_C)
#define Z					!!(flags & FLAG_Z)
#define D					!!(flags & FLAG_D)
#define I					!!(flags & FLAG_I)

#define SET_C()					{flags |= FLAG_C;}
#define CLEAR_C()				{flags &= ~FLAG_C;}
#define SET_Z()					{flags |= FLAG_Z;}
#define CLEAR_Z()				{flags &= ~FLAG_Z;}
#define SET_D()					{flags |= FLAG_D;}
#define CLEAR_D()				{flags &= ~FLAG_D;}
#define SET_I()					{flags |= FLAG_I;}
#define CLEAR_I()				{flags &= ~FLAG_I;}

#define REG_CLK_INT_FACTOR_FLAGS		0xF00
#define REG_SW_INT_FACTOR_FLAGS			0xF01
#define REG_PROG_INT_FACTOR_FLAGS		0xF02
#define REG_SERIAL_INT_FACTOR_FLAGS		0xF03
#define REG_K00_K03_INT_FACTOR_FLAGS		0xF04
#define REG_K10_K13_INT_FACTOR_FLAGS		0xF05
#define REG_CLOCK_INT_MASKS			0xF10
#define REG_SW_INT_MASKS			0xF11
#define REG_PROG_INT_MASKS			0xF12
#define REG_SERIAL_INT_MASKS			0xF13
#define REG_K00_K03_INT_MASKS			0xF14
#define REG_K10_K13_INT_MASKS			0xF15
#define REG_CLOCK_TIMER_DATA_1			0xF20
#define REG_CLOCK_TIMER_DATA_2			0xF21
#define REG_SW_TIMER_DATA_L			0xF22
#define REG_SW_TIMER_DATA_H			0xF23
#define REG_PROG_TIMER_DATA_L			0xF24
#define REG_PROG_TIMER_DATA_H			0xF25
#define REG_PROG_TIMER_RELOAD_DATA_L		0xF26
#define REG_PROG_TIMER_RELOAD_DATA_H		0xF27
#define REG_SERIAL_IF_DATA_L			0xF30
#define REG_SERIAL_IF_DATA_H			0xF31
#define REG_K00_K03_INPUT_PORT			0xF40
#define REG_K00_K03_INPUT_RELATION		0xF41
#define REG_K10_K13_INPUT_PORT			0xF42
#define REG_R00_R03_OUTPUT_PORT			0xF50
#define REG_R10_R13_OUTPUT_PORT			0xF51
#define REG_R20_R23_OUTPUT_PORT			0xF52
#define REG_R30_R33_OUTPUT_PORT			0xF53
#define REG_R40_R43_BZ_OUTPUT_PORT		0xF54
#define REG_P00_P03_IO_PORT			0xF60
#define REG_P10_P13_IO_PORT			0xF61
#define REG_P20_P23_IO_PORT			0xF62
#define REG_P30_P33_IO_PORT			0xF63
#define REG_CPU_OSC3_CTRL			0xF70
#define REG_LCD_CTRL				0xF71
#define REG_LCD_CONTRAST			0xF72
#define REG_SVD_CTRL				0xF73
#define REG_BUZZER_CTRL1			0xF74
#define REG_BUZZER_CTRL2			0xF75
#define REG_CLK_WD_TIMER_CTRL			0xF76
#define REG_SW_TIMER_CTRL			0xF77
#define REG_PROG_TIMER_CTRL			0xF78
#define REG_PROG_TIMER_CLK_SEL			0xF79
#define REG_SERIAL_IF_CLK_SEL			0xF7A
#define REG_HIGH_IMPEDANCE_OUTPUT_CTRL		0xF7B
#define REG_IO_CTRL				0xF7D
#define REG_IO_PULLUP_CFG			0xF7E

#define INPUT_PORT_NUM				2

typedef struct {
	const char __wf_rom *log;
	u12_t code;
	u12_t mask;
	u12_t shift_arg0;
	u12_t mask_arg0;			// != 0 only if there are two arguments
	u8_t cycles;
	void (*cb)(u8_t arg0, u8_t arg1);
} op_t;

typedef struct {
	u4_t states;
} input_port_t;

/* Registers */
static u13_t pc, next_pc;
static u12_t x, y;
static u4_t a, b;
static u5_t np;
static u8_t sp;

/* Flags */
static u4_t flags;

static const u12_t __wf_rom* g_program = NULL;
static MEM_BUFFER_TYPE __wf_iram memory[MEM_BUFFER_SIZE];

static input_port_t inputs[INPUT_PORT_NUM] = {{0}};

/* Interrupts (in priority order) */
static interrupt_t interrupts[INT_SLOT_NUM] = {
	{0x0, 0x0, 0, 0x0C}, // Prog timer
	{0x0, 0x0, 0, 0x0A}, // Serial interface
	{0x0, 0x0, 0, 0x08}, // Input (K10-K13)
	{0x0, 0x0, 0, 0x06}, // Input (K00-K03)
	{0x0, 0x0, 0, 0x04}, // Stopwatch timer
	{0x0, 0x0, 0, 0x02}, // Clock timer
};

#ifdef ENABLE_LOGS
static const char __wf_rom* interrupt_names[] = 
{
	INTERUPT_PROG_TIMER_SLOT,
	INTERUPT_SERIAL_SLOT,
	INTERUPT_K10_K13_SLOT,
	INTERUPT_K00_K03_SLOT,
	INTERUPT_STOPWATCH_SLOT,
	INTERUPT_CLOCK_TIMER_SLOT,
};
#endif // ENABLE_LOGS

static u32_t clk_timer_2hz_timestamp = 0; // in ticks
static u32_t clk_timer_4hz_timestamp = 0; // in ticks
static u32_t clk_timer_8hz_timestamp = 0; // in ticks
static u32_t clk_timer_16hz_timestamp = 0; // in ticks
static u32_t clk_timer_32hz_timestamp = 0; // in ticks
static u32_t clk_timer_64hz_timestamp = 0; // in ticks
static u32_t clk_timer_128hz_timestamp = 0; // in ticks
static u32_t clk_timer_256hz_timestamp = 0; // in ticks
static u32_t prog_timer_timestamp = 0; // in ticks
static bool_t prog_timer_enabled = 0;
static u8_t prog_timer_data = 0;
static u8_t prog_timer_rld = 0;

static u32_t tick_counter = 0;
static u32_t ts_freq;
static timestamp_t ref_ts;

static bool_t cpu_halted = 0;
static u32_t cpu_frequency = OSC1_FREQUENCY; // in hz
static u32_t scaled_cycle_accumulator = 0;


static state_t cpu_state = {
	.pc = &pc,
	.x = &x,
	.y = &y,
	.a = &a,
	.b = &b,
	.np = &np,
	.sp = &sp,
	.flags = &flags,

	.tick_counter = &tick_counter,
	.clk_timer_2hz_timestamp = &clk_timer_2hz_timestamp,
	.clk_timer_4hz_timestamp = &clk_timer_4hz_timestamp,
	.clk_timer_8hz_timestamp = &clk_timer_8hz_timestamp,
	.clk_timer_16hz_timestamp = &clk_timer_16hz_timestamp,
	.clk_timer_32hz_timestamp = &clk_timer_32hz_timestamp,
	.clk_timer_64hz_timestamp = &clk_timer_64hz_timestamp,
	.clk_timer_128hz_timestamp = &clk_timer_128hz_timestamp,
	.clk_timer_256hz_timestamp = &clk_timer_256hz_timestamp,
	.prog_timer_timestamp = &prog_timer_timestamp,
	.prog_timer_enabled = &prog_timer_enabled,
	.prog_timer_data = &prog_timer_data,
	.prog_timer_rld = &prog_timer_rld,

	.interrupts = interrupts,

	.cpu_halted = &cpu_halted,

	.memory = memory,
};

state_t * cpu_get_state(void)
{
	return &cpu_state;
}

static void generate_interrupt(int_slot_t slot, u8_t bit)
{
	/* Set the factor flag no matter what */
	interrupts[slot].factor_flag_reg = interrupts[slot].factor_flag_reg | (0x1 << bit);

	/* Trigger the INT only if not masked */
	if (interrupts[slot].mask_reg & (0x1 << bit)) {
		interrupts[slot].triggered = 1;
	}
}

void cpu_set_input_pin(pin_t pin, pin_state_t state)
{
	u4_t old_state = (inputs[pin & 0x4].states >> (pin & 0x3)) & 0x1;

	/* Trigger the interrupt if the state changed */
	if (state != old_state) {
		switch ((pin & 0x4) >> 2) {
			case 0:
				/* Active HIGH/LOW depending on the relation register */
				if (state != ((GET_IO_MEMORY(memory, REG_K00_K03_INPUT_RELATION) >> (pin & 0x3)) & 0x1)) {
					generate_interrupt(INT_K00_K03_SLOT, pin & 0x3);
				}
				break;

			case 1:
				/* Active LOW */
				if (state == PIN_STATE_LOW) {
					generate_interrupt(INT_K10_K13_SLOT, pin & 0x3);
				}
				break;
		}
	}

	/* Set the I/O */
	inputs[pin & 0x4].states = (inputs[pin & 0x4].states & ~(0x1 << (pin & 0x3))) | (state << (pin & 0x3));
}

void cpu_sync_ref_timestamp(void)
{
	ref_ts = g_hal->get_timestamp();
}

static u4_t get_io(u12_t n)
{
	u4_t tmp;

	switch (n) {
		case REG_CLK_INT_FACTOR_FLAGS:
			/* Interrupt factor flags (clock timer) */
			tmp = interrupts[INT_CLOCK_TIMER_SLOT].factor_flag_reg;
			interrupts[INT_CLOCK_TIMER_SLOT].factor_flag_reg = 0;
			return tmp;

		case REG_SW_INT_FACTOR_FLAGS:
			/* Interrupt factor flags (stopwatch) */
			tmp = interrupts[INT_STOPWATCH_SLOT].factor_flag_reg;
			interrupts[INT_STOPWATCH_SLOT].factor_flag_reg = 0;
			return tmp;

		case REG_PROG_INT_FACTOR_FLAGS:
			/* Interrupt factor flags (prog timer) */
			tmp = interrupts[INT_PROG_TIMER_SLOT].factor_flag_reg;
			interrupts[INT_PROG_TIMER_SLOT].factor_flag_reg = 0;
			return tmp;

		case REG_SERIAL_INT_FACTOR_FLAGS:
			/* Interrupt factor flags (serial) */
			tmp = interrupts[INT_SERIAL_SLOT].factor_flag_reg;
			interrupts[INT_SERIAL_SLOT].factor_flag_reg = 0;
			return tmp;

		case REG_K00_K03_INT_FACTOR_FLAGS:
			/* Interrupt factor flags (K00-K03) */
			tmp = interrupts[INT_K00_K03_SLOT].factor_flag_reg;
			interrupts[INT_K00_K03_SLOT].factor_flag_reg = 0;
			return tmp;

		case REG_K10_K13_INT_FACTOR_FLAGS:
			/* Interrupt factor flags (K10-K13) */
			tmp = interrupts[INT_K10_K13_SLOT].factor_flag_reg;
			interrupts[INT_K10_K13_SLOT].factor_flag_reg = 0;
			return tmp;

		case REG_CLOCK_INT_MASKS:
			/* Clock timer interrupt masks */
			return interrupts[INT_CLOCK_TIMER_SLOT].mask_reg;

		case REG_SW_INT_MASKS:
			/* Stopwatch interrupt masks */
			return interrupts[INT_STOPWATCH_SLOT].mask_reg & 0x3;

		case REG_PROG_INT_MASKS:
			/* Prog timer interrupt masks */
			return interrupts[INT_PROG_TIMER_SLOT].mask_reg & 0x1;

		case REG_SERIAL_INT_MASKS:
			/* Serial interface interrupt masks */
			return interrupts[INT_SERIAL_SLOT].mask_reg & 0x1;

		case REG_K00_K03_INT_MASKS:
			/* Input (K00-K03) interrupt masks */
			return interrupts[INT_K00_K03_SLOT].mask_reg;

		case REG_K10_K13_INT_MASKS:
			/* Input (K10-K13) interrupt masks */
			return interrupts[INT_K10_K13_SLOT].mask_reg;

		case REG_CLOCK_TIMER_DATA_1:
			/* Clock timer data (16-128Hz) */
			return GET_IO_MEMORY(memory, n);

		case REG_CLOCK_TIMER_DATA_2:
			/* Clock timer data (1-8Hz) */
			return GET_IO_MEMORY(memory, n);

		case REG_PROG_TIMER_DATA_L:
			/* Prog timer data (low) */
			return prog_timer_data & 0xF;

		case REG_PROG_TIMER_DATA_H:
			/* Prog timer data (high) */
			return (prog_timer_data >> 4) & 0xF;

		case REG_PROG_TIMER_RELOAD_DATA_L:
			/* Prog timer reload data (low) */
			return prog_timer_rld & 0xF;

		case REG_PROG_TIMER_RELOAD_DATA_H:
			/* Prog timer reload data (high) */
			return (prog_timer_rld >> 4) & 0xF;

		case REG_K00_K03_INPUT_PORT:
			/* Input port (K00-K03) */
			return inputs[0].states;

		case REG_K00_K03_INPUT_RELATION:
			/* Input relation register (K00-K03) */
			return GET_IO_MEMORY(memory, n);

		case REG_K10_K13_INPUT_PORT:
			/* Input port (K10-K13) */
			return inputs[1].states;

		case REG_R40_R43_BZ_OUTPUT_PORT:
			/* Output port (R40-R43) */
			return GET_IO_MEMORY(memory, n);

		case REG_CPU_OSC3_CTRL:
			/* CPU/OSC3 clocks switch, CPU voltage switch */
			return GET_IO_MEMORY(memory, n);

		case REG_LCD_CTRL:
			/* LCD control */
			return GET_IO_MEMORY(memory, n);

		case REG_LCD_CONTRAST:
			/* LCD contrast */
			break;

		case REG_SVD_CTRL:
			/* SVD */
			return GET_IO_MEMORY(memory, n) & 0x7; // Voltage always OK

		case REG_BUZZER_CTRL1:
			/* Buzzer config 1 */
			return GET_IO_MEMORY(memory, n);

		case REG_BUZZER_CTRL2:
			/* Buzzer config 2 */
			return GET_IO_MEMORY(memory, n) & 0x3; // Buzzer ready

		case REG_CLK_WD_TIMER_CTRL:
			/* Clock/Watchdog timer reset */
			break;

		case REG_SW_TIMER_CTRL:
			/* Stopwatch stop/run/reset */
			break;

		case REG_PROG_TIMER_CTRL:
			/* Prog timer stop/run/reset */
			return !!prog_timer_enabled;

		case REG_PROG_TIMER_CLK_SEL:
			/* Prog timer clock selection */
			break;

		default:
			PRINT_LOG(LOG_ERROR, LOG_ERROR_UNIMPLEMENTED_IO_READ, n, pc);
			break;
	}

	return 0;
}

static void set_io(u12_t n, u4_t v)
{
	switch (n) {
		case REG_CLOCK_INT_MASKS:
			/* Clock timer interrupt masks */
			interrupts[INT_CLOCK_TIMER_SLOT].mask_reg = v;
			break;

		case REG_SW_INT_MASKS:
			/* Stopwatch interrupt masks */
			/* Assume all INT disabled */
			interrupts[INT_STOPWATCH_SLOT].mask_reg = v;
			break;

		case REG_PROG_INT_MASKS:
			/* Prog timer interrupt masks */
			/* Assume Prog timer INT enabled (0x1) */
			interrupts[INT_PROG_TIMER_SLOT].mask_reg = v;
			break;

		case REG_SERIAL_INT_MASKS:
			/* Serial interface interrupt masks */
			/* Assume all INT disabled */
			interrupts[INT_K10_K13_SLOT].mask_reg = v;
			break;

		case REG_K00_K03_INT_MASKS:
			/* Input (K00-K03) interrupt masks */
			/* Assume all INT disabled */
			interrupts[INT_SERIAL_SLOT].mask_reg = v;
			break;

		case REG_K10_K13_INT_MASKS:
			/* Input (K10-K13) interrupt masks */
			/* Assume all INT disabled */
			interrupts[INT_K10_K13_SLOT].mask_reg = v;
			break;

		case REG_CLOCK_TIMER_DATA_1:
			/* Write not allowed */
			/* Clock timer data (16-128Hz) */
			break;

		case REG_CLOCK_TIMER_DATA_2:
			/* Write not allowed */
			/* Clock timer data (1-8Hz) */
			break;

		case REG_PROG_TIMER_RELOAD_DATA_L:
			/* Prog timer reload data (low) */
			prog_timer_rld = v | (prog_timer_rld & 0xF0);
			break;

		case REG_PROG_TIMER_RELOAD_DATA_H:
			/* Prog timer reload data (high) */
			prog_timer_rld = (prog_timer_rld & 0xF) | (v << 4);
			break;

		case REG_K00_K03_INPUT_PORT:
			/* Input port (K00-K03) */
			/* Write not allowed */
			break;

		case REG_K00_K03_INPUT_RELATION:
			/* Input relation register (K00-K03) */
			break;

		case REG_R40_R43_BZ_OUTPUT_PORT:
			/* Output port (R40-R43) */
			PRINT_LOG(LOG_INFO, LOG_INFO_BUZZER, v);
			hw_enable_buzzer(!(v & 0x8));
			break;

		case REG_CPU_OSC3_CTRL:
			/* CPU/OSC3 clocks switch, CPU voltage switch */
			/* Do not care about OSC3 state nor operating voltage */
			if ((v & 0x8) && cpu_frequency != OSC3_FREQUENCY) {
				/* OSC3 */
				cpu_frequency = (u32_t)OSC3_FREQUENCY;
				scaled_cycle_accumulator = 0;
				PRINT_LOG(LOG_INFO, LOG_INFO_OS3);
			}
			if (!(v & 0x8) && cpu_frequency != OSC1_FREQUENCY) {
				/* OSC1 */
				cpu_frequency = OSC1_FREQUENCY;
				scaled_cycle_accumulator = 0;
				PRINT_LOG(LOG_INFO, LOG_INFO_OS1);
			}
			break;

		case REG_LCD_CTRL:
			/* LCD control */
			break;

		case REG_LCD_CONTRAST:
			/* LCD contrast */
			/* Assume medium contrast (0x8) */
			break;

		case REG_SVD_CTRL:
			/* SVD */
			/* Assume battery voltage always OK (0x6) */
			break;

		case REG_BUZZER_CTRL1:
			/* Buzzer config 1 */
			hw_set_buzzer_freq(v & 0x7);
			break;

		case REG_BUZZER_CTRL2:
			/* Buzzer config 2 */
			break;

		case REG_CLK_WD_TIMER_CTRL:
			/* Clock/Watchdog timer reset */
			/* Ignore watchdog */
			break;

		case REG_SW_TIMER_CTRL:
			/* Stopwatch stop/run/reset */
			break;

		case REG_PROG_TIMER_CTRL:
			/* Prog timer stop/run/reset */
			if (v & 0x2) {
				prog_timer_data = prog_timer_rld;
			}

			if ((v & 0x1) && !prog_timer_enabled) {
				prog_timer_timestamp = tick_counter;
			}

			prog_timer_enabled = v & 0x1;
			break;

		case REG_PROG_TIMER_CLK_SEL:
			/* Prog timer clock selection */
			/* Assume 256Hz, output disabled */
			break;

		default:
			PRINT_LOG(LOG_ERROR, LOG_ERROR_UNIMPLEMENTED_IO_WRITE, v, n, pc);
			break;
	}
}

static void set_lcd(u12_t n, u4_t v)
{
	u8_t i;
	u8_t seg, com0;

	seg = ((n & 0x7F) >> 1);
	com0 = (((n & 0x80) >> 7) * 8 + (n & 0x1) * 4);

	for (i = 0; i < 4; i++) {
		hw_set_lcd_pin(seg, com0 + i, (v >> i) & 0x1);
	}
}

static u4_t get_memory(u12_t n)
{
	u4_t res = 0;

	if (n < MEM_RAM_SIZE) {
		/* RAM */
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_RAM);
		res = GET_RAM_MEMORY(memory, n);
	} else if (n >= MEM_DISPLAY1_ADDR && n < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {
		/* Display Memory 1 */
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DISPLAY, 1);
		res = GET_DISP1_MEMORY(memory, n);
	} else if (n >= MEM_DISPLAY2_ADDR && n < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {
		/* Display Memory 2 */
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DISPLAY, 2);
		res = GET_DISP2_MEMORY(memory, n);
	} else if (n >= MEM_IO_ADDR && n < (MEM_IO_ADDR + MEM_IO_SIZE)) {
		/* I/O Memory */
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_IO);
		res = get_io(n);
	} else {
		PRINT_LOG(LOG_ERROR, LOG_ERROR_INVALID_MEMORY_READ, n, pc);
		return 0;
	}

	PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DATA_READ, res);
	PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DATA_ADDRESS, n);
	PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DATA_PC, pc);

	return res;
}

static void set_memory(u12_t n, u4_t v)
{
	/* Cache any data written to a valid address, and process it */
	if (n < MEM_RAM_SIZE) {
		/* RAM */
		SET_RAM_MEMORY(memory, n, v);
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_RAM);
	} else if (n >= MEM_DISPLAY1_ADDR && n < (MEM_DISPLAY1_ADDR + MEM_DISPLAY1_SIZE)) {
		/* Display Memory 1 */
		SET_DISP1_MEMORY(memory, n, v);
		set_lcd(n, v);
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DISPLAY, 1);
	} else if (n >= MEM_DISPLAY2_ADDR && n < (MEM_DISPLAY2_ADDR + MEM_DISPLAY2_SIZE)) {
		/* Display Memory 2 */
		SET_DISP2_MEMORY(memory, n, v);
		set_lcd(n, v);
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DISPLAY, 2);
	} else if (n >= MEM_IO_ADDR && n < (MEM_IO_ADDR + MEM_IO_SIZE)) {
		/* I/O Memory */
		SET_IO_MEMORY(memory, n, v);
		set_io(n, v);
		PRINT_LOG(LOG_MEMORY, LOG_MEMORY_IO);
	} else {
		PRINT_LOG(LOG_ERROR, LOG_ERROR_INVALID_MEMORY_WRITE, v, n, pc);
		return;
	}

	PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DATA_WRITE, v);
	PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DATA_ADDRESS, n);
	PRINT_LOG(LOG_MEMORY, LOG_MEMORY_DATA_PC, pc);
}

void cpu_refresh_hw(void)
{
	static const struct range {
		u12_t addr;
		u12_t size;
	} refresh_locs[] = {
		{ MEM_DISPLAY1_ADDR, MEM_DISPLAY1_SIZE }, /* Display Memory 1 */
		{ MEM_DISPLAY2_ADDR, MEM_DISPLAY2_SIZE }, /* Display Memory 2 */
		{ REG_BUZZER_CTRL1, 1 }, /* Buzzer frequency */
		{ REG_R40_R43_BZ_OUTPUT_PORT, 1 }, /* Buzzer enabled */

		{ 0, 0 }, // end of list
	};

	for (int i = 0; refresh_locs[i].size != 0; i++) {
		for (u12_t n = refresh_locs[i].addr; n < (refresh_locs[i].addr + refresh_locs[i].size); n++) {
			set_memory(n, GET_MEMORY(memory, n));
		}
	}
}

static u4_t get_rq(u12_t rq)
{
	switch (rq & 0x3) {
		case 0x0:
			return a;

		case 0x1:
			return b;

		case 0x2:
			return M(x);

		case 0x3:
			return M(y);
	}

	return 0;
}

static void set_rq(u12_t rq, u4_t v)
{
	switch (rq & 0x3) {
		case 0x0:
			a = v;
			break;

		case 0x1:
			b = v;
			break;

		case 0x2:
			SET_M(x, v);
			break;

		case 0x3:
			SET_M(y, v);
			break;
	}
}

/* Instructions */
static void op_pset_cb(u8_t arg0, u8_t arg1)
{
	np = arg0;
}

static void op_jp_cb(u8_t arg0, u8_t arg1)
{
	next_pc = arg0 | (np << 8);
}

static void op_jp_c_cb(u8_t arg0, u8_t arg1)
{
	if (flags & FLAG_C) {
		next_pc = arg0 | (np << 8);
	}
}

static void op_jp_nc_cb(u8_t arg0, u8_t arg1)
{
	if (!(flags & FLAG_C)) {
		next_pc = arg0 | (np << 8);
	}
}

static void op_jp_z_cb(u8_t arg0, u8_t arg1)
{
	if (flags & FLAG_Z) {
		next_pc = arg0 | (np << 8);
	}
}

static void op_jp_nz_cb(u8_t arg0, u8_t arg1)
{
	if (!(flags & FLAG_Z)) {
		next_pc = arg0 | (np << 8);
	}
}

static void op_jpba_cb(u8_t arg0, u8_t arg1)
{
	next_pc = a | (b << 4) | (np << 8);
}

static void op_call_cb(u8_t arg0, u8_t arg1)
{
	pc = (pc + 1) & 0x1FFF; // This does not actually change the PC register
	SET_M((sp - 1) & 0xFF, PCP);
	SET_M((sp - 2) & 0xFF, PCSH);
	SET_M((sp - 3) & 0xFF, PCSL);
	sp = (sp - 3) & 0xFF;
	next_pc = TO_PC(PCB, NPP, arg0);
}

static void op_calz_cb(u8_t arg0, u8_t arg1)
{
	pc = (pc + 1) & 0x1FFF; // This does not actually change the PC register
	SET_M((sp - 1) & 0xFF, PCP);
	SET_M((sp - 2) & 0xFF, PCSH);
	SET_M((sp - 3) & 0xFF, PCSL);
	sp = (sp - 3) & 0xFF;
	next_pc = TO_PC(PCB, 0, arg0);
}

static void op_ret_cb(u8_t arg0, u8_t arg1)
{
	next_pc = M(sp) | (M((sp + 1) & 0xFF) << 4) | (M((sp + 2) & 0xFF) << 8) | (PCB << 12);
	sp = (sp + 3) & 0xFF;
}

static void op_rets_cb(u8_t arg0, u8_t arg1)
{
	next_pc = M(sp) | (M((sp + 1) & 0xFF) << 4) | (M((sp + 2) & 0xFF) << 8) | (PCB << 12);
	sp = (sp + 3) & 0xFF;
	next_pc = (next_pc + 1) & 0x1FFF;
}

static void op_retd_cb(u8_t arg0, u8_t arg1)
{
	next_pc = M(sp) | (M((sp + 1) & 0xFF) << 4) | (M((sp + 2) & 0xFF) << 8) | (PCB << 12);
	sp = (sp + 3) & 0xFF;
	SET_M(x, arg0 & 0xF);
	SET_M(((x + 1) & 0xFF) | (XP << 8), (arg0 >> 4) & 0xF);
	x = ((x + 2) & 0xFF) | (XP << 8);
}

static void op_nop5_cb(u8_t arg0, u8_t arg1)
{
}

static void op_nop7_cb(u8_t arg0, u8_t arg1)
{
}

static void op_halt_cb(u8_t arg0, u8_t arg1)
{
	cpu_halted = 1;
}

static void op_inc_x_cb(u8_t arg0, u8_t arg1)
{
	x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_inc_y_cb(u8_t arg0, u8_t arg1)
{
	y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_ld_x_cb(u8_t arg0, u8_t arg1)
{
	x = arg0 | (XP << 8);
}

static void op_ld_y_cb(u8_t arg0, u8_t arg1)
{
	y = arg0 | (YP << 8);
}

static void op_ld_xp_r_cb(u8_t arg0, u8_t arg1)
{
	x = XHL | (RQ(arg0) << 8);
}

static void op_ld_xh_r_cb(u8_t arg0, u8_t arg1)
{
	x = XL | (RQ(arg0) << 4) | (XP << 8);
}

static void op_ld_xl_r_cb(u8_t arg0, u8_t arg1)
{
	x = RQ(arg0) | (XH << 4) | (XP << 8);
}

static void op_ld_yp_r_cb(u8_t arg0, u8_t arg1)
{
	y = YHL | (RQ(arg0) << 8);
}

static void op_ld_yh_r_cb(u8_t arg0, u8_t arg1)
{
	y = YL | (RQ(arg0) << 4) | (YP << 8);
}

static void op_ld_yl_r_cb(u8_t arg0, u8_t arg1)
{
	y = RQ(arg0) | (YH << 4) | (YP << 8);
}

static void op_ld_r_xp_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, XP);
}

static void op_ld_r_xh_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, XH);
}

static void op_ld_r_xl_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, XL);
}

static void op_ld_r_yp_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, YP);
}

static void op_ld_r_yh_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, YH);
}

static void op_ld_r_yl_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, YL);
}

static void op_adc_xh_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = XH + arg0 + C;
	x = XL | ((tmp & 0xF) << 4)| (XP << 8);
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!(tmp & 0xF)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_adc_xl_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = XL + arg0 + C;
	x = (tmp & 0xF) | (XH << 4) | (XP << 8);
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!(tmp & 0xF)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_adc_yh_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = YH + arg0 + C;
	y = YL | ((tmp & 0xF) << 4)| (YP << 8);
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!(tmp & 0xF)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_adc_yl_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = YL + arg0 + C;
	y = (tmp & 0xF) | (YH << 4) | (YP << 8);
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!(tmp & 0xF)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_cp_xh_cb(u8_t arg0, u8_t arg1)
{
	if (XH < arg0) { SET_C(); } else { CLEAR_C(); }
	if (XH == arg0) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_cp_xl_cb(u8_t arg0, u8_t arg1)
{
	if (XL < arg0) { SET_C(); } else { CLEAR_C(); }
	if (XL == arg0) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_cp_yh_cb(u8_t arg0, u8_t arg1)
{
	if (YH < arg0) { SET_C(); } else { CLEAR_C(); }
	if (YH == arg0) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_cp_yl_cb(u8_t arg0, u8_t arg1)
{
	if (YL < arg0) { SET_C(); } else { CLEAR_C(); }
	if (YL == arg0) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_ld_r_i_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, arg1);
}

static void op_ld_r_q_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg1));
}

static void op_ld_a_mn_cb(u8_t arg0, u8_t arg1)
{
	a = M(arg0);
}

static void op_ld_b_mn_cb(u8_t arg0, u8_t arg1)
{
	b = M(arg0);
}

static void op_ld_mn_a_cb(u8_t arg0, u8_t arg1)
{
	SET_M(arg0, a);
}

static void op_ld_mn_b_cb(u8_t arg0, u8_t arg1)
{
	SET_M(arg0, b);
}

static void op_ldpx_mx_cb(u8_t arg0, u8_t arg1)
{
	SET_M(x, arg0);
	x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_ldpx_r_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg1));
	x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_ldpy_my_cb(u8_t arg0, u8_t arg1)
{
	SET_M(y, arg0);
	y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_ldpy_r_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg1));
	y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_lbpx_cb(u8_t arg0, u8_t arg1)
{
	SET_M(x, arg0 & 0xF);
	SET_M(((x + 1) & 0xFF) | (XP << 8), (arg0 >> 4) & 0xF);
	x = ((x + 2) & 0xFF) | (XP << 8);
}

static void op_set_cb(u8_t arg0, u8_t arg1)
{
	flags |= arg0;
}

static void op_rst_cb(u8_t arg0, u8_t arg1)
{
	flags &= arg0;
}

static void op_scf_cb(u8_t arg0, u8_t arg1)
{
	SET_C();
}

static void op_rcf_cb(u8_t arg0, u8_t arg1)
{
	CLEAR_C();
}

static void op_szf_cb(u8_t arg0, u8_t arg1)
{
	SET_Z();
}

static void op_rzf_cb(u8_t arg0, u8_t arg1)
{
	CLEAR_Z();
}

static void op_sdf_cb(u8_t arg0, u8_t arg1)
{
	SET_D();
}

static void op_rdf_cb(u8_t arg0, u8_t arg1)
{
	CLEAR_D();
}

static void op_ei_cb(u8_t arg0, u8_t arg1)
{
	SET_I();
}

static void op_di_cb(u8_t arg0, u8_t arg1)
{
	CLEAR_I();
}

static void op_inc_sp_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp + 1) & 0xFF;
}

static void op_dec_sp_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
}

static void op_push_r_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, RQ(arg0));
}

static void op_push_xp_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, XP);
}

static void op_push_xh_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, XH);
}

static void op_push_xl_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, XL);
}

static void op_push_yp_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, YP);
}

static void op_push_yh_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, YH);
}

static void op_push_yl_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, YL);
}

static void op_push_f_cb(u8_t arg0, u8_t arg1)
{
	sp = (sp - 1) & 0xFF;
	SET_M(sp, flags);
}

static void op_pop_r_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, M(sp));
	sp = (sp + 1) & 0xFF;
}

static void op_pop_xp_cb(u8_t arg0, u8_t arg1)
{
	x = XL | (XH << 4)| (M(sp) << 8);
	sp = (sp + 1) & 0xFF;
}

static void op_pop_xh_cb(u8_t arg0, u8_t arg1)
{
	x = XL | (M(sp) << 4)| (XP << 8);
	sp = (sp + 1) & 0xFF;
}

static void op_pop_xl_cb(u8_t arg0, u8_t arg1)
{
	x = M(sp) | (XH << 4)| (XP << 8);
	sp = (sp + 1) & 0xFF;
}

static void op_pop_yp_cb(u8_t arg0, u8_t arg1)
{
	y = YL | (YH << 4)| (M(sp) << 8);
	sp = (sp + 1) & 0xFF;
}

static void op_pop_yh_cb(u8_t arg0, u8_t arg1)
{
	y = YL | (M(sp) << 4)| (YP << 8);
	sp = (sp + 1) & 0xFF;
}

static void op_pop_yl_cb(u8_t arg0, u8_t arg1)
{
	y = M(sp) | (YH << 4)| (YP << 8);
	sp = (sp + 1) & 0xFF;
}

static void op_pop_f_cb(u8_t arg0, u8_t arg1)
{
	flags = M(sp);
	sp = (sp + 1) & 0xFF;
}

static void op_ld_sph_r_cb(u8_t arg0, u8_t arg1)
{
	sp = SPL | (RQ(arg0) << 4);
}

static void op_ld_spl_r_cb(u8_t arg0, u8_t arg1)
{
	sp = RQ(arg0) | (SPH << 4);
}

static void op_ld_r_sph_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, SPH);
}

static void op_ld_r_spl_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, SPL);
}

static void op_add_r_i_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) + arg1;
	if (D) {
		if (tmp >= 10) {
			SET_RQ(arg0, (tmp - 10) & 0xF);
			SET_C();
		} else {
			SET_RQ(arg0, tmp);
			CLEAR_C();
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
		if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	}
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_add_r_q_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) + RQ(arg1);
	if (D) {
		if (tmp >= 10) {
			SET_RQ(arg0, (tmp - 10) & 0xF);
			SET_C();
		} else {
			SET_RQ(arg0, tmp);
			CLEAR_C();
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
		if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	}
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_adc_r_i_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) + arg1 + C;
	if (D) {
		if (tmp >= 10) {
			SET_RQ(arg0, (tmp - 10) & 0xF);
			SET_C();
		} else {
			SET_RQ(arg0, tmp);
			CLEAR_C();
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
		if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	}
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_adc_r_q_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) + RQ(arg1) + C;
	if (D) {
		if (tmp >= 10) {
			SET_RQ(arg0, (tmp - 10) & 0xF);
			SET_C();
		} else {
			SET_RQ(arg0, tmp);
			CLEAR_C();
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
		if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	}
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_sub_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) - RQ(arg1);
	if (D) {
		if (tmp >> 4) {
			SET_RQ(arg0, (tmp - 6) & 0xF);
		} else {
			SET_RQ(arg0, tmp);
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
	}
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_sbc_r_i_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) - arg1 - C;
	if (D) {
		if (tmp >> 4) {
			SET_RQ(arg0, (tmp - 6) & 0xF);
		} else {
			SET_RQ(arg0, tmp);
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
	}
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_sbc_r_q_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = RQ(arg0) - RQ(arg1) - C;
	if (D) {
		if (tmp >> 4) {
			SET_RQ(arg0, (tmp - 6) & 0xF);
		} else {
			SET_RQ(arg0, tmp);
		}
	} else {
		SET_RQ(arg0, tmp & 0xF);
	}
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_and_r_i_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg0) & arg1);
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_and_r_q_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg0) & RQ(arg1));
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_or_r_i_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg0) | arg1);
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_or_r_q_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg0) | RQ(arg1));
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_xor_r_i_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg0) ^ arg1);
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_xor_r_q_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, RQ(arg0) ^ RQ(arg1));
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_cp_r_i_cb(u8_t arg0, u8_t arg1)
{
	if (RQ(arg0) < arg1) { SET_C(); } else { CLEAR_C(); }
	if (RQ(arg0) == arg1) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_cp_r_q_cb(u8_t arg0, u8_t arg1)
{
	if (RQ(arg0) < RQ(arg1)) { SET_C(); } else { CLEAR_C(); }
	if (RQ(arg0) == RQ(arg1)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_fan_r_i_cb(u8_t arg0, u8_t arg1)
{
	if (!(RQ(arg0) & arg1)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_fan_r_q_cb(u8_t arg0, u8_t arg1)
{
	if (!(RQ(arg0) & RQ(arg1))) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_rlc_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = (RQ(arg0) << 1) | C;
	if (RQ(arg0) & 0x8) { SET_C(); } else { CLEAR_C(); }
	SET_RQ(arg0, tmp & 0xF);
	/* No need to set Z (issue in DS) */
}

static void op_rrc_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = (RQ(arg0) >> 1) | (C << 3);
	if (RQ(arg0) & 0x1) { SET_C(); } else { CLEAR_C(); }
	SET_RQ(arg0, tmp & 0xF);
	/* No need to set Z (issue in DS) */
}

static void op_inc_mn_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = M(arg0) + 1;
	SET_M(arg0, tmp & 0xF);
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!M(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_dec_mn_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = M(arg0) - 1;
	SET_M(arg0, tmp & 0xF);
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!M(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

static void op_acpx_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = M(x) + RQ(arg0) + C;
	if (D) {
		if (tmp >= 10) {
			SET_M(x, (tmp - 10) & 0xF);
			SET_C();
		} else {
			SET_M(x, tmp);
			CLEAR_C();
		}
	} else {
		SET_M(x, tmp & 0xF);
		if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	}
	if (!M(x)) { SET_Z(); } else { CLEAR_Z(); }
	x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_acpy_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = M(y) + RQ(arg0) + C;
	if (D) {
		if (tmp >= 10) {
			SET_M(y, (tmp - 10) & 0xF);
			SET_C();
		} else {
			SET_M(y, tmp);
			CLEAR_C();
		}
	} else {
		SET_M(y, tmp & 0xF);
		if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	}
	if (!M(y)) { SET_Z(); } else { CLEAR_Z(); }
	y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_scpx_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = M(x) - RQ(arg0) - C;
	if (D) {
		if (tmp >> 4) {
			SET_M(x, (tmp - 6) & 0xF);
		} else {
			SET_M(x, tmp);
		}
	} else {
		SET_M(x, tmp & 0xF);
	}
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!M(x)) { SET_Z(); } else { CLEAR_Z(); }
	x = ((x + 1) & 0xFF) | (XP << 8);
}

static void op_scpy_cb(u8_t arg0, u8_t arg1)
{
	u8_t tmp;

	tmp = M(y) - RQ(arg0) - C;
	if (D) {
		if (tmp >> 4) {
			SET_M(y, (tmp - 6) & 0xF);
		} else {
			SET_M(y, tmp);
		}
	} else {
		SET_M(y, tmp & 0xF);
	}
	if (tmp >> 4) { SET_C(); } else { CLEAR_C(); }
	if (!M(y)) { SET_Z(); } else { CLEAR_Z(); }
	y = ((y + 1) & 0xFF) | (YP << 8);
}

static void op_not_cb(u8_t arg0, u8_t arg1)
{
	SET_RQ(arg0, ~RQ(arg0) & 0xF);
	if (!RQ(arg0)) { SET_Z(); } else { CLEAR_Z(); }
}

#ifdef ENABLE_LOGS
#define LOG_STRING(str) str
#else
#define LOG_STRING(str) NULL
#endif // ENABLE_LOGS


#define X(log, code, mask, shift_arg0, mask_arg0, cycles, func) {LOG_STRING(log), code, mask, shift_arg0, mask_arg0, cycles, func},
static const op_t __wf_rom ops[] = 
{
	OPCODE_LIST
	{NULL, 0, 0, 0, 0, 0, NULL}
};
#undef X

#define X(log, code, mask, shift_arg0, mask_arg0, cycles, func) [code] = __COUNTER__,
static uint8_t __wf_rom opcode_table[] = 
{
	OPCODE_LIST
};
#undef X

static timestamp_t wait_for_cycles(timestamp_t since, u8_t cycles) {
	u32_t ticks_pending;

	/* The tick counter always works at TICK_FREQUENCY,
	 * while the CPU runs at cpu_frequency
	 */
	scaled_cycle_accumulator += cycles * TICK_FREQUENCY;
	ticks_pending = scaled_cycle_accumulator/cpu_frequency;

	if (ticks_pending > 0) {
		tick_counter += ticks_pending;
		scaled_cycle_accumulator -= ticks_pending * cpu_frequency;
	}

	/* Emulation will be as fast as possible */
	return g_hal->get_timestamp();
}

static void process_interrupts(void)
{
	u8_t i;

	/* Process interrupts in priority order */
	for (i = 0; i < INT_SLOT_NUM; i++) {
		if (interrupts[i].triggered) {
			PRINT_LOG(LOG_INT, LOG_INTERRUPT_TRIGGERED, interrupt_names[i], i);
			SET_M((sp - 1) & 0xFF, PCP);
			SET_M((sp - 2) & 0xFF, PCSH);
			SET_M((sp - 3) & 0xFF, PCSL);
			sp = (sp - 3) & 0xFF;
			CLEAR_I();
			np = TO_NP(NBP, 1);
			pc = TO_PC(PCB, 1, interrupts[i].vector);
			cpu_halted = 0;

			ref_ts = wait_for_cycles(ref_ts, 12);
			interrupts[i].triggered = 0;
			return;
		}
	}
}

static void print_state(const op_t __wf_rom* opcode, u12_t op, u13_t addr)
{
	u8_t i;

	if (!g_hal->is_log_enabled(LOG_CPU) && g_hal->is_log_enabled(LOG_OP)) 
	{
		if (opcode->mask_arg0 != 0) 
		{
			/* Two arguments */
			PRINT_LOG(LOG_OP, opcode->log, (op & opcode->mask_arg0) >> opcode->shift_arg0, op & ~(opcode->mask | opcode->mask_arg0));
		} 
		else 
		{
			/* One argument */
			PRINT_LOG(LOG_OP, opcode->log, (op & ~opcode->mask) >> opcode->shift_arg0);
		}
		PRINT_LOG(LOG_OP, LOG_OP_ADDR, addr);
		return;
	}

	PRINT_LOG(LOG_CPU, LOG_CPU_ADDR, addr);

		PRINT_LOG(LOG_CPU, LOG_CPU_ARROW);

	if (opcode->mask_arg0 != 0) {
		/* Two arguments */
		PRINT_LOG(LOG_OP, opcode->log, (op & opcode->mask_arg0) >> opcode->shift_arg0, op & ~(opcode->mask | opcode->mask_arg0));
	} else {
		/* One argument */
		PRINT_LOG(LOG_OP, opcode->log, (op & ~opcode->mask) >> opcode->shift_arg0);
	}

	PRINT_LOG(LOG_CPU, LOG_CPU_OPCODE, op);
	for (i = 0; i < 12; i++) {
		PRINT_LOG(LOG_CPU, LOG_CPU_STR, ((op >> (11 - i)) & 0x1) ? "1" : "0");
	}
	PRINT_LOG(LOG_CPU, LOG_CPU_STATS, pc, sp, np, x, y, a, b, flags);
}

static void handle_timers(void)
{
	/* Handle timers using the internal tick counter */
	if (tick_counter - clk_timer_2hz_timestamp >= TIMER_2HZ_PERIOD) {
		do {
			clk_timer_2hz_timestamp += TIMER_2HZ_PERIOD;
		} while (tick_counter - clk_timer_2hz_timestamp >= TIMER_2HZ_PERIOD);

		/* Update clock timer data for 1Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) ^ (0x1 << 3));

		/* Generate interrupt on falling edge only (1Hz) */
		if (!((GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) >> 3) & 0x1 )) {
			generate_interrupt(INT_CLOCK_TIMER_SLOT, 3);
		}
	}

	if (tick_counter - clk_timer_4hz_timestamp >= TIMER_4HZ_PERIOD) {
		do {
			clk_timer_4hz_timestamp += TIMER_4HZ_PERIOD;
		} while (tick_counter - clk_timer_4hz_timestamp >= TIMER_4HZ_PERIOD);

		/* Update clock timer data for 2Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) ^ (0x1 << 2));

		/* Generate interrupt on falling edge only (2Hz) */
		if (!((GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) >> 2) & 0x1 )) {
			generate_interrupt(INT_CLOCK_TIMER_SLOT, 2);
		}
	}

	if (tick_counter - clk_timer_8hz_timestamp >= TIMER_8HZ_PERIOD) {
		do {
			clk_timer_8hz_timestamp += TIMER_8HZ_PERIOD;
		} while (tick_counter - clk_timer_8hz_timestamp >= TIMER_8HZ_PERIOD);

		/* Update clock timer data for 4Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) ^ (0x1 << 1));
	}

	if (tick_counter - clk_timer_16hz_timestamp >= TIMER_16HZ_PERIOD) {
		do {
			clk_timer_16hz_timestamp += TIMER_16HZ_PERIOD;
		} while (tick_counter - clk_timer_16hz_timestamp >= TIMER_16HZ_PERIOD);

		/* Update clock timer data for 8Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) ^ (0x1 << 0));

		/* Generate interrupt on falling edge only (8Hz) */
		if (!((GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_2) >>0) & 0x1 )) {
			generate_interrupt(INT_CLOCK_TIMER_SLOT, 1);
		}
	}

	if (tick_counter - clk_timer_32hz_timestamp >= TIMER_32HZ_PERIOD) {
		do {
			clk_timer_32hz_timestamp += TIMER_32HZ_PERIOD;
		} while (tick_counter - clk_timer_32hz_timestamp >= TIMER_32HZ_PERIOD);

		/* Update clock timer data for 16Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1) ^ (0x1 << 3));
	}

	if (tick_counter - clk_timer_64hz_timestamp >= TIMER_64HZ_PERIOD) {
		do {
			clk_timer_64hz_timestamp += TIMER_64HZ_PERIOD;
		} while (tick_counter - clk_timer_64hz_timestamp >= TIMER_64HZ_PERIOD);

		/* Update clock timer data for 32Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1) ^ (0x1 << 2));

		/* Generate interrupt on falling edge only (32Hz) */
		if (!((GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1) >> 2) & 0x1 )) {
			generate_interrupt(INT_CLOCK_TIMER_SLOT, 0);
		}
	}

	if (tick_counter - clk_timer_128hz_timestamp >= TIMER_128HZ_PERIOD) {
		do {
			clk_timer_128hz_timestamp += TIMER_128HZ_PERIOD;
		} while (tick_counter - clk_timer_128hz_timestamp >= TIMER_128HZ_PERIOD);

		/* Update clock timer data for 64Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1) ^ (0x1 << 1));
	}

	if (tick_counter - clk_timer_256hz_timestamp >= TIMER_256HZ_PERIOD) {
		do {
			clk_timer_256hz_timestamp += TIMER_256HZ_PERIOD;
		} while (tick_counter - clk_timer_256hz_timestamp >= TIMER_256HZ_PERIOD);

		/* Update clock timer data for 128Hz */
		SET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1, GET_IO_MEMORY(memory, REG_CLOCK_TIMER_DATA_1) ^ (0x1 << 0));
	}

	if (prog_timer_enabled && tick_counter - prog_timer_timestamp >= TIMER_256HZ_PERIOD) {
		do {
			prog_timer_timestamp += TIMER_256HZ_PERIOD;
			prog_timer_data--;

			if (prog_timer_data == 0) {
				prog_timer_data = prog_timer_rld;
				generate_interrupt(INT_PROG_TIMER_SLOT, 0);
			}
		} while (tick_counter - prog_timer_timestamp >= TIMER_256HZ_PERIOD);
	}
}

void cpu_reset(void)
{
	u13_t i;

	/* Registers and variables init */
	pc = TO_PC(0, 1, 0x00); // PC starts at bank 0, page 1, step 0
	np = TO_NP(0, 1); // NP starts at page 1
	a = 0; // undef
	b = 0; // undef
	x = 0; // undef
	y = 0; // undef
	sp = 0; // undef
	flags = 0;

	/* Init RAM to zeros */
	for (i = 0; i < MEM_BUFFER_SIZE; i++) {
		memory[i] = 0;
	}

	SET_IO_MEMORY(memory, REG_R40_R43_BZ_OUTPUT_PORT, 0xF); // Output port (R40-R43)
	SET_IO_MEMORY(memory, REG_LCD_CTRL, 0x8); // LCD control
	SET_IO_MEMORY(memory, REG_K00_K03_INPUT_RELATION, 0xF); // Active high

	cpu_frequency = OSC1_FREQUENCY;

	cpu_sync_ref_timestamp();
}

bool_t cpu_init(const u12_t __wf_rom* program, u32_t freq)
{
	g_program = program;
	ts_freq = freq;

	cpu_reset();

	return 0;
}

void cpu_release(void)
{
}

int cpu_step(void)
{
	u12_t op;
	const op_t __wf_rom* opcode = NULL;
	static u8_t previous_cycles = 0;

	if (!cpu_halted) 
	{
		op = g_program[pc];
		
		/* Lookup the OP code */
		opcode = ops + opcode_table[op & MASK_4B];
		if ((op & opcode->mask) == opcode->code) 
		{
			goto HAVE_OPCODE;
		}

		opcode = ops + opcode_table[op & MASK_6B];
		if ((op & opcode->mask) == opcode->code) 
		{
			goto HAVE_OPCODE;
		}

		opcode = ops + opcode_table[op & MASK_7B];
		if ((op & opcode->mask) == opcode->code) 
		{
			goto HAVE_OPCODE;
		}

		opcode = ops + opcode_table[op & MASK_8B];
		if ((op & opcode->mask) == opcode->code) 
		{
			goto HAVE_OPCODE;
		}

		opcode = ops + opcode_table[op & MASK_10B];
		if ((op & opcode->mask) == opcode->code) 
		{
			goto HAVE_OPCODE;
		}

		opcode = ops + opcode_table[op & MASK_12B];
		if ((op & opcode->mask) == opcode->code) 
		{
			goto HAVE_OPCODE;
		}

		PRINT_LOG(LOG_ERROR, LOG_ERROR_UNKNOWN_OPCODE, op, pc);
		return 1;

HAVE_OPCODE:
		next_pc = (pc + 1) & 0x1FFF;
		
		/* Display the operation along with the current state of the processor */
		print_state(opcode, op, pc);

		/* Match the speed of the real processor
		* NOTE: For better accuracy, the final wait should happen here, however
		* the downside is that all interrupts will likely be delayed by one OP
		*/
		ref_ts = wait_for_cycles(ref_ts, previous_cycles);

		/* Process the OP code */
		if (opcode->mask_arg0 != 0) 
		{
			/* Two arguments */
			opcode->cb((op & opcode->mask_arg0) >> opcode->shift_arg0, op & ~(opcode->mask | opcode->mask_arg0));
		} 
		else 
		{
			/* One arguments */
			opcode->cb((op & ~opcode->mask) >> opcode->shift_arg0, 0);
		}

		/* Prepare for the next instruction */
		pc = next_pc;
		previous_cycles = opcode->cycles;

		if (opcode->code != 0xE40) 
		{
			/* OP code is not PSET, reset NP */
			np = (pc >> 8) & 0x1F;
		}
	}
	else 
	{
		/* Wait at least once for the duration of a HALT and as long as required
		 * (to increment the tick counter), but make sure there will be no wait once
		 * the CPU is restarted
		 */
		ref_ts = wait_for_cycles(ref_ts, 5);
		previous_cycles = 0;
	}

	handle_timers();

	/* Check if there is any pending interrupt */
	if (I && 
		opcode->code != 0xE40 && 
		opcode->code != 0xF48) 
	{ 
		process_interrupts();
	}

	return 0;
}