#pragma once
#include <memory>
#include <string>

enum class TrackAction { Verify, Process, Invalidate };

class TrackPointState {
public:
    virtual ~TrackPointState() = default;
    virtual std::string name() const = 0;
    virtual std::unique_ptr<TrackPointState> next(TrackAction action) const = 0;
};

class ReceivedState final : public TrackPointState {
public:
    std::string name() const override;
    std::unique_ptr<TrackPointState> next(TrackAction action) const override;
};
class VerifiedState final : public TrackPointState {
public:
    std::string name() const override;
    std::unique_ptr<TrackPointState> next(TrackAction action) const override;
};
class ProcessedState final : public TrackPointState {
public:
    std::string name() const override;
    std::unique_ptr<TrackPointState> next(TrackAction action) const override;
};
class InvalidState final : public TrackPointState {
public:
    std::string name() const override;
    std::unique_ptr<TrackPointState> next(TrackAction action) const override;
};

class TrackPointContext {
public:
    explicit TrackPointContext(const std::string& state);
    std::string state() const;
    bool transition(TrackAction action);
private:
    std::unique_ptr<TrackPointState> state_;
};
