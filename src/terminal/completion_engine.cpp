#include <cortan/terminal/completion_engine.hpp>

namespace cortan::terminal {

CompletionEngine::CompletionEngine() {
    // TODO: Initialize completion engine
}

CompletionEngine::~CompletionEngine() = default;

std::vector<std::string> CompletionEngine::get_completions(const std::string& prefix) {
    // TODO: Implement command completion
    return {};
}

void CompletionEngine::add_command(const std::string& command) {
    // TODO: Implement command addition
}

} // namespace cortan::terminal
