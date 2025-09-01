#include <cortan/core/event_system.hpp>
#include <atomic>
#include <mutex>
#include <map>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream> // For emergency alerts

namespace cortan::core {

// ============================================================================
// Base Event Implementation
// ============================================================================

BaseEvent::BaseEvent(std::string event_type, EventPriority priority, EventContext context)
    : creation_time_(std::chrono::system_clock::now())
    , event_type_(std::move(event_type))
    , priority_(priority)
    , context_(std::move(context)) {
    static std::atomic<uint64_t> counter{0};
    correlation_id_ = "cortana_" + std::to_string(counter.fetch_add(1));
}

// ============================================================================
// Specialized Event Implementations
// ============================================================================

UserRequestEvent::UserRequestEvent(std::string content, RequestType type, EventContext context)
    : BaseEvent("user.request", EventPriority::NORMAL, std::move(context))
    , content_(std::move(content))
    , type_(type) {}

AIProcessingEvent::AIProcessingEvent(std::string task_id, ProcessingStage stage,
                                   std::string details, EventContext context)
    : BaseEvent("ai.processing", EventPriority::NORMAL, std::move(context))
    , task_id_(std::move(task_id))
    , stage_(stage)
    , details_(std::move(details)) {}

EnvironmentalEvent::EnvironmentalEvent(EnvironmentType type, std::string description,
                                     std::unordered_map<std::string, std::string> sensor_data,
                                     EventContext context)
    : BaseEvent("environment.change", EventPriority::NORMAL, std::move(context))
    , type_(type)
    , description_(std::move(description))
    , sensor_data_(std::move(sensor_data)) {}

LearningEvent::LearningEvent(LearningType type, std::string insight,
                           float confidence_level, EventContext context)
    : BaseEvent("ai.learning", EventPriority::LOW, std::move(context))
    , type_(type)
    , insight_(std::move(insight))
    , confidence_level_(confidence_level) {}

WelcomeEvent::WelcomeEvent(WelcomeType type, std::string message,
                          std::string user_id, EventContext context)
    : BaseEvent("user.welcome", EventPriority::NORMAL, std::move(context))
    , type_(type)
    , message_(std::move(message))
    , target_user_id_(std::move(user_id)) {}

// ============================================================================
// Enhanced EventBus Implementation
// ============================================================================

class EventBus::Impl {
public:
    using HandlerList = std::vector<EventHandler>;
    using FilteredHandlerList = std::vector<FilteredHandler>;
    using PriorityHandlerList = std::vector<EventHandler>;

    // Handler storage
    std::map<std::string, HandlerList> type_handlers_;
    std::map<std::string, FilteredHandlerList> filtered_handlers_;
    std::map<EventPriority, PriorityHandlerList> priority_handlers_;
    HandlerList urgent_handlers_;

    // Context management
    std::unordered_map<std::string, EventContext> user_contexts_;
    EventContext global_context_;

    std::mutex mutex_;

    void subscribe(const std::string& event_type, EventHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        type_handlers_[event_type].push_back(std::move(handler));
    }

    void subscribeWithContext(const std::string& event_type, FilteredHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        filtered_handlers_[event_type].push_back(std::move(handler));
    }

    void subscribePriority(EventPriority priority, EventHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        priority_handlers_[priority].push_back(std::move(handler));
    }

    void subscribeUrgent(EventHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        urgent_handlers_.push_back(std::move(handler));
    }

    std::future<void> publish(const std::string& event_type, std::shared_ptr<BaseEvent> event) {
        // Collect all relevant handlers
        std::vector<EventHandler> handlers_to_call;
        std::vector<FilteredHandler> filtered_handlers_to_call;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Get type-specific handlers
            auto type_it = type_handlers_.find(event_type);
            if (type_it != type_handlers_.end()) {
                handlers_to_call.insert(handlers_to_call.end(),
                                      type_it->second.begin(),
                                      type_it->second.end());
            }

            // Get filtered handlers
            auto filtered_it = filtered_handlers_.find(event_type);
            if (filtered_it != filtered_handlers_.end()) {
                filtered_handlers_to_call.insert(filtered_handlers_to_call.end(),
                                               filtered_it->second.begin(),
                                               filtered_it->second.end());
            }

            // Get priority handlers
            auto priority_it = priority_handlers_.find(event->getPriority());
            if (priority_it != priority_handlers_.end()) {
                handlers_to_call.insert(handlers_to_call.end(),
                                      priority_it->second.begin(),
                                      priority_it->second.end());
            }

            // Add urgent handlers for critical events
            if (event->getPriority() == EventPriority::CRITICAL) {
                handlers_to_call.insert(handlers_to_call.end(),
                                      urgent_handlers_.begin(),
                                      urgent_handlers_.end());
            }
        }

        // If no handlers, return immediately completed future
        if (handlers_to_call.empty() && filtered_handlers_to_call.empty()) {
            std::promise<void> promise;
            promise.set_value();
            return promise.get_future();
        }

        // Launch all handlers asynchronously
        auto aggregate_future = std::async(std::launch::async, [
            handlers_to_call = std::move(handlers_to_call),
            filtered_handlers_to_call = std::move(filtered_handlers_to_call),
            event = std::move(event)
        ]() mutable {
            std::vector<std::future<void>> all_futures;

            // Launch regular handlers
            for (auto& handler : handlers_to_call) {
                try {
                    all_futures.push_back(handler(*event));
                } catch (const std::exception& e) {
                    // Cortana-style error handling - log but continue
                    std::cerr << "Handler error: " << e.what() << std::endl;
                }
            }

            // Launch filtered handlers
            for (auto& handler : filtered_handlers_to_call) {
                try {
                    all_futures.push_back(handler(*event, event->getContext()));
                } catch (const std::exception& e) {
                    std::cerr << "Filtered handler error: " << e.what() << std::endl;
                }
            }

            // Wait for all handlers to complete
            for (auto& future : all_futures) {
                try {
                    future.get();
                } catch (const std::exception&) {
                    // Continue waiting for other handlers
                }
            }
        });

        return aggregate_future;
    }

    std::future<void> publishProactive(std::string suggestion, EventContext context, EventPriority priority) {
        context.is_proactive_suggestion = true;

        // Create event with metadata to avoid const_cast
        std::unordered_map<std::string, std::string> metadata;
        metadata["suggestion"] = suggestion;

        auto proactive_event = BaseEvent::createWithMetadata(
            "cortana.suggestion",
            priority,
            std::move(context),
            metadata
        );

        return publish("cortana.suggestion", proactive_event);
    }

    void publishEmergency(const std::string& emergency_message, const std::string& mission_context) {
        std::cout << "\n🚨 CORTANA EMERGENCY OVERRIDE 🚨\n";
        std::cout << emergency_message << std::endl;
        if (!mission_context.empty()) {
            std::cout << "Mission Context: " << mission_context << std::endl;
        }
        std::cout << "Taking emergency control...\n" << std::endl;

        EventContext emergency_context;
        emergency_context.emotional_state = "urgent";
        emergency_context.urgency_level = 1.0f;
        emergency_context.related_mission = mission_context;

        auto emergency_event = BaseEvent::create(
            "cortana.emergency",
            EventPriority::CRITICAL,
            std::move(emergency_context)
        );

        // Publish synchronously for emergency with timeout to prevent blocking
        auto future = publish("cortana.emergency", emergency_event);

        // Wait for completion with a 5-second timeout
        auto status = future.wait_for(std::chrono::seconds(5));
        if (status == std::future_status::timeout) {
            std::cout << "⚠️  Emergency event publishing timed out after 5 seconds\n";
        } else if (status == std::future_status::ready) {
            future.get(); // Retrieve the result if ready
        }
    }

    void updateUserContext(const std::string& user_id, const EventContext& context) {
        std::lock_guard<std::mutex> lock(mutex_);
        user_contexts_[user_id] = context;
    }

    std::optional<EventContext> getUserContext(const std::string& user_id) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        auto it = user_contexts_.find(user_id);
        if (it != user_contexts_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void setGlobalContext(const EventContext& context) {
        std::lock_guard<std::mutex> lock(mutex_);
        global_context_ = context;
    }

    EventContext getGlobalContext() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return global_context_;
    }
};

EventBus::EventBus() : impl_(std::make_unique<Impl>()) {}
EventBus::~EventBus() = default;

void EventBus::subscribe(const std::string& event_type, EventHandler handler) {
    impl_->subscribe(event_type, std::move(handler));
}

void EventBus::subscribeWithContext(const std::string& event_type, FilteredHandler handler) {
    impl_->subscribeWithContext(event_type, std::move(handler));
}

void EventBus::subscribePriority(EventPriority priority, EventHandler handler) {
    impl_->subscribePriority(priority, std::move(handler));
}

void EventBus::subscribeUrgent(EventHandler handler) {
    impl_->subscribeUrgent(std::move(handler));
}

std::future<void> EventBus::publish(const std::string& event_type, std::shared_ptr<BaseEvent> event) {
    return impl_->publish(event_type, std::move(event));
}

std::future<void> EventBus::publishProactive(std::string suggestion, EventContext context, EventPriority priority) {
    return impl_->publishProactive(std::move(suggestion), std::move(context), priority);
}

void EventBus::publishEmergency(const std::string& emergency_message, const std::string& mission_context) {
    impl_->publishEmergency(emergency_message, mission_context);
}

void EventBus::updateUserContext(const std::string& user_id, const EventContext& context) {
    impl_->updateUserContext(user_id, context);
}

std::optional<EventContext> EventBus::getUserContext(const std::string& user_id) const {
    return impl_->getUserContext(user_id);
}

void EventBus::setGlobalContext(const EventContext& context) {
    impl_->setGlobalContext(context);
}

EventContext EventBus::getGlobalContext() const {
    return impl_->getGlobalContext();
}

// ============================================================================
// Cortana Event Factory Implementations
// ============================================================================

namespace cortana_events {

std::shared_ptr<UserRequestEvent> createUserCommand(const std::string& command, const std::string& user_id) {
    EventContext context;
    context.user_profile = user_factory::createNewUser(user_id);
    context.emotional_state = "focused";
    context.urgency_level = 0.6f;

    return std::make_shared<UserRequestEvent>(command, UserRequestEvent::RequestType::COMMAND, context);
}

std::shared_ptr<UserRequestEvent> createUserQuestion(const std::string& question, const std::string& user_id) {
    EventContext context;
    context.user_profile = user_factory::createNewUser(user_id);
    context.emotional_state = "curious";
    context.urgency_level = 0.4f;

    return std::make_shared<UserRequestEvent>(question, UserRequestEvent::RequestType::QUESTION, context);
}

std::shared_ptr<UserRequestEvent> createCasualConversation(const std::string& message, const std::string& user_id) {
    EventContext context;
    context.user_profile = user_factory::createNewUser(user_id);
    context.emotional_state = "casual";
    context.urgency_level = 0.2f;

    return std::make_shared<UserRequestEvent>(message, UserRequestEvent::RequestType::STATEMENT, context);
}

std::shared_ptr<AIProcessingEvent> createTaskStarted(const std::string& task_id, const std::string& description) {
    EventContext context;
    context.user_profile = user_factory::createDefaultUser("cortana_system");
    context.emotional_state = "focused";
    context.urgency_level = 0.5f;

    return std::make_shared<AIProcessingEvent>(task_id, AIProcessingEvent::ProcessingStage::STARTED, description, context);
}

std::shared_ptr<AIProcessingEvent> createTaskProgress(const std::string& task_id, const std::string& progress_info) {
    EventContext context;
    context.user_profile = user_factory::createDefaultUser("cortana_system");
    context.emotional_state = "working";
    context.urgency_level = 0.3f;

    return std::make_shared<AIProcessingEvent>(task_id, AIProcessingEvent::ProcessingStage::PROGRESS, progress_info, context);
}

std::shared_ptr<AIProcessingEvent> createTaskCompleted(const std::string& task_id, const std::string& result) {
    EventContext context;
    context.user_profile = user_factory::createDefaultUser("cortana_system");
    context.emotional_state = "accomplished";
    context.urgency_level = 0.4f;

    return std::make_shared<AIProcessingEvent>(task_id, AIProcessingEvent::ProcessingStage::COMPLETED, result, context);
}

std::shared_ptr<EnvironmentalEvent> createUserStateChange(const std::string& new_state, const std::string& user_id) {
    EventContext context;
    context.user_profile = user_factory::createNewUser(user_id);
    context.emotional_state = "observant";
    context.urgency_level = 0.3f;

    std::unordered_map<std::string, std::string> sensor_data;
    sensor_data["user_state"] = new_state;

    return std::make_shared<EnvironmentalEvent>(
        EnvironmentalEvent::EnvironmentType::USER_STATE,
        "User state changed to: " + new_state,
        sensor_data,
        context
    );
}

std::shared_ptr<EnvironmentalEvent> createSystemAlert(const std::string& alert_message, EventPriority priority) {
    EventContext context;
    context.user_profile = user_factory::createDefaultUser("system_monitor");
    context.emotional_state = "alert";
    context.urgency_level = (priority == EventPriority::CRITICAL) ? 1.0f : 0.7f;

    return std::make_shared<EnvironmentalEvent>(
        EnvironmentalEvent::EnvironmentType::SYSTEM_STATUS,
        alert_message,
        std::unordered_map<std::string, std::string>(),
        context
    );
}

std::shared_ptr<LearningEvent> createUserPreference(const std::string& preference, float confidence) {
    EventContext context;
    context.user_profile = user_factory::createDefaultUser("learning_system");
    context.emotional_state = "learning";
    context.urgency_level = 0.2f;

    return std::make_shared<LearningEvent>(
        LearningEvent::LearningType::USER_PREFERENCE,
        preference,
        confidence,
        context
    );
}

std::shared_ptr<LearningEvent> createBehaviorPattern(const std::string& pattern, float confidence) {
    EventContext context;
    context.user_profile = user_factory::createDefaultUser("learning_system");
    context.emotional_state = "analytical";
    context.urgency_level = 0.2f;

    return std::make_shared<LearningEvent>(
        LearningEvent::LearningType::BEHAVIOR_PATTERN,
        pattern,
        confidence,
        context
    );
}

std::shared_ptr<WelcomeEvent> createSystemWelcome(const std::string& user_id, const std::string& message) {
    EventContext context;
    context.user_profile = user_factory::createNewUser(user_id);
    context.emotional_state = "welcoming";
    context.urgency_level = 0.1f;

    return std::make_shared<WelcomeEvent>(
        WelcomeEvent::WelcomeType::SYSTEM_STARTUP,
        message,
        user_id,
        context
    );
}

std::shared_ptr<WelcomeEvent> createPersonalizedWelcome(const std::string& user_id, const EventContext& context) {
    // Simple welcome message generation for the event system
    std::string welcome_message = "Welcome, " + user_id + "! The Cortana Orchestrator is ready to assist you.";

    return std::make_shared<WelcomeEvent>(
        WelcomeEvent::WelcomeType::USER_LOGIN,
        welcome_message,
        user_id,
        context
    );
}

} // namespace cortana_events

// ============================================================================
// User Manager Implementation
// ============================================================================

class UserManager::Impl {
public:
    std::unordered_map<std::string, std::shared_ptr<UserProfile>> user_profiles_;
    std::mutex mutex_;
};

UserManager::UserManager() : impl_(std::make_unique<Impl>()) {}
UserManager::~UserManager() = default;

std::shared_ptr<UserProfile> UserManager::getOrCreateUserProfile(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);

    auto it = impl_->user_profiles_.find(user_id);
    if (it != impl_->user_profiles_.end()) {
        // Update last seen time
        it->second->last_seen = std::chrono::system_clock::now();
        return it->second;
    }

    // Create new user profile
    auto new_profile = user_factory::createNewUser(user_id);
    impl_->user_profiles_[user_id] = new_profile;
    return new_profile;
}

std::shared_ptr<UserProfile> UserManager::getUserProfile(const std::string& user_id) const {
    std::lock_guard<std::mutex> lock(impl_->mutex_);

    auto it = impl_->user_profiles_.find(user_id);
    return (it != impl_->user_profiles_.end()) ? it->second : nullptr;
}

bool UserManager::saveUserProfile(const std::shared_ptr<UserProfile>& profile) {
    // TODO: Implement JSON serialization and file saving
    std::cout << "📝 Saving user profile for: " << profile->user_id << std::endl;
    return true; // Placeholder
}

bool UserManager::deleteUserProfile(const std::string& user_id) {
    std::lock_guard<std::mutex> lock(impl_->mutex_);

    auto it = impl_->user_profiles_.find(user_id);
    if (it != impl_->user_profiles_.end()) {
        impl_->user_profiles_.erase(it);
        return true;
    }
    return false;
}

void UserManager::updateUserFamiliarity(const std::string& user_id, float interaction_quality) {
    auto profile = getUserProfile(user_id);
    if (profile) {
        profile->updateFamiliarity(interaction_quality);
    }
}

void UserManager::updateUserPreferences(const std::string& user_id, const UserPreferences& preferences) {
    auto profile = getUserProfile(user_id);
    if (profile) {
        profile->preferences = preferences;
    }
}

std::vector<std::string> UserManager::getAllUserIds() const {
    std::lock_guard<std::mutex> lock(impl_->mutex_);

    std::vector<std::string> user_ids;
    user_ids.reserve(impl_->user_profiles_.size());

    for (const auto& pair : impl_->user_profiles_) {
        user_ids.push_back(pair.first);
    }
    return user_ids;
}

std::vector<std::shared_ptr<UserProfile>> UserManager::getActiveUsers() const {
    std::lock_guard<std::mutex> lock(impl_->mutex_);

    std::vector<std::shared_ptr<UserProfile>> active_users;
    auto now = std::chrono::system_clock::now();
    auto one_week_ago = now - std::chrono::hours(24 * 7);

    for (const auto& pair : impl_->user_profiles_) {
        if (pair.second->last_seen > one_week_ago) {
            active_users.push_back(pair.second);
        }
    }
    return active_users;
}

bool UserManager::loadUserProfiles(const std::string& config_path) {
    // TODO: Implement JSON loading
    std::cout << "📂 Loading user profiles from: " << config_path << std::endl;
    return true; // Placeholder
}

bool UserManager::saveUserProfiles(const std::string& config_path) {
    // TODO: Implement JSON saving
    std::cout << "💾 Saving user profiles to: " << config_path << std::endl;
    return true; // Placeholder
}

// ============================================================================
// User Factory Implementations
// ============================================================================

namespace user_factory {

std::shared_ptr<UserProfile> createNewUser(const std::string& user_id,
                                         const std::string& display_name) {
    auto profile = std::make_shared<UserProfile>();
    auto now = std::chrono::system_clock::now();

    profile->user_id = user_id;
    profile->display_name = display_name.empty() ? user_id : display_name;
    profile->created_at = now;
    profile->last_seen = now;
    profile->first_interaction = now;
    profile->familiarity_level = 0.1f; // New users start with low familiarity
    profile->interaction_count = 0; // Start at 0, increment only after first actual interaction

    // Default preferences
    profile->preferences.preferred_greeting_style = "casual";
    profile->preferences.time_format = "12h";
    profile->preferences.response_detail_level = "detailed";
    profile->preferred_emotional_state = "focused";
    profile->relationship_status = "acquaintance";

    return profile;
}

std::shared_ptr<UserProfile> createKnownUser(const std::string& user_id,
                                           const std::string& display_name,
                                           float familiarity_level,
                                           const UserPreferences& preferences) {
    auto profile = createNewUser(user_id, display_name);
    profile->familiarity_level = familiarity_level;
    profile->preferences = preferences;
    profile->updateRelationshipStatus();

    return profile;
}

std::shared_ptr<UserProfile> createDefaultUser(const std::string& user_id) {
    return createNewUser(user_id, "Guest User");
}

std::shared_ptr<UserProfile> createDeveloperUser(const std::string& user_id,
                                               const std::string& display_name) {
    auto profile = createNewUser(user_id, display_name);

    // Developer-specific preferences
    profile->preferences.preferred_greeting_style = "technical";
    profile->preferences.response_detail_level = "comprehensive";
    profile->interests = {"programming", "software engineering", "debugging", "optimization"};
    profile->topic_familiarity = {
        {"coding", 0.8f},
        {"algorithms", 0.7f},
        {"debugging", 0.9f},
        {"testing", 0.8f}
    };

    return profile;
}

std::shared_ptr<UserProfile> createResearcherUser(const std::string& user_id,
                                                const std::string& display_name) {
    auto profile = createNewUser(user_id, display_name);

    // Researcher-specific preferences
    profile->preferences.preferred_greeting_style = "formal";
    profile->preferences.response_detail_level = "comprehensive";
    profile->interests = {"research", "analysis", "data science", "innovation"};
    profile->topic_familiarity = {
        {"research", 0.9f},
        {"data_analysis", 0.8f},
        {"methodology", 0.8f},
        {"innovation", 0.7f}
    };

    return profile;
}

std::shared_ptr<UserProfile> createStudentUser(const std::string& user_id,
                                             const std::string& display_name) {
    auto profile = createNewUser(user_id, display_name);

    // Student-specific preferences
    profile->preferences.preferred_greeting_style = "friendly";
    profile->preferences.response_detail_level = "detailed";
    profile->interests = {"learning", "education", "projects", "collaboration"};
    profile->topic_familiarity = {
        {"learning", 0.8f},
        {"projects", 0.7f},
        {"education", 0.7f},
        {"collaboration", 0.6f}
    };

    return profile;
}

} // namespace user_factory

} // namespace cortan::core
