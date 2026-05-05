#include "term.hpp"

#include <iostream>
#include <termios.h>
#include <unistd.h>

struct termios old_term = {0};

void Terminal::begin()
{
	struct termios tio = {0};
	tcgetattr(STDOUT_FILENO, &tio);
	old_term = tio;
	cfmakeraw(&tio);
	tcsetattr(STDOUT_FILENO, TCSADRAIN, &tio);

	std::cout << "\x1b[?1049h"
	          << "\x1b[?25l"
	          << "\x1b[1;1H";
}

void Terminal::end()
{
	std::cout << "\x1b[?25h"
	          << "\x1b[?1049l" << std::flush;

	tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term);
}
