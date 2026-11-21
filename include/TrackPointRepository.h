#pragma once
#include "Database.h"
#include <vector>
#include <optional>

// Окремий компонент для залежної сутності (без додаткового інтерфейсу).
class TrackPointRepository {
public:
    explicit TrackPointRepository(GpsStorage& storage);
    int add(const TrackPoint& point);
    std::vector<TrackPoint> findByObjectId(int objectId);
    std::optional<TrackPoint> findById(int id);
    void saveTransition(int id, const std::string& oldState,
                        const std::string& newState, const std::string& changedAt);
    std::vector<TrackPointStateHistory> history(int pointId);
private:
    GpsStorage& storage_;
};
