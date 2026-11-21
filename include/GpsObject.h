#pragma once
#include <string>

// Основна сутність: об'єкт спостереження (один об'єкт має багато точок треку).
struct GpsObject {
    int id = 0;                     // первинний ключ
    std::string name;                // назва об'єкта
    std::string objectType;          // тип: Автомобіль, Автобус тощо
    std::string deviceCode;          // код GPS-трекера
    std::string status;              // стан спостереження
};
