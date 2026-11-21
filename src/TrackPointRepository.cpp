#include "TrackPointRepository.h"
#include <exception>
#include <stdexcept>

TrackPointRepository::TrackPointRepository(GpsStorage& storage) : storage_(storage) {}

int TrackPointRepository::add(const TrackPoint& point) {
    return storage_.insert(point);
}

std::vector<TrackPoint> TrackPointRepository::findByObjectId(int objectId) {
    using namespace sqlite_orm;
    return storage_.get_all<TrackPoint>(where(c(&TrackPoint::objectId) == objectId),
                                        order_by(&TrackPoint::recordedAt).asc());
}

std::optional<TrackPoint> TrackPointRepository::findById(int id) {
    return storage_.get_optional<TrackPoint>(id);
}

void TrackPointRepository::saveTransition(int id, const std::string& oldState,
                                         const std::string& newState,
                                         const std::string& changedAt) {
    std::exception_ptr error;
    const bool committed = storage_.transaction([&] {
        try {
            auto point = findById(id);
            if (!point || point->state != oldState)
                throw std::runtime_error("Точка відсутня або її стан змінився");
            point->state = newState;
            storage_.update(*point);
            storage_.insert(TrackPointStateHistory{0, id, oldState, newState, changedAt});
            return true;
        } catch (...) {
            error = std::current_exception();
            return false; // ROLLBACK обох операцій
        }
    });
    if (error) std::rethrow_exception(error);
    if (!committed) throw std::runtime_error("Транзакцію не завершено");
}

std::vector<TrackPointStateHistory> TrackPointRepository::history(int pointId) {
    using namespace sqlite_orm;
    return storage_.get_all<TrackPointStateHistory>(
        where(c(&TrackPointStateHistory::pointId) == pointId),
        order_by(&TrackPointStateHistory::id).asc());
}
