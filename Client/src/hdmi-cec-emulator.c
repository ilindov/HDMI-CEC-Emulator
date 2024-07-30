#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

Display *display;
unsigned int keycode;
// FILE *log_file;
int serial_port;
pthread_t stop_thread_id;
volatile int stop_pending = 0;

void signal_handler (int signal) {
	// fprintf(log_file, "Terminating... :)\n");
	XCloseDisplay(display);
	// fclose(log_file);
	close(serial_port);
	exit(0);
}

void hitKey(int key) {
	keycode = XKeysymToKeycode(display, key);
	XTestFakeKeyEvent(display, keycode, True, 0);
	XTestFakeKeyEvent(display, keycode, False, 10);
	XFlush(display);
}

void *kill_timer(void *args) {
	stop_pending = 1;
	sleep(10);
	system("/usr/bin/flatpak kill tv.kodi.Kodi");
	stop_pending = 0;
}

int main() {
	signal(SIGTERM, signal_handler);

	// log_file = fopen("/tmp/log.log", "a");
	serial_port = open("/dev/ttyUSB0", O_RDWR);
	if (serial_port < 0) {
    	// fprintf(log_file, "Error %i from open: %s\n", errno, strerror(errno));
	}
	else
	{
		// fprintf(log_file, "Port opened!\n");
	}

	// Create new termios struct
	struct termios tty;
	memset(&tty, 0, sizeof tty);

	// Read in existing settings, and handle any error
	if(tcgetattr(serial_port, &tty) != 0) {
		// fprintf(log_file, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}
	
	tty.c_cflag &= ~PARENB; // Clear parity bit
	tty.c_cflag &= ~CSTOPB; // Use one stop bit
	tty.c_cflag |= CS8; // 8 bits per byte
	tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	tty.c_lflag &= ICANON; // Enable canonical mode, i.e. process after \n is received.
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 0;
	cfsetispeed(&tty, B115200);
	cfsetospeed(&tty, B115200);

	// Saving termios settings
	if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
		// fprintf(log_file, "Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}

	display = XOpenDisplay(NULL);
	
	char read_buf [256];
	int n;
	while(1) {
		memset(read_buf, 0, sizeof(read_buf));
		n = read(serial_port, &read_buf, sizeof(read_buf));
		if (n == -1) continue;

		read_buf[strlen(read_buf) - 2] = '\0';
		// fprintf(log_file, "%ld -> '%s'\n",strlen(read_buf), read_buf);
		// fprintf(log_file, "%d -> '%s'\n",n, read_buf);
		
		if (strcmp(read_buf, "KEY_DOWN_ARROW") == 0) {
			hitKey(XK_Down);
		}
		else if (strcmp(read_buf, "KEY_UP_ARROW") == 0) {
			hitKey(XK_Up);
		}
		else if (strcmp(read_buf, "KEY_LEFT_ARROW") == 0) {
			hitKey(XK_Left);
		}
		else if (strcmp(read_buf, "KEY_RIGHT_ARROW") == 0) {
			hitKey(XK_Right);
		}
		else if (strcmp(read_buf, "KEY_RETURN") == 0) {
			hitKey(XK_Return);
		}
		else if (strcmp(read_buf, "KEY_ESC") == 0) {
			hitKey(XK_BackSpace);
		}
		else if (strcmp(read_buf, "KEY_FFWD") == 0) {
			hitKey(XK_F);
		}
		else if (strcmp(read_buf, "KEY_RWND") == 0) {
			hitKey(XK_R);
		}
		else if (strcmp(read_buf, "KEY_PLAY") == 0) {
			hitKey(XK_P);
		}
		else if (strcmp(read_buf, "KEY_STOP") == 0) {
			hitKey(XK_X);
		}
		else if (strcmp(read_buf, "KEY_PAUSE") == 0) {
			hitKey(XK_space);
		}
		else if (strcmp(read_buf, "KEY_INFO") == 0) {
			hitKey(XK_I);
		}
		else if (strcmp(read_buf, "KEY_OPTIONS") == 0) {
			hitKey(XK_C);
		}
		else if (strcmp(read_buf, "KEY_SUBS") == 0) {
			hitKey(XK_L);
		}
		else if (strcmp(read_buf, "KEY_HOME") == 0) {
			hitKey(XK_Escape);
		}
		else if (strcmp(read_buf, "KEY_TT_GREEN") == 0) {
			hitKey(XK_Tab);
		}
		else if (strcmp(read_buf, "KEY_GUIDE") == 0) {
			hitKey(XK_E);
		}
		else if (strcmp(read_buf, "FOCUS_GAINED") == 0) {
			if (stop_pending) {
				pthread_kill(stop_thread_id, SIGKILL);
			}
			else {
				if (system("flatpak ps | grep 'tv.kodi.Kodi'"))
					system("env DISPLAY=:0 XAUTHORITY=/home/kodi/.Xauthority /usr/bin/flatpak run --branch=stable --arch=x86_64 --command=kodi tv.kodi.Kodi &");
			}
		}
		else if (strcmp(read_buf, "FOCUS_LOST") == 0) {
			if (!stop_pending) {
				pthread_create(&stop_thread_id, NULL, kill_timer, NULL);
			}
			else {
				pthread_kill(stop_thread_id, SIGKILL);
				pthread_create(&stop_thread_id, NULL, kill_timer, NULL);
			}
		}
	}
	pthread_exit(NULL);
}
