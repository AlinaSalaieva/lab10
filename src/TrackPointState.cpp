#include "TrackPointState.h"
#include <stdexcept>
#include <utility>

std::string ReceivedState::name() const { return "Отримана"; }
std::string VerifiedState::name() const { return "Перевірена"; }
std::string ProcessedState::name() const { return "Опрацьована"; }
std::string InvalidState::name() const { return "Некоректна"; }

std::unique_ptr<TrackPointState> ReceivedState::next(TrackAction action) const {
    if (action == TrackAction::Verify) return std::make_unique<VerifiedState>();
    if (action == TrackAction::Invalidate) return std::make_unique<InvalidState>();
    return nullptr;
}
std::unique_ptr<TrackPointState> VerifiedState::next(TrackAction action) const {
    if (action == TrackAction::Process) return std::make_unique<ProcessedState>();
    if (action == TrackAction::Invalidate) return std::make_unique<InvalidState>();
    return nullptr;
}
std::unique_ptr<TrackPointState> ProcessedState::next(TrackAction) const { return nullptr; }
std::unique_ptr<TrackPointState> InvalidState::next(TrackAction) const { return nullptr; }

TrackPointContext::TrackPointContext(const std::string& state) {
    if (state == "Отримана") state_ = std::make_unique<ReceivedState>();
    else if (state == "Перевірена") state_ = std::make_unique<VerifiedState>();
    else if (state == "Опрацьована") state_ = std::make_unique<ProcessedState>();
    else if (state == "Некоректна") state_ = std::make_unique<InvalidState>();
    else throw std::invalid_argument("Невідомий стан точки треку");
}
std::string TrackPointContext::state() const { return state_->name(); }
bool TrackPointContext::transition(TrackAction action) {
    auto nextState = state_->next(action);
    if (!nextState) return false;
    state_ = std::move(nextState);
    return true;
}
