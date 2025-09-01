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
// User Profile (Dynamic User Management)
// ============================================================================

struct UserPreferences {
    std::string preferred_greeting_style;  // "formal", "casual", "technical", "friendly"
    std::string time_format;              // "12h", "24h"
    std::string response_detail_level;     // "brief", "detailed", "comprehensive"
    std::unordered_map<std::string, std::string> custom_settings;
};

struct UserProfile {
    std::string user_id;
    std::string display_name;
    std::string email;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_seen;

    // Dynamic familiarity and interaction tracking
    float familiarity_level = 0.0f;        // 0.0 = new user, 1.0 = best friend
    int interaction_count = 0;
    std::chrono::system_clock::time_point first_interaction;

    // Personality and preferences
    UserPreferences preferences;
    std::string preferred_emotional_state;  // User's typical mood when interacting
    std::vector<std::string> interests;     // Topics the user is interested in
    std::unordered_map<std::string, int> interaction_patterns; // Tracks user behavior

    // Cortana's relationship with user
    std::string relationship_status;        // "acquaintance", "colleague", "friend", "confidant"
    std::vector<std::string> shared_memories; // Important interactions to remember
    std::unordered_map<std::string, float> topic_familiarity; // How familiar user is with topics

    // Methods for dynamic updates
    void updateFamiliarity(float interaction_quality = 1.0f) {
        // Gradual familiarity increase based on interactions
        float familiarity_boost = interaction_quality * 0.1f;
        familiarity_level = std::min(1.0f, familiarity_level + familiarity_boost);
        interaction_count++;
        last_seen = std::chrono::system_clock::now();

        // Update relationship status based on familiarity
        updateRelationshipStatus();
    }

    void updateRelationshipStatus() {
        if (familiarity_level >= 0.8f) {
            relationship_status = "confidant";
        } else if (familiarity_level >= 0.6f) {
            relationship_status = "friend";
        } else if (familiarity_level >= 0.3f) {
            relationship_status = "colleague";
        } else {
            relationship_status = "acquaintance";
        }
    }

    std::string getPersonalizedGreeting() const {
        if (familiarity_level > 0.8f) {
            return "old_friend";
        } else if (familiarity_level > 0.5f) {
            return "regular_user";
        } else {
            return "new_user";
        }
    }
};

// ============================================================================
// Event Context (Cortana's Situational Awareness)
// ============================================================================

struct EventContext {
    std::shared_ptr<UserProfile> user_profile;  // Dynamic user information
    std::string session_id;
    std::string location_context;  // "mission_control", "field_ops", "personal_time"
    std::string emotional_state;   // "focused", "concerned", "playful", "exhausted"
    std::unordered_map<std::string, std::string> metadata;

    // Cortana's personality traits (now context-aware)
    float urgency_level = 0.5f;        // 0.0 = casual, 1.0 = emergency
    bool is_proactive_suggestion = false;
    std::optional<std::string> related_mission;

    // Convenience accessors
    std::string getUserId() const {
        return user_profile ? user_profile->user_id : "unknown";
    }

    float getFamiliarityLevel() const {
        return user_profile ? user_profile->familiarity_level : 0.0f;
    }

    std::string getPreferredGreetingStyle() const {
        return user_profile ? user_profile->preferences.preferred_greeting_style : "casual";
    }
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
        // Based on context and user profile, determine how Cortana should respond
        auto familiarity = context_.getFamiliarityLevel();
        auto greeting_style = context_.getPreferredGreetingStyle();

        if (context_.emotional_state == "playful" && context_.urgency_level < 0.3f) {
            return "witty";
        }
        if (priority_ == EventPriority::CRITICAL) {
            return "urgent";
        }
        if (familiarity > 0.7f) {
            return greeting_style == "formal" ? "personal_formal" : "personal";
        }
        if (greeting_style == "technical") {
            return "technical";
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

// Welcome and User Experience Events
class WelcomeEvent : public BaseEvent {
public:
    enum class WelcomeType { SYSTEM_STARTUP, USER_LOGIN, SESSION_RESUME, CONTEXT_CHANGE };

    WelcomeEvent(WelcomeType type,
                 std::string message,
                 std::string user_id,
                 EventContext context = {});

    WelcomeType getWelcomeType() const { return type_; }
    const std::string& getMessage() const { return message_; }
    const std::string& getTargetUserId() const { return target_user_id_; }

private:
    WelcomeType type_;
    std::string message_;
    std::string target_user_id_;
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
// User Manager (Dynamic User Profile Management)
// ============================================================================

class UserManager {
public:
    UserManager();
    ~UserManager();

    // User profile management
    std::shared_ptr<UserProfile> getOrCreateUserProfile(const std::string& user_id);
    std::shared_ptr<UserProfile> getUserProfile(const std::string& user_id) const;
    bool saveUserProfile(const std::shared_ptr<UserProfile>& profile);
    bool deleteUserProfile(const std::string& user_id);

    // User preference updates
    void updateUserFamiliarity(const std::string& user_id, float interaction_quality = 1.0f);
    void updateUserPreferences(const std::string& user_id, const UserPreferences& preferences);

    // Profile queries
    std::vector<std::string> getAllUserIds() const;
    std::vector<std::shared_ptr<UserProfile>> getActiveUsers() const;

    // Persistence
    bool loadUserProfiles(const std::string& config_path = "config/users.json");
    bool saveUserProfiles(const std::string& config_path = "config/users.json");

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// ============================================================================
// User Factory Functions (Create Dynamic User Profiles)
// ============================================================================

namespace user_factory {

std::shared_ptr<UserProfile> createNewUser(const std::string& user_id,
                                         const std::string& display_name = "");

std::shared_ptr<UserProfile> createKnownUser(const std::string& user_id,
                                           const std::string& display_name,
                                           float familiarity_level,
                                           const UserPreferences& preferences);

std::shared_ptr<UserProfile> createDefaultUser(const std::string& user_id = "guest");

// Specialized user templates
std::shared_ptr<UserProfile> createDeveloperUser(const std::string& user_id,
                                               const std::string& display_name);

std::shared_ptr<UserProfile> createResearcherUser(const std::string& user_id,
                                                const std::string& display_name);

std::shared_ptr<UserProfile> createStudentUser(const std::string& user_id,
                                             const std::string& display_name);

} // namespace user_factory

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

// Welcome helpers
std::shared_ptr<WelcomeEvent> createSystemWelcome(const std::string& user_id,
                                                 const std::string& message);

std::shared_ptr<WelcomeEvent> createPersonalizedWelcome(const std::string& user_id,
                                                       const EventContext& context);

} // namespace cortana_events

} // namespace cortan::core
