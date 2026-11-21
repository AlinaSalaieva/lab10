#include "GpsMonitoringFacade.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

static std::string currentTime() {
    const auto now = std::time(nullptr);
    std::tm utc{};
#ifdef _WIN32
    if (gmtime_s(&utc, &now) != 0) throw std::runtime_error("Помилка часу");
#else
    if (!gmtime_r(&now, &utc)) throw std::runtime_error("Помилка часу");
#endif
    std::ostringstream result;
    result << std::put_time(&utc, "%Y-%m-%d %H:%M:%S");
    return result.str();
}

GpsMonitoringFacade::GpsMonitoringFacade(IGpsObjectRepository& objects, TrackPointRepository& tracks)
    : objects_(objects), tracks_(tracks) {}

int GpsMonitoringFacade::receivePoint(TrackPoint point) {
    if (!objects_.findById(point.objectId)) throw std::invalid_argument("GPS-об'єкт не знайдено");
    point.state = "Отримана";
    const int id = tracks_.add(point);
    events_.notify({TrackEventType::PointReceived, id, point.objectId, point.speedKmh, 0.0});
    return id;
}
bool GpsMonitoringFacade::changeState(int pointId, TrackAction action) {
    const auto point = tracks_.findById(pointId);
    if (!point) throw std::invalid_argument("Точку треку не знайдено");
    TrackPointContext context(point->state);
    if (!context.transition(action)) return false;
    tracks_.saveTransition(pointId, point->state, context.state(), currentTime());
    return true;
}
void GpsMonitoringFacade::setStrategy(std::unique_ptr<SpeedStrategy> strategy) {
    speed_.setStrategy(std::move(strategy));
}
double GpsMonitoringFacade::assessSpeed(int pointId, const SpeedInput& input) {
    const auto point = tracks_.findById(pointId);
    if (!point) throw std::invalid_argument("Точку треку не знайдено");
    const double result = speed_.calculate(input);
    const double actualSpeed = AverageSpeed().calculate(input);
    if (actualSpeed > input.allowedKmh)
        events_.notify({TrackEventType::SpeedDeviation, pointId, point->objectId, actualSpeed, input.allowedKmh});
    return result;
}
std::string GpsMonitoringFacade::strategyName() const { return speed_.name(); }
std::string GpsMonitoringFacade::strategyUnit() const { return speed_.unit(); }
void GpsMonitoringFacade::subscribe(const std::shared_ptr<TrackObserver>& observer) { events_.subscribe(observer); }
void GpsMonitoringFacade::unsubscribe(const std::shared_ptr<TrackObserver>& observer) { events_.unsubscribe(observer); }
