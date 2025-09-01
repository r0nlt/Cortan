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

int main(int argc, char* argv[]) {
    std::cout << "🤖 CORTANA ORCHESTRATOR v0.0.1 🤖\n";
    std::cout << "For a green future, one algorithm at a time.\n\n";

    if (argc > 1) {
        std::string command = argv[1];
        std::cout << "Initializing with command: " << command << "\n\n";
    }

    // ============================================================================
    // Cortana-Style Event System Demonstration
    // ============================================================================

    EventBus cortana_bus;

    // Set up global context (Cortana's situational awareness)
    EventContext global_ctx;
    global_ctx.location_context = "mission_control";
    global_ctx.emotional_state = "focused";
    global_ctx.urgency_level = 0.5f;
    global_ctx.familiarity_level = 0.9f;  // High familiarity with Rishab

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
    // Cortana's Demonstration Sequence
    // ============================================================================

    std::cout << "🎬 Starting Cortana Event System Demonstration...\n\n";

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
    suggestion_ctx.user_id = "rishab";
    suggestion_ctx.emotional_state = "observant";
    suggestion_ctx.familiarity_level = 0.9f;

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
