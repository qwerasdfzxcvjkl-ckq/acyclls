#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>

const struct input_event 
syn = {.type = EV_SYN , .code = SYN_REPORT, .value = 0}, 
	alt_up = {.type = EV_KEY , .code = KEY_LEFTALT, .value = 0}, 
	alt_down = {.type = EV_KEY , .code = KEY_LEFTALT, .value = 1}, 
	shift_up = {.type = EV_KEY , .code = KEY_LEFTSHIFT, .value = 0}, 
	shift_down = {.type = EV_KEY , .code = KEY_LEFTSHIFT, .value = 1}; 

enum {
	PRESSED,
	RELEASED,
	START,
	BOTH_HELD,
	FINAL,
};

void
write_event_with_delay_and_syn(const struct input_event *event)
{
	const int delay = 20000;
	usleep(delay);
	fwrite(event, sizeof(*event), 1, stdout);
	fwrite(&syn, sizeof(*event), 1, stdout);
}

int 
main(void) 
{
	setbuf(stdin, NULL), setbuf(stdout, NULL);

	int shift_left_held = 0;
	int alt_left_held = 0;
	int used = 0;
	int lused = 1;
	int num_of_layouts = 3;


	int state = START;
	int alt_state_after_held = 0;
	int shift_state_after_held = 0;
	int either_pressed_again = 0;

	struct input_event event;
	while (fread(&event, sizeof(event), 1, stdin) == 1) {

		if (event.type == EV_MSC && event.code == MSC_SCAN)
			continue;

		if (event.type == EV_KEY && event.code == KEY_LEFTSHIFT) {
			shift_left_held = event.value;
		} else if (event.type == EV_KEY && event.code == KEY_LEFTALT) {
			alt_left_held = event.value;
		}

		switch (state) {
			case START: 
				{
					if (shift_left_held && alt_left_held) {
						state = BOTH_HELD;
					} 
				}
				break;

			case BOTH_HELD: 
				{
					if (!alt_left_held && !shift_left_held) {
						state = FINAL;
					} 

					if (!shift_left_held) {
						shift_state_after_held = RELEASED;
					} else if (!alt_left_held) {
						alt_state_after_held = RELEASED;
					} 

					if (alt_state_after_held == RELEASED) {
						if (alt_left_held) {
							alt_state_after_held = PRESSED;
							either_pressed_again = 1;
							lused = used;
							used = (used + 1) % num_of_layouts;
						}
					} else  if (shift_state_after_held == RELEASED) {
						if (shift_left_held) {
							shift_state_after_held = PRESSED;
							either_pressed_again = 1;
							lused = used;
							used = (used + 1) % num_of_layouts;
						}
					}

				}
				break;

			case FINAL: 
				{
					if (!either_pressed_again) {
						int n = lused > used ? lused - used - 1 : (lused + num_of_layouts) - used - 1;
						for (int i = 0; i < n; i++) {
							write_event_with_delay_and_syn(&alt_down);

							write_event_with_delay_and_syn(&shift_down);

							write_event_with_delay_and_syn(&alt_up);

							write_event_with_delay_and_syn(&shift_up);
						}

						int temp = used;
						used = lused;
						lused = temp;
					}

					alt_state_after_held = 0;
					shift_state_after_held = 0;
					either_pressed_again = 0;
					state = START;
				}
				break;
		}

		fwrite(&event, sizeof(event), 1, stdout);
	}
}
