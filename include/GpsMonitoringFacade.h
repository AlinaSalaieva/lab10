#pragma once
#include "TrackPointRepository.h"
#include "IGpsObjectRepository.h"
#include "TrackPointState.h"
#include "SpeedStrategy.h"
#include "TrackObserver.h"

class GpsMonitoringFacade {
public:
    GpsMonitoringFacade(IGpsObjectRepository& objects, TrackPointRepository& tracks);
    int receivePoint(TrackPoint point);
    bool changeState(int pointId, TrackAction action);
    void setStrategy(std::unique_ptr<SpeedStrategy> strategy);
    double assessSpeed(int pointId, const SpeedInput& input);
    std::string strategyName() const;
    std::string strategyUnit() const;
    void subscribe(const std::shared_ptr<TrackObserver>& observer);
    void unsubscribe(const std::shared_ptr<TrackObserver>& observer);
private:
    IGpsObjectRepository& objects_;
    TrackPointRepository& tracks_;
    SpeedContext speed_;
    TrackSubject events_;
};
