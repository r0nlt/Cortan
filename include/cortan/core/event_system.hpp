#pragma once

#include <chrono>
#include <string>
#include <functional>
#include <memory>
#include <future>

namespace cortan::core {

class BaseEvent {
public:
    virtual ~BaseEvent() = default;

    std::chrono::system_clock::time_point timestamp() const {
        return creation_time_;
    }

    const std::string& getCorrelationId() const {
        return correlation_id_;
    }

protected:
    BaseEvent();

private:
    std::chrono::system_clock::time_point creation_time_;
    std::string correlation_id_;
};

class EventBus {
public:
    using EventHandler = std::function<std::future<void>(const BaseEvent&)>;

    void subscribe(const std::string& event_type, EventHandler handler);
    std::future<void> publish(const std::string& event_type, std::shared_ptr<BaseEvent> event);

private:
    // Implementation details
};

} // namespace cortan::core
