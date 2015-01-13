#include <nds.h>
#include <stdio.h>

vu8* const SENSOR_BASE = SRAM + 0x8000;
vu16* const GPIO_DATA = (vu16*) 0x080000C4;
vu16* const GPIO_DIR = (vu16*) 0x080000C6;
vu16* const GPIO_CNT = (vu16*) 0x080000C8;

enum Sensor {
	SENSOR_TILT = 1,
	SENSOR_GYRO = 2,
	SENSOR_LIGHT = 4,
	SENSOR_RTC = 8,
	SENSOR_RUMBLE = 16
};

const struct GameData {
	const char* const code;
	const char* const name;
	int sensors;
} data[] = {
	{ "KYGE", "Yoshi's Topsy-Turvy", SENSOR_TILT },
	{ "RZWE", "WarioWare Twisted!", SENSOR_GYRO | RUMBLE },
	{ 0, 0, 0 }
};

const struct GameData* detectGame(void) {
	int i;
	for (i = 0; data[i].code; ++i) {
		if (memcmp(data[i].code, GBA_HEADER.gamecode, 4) == 0) {
			return &data[i];
		}
	}
	return 0;
}

void setupGPIO(int pins) {
	*GPIO_CNT = 1;
	*GPIO_DIR = pins & 0xF;
}

void setupGyro(void) {
	setupGPIO(0xB);
	u16 exmem = REG_EXMEMCNT;
	exmem &= ~0x7F;
	exmem |= 0x17;
	REG_EXMEMCNT = exmem;
	printf("Preparing cartridge, setting to %04X\n", exmem);
}

void testGyro(void) {
	u16 state = *GPIO_DATA & 0x8;
	*GPIO_DATA = state | 3;
	*GPIO_DATA = state | 2;
	int i;
	u16 data = 0;
	for (i = 0; i < 16; ++i) {
		u16 bit = (*GPIO_DATA & 4) >> 2;
		*GPIO_DATA = state;
		data |= bit << (15 - i);
		*GPIO_DATA = state | 2;
	}
	data &= 0xFFF;
	printf("Gyro z: %03X\n", data);
}

void setupTilt(void) {
	u16 exmem = REG_EXMEMCNT;
	exmem &= ~0x63;
	exmem |= 0x23;
	REG_EXMEMCNT = exmem;
	printf("Preparing cartridge, setting to %04X\n", exmem);
}

void testTilt(void) {
	*SENSOR_BASE = 0x55;
	*(SENSOR_BASE + 0x100) = 0xAA;
	while (!(*(SENSOR_BASE + 0x300) & 0x80));
	u16 x = *(SENSOR_BASE + 0x200);
	x += (*(SENSOR_BASE + 0x300) & 0xF) << 8;
	u16 y = *(SENSOR_BASE + 0x400);
	y += (*(SENSOR_BASE + 0x500) & 0xF) << 8;
	printf("Tilt x: %03X, y: %03X\n", x, y);
}

int main(void) {
	consoleDemoInit();
	videoSetMode(MODE_FB0);
	vramSetBankA(VRAM_A_LCD);

	sysSetCartOwner(1);

	const struct GameData* game = detectGame();
	if (!game) {
		printf("Incompatible game found\n");
	} else {
		printf("Detected game: %s\n", game->name);
		if (game->sensors & SENSOR_TILT) {
			setupTilt();
		}
		if (game->sensors & SENSOR_GYRO) {
			setupGyro();
		}
	}
	while (1) {
		swiWaitForVBlank();
		if (!game) {
			continue;
		}
		if (game->sensors & SENSOR_TILT) {
			testTilt();
		}
		if (game->sensors & SENSOR_GYRO) {
			testGyro();
		}

	}

	return 0;
}
