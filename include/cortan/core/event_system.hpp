#pragma once

#include <chrono>
#include <string>
#include <functional>
#include <memory>
#include <future>
#include <variant>
#include <unordered_map>
#include <vector>
#include <optional>

namespace cortan::core {

// ============================================================================
// Event Priority System (Cortana-style)
// ============================================================================

enum class EventPriority {
    CRITICAL = 0,    // Mission-critical, immediate response required
    HIGH = 1,        // Important user requests, security alerts
    NORMAL = 2,      // Standard interactions, routine tasks
    LOW = 3,         // Background tasks, suggestions, learning
    BACKGROUND = 4   // Maintenance, cleanup, passive monitoring
};

// ============================================================================
// Event Context (Cortana's Situational Awareness)
// ============================================================================

struct EventContext {
    std::string user_id;
    std::string session_id;
    std::string location_context;  // "mission_control", "field_ops", "personal_time"
    std::string emotional_state;   // "focused", "concerned", "playful", "exhausted"
    std::unordered_map<std::string, std::string> metadata;

    // Cortana's personality traits
    float urgency_level = 0.5f;        // 0.0 = casual, 1.0 = emergency
    float familiarity_level = 0.3f;    // How well Cortana knows the user
    bool is_proactive_suggestion = false;
    std::optional<std::string> related_mission;
};

// ============================================================================
// Enhanced Base Event (Cortana-capable)
// ============================================================================

class BaseEvent {
public:
    virtual ~BaseEvent() = default;

    // Core timing and tracking
    std::chrono::system_clock::time_point timestamp() const {
        return creation_time_;
    }

    const std::string& getCorrelationId() const {
        return correlation_id_;
    }

    // Cortana-specific enhancements
    EventPriority getPriority() const { return priority_; }
    const EventContext& getContext() const { return context_; }
    const std::string& getEventType() const { return event_type_; }

    // Cortana's predictive capabilities
    virtual bool requiresImmediateResponse() const {
        return priority_ <= EventPriority::HIGH;
    }

    virtual bool isProactiveSuggestion() const {
        return context_.is_proactive_suggestion;
    }

    virtual std::string getCortanaResponseStyle() const {
        // Based on context, determine how Cortana should respond
        if (context_.emotional_state == "playful" && context_.urgency_level < 0.3f) {
            return "witty";
        }
        if (priority_ == EventPriority::CRITICAL) {
            return "urgent";
        }
        if (context_.familiarity_level > 0.7f) {
            return "personal";
        }
        return "professional";
    }

    // Public factory method for creating BaseEvent instances
    static std::shared_ptr<BaseEvent> create(std::string event_type,
                                           EventPriority priority = EventPriority::NORMAL,
                                           EventContext context = {}) {
        struct BaseEventFactory : public BaseEvent {
            BaseEventFactory(std::string type, EventPriority pri, EventContext ctx)
                : BaseEvent(std::move(type), pri, std::move(ctx)) {}
        };
        return std::make_shared<BaseEventFactory>(std::move(event_type), priority, std::move(context));
    }

protected:
    BaseEvent(std::string event_type,
              EventPriority priority = EventPriority::NORMAL,
              EventContext context = {});

private:
    std::chrono::system_clock::time_point creation_time_;
    std::string correlation_id_;
    std::string event_type_;
    EventPriority priority_;
    EventContext context_;
};

// ============================================================================
// Specialized Event Types (Cortana's Communication Modes)
// ============================================================================

// User Interaction Events
class UserRequestEvent : public BaseEvent {
public:
    enum class RequestType { COMMAND, QUESTION, STATEMENT, EMOTION };

    UserRequestEvent(std::string content,
                     RequestType type,
                     EventContext context = {});

    const std::string& getContent() const { return content_; }
    RequestType getRequestType() const { return type_; }

private:
    std::string content_;
    RequestType type_;
};

// AI Processing Events
class AIProcessingEvent : public BaseEvent {
public:
    enum class ProcessingStage { STARTED, PROGRESS, COMPLETED, FAILED };

    AIProcessingEvent(std::string task_id,
                     ProcessingStage stage,
                     std::string details = "",
                     EventContext context = {});

    const std::string& getTaskId() const { return task_id_; }
    ProcessingStage getStage() const { return stage_; }
    const std::string& getDetails() const { return details_; }

private:
    std::string task_id_;
    ProcessingStage stage_;
    std::string details_;
};

// Environmental Awareness Events
class EnvironmentalEvent : public BaseEvent {
public:
    enum class EnvironmentType { USER_STATE, SYSTEM_STATUS, EXTERNAL_THREAT, OPPORTUNITY };

    EnvironmentalEvent(EnvironmentType type,
                      std::string description,
                      std::unordered_map<std::string, std::string> sensor_data = {},
                      EventContext context = {});

    EnvironmentType getEnvironmentType() const { return type_; }
    const std::string& getDescription() const { return description_; }
    const std::unordered_map<std::string, std::string>& getSensorData() const { return sensor_data_; }

private:
    EnvironmentType type_;
    std::string description_;
    std::unordered_map<std::string, std::string> sensor_data_;
};

// Learning and Adaptation Events
class LearningEvent : public BaseEvent {
public:
    enum class LearningType { USER_PREFERENCE, BEHAVIOR_PATTERN, SKILL_ACQUISITION, RELATIONSHIP_UPDATE };

    LearningEvent(LearningType type,
                  std::string insight,
                  float confidence_level,
                  EventContext context = {});

    LearningType getLearningType() const { return type_; }
    const std::string& getInsight() const { return insight_; }
    float getConfidenceLevel() const { return confidence_level_; }

private:
    LearningType type_;
    std::string insight_;
    float confidence_level_;
};

// ============================================================================
// Enhanced EventBus (Cortana's Intelligence Core)
// ============================================================================

class EventBus {
public:
    using EventHandler = std::function<std::future<void>(const BaseEvent&)>;
    using FilteredHandler = std::function<std::future<void>(const BaseEvent&, const EventContext&)>;

    EventBus();
    ~EventBus();

    // Standard subscription
    void subscribe(const std::string& event_type, EventHandler handler);

    // Cortana-specific subscriptions with context awareness
    void subscribeWithContext(const std::string& event_type, FilteredHandler handler);
    void subscribePriority(EventPriority priority, EventHandler handler);
    void subscribeUrgent(EventHandler handler); // For critical situations

    // Enhanced publishing with Cortana intelligence
    std::future<void> publish(const std::string& event_type, std::shared_ptr<BaseEvent> event);

    // Cortana's predictive publishing
    std::future<void> publishProactive(std::string suggestion,
                                      EventContext context,
                                      EventPriority priority = EventPriority::LOW);

    // Emergency override (Cortana takes control)
    void publishEmergency(const std::string& emergency_message,
                         const std::string& mission_context = "");

    // Context management
    void updateUserContext(const std::string& user_id, const EventContext& context);
    std::optional<EventContext> getUserContext(const std::string& user_id) const;

    // Cortana's situational awareness
    void setGlobalContext(const EventContext& context);
    EventContext getGlobalContext() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// ============================================================================
// Cortana-Specific Event Factory Functions
// ============================================================================

namespace cortana_events {

// User interaction helpers
std::shared_ptr<UserRequestEvent> createUserCommand(const std::string& command,
                                                   const std::string& user_id = "default");

std::shared_ptr<UserRequestEvent> createUserQuestion(const std::string& question,
                                                    const std::string& user_id = "default");

std::shared_ptr<UserRequestEvent> createCasualConversation(const std::string& message,
                                                          const std::string& user_id = "default");

// AI processing helpers
std::shared_ptr<AIProcessingEvent> createTaskStarted(const std::string& task_id,
                                                    const std::string& description = "");

std::shared_ptr<AIProcessingEvent> createTaskProgress(const std::string& task_id,
                                                     const std::string& progress_info);

std::shared_ptr<AIProcessingEvent> createTaskCompleted(const std::string& task_id,
                                                      const std::string& result = "");

// Environmental awareness helpers
std::shared_ptr<EnvironmentalEvent> createUserStateChange(const std::string& new_state,
                                                         const std::string& user_id = "default");

std::shared_ptr<EnvironmentalEvent> createSystemAlert(const std::string& alert_message,
                                                     EventPriority priority = EventPriority::HIGH);

// Learning helpers
std::shared_ptr<LearningEvent> createUserPreference(const std::string& preference,
                                                   float confidence = 0.8f);

std::shared_ptr<LearningEvent> createBehaviorPattern(const std::string& pattern,
                                                    float confidence = 0.7f);

} // namespace cortana_events

} // namespace cortan::core
