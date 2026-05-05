#include "interpreter.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

inline bool isValidChar(const char a_char)
{
	for (const char valid_c : "-+,.[]<>") {
		if (a_char == valid_c) {
			return true;
		}
	}
	return false;
}

std::string state_to_string(State a_state)
{
	switch (a_state) {
		case State::Stopped:
			return "STOPPED";
		case State::Paused:
			return "PAUSED";
		case State::Running:
			return "RUNNING";
		case State::Interrupted:
			return "INTERRUPTED";
		case State::WaitingInput:
			return "WAITING_INPUT";
	}
	return "UNKNOWN";
}

void Interpreter::print_info() const
{
	std::stringstream buffer;

	// Clear screen
	buffer << "\x1b[2J"
	       << "\x1b[1;1H";

	// Source code
	for (unsigned long int i = 0; i != m_code.size(); ++i) {
		buffer << ((i == m_instruction_pointer) ? "\x1b[1;30;103m" : "\x1b[1;93m")
		       << m_code[i] << "\x1b[m";
		if (m_code[i] == '\n') {
			buffer << '\r';
		}
	}

	// Separator
	buffer << "\r\n\n";

	// Vector
	for (unsigned long int i = 0; i != m_vector.size(); ++i) {
		buffer << "[" << (i == m_head ? "\x1b[1;30;103m" : "\x1b[1;93m") << " "
		       << static_cast<int>(m_vector[i]) << " \x1b[m] ";
	}

	// Separator
	buffer << "\r\n\n----------\r\n\n";

	// Output
	buffer << "\x1b[1;93m" << m_output.str() << "\x1b[m";

	// Separator
	buffer << "\r\n\n";

	// State (if necessary)
	if (m_state != State::Running) {
		buffer << "\x1b[1;93m[" << state_to_string(m_state) << "]\x1b[m";
	}

	// Flush buffer to screen
	std::cout << buffer.str() << std::flush;
}

Interpreter::Interpreter()
{
	m_input_m.lock();
}

bool Interpreter::should_quit() const
{
	return m_state == State::Stopped || m_state == State::Interrupted;
}

void Interpreter::load_from_file(const std::string& a_filename)
{
	std::ifstream file(a_filename);
	if (file) {
		std::stringstream sstr;
		sstr << file.rdbuf();
		load_from_string(sstr.str());
		file.close();
	}
}

void Interpreter::load_from_string(const std::string& a_code)
{
	m_code = a_code;
	while (!isValidChar(m_code[m_instruction_pointer])) {
		++m_instruction_pointer;
	}
}

std::string Interpreter::run(const bool a_verbose)
{
	m_state = State::Running;
	while (m_state == State::Running || m_state == State::Paused) {
		step();
		if (a_verbose) {
			print_info();
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(50ms);
		}
	}
	if (a_verbose) {
		std::cout << "\r\nPress \x1b[1;93many key\x1b[m to exit" << std::flush;
	}
	return m_output.str();
}

void Interpreter::step()
{
	if (m_head == static_cast<size_t>(-1)) {
		m_vector.insert(m_vector.begin(), 0);
		m_head = 0;
	}
	else if (m_head >= m_vector.size()) {
		m_vector.resize(m_head + 1);
		m_vector[m_head] = 0;
	}

	if (m_state == State::Paused) {
		if (m_steps > 0)
			m_steps--;
		else
			return;
	}

	switch (m_code[m_instruction_pointer]) {
		case '+':
			++m_vector[m_head];
			break;
		case '-':
			--m_vector[m_head];
			break;
		case '>':
			++m_head;
			break;
		case '<':
			--m_head;
			break;
		case ',': {
				auto old_state = m_state;
				m_state = State::WaitingInput;
				m_input_m.lock();
				m_state = old_state;
			}
			m_vector[m_head] = m_input;
			if (m_vector[m_head] >= '0' && m_vector[m_head] <= '9') {
				m_vector[m_head] -= '0';
			}
			break;
		case '.':
			m_output << static_cast<char>(m_vector[m_head]);
			if (static_cast<char>(m_vector[m_head]) == '\n') {
				m_output << '\r';
			}
			break;
		case '[':
			m_return_stack.push(m_instruction_pointer);
			break;
		case ']':
			if (m_vector[m_head] != 0) {
				if (!m_return_stack.empty()) {
					m_instruction_pointer = m_return_stack.top();
				}
				else {
					throw(std::runtime_error("No loop to return to"));
				}
			}
			else {
				m_return_stack.pop();
			}
			break;
		default:
			m_state = State::Stopped;
			break;
	}
	do {
		++m_instruction_pointer;
	} while (m_instruction_pointer < m_code.size() &&
	         !isValidChar(m_code[m_instruction_pointer]));
}

void Interpreter::send_input(unsigned char a_input)
{
	m_input = a_input;
	m_input_m.unlock();
}

bool Interpreter::waiting_input() const
{
	return m_state == State::WaitingInput;
}

void Interpreter::interrupt()
{
	m_state = State::Interrupted;
}

void Interpreter::toggle_pause()
{
	if (m_state == State::Paused) {
		m_state = State::Running;
	}
	else if (m_state == State::Running) {
		m_state = State::Paused;
	}
}

void Interpreter::send_step()
{
	if (m_state == State::Paused)
		m_steps = 1;
}
