#pragma once

#include <string>
#include <vector>
#include <memory>
#include <future>

namespace cortan::ai {

class ModelInterface {
public:
    virtual ~ModelInterface() = default;

    virtual std::string getName() const = 0;
    virtual bool isLoaded() const = 0;
    virtual std::future<std::pair<bool, std::string>> processAsync(const std::string& prompt) = 0;
};

class OllamaModel : public ModelInterface {
public:
    OllamaModel(std::string name, std::string endpoint = "http://localhost:11434");

    std::string getName() const override;
    bool isLoaded() const override;
    std::future<std::pair<bool, std::string>> processAsync(const std::string& prompt) override;

private:
    std::string name_;
    std::string endpoint_;
    bool loaded_ = false;
};

class ModelManager {
public:
    ModelManager();
    ~ModelManager();

    void addModel(std::unique_ptr<ModelInterface> model);
    ModelInterface* getModel(const std::string& name);
    std::vector<std::string> getAvailableModels() const;

        std::future<std::pair<bool, std::string>> processRequest(
        const std::string& model_name,
        const std::string& prompt
    );

private:
    std::vector<std::unique_ptr<ModelInterface>> models_;
};

} // namespace cortan::ai
