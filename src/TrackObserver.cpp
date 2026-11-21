#include "TrackObserver.h"
#include <algorithm>
#include <exception>
#include <stdexcept>

void TrackSubject::subscribe(const std::shared_ptr<TrackObserver>& observer) {
    if (!observer) throw std::invalid_argument("Підписника не визначено");
    std::erase_if(observers_, [](const auto& item) { return item.expired(); });
    for (const auto& item : observers_) if (item.lock() == observer) return;
    observers_.push_back(observer);
}
void TrackSubject::unsubscribe(const std::shared_ptr<TrackObserver>& observer) {
    std::erase_if(observers_, [&](const auto& item) {
        auto ptr = item.lock();
        return !ptr || ptr == observer;
    });
}
void TrackSubject::notify(const TrackEvent& event) {
    std::erase_if(observers_, [](const auto& item) { return item.expired(); });
    const auto snapshot = observers_;
    std::exception_ptr error;
    for (const auto& item : snapshot) {
        if (auto observer = item.lock()) {
            try { observer->update(event); }
            catch (...) { if (!error) error = std::current_exception(); }
        }
    }
    if (error) std::rethrow_exception(error);
}

TrackLogger::TrackLogger(std::ostream& out) : out_(out) {}
void TrackLogger::update(const TrackEvent& event) {
    out_ << "[Журнал треку] Точка " << event.pointId;
    if (event.type == TrackEventType::PointReceived) out_ << ": отримана";
    else out_ << ": відхилення швидкості " << event.speedKmh << " км/год";
    out_ << '\n';
}
ObjectMonitor::ObjectMonitor(std::ostream& out) : out_(out) {}
void ObjectMonitor::update(const TrackEvent& event) {
    out_ << "[Монітор об'єкта] Об'єкт " << event.objectId;
    if (event.type == TrackEventType::PointReceived)
        out_ << ": оновлено дані, точка " << event.pointId;
    else out_ << ": перевищення " << event.speedKmh - event.allowedKmh << " км/год";
    out_ << '\n';
}
