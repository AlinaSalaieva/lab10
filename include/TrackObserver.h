#pragma once
#include <memory>
#include <ostream>
#include <vector>

enum class TrackEventType { PointReceived, SpeedDeviation };
struct TrackEvent {
    TrackEventType type;
    int pointId;
    int objectId;
    double speedKmh;
    double allowedKmh;
};

class TrackObserver {
public:
    virtual ~TrackObserver() = default;
    virtual void update(const TrackEvent& event) = 0;
};
class TrackSubject {
public:
    void subscribe(const std::shared_ptr<TrackObserver>& observer);
    void unsubscribe(const std::shared_ptr<TrackObserver>& observer);
    void notify(const TrackEvent& event);
private:
    std::vector<std::weak_ptr<TrackObserver>> observers_;
};
class TrackLogger final : public TrackObserver {
public:
    explicit TrackLogger(std::ostream& out);
    void update(const TrackEvent& event) override;
private:
    std::ostream& out_;
};
class ObjectMonitor final : public TrackObserver {
public:
    explicit ObjectMonitor(std::ostream& out);
    void update(const TrackEvent& event) override;
private:
    std::ostream& out_;
};
