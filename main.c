#include <stdio.h>
#include <string.h>
#include "node.h"

int main() {
	const char* playlist_1[] = {
		"AAA",
		"BBB",
		"CCC",
		"DDD",
		"EEE",
		"FFF",
		"GGG",
		"HHH",
		"III",
		"JJJ",
		"KKK",
	};

	const char* playlist_2[] = {
		"AAA",
		"BBB",
		"EEE",
		"FFF",
		"JJJ",
		"UUU new",
		"VVV new",
		"WWW new",
		"XXX new",
		"YYY new",
		"ZZZ new",
	};

	init_hash_engine(1, sizeof(music_t), sizeof(playlist_prop_t));

	printf("%lu books in total.\n", sizeof(playlist_1) / sizeof(char*));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		add_music(playlist_1[i]);
	}

	show_playlist();

	reset_playlist();

	for (int i = 0; i < sizeof(playlist_2) / sizeof(char*); i++) {
		add_music(playlist_2[i]);
	}

	show_playlist();

	playlist_prop_t prop;

	get_playlist_prop();

	prop.reserved = 0x12345678;
	prop.which_album_to_handle = 0x1;
	set_playlist_prop(&prop);

	get_playlist_prop();

	prop.reserved = 0x87654321;
	prop.which_album_to_handle = 0x3;
	set_playlist_prop(&prop);

	get_playlist_prop();
}
