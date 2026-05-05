#pragma once

#include <mutex>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

enum class State
{
	Stopped,
	Paused,
	Running,
	Interrupted,
	WaitingInput
};

class Interpreter {
 private:
	State m_state {State::Stopped};
	size_t m_head {0};
	size_t m_steps {0};
	size_t m_instruction_pointer {0};
	std::string m_code {std::string("++++++[>++++++++<-]++++++++++[>.+<-]")};
	std::stringstream m_output;
	std::stack<size_t> m_return_stack {};
	std::vector<uint8_t> m_vector {};
	std::mutex m_input_m;
	unsigned char m_input;

	void print_info() const;

 public:
	Interpreter();

	bool should_quit() const;
	bool waiting_input() const;

	void load_from_file(const std::string& filename);
	void load_from_string(const std::string& code);

	std::string run(const bool verbose = false);
	void step();
	void interrupt();
	void toggle_pause();
	void send_input(unsigned char input);
	void send_step();
};
