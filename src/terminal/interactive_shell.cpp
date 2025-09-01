#include <cortan/terminal/interactive_shell.hpp>
#include <iostream>

namespace cortan::terminal {

InteractiveShell::InteractiveShell() : running_(false) {
    // TODO: Initialize interactive shell
}

InteractiveShell::~InteractiveShell() = default;

void InteractiveShell::start() {
    running_ = true;
    std::cout << "Cortan Orchestrator Interactive Shell\n";
    std::cout << "Type 'help' for commands or 'exit' to quit.\n";

    while (running_) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit" || input == "quit") {
            running_ = false;
        } else if (input == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  help  - Show this help message\n";
            std::cout << "  exit  - Exit the shell\n";
        } else {
            std::cout << "Unknown command: " << input << "\n";
        }
    }
}

void InteractiveShell::stop() {
    running_ = false;
}

} // namespace cortan::terminal
