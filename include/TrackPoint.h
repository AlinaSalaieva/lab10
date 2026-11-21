#pragma once
#include <string>

// Залежна сутність: точка треку, FK -> GpsObject.id.
struct TrackPoint {
    int id = 0;                   // первинний ключ
    int objectId = 0;             // зовнішній ключ
    std::string recordedAt;       // час YYYY-MM-DD HH:MM:SS
    double latitude = 0.0;        // широта
    double longitude = 0.0;       // довгота
    double speedKmh = 0.0;        // швидкість, км/год (для AVG/SUM)
    double accuracyMeters = 0.0;  // точність GPS, метри
    std::string state = "Отримана";  // поточний стан точки треку
};
