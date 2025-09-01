#pragma once

#include <coroutine>
#include <string>
#include <vector>
#include <memory>

namespace cortan::core {

template<typename T>
struct WorkflowTask {
    struct promise_type {
        T value;

        WorkflowTask get_return_object() {
            return WorkflowTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
        void return_value(T val) { value = std::move(val); }
    };

    std::coroutine_handle<promise_type> coro;

    WorkflowTask(std::coroutine_handle<promise_type> handle) : coro(handle) {}
    ~WorkflowTask() { if (coro) coro.destroy(); }

    // Move-only
    WorkflowTask(const WorkflowTask&) = delete;
    WorkflowTask& operator=(const WorkflowTask&) = delete;
    WorkflowTask(WorkflowTask&& other) noexcept : coro(std::exchange(other.coro, {})) {}
    WorkflowTask& operator=(WorkflowTask&& other) noexcept {
        if (this != &other) {
            if (coro) coro.destroy();
            coro = std::exchange(other.coro, {});
        }
        return *this;
    }

    bool ready() const { return coro && coro.done(); }
    T result() {
        if (!coro || !coro.done()) throw std::runtime_error("Task not completed");
        return std::move(coro.promise().value);
    }
};

class WorkflowEngine {
public:
    WorkflowEngine();
    ~WorkflowEngine();

    template<typename T>
    WorkflowTask<T> executeWorkflow(const std::string& workflow_id);

private:
    // Implementation details
};

} // namespace cortan::core
