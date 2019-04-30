#include <stdio.h>
#include <string.h>
#include "node.h"

int main() {
	record_property_t prop;

	const music_t playlist_1[] = {
		{"AAA"},
		{"BBB"},
		{"CCC"},
		{"DDD"},
		{"EEE"},
		{"FFF"},
		{"GGG"},
		{"HHH"},
		{"III"},
		{"JJJ"},
		{"KKK"},
		{"LLL"},
		{"MMM"},
	};

	const music_t playlist_2[] = {
		{"UUU new"},
		{"VVV new"},
		{"WWW new"},
		{"XXX new"},
		{"YYY new"},
		{"ZZZ new"},
	};

	init_hash_engine(3, sizeof(music_t));

	printf("%lu books in total.\n", sizeof(playlist_1) / sizeof(music_t));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(music_t); i++) {
		add_music(playlist_1[i].path);
	}

	show_playlist();

	del_music("AAA");
	del_music("AAA");
	get_record_prop(PLAYLIST_PATH, &prop);
	prop.which_album_to_add = 9;
	prop.reserved = 0xaabbccdd;
	set_record_prop(PLAYLIST_PATH, &prop);
	del_music("FFF");
	del_music("HHH");
	del_music("GGG");
	get_record_prop(PLAYLIST_PATH, &prop);
	del_music("LLL");
	del_music("MMM");
	del_music("ABC");

	show_playlist();

	for (int i = 0; i < sizeof(playlist_2) / sizeof(music_t); i++) {
		add_music(playlist_2[i].path);
	}

	show_playlist();

	get_record_prop(PLAYLIST_PATH, &prop);
}
