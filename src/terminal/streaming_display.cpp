#include <cortan/terminal/streaming_display.hpp>
#include <iostream>

namespace cortan::terminal {

StreamingDisplay::StreamingDisplay() {
    // TODO: Initialize streaming display
}

StreamingDisplay::~StreamingDisplay() = default;

void StreamingDisplay::display_message(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void StreamingDisplay::display_error(const std::string& error) {
    std::cerr << "[ERROR] " << error << std::endl;
}

void StreamingDisplay::display_progress(float progress) {
    std::cout << "\rProgress: " << static_cast<int>(progress * 100) << "%" << std::flush;
}

} // namespace cortan::terminal
