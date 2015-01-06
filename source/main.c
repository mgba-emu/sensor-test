#include <nds.h>
#include <stdio.h>

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
	}
	while(1){
		swiWaitForVBlank();
	}

	return 0;
}
