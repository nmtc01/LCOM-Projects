#pragma once

#define STAT_REG        0x64
#define OBF				0x01
#define IBF				0x02
#define PAR_ERR			0x80
#define TO_ERR			0x40
#define KBC_CMD_REG     0x64
#define OUT_BUF         0x60

#define ESC_BREAK       0x81
#define DELAY_US        20000

#define TWO_LONG		0xE0
#define MK_OR_BRK		0x0080
#define READ_CMD		0x20
#define WRT_CMD		    0x60

#define IRQ1			0x01


