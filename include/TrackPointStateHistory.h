#pragma once
#include <string>

struct TrackPointStateHistory {
    int id = 0;
    int pointId = 0;
    std::string oldState;
    std::string newState;
    std::string changedAt;
};
