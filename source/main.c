#include <nds.h>
#include <stdio.h>

vu8* const SENSOR_BASE = SRAM + 0x8000;

enum Sensor {
	SENSOR_TILT = 1
};

const struct GameData {
	const char* const code;
	const char* const name;
	int sensors;
} data[] = {
	{ "KYGE", "Yoshi's Topsy-Turvy", SENSOR_TILT },
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

void setupTilt(void) {
	u16 exmem = REG_EXMEMCNT;
	exmem &= ~0x63;
	exmem |= 0x23;
	REG_EXMEMCNT = exmem;
	printf("Preparing cartridge, setting to %04X", exmem);
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
	}
	while(1){
		if (game && game->sensors & SENSOR_TILT) {
			testTilt();
		}
		swiWaitForVBlank();
	}

	return 0;
}
