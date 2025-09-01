#include <cortan/ai/model_manager.hpp>
#include <algorithm>

namespace cortan::ai {

OllamaModel::OllamaModel(std::string name, std::string endpoint)
    : name_(std::move(name)), endpoint_(std::move(endpoint)) {
}

std::string OllamaModel::getName() const {
    return name_;
}

bool OllamaModel::isLoaded() const {
    return loaded_;
}

std::future<std::pair<bool, std::string>> OllamaModel::processAsync(const std::string& prompt) {
    return std::async(std::launch::async, [this, prompt]() -> std::pair<bool, std::string> {
        // TODO: Implement actual Ollama API call
        return {true, "Response from " + name_ + " to: " + prompt};
    });
}

ModelManager::ModelManager() = default;
ModelManager::~ModelManager() = default;

void ModelManager::addModel(std::unique_ptr<ModelInterface> model) {
    models_.push_back(std::move(model));
}

ModelInterface* ModelManager::getModel(const std::string& name) {
    auto it = std::find_if(models_.begin(), models_.end(),
        [&name](const auto& model) {
            return model->getName() == name;
        });

    return it != models_.end() ? it->get() : nullptr;
}

std::vector<std::string> ModelManager::getAvailableModels() const {
    std::vector<std::string> names;
    for (const auto& model : models_) {
        names.push_back(model->getName());
    }
    return names;
}

std::future<std::pair<bool, std::string>> ModelManager::processRequest(
    const std::string& model_name,
    const std::string& prompt) {

    auto* model = getModel(model_name);
    if (!model) {
        return std::async(std::launch::deferred, []() -> std::pair<bool, std::string> {
            return {false, "Model not found"};
        });
    }

    return model->processAsync(prompt);
}

} // namespace cortan::ai
