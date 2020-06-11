#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int main(void)
{
	while (1) {
		int c = 0;
		static struct termios oldt = {};
		static struct termios newt = {};

		// Don't wait for Enter (buffer)
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON);
		tcsetattr( STDIN_FILENO, TCSANOW, &newt);

		c = getchar();
		printf("%u 0x%x\n", (unsigned char)c, (unsigned char)c);

		// Restore terminal
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
	}

	return 0;
}

