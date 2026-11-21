#include "TestData.h"
#include "SQLiteGpsObjectRepository.h"
#include "TrackPointRepository.h"
#include <array>
#include <stdexcept>

void seedTestData(GpsStorage& storage) {
    SQLiteGpsObjectRepository objects(storage);
    TrackPointRepository tracks(storage);
    const std::array<GpsObject, 5> examples = {{
        {0, "Автомобіль служби доставки", "Автомобіль", "GPS-001", "Активний"},
        {0, "Міський автобус №24", "Автобус", "GPS-002", "Активний"},
        {0, "Вантажівка складу", "Вантажівка", "GPS-003", "На стоянці"},
        {0, "Службовий мотоцикл", "Мотоцикл", "GPS-004", "Активний"},
        {0, "Транспорт техпідтримки", "Автомобіль", "GPS-005", "Неактивний"}
    }};
    const std::array<std::array<double, 3>, 5> speed = {{
        {{42.0, 50.0, 46.0}}, {{31.0, 34.0, 28.0}},
        {{0.0, 0.0, 6.0}}, {{55.0, 62.0, 48.0}},
        {{20.0, 18.0, 0.0}}
    }};
    for (size_t i = 0; i < examples.size(); ++i) {
        const int id = objects.add(examples[i]);
        for (int j = 0; j < 3; ++j) {
            tracks.add(TrackPoint{
                0, id,
                "2026-11-08 09:" + std::string(j == 0 ? "00:00" : (j == 1 ? "10:00" : "20:00")),
                50.4501 + static_cast<double>(i) * 0.01 + j * 0.001,
                30.5234 + static_cast<double>(i) * 0.01 + j * 0.001,
                speed[i][j],
                3.0 + j
            });
        }
    }
    if (storage.count<GpsObject>() != 5 || storage.count<TrackPoint>() != 15)
        throw std::runtime_error("Некоректна кількість тестових записів");
}
