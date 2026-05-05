#include "interpreter.hpp"
#include "term.hpp"

#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
	Interpreter interpreter;

	if (argc == 2) {
		interpreter.load_from_file(argv[1]);
	}

	Terminal::begin();
	std::thread t([&]() { interpreter.run(true); });
	try {
		while (true) {
			unsigned char input = static_cast<unsigned char>(std::cin.get());
			if (interpreter.should_quit()) {
				break;
			}
			else if (interpreter.waiting_input()) {
				interpreter.send_input(input);
			}
			else {
				switch (input) {
					case 'q':
						interpreter.interrupt();
						break;
					case 's':
						interpreter.send_step();
						break;
					case ' ':
						interpreter.toggle_pause();
						break;
				}
			}
		}
		t.join();
	}
	catch (std::exception& e) {
		std::cerr << "\x1b[1;91m[ERROR]\x1b[m " << e.what();
	}
	Terminal::end();
}
