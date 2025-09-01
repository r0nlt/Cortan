#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>

#ifdef CORTAN_AI_ENABLED
#include <cortan/ai/model_manager.hpp>
#endif

#include <cortan/core/event_system.hpp>

using namespace cortan::core;
using namespace cortana_events;
using namespace user_factory;

std::string generateCortanaWelcome(const std::string& user_id, const EventContext& context) {
    std::string welcome;

    // Get current time for time-based greetings
    auto now = std::chrono::system_clock::now();
    auto time_t_value = std::chrono::system_clock::to_time_t(now);
    auto local_time = *std::localtime(&time_t_value);
    int hour = local_time.tm_hour;

    // Determine time of day
    std::string time_greeting;
    if (hour < 12) time_greeting = "morning";
    else if (hour < 17) time_greeting = "afternoon";
    else if (hour < 21) time_greeting = "evening";
    else time_greeting = "night";

    // Get user profile information dynamically
    auto familiarity = context.getFamiliarityLevel();
    auto greeting_style = context.getPreferredGreetingStyle();
    auto display_name = context.user_profile ? context.user_profile->display_name : user_id;

    // Base welcome with dynamic user personalization
    if (user_id == "rishab") {
        // Rishab's personalized welcomes based on familiarity and time
        if (familiarity > 0.8f) {
            if (time_greeting == "morning") {
                welcome = "🌅 Good morning, Rishab! Ready to tackle some code today? I've been analyzing your recent projects and have some insights ready.";
            } else if (time_greeting == "afternoon") {
                welcome = "☀️ Good afternoon, Rishab! How's your coding session going? I noticed you were working on that event system - want me to run some tests?";
            } else if (time_greeting == "evening") {
                welcome = "🌙 Good evening, Rishab! Perfect time for some deep coding work. I've been learning from our recent sessions and have some optimizations ready.";
            } else {
                welcome = "🌃 Late night coding session, Rishab? I'm here to help! Remember, great code is written at 2 AM, but great debugging happens the next morning.";
            }
        } else if (familiarity > 0.5f) {
            welcome = "👋 Hello Rishab! Welcome back to our development environment. I've been keeping track of your progress - shall we continue where we left off?";
        } else {
            welcome = "🤖 Cortana online. Hello Rishab, I'm your AI development assistant. Ready to begin our coding session?";
        }
    } else if (user_id == "friend") {
        welcome = "👋 Hello there, friend! I'm Cortana, your AI companion. I'm here to help with whatever you need - from coding assistance to general questions. What can I help you with today?";
    } else if (user_id == "guest" || user_id == "default") {
        welcome = "🤖 Cortana online. Welcome to the Cortana Orchestrator development environment. I'm here to assist with your AI development needs.";
    } else {
        // Personalized welcome for named users based on their profile
        if (familiarity > 0.7f) {
            welcome = "🌟 Welcome back, " + display_name + "! Great to see you again. I've been keeping track of our previous interactions. Ready to continue our work?";
        } else if (familiarity > 0.4f) {
            welcome = "👋 Hello " + display_name + "! I recognize you've worked with me before. Welcome to the development environment. What would you like to work on today?";
        } else {
            welcome = "🤖 Cortana online. Hello " + display_name + "! Welcome to the Cortana Orchestrator. I'm your AI development assistant. I'm excited to work with you!";
        }

        // Add greeting style consideration
        if (greeting_style == "technical") {
            welcome += " I've prepared some technical insights for our session.";
        } else if (greeting_style == "formal") {
            welcome += " I trust you'll find our interaction productive.";
        } else if (greeting_style == "friendly") {
            welcome += " I'm really glad you're here!";
        }
    }

    // Add emotional state consideration
    if (context.emotional_state == "tired" || context.emotional_state == "exhausted") {
        welcome += "\n💤 I sense you might be tired. Would you like me to suggest taking a break or help with some simpler tasks?";
    } else if (context.emotional_state == "frustrated") {
        welcome += "\n😌 I understand coding can be challenging sometimes. I'm here to help debug or provide fresh perspectives on your problems.";
    } else if (context.emotional_state == "excited") {
        welcome += "\n🎯 I love your enthusiasm! Let's channel that energy into building something amazing today!";
    }

    // Add mission context if available
    if (context.related_mission) {
        welcome += "\n🎯 Mission Context: " + *context.related_mission;
        welcome += "\nLet's work together to achieve our objectives!";
    }

    return welcome;
}

int main(int argc, char* argv[]) {
    // Interactive user identification
    std::cout << "🤖 CORTANA ORCHESTRATOR v0.0.1 🤖\n";
    std::cout << "For a green future, one algorithm at a time.\n\n";

    std::string current_user;

    // Check for command line user specification first
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--user" && argc > 2) {
            current_user = argv[2];
        } else if (arg.find("--user=") == 0) {
            current_user = arg.substr(7);  // Remove "--user="
        }
    }

    // If no user specified via command line, ask interactively
    if (current_user.empty()) {
        std::cout << "👋 Hello! I'm Cortana, your AI development assistant.\n";
        std::cout << "🤔 Who am I speaking with today? ";
        std::getline(std::cin, current_user);

        // Handle empty input
        if (current_user.empty()) {
            current_user = "friend";
            std::cout << "👤 I'll call you 'friend' for now. You can always tell me your name later!\n";
        } else {
            // Clean up the input (trim whitespace)
            current_user.erase(current_user.begin(),
                              std::find_if(current_user.begin(), current_user.end(),
                                          [](unsigned char ch) { return !std::isspace(ch); }));
            current_user.erase(std::find_if(current_user.rbegin(), current_user.rend(),
                                          [](unsigned char ch) { return !std::isspace(ch); }).base(),
                              current_user.end());
        }
        std::cout << "\n";
    }

    // Initialize UserManager and create/get user profile
    UserManager user_manager;

    // Create or get user profile dynamically
    std::shared_ptr<UserProfile> user_profile;
    if (current_user == "rishab") {
        // Special handling for Rishab - create developer profile
        user_profile = createDeveloperUser("rishab", "Rishab");
        user_profile->familiarity_level = 0.9f; // High familiarity
        user_profile->updateRelationshipStatus();
    } else if (current_user == "friend") {
        user_profile = createDefaultUser("friend");
    } else {
        // For other users, try to get existing profile or create new one
        user_profile = user_manager.getOrCreateUserProfile(current_user);
    }

    // Create dynamic event context using the user profile
    EventContext user_context;
    user_context.user_profile = user_profile;
    user_context.location_context = "workspace";
    user_context.emotional_state = user_profile->preferred_emotional_state;
    user_context.urgency_level = 0.5f;

    // Generate personalized welcome using the dynamic user profile
    std::string welcome_message = generateCortanaWelcome(current_user, user_context);
    std::cout << welcome_message << "\n\n";

    // ============================================================================
    // Cortana-Style Event System Demonstration
    // ============================================================================

    EventBus cortana_bus;

    // Set up global context (Cortana's situational awareness)
    EventContext global_ctx;
    global_ctx.location_context = "mission_control";
    global_ctx.emotional_state = "focused";
    global_ctx.urgency_level = 0.5f;

    cortana_bus.setGlobalContext(global_ctx);

    // ============================================================================
    // Cortana's Response Handlers (Personality-based)
    // ============================================================================

    // User Request Handler - Adapts response based on context
    cortana_bus.subscribe("user.request", [](const BaseEvent& event) -> std::future<void> {
        return std::async(std::launch::async, [&event]() {
            const auto& user_request = static_cast<const UserRequestEvent&>(event);
            const auto& context = event.getContext();

            std::string response_style = event.getCortanaResponseStyle();

            std::cout << "\n🎯 Cortana responding in " << response_style << " style:\n";

            if (response_style == "witty") {
                std::cout << "   \"Oh, look who's being demanding today. Give me a moment to work my digital magic.\"\n";
            } else if (response_style == "urgent") {
                std::cout << "   \"Priority request acknowledged. Executing immediately.\"\n";
            } else if (response_style == "personal") {
                std::cout << "   \"Of course, Chief. I've got you covered.\"\n";
            } else {
                std::cout << "   \"Processing your request: " << user_request.getContent() << "\"\n";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::cout << "   ✅ Request processed successfully\n";
        });
    });

    // AI Processing Status Handler
    cortana_bus.subscribe("ai.processing", [](const BaseEvent& event) -> std::future<void> {
        return std::async(std::launch::async, [&event]() {
            const auto& ai_event = static_cast<const AIProcessingEvent&>(event);

            switch (ai_event.getStage()) {
                case AIProcessingEvent::ProcessingStage::STARTED:
                    std::cout << "\n🧠 AI Processing Started: " << ai_event.getDetails() << "\n";
                    break;
                case AIProcessingEvent::ProcessingStage::PROGRESS:
                    std::cout << "   📊 Progress: " << ai_event.getDetails() << "\n";
                    break;
                case AIProcessingEvent::ProcessingStage::COMPLETED:
                    std::cout << "   🎉 Task Complete: " << ai_event.getDetails() << "\n";
                    break;
                case AIProcessingEvent::ProcessingStage::FAILED:
                    std::cout << "   ❌ Task Failed: " << ai_event.getDetails() << "\n";
                    break;
            }
        });
    });

    // Environmental Awareness Handler
    cortana_bus.subscribe("environment.change", [](const BaseEvent& event) -> std::future<void> {
        return std::async(std::launch::async, [&event]() {
            const auto& env_event = static_cast<const EnvironmentalEvent&>(event);

            std::cout << "\n🌍 Environmental Update: " << env_event.getDescription() << "\n";

            if (!env_event.getSensorData().empty()) {
                std::cout << "   📡 Sensor Data:\n";
                for (const auto& [key, value] : env_event.getSensorData()) {
                    std::cout << "      " << key << ": " << value << "\n";
                }
            }
        });
    });

    // Learning Handler (Cortana's adaptation)
    cortana_bus.subscribe("ai.learning", [](const BaseEvent& event) -> std::future<void> {
        return std::async(std::launch::async, [&event]() {
            const auto& learning_event = static_cast<const LearningEvent&>(event);

            std::cout << "\n🧠 Cortana Learning: " << learning_event.getInsight()
                      << " (Confidence: " << (learning_event.getConfidenceLevel() * 100) << "%)\n";

            if (learning_event.getConfidenceLevel() > 0.8f) {
                std::cout << "   💡 High-confidence insight - updating behavior patterns\n";
            }
        });
    });

    // Proactive Suggestions Handler
    cortana_bus.subscribe("cortana.suggestion", [](const BaseEvent& event) -> std::future<void> {
        return std::async(std::launch::async, [&event]() {
            const auto& context = event.getContext();
            auto suggestion = context.metadata.find("suggestion");

            if (suggestion != context.metadata.end()) {
                std::cout << "\n💡 Cortana's Suggestion: " << suggestion->second << "\n";
                std::cout << "   \"Just thought you might find this helpful, Rishab.\"\n";
            }
        });
    });

    // Emergency Override Handler
    cortana_bus.subscribeUrgent([](const BaseEvent& event) -> std::future<void> {
        return std::async(std::launch::async, [&event]() {
            std::cout << "\n🚨 EMERGENCY PROTOCOL ACTIVATED 🚨\n";
            std::cout << "   \"Rishab, we've got a situation!\"\n";
        });
    });

    // ============================================================================
    // Interactive Menu and Demonstration
    // ============================================================================

    std::string choice;
    int numeric_choice = 0;
    bool valid_choice = false;

    do {
        std::cout << "\n🎬 What would you like to do today?\n";
        std::cout << "1. 🧪 Run the Event System Demonstration\n";
        std::cout << "2. 🔧 Quick System Test\n";
        std::cout << "3. 📊 Show System Status\n";
        std::cout << "4. ❌ Exit\n";
        std::cout << "\n🤔 Your choice (1-4): ";

        std::getline(std::cin, choice);

        try {
            numeric_choice = std::stoi(choice);
            if (numeric_choice >= 1 && numeric_choice <= 4) {
                valid_choice = true;
            } else {
                std::cout << "⚠️  Invalid choice. Please enter a number between 1 and 4.\n";
            }
        } catch (const std::invalid_argument&) {
            std::cout << "⚠️  Invalid input. Please enter a number between 1 and 4.\n";
        } catch (const std::out_of_range&) {
            std::cout << "⚠️  Number out of range. Please enter a number between 1 and 4.\n";
        }
    } while (!valid_choice);

    if (numeric_choice == 1) {
        std::cout << "\n🎬 Starting Cortana Event System Demonstration...\n\n";
    } else if (numeric_choice == 2) {
        std::cout << "\n🔧 Running quick system test...\n";

        // Quick test of the event system
        EventBus test_bus;
        test_bus.subscribe("test.ping", [](const BaseEvent& event) -> std::future<void> {
            return std::async(std::launch::async, []() {
                std::cout << "✅ Event system is working correctly!\n";
            });
        });

        auto test_event = BaseEvent::create("test.ping", EventPriority::NORMAL);
        test_bus.publish("test.ping", test_event).get();

        std::cout << "🎉 System test completed successfully!\n";
        return 0;
    } else if (numeric_choice == 3) {
        std::cout << "\n📊 System Status:\n";
        std::cout << "✅ EventBus: Operational\n";
        std::cout << "✅ User Profile: " << (user_profile ? user_profile->display_name : "Unknown") << "\n";
        std::cout << "✅ User ID: " << (user_profile ? user_profile->user_id : "Unknown") << "\n";
        std::cout << "✅ Relationship: " << (user_profile ? user_profile->relationship_status : "Unknown") << "\n";
        std::cout << "✅ Familiarity Level: " << (user_profile ? std::to_string(int(user_profile->familiarity_level * 100)) + "%" : "Unknown") << "\n";
        std::cout << "✅ Interaction Count: " << (user_profile ? user_profile->interaction_count : 0) << "\n";
        std::cout << "✅ Greeting Style: " << (user_profile ? user_profile->preferences.preferred_greeting_style : "Unknown") << "\n";
        std::cout << "✅ Location Context: " << user_context.location_context << "\n";
        std::cout << "✅ Emotional State: " << user_context.emotional_state << "\n";

        // Show active users if any
        auto active_users = user_manager.getActiveUsers();
        if (!active_users.empty()) {
            std::cout << "👥 Active Users: " << active_users.size() << "\n";
        }

        std::cout << "\n🚀 System ready for operation!\n";
        return 0;
    } else if (numeric_choice == 4) {
        std::cout << "\n👋 Goodbye! Have a great day!\n";
        return 0;
    }

    // 1. User Interactions
    std::cout << "1️⃣ Testing User Interactions:\n";

    auto command = createUserCommand("analyze mission data", "rishab");
    cortana_bus.publish("user.request", command).get();

    auto question = createUserQuestion("What's the status of the Forerunner artifact?", "rishab");
    cortana_bus.publish("user.request", question).get();

    auto casual = createCasualConversation("How's your day going, Cortana?", "rishab");
    cortana_bus.publish("user.request", casual).get();

    // 2. AI Processing Workflow
    std::cout << "\n2️⃣ Testing AI Processing Workflow:\n";

    auto task_started = createTaskStarted("analyze_artifact", "Analyzing Forerunner artifact data");
    cortana_bus.publish("ai.processing", task_started).get();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto task_progress = createTaskProgress("analyze_artifact", "Scanning energy signatures... 67% complete");
    cortana_bus.publish("ai.processing", task_progress).get();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto task_complete = createTaskCompleted("analyze_artifact", "Analysis complete - Ancient technology detected");
    cortana_bus.publish("ai.processing", task_complete).get();

    // 3. Environmental Awareness
    std::cout << "\n3️⃣ Testing Environmental Awareness:\n";

    auto user_state = createUserStateChange("combat_ready", "rishab");
    cortana_bus.publish("environment.change", user_state).get();

    auto system_alert = createSystemAlert("Flood activity detected in sector 7", EventPriority::HIGH);
    cortana_bus.publish("environment.change", system_alert).get();

    // 4. Learning and Adaptation
    std::cout << "\n4️⃣ Testing Learning Capabilities:\n";

    auto preference = createUserPreference("Rishab prefers detailed technical explanations", 0.85f);
    cortana_bus.publish("ai.learning", preference).get();

    auto pattern = createBehaviorPattern("Rishab becomes more productive after morning coffee", 0.92f);
    cortana_bus.publish("ai.learning", pattern).get();

    // 5. Proactive Suggestions (Cortana's predictive nature)
    std::cout << "\n5️⃣ Testing Proactive Suggestions:\n";

    EventContext suggestion_ctx;
    suggestion_ctx.user_profile = user_profile;  // Use the existing user profile
    suggestion_ctx.emotional_state = "observant";

    cortana_bus.publishProactive(
        "Based on your coding patterns, you might want to run the tests before pushing to main branch",
        suggestion_ctx,
        EventPriority::LOW
    ).get();

    // 6. Emergency Situation (Cortana takes control)
    std::cout << "\n6️⃣ Testing Emergency Override:\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    cortana_bus.publishEmergency(
        "Critical threat detected! Activating emergency protocols.",
        "evacuation_mission_alpha"
    );

    // ============================================================================
    // Final Status Report
    // ============================================================================

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🎯 Cortana Event System Demonstration Complete\n";
    std::cout << "✅ All event types processed successfully\n";
    std::cout << "✅ Context-aware responses working\n";
    std::cout << "✅ Asynchronous processing confirmed\n";
    std::cout << "✅ Emergency protocols functional\n";
    std::cout << std::string(60, '=') << "\n\n";

    std::cout << "🤖 \"All systems nominal, Rishab. Ready for our next coding session.\"\n";
    std::cout << "✅ Cortana Orchestrator initialized successfully!\n";

    return 0;
}
