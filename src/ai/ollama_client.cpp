#include <cortan/ai/ollama_client.hpp>

namespace cortan::ai {

OllamaClient::OllamaClient(std::string endpoint) : endpoint_(std::move(endpoint)) {
    // TODO: Initialize Ollama client
}

OllamaClient::~OllamaClient() = default;

std::vector<std::string> OllamaClient::list_models() {
    // TODO: Implement model listing from Ollama
    return {"llama3:8b", "codellama:latest"};
}

std::future<std::pair<bool, std::string>> OllamaClient::generate(
    const std::string& model,
    const std::string& prompt) {

    return std::async(std::launch::async, [model, prompt]() -> std::pair<bool, std::string> {
        // TODO: Implement actual Ollama API call
        return {true, "Generated response from " + model + " for prompt: " + prompt};
    });
}

} // namespace cortan::ai
