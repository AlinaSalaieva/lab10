#include "Database.h"
#include "GpsMonitoringFacade.h"
#include "SQLiteGpsObjectRepository.h"
#include "TestData.h"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

static TrackPoint examplePoint() {
    return {0, 1, "2026-11-08 09:30:00", 50.4570, 30.5300, 72.0, 4.0};
}
static void printHistory(TrackPointRepository& tracks, int id) {
    std::cout << "Історія з БД для точки " << id << ":\n";
    for (const auto& row : tracks.history(id)) {
        std::cout << row.oldState << " -> " << row.newState << " | "
                  << row.changedAt << " UTC\n";
    }
}
static void showTransition(GpsMonitoringFacade& facade, TrackPointRepository& tracks,
                           int id, TrackAction action, const std::string& operation) {
    const auto before = tracks.findById(id).value().state;
    const bool accepted = facade.changeState(id, action);
    const auto after = tracks.findById(id).value().state;
    std::cout << operation << ": " << before << " -> " << after
              << (accepted ? " [дозволено]" : " [заборонено]") << '\n';
}
static void stateDemo(GpsMonitoringFacade& facade, TrackPointRepository& tracks) {
    std::cout << "\n1. STATE — КЕРУВАННЯ СТАНАМИ\n";
    const int id = facade.receivePoint(examplePoint());
    std::cout << "Точка " << id << ": " << tracks.findById(id).value().state << '\n';
    showTransition(facade, tracks, id, TrackAction::Process, "Опрацювання без перевірки");
    showTransition(facade, tracks, id, TrackAction::Verify, "Перевірка");
    showTransition(facade, tracks, id, TrackAction::Process, "Опрацювання");
    showTransition(facade, tracks, id, TrackAction::Verify, "Повторна перевірка");
    printHistory(tracks, id);
    auto invalid = examplePoint();
    invalid.accuracyMeters = -1.0;
    const int invalidId = facade.receivePoint(invalid);
    std::cout << "Недостовірна точка " << invalidId << " (від'ємна точність):\n";
    showTransition(facade, tracks, invalidId, TrackAction::Invalidate, "Позначення некоректною");
    showTransition(facade, tracks, invalidId, TrackAction::Process, "Спроба опрацювання");
    printHistory(tracks, invalidId);
}
static void selectStrategy(GpsMonitoringFacade& facade, int number) {
    if (number == 1) facade.setStrategy(std::make_unique<AverageSpeed>());
    else if (number == 2) facade.setStrategy(std::make_unique<NormalizedSpeed>());
    else if (number == 3) facade.setStrategy(std::make_unique<ToleranceSpeed>(1.1));
    else throw std::invalid_argument("Номер стратегії має бути 1, 2 або 3");
}
static void strategyDemo(GpsMonitoringFacade& facade, int choice) {
    std::cout << "\n2. STRATEGY — ОЦІНКА ШВИДКОСТІ\n";
    const int id = facade.receivePoint(examplePoint());
    const SpeedInput input{1.2, 60.0, 60.0};
    std::cout << "Відстань: 1.2 км; час: 60 с; допустима швидкість: 60 км/год\n"
              << "Коефіцієнт допустимого відхилення: 1.1\n";
    for (int n = 1; n <= 3; ++n) {
        if (choice != 0 && choice != n) continue;
        selectStrategy(facade, n);
        const auto value = facade.assessSpeed(id, input);
        std::cout << n << ". " << facade.strategyName() << ": "
                  << std::fixed << std::setprecision(2) << value << ' '
                  << facade.strategyUnit() << '\n';
    }
    facade.setStrategy(std::make_unique<AverageSpeed>());
}
static void observerDemo(GpsMonitoringFacade& facade) {
    std::cout << "\n3. OBSERVER — СПОВІЩЕННЯ ПРО ПОДІЇ\n";
    const auto logger = std::make_shared<TrackLogger>(std::cout);
    const auto monitor = std::make_shared<ObjectMonitor>(std::cout);
    facade.subscribe(logger);
    facade.subscribe(monitor);
    std::cout << "Підключено журнал треку та монітор об'єкта.\n";
    int id = facade.receivePoint(examplePoint());
    facade.assessSpeed(id, {1.2, 60.0, 60.0});
    std::cout << "Відключено монітор об'єкта.\n";
    facade.unsubscribe(monitor);
    id = facade.receivePoint(examplePoint());
    facade.assessSpeed(id, {1.2, 60.0, 60.0});
    std::cout << "Повторно підключено монітор; відключено журнал.\n";
    facade.subscribe(monitor);
    facade.unsubscribe(logger);
    id = facade.receivePoint(examplePoint());
    facade.assessSpeed(id, {1.2, 60.0, 60.0});
    facade.unsubscribe(monitor);
}
static void facadeDemo(GpsMonitoringFacade& facade, TrackPointRepository& tracks) {
    std::cout << "\n4. FACADE — СПІЛЬНИЙ ІНТЕРФЕЙС GPS-МОНІТОРИНГУ\n";
    const auto logger = std::make_shared<TrackLogger>(std::cout);
    const auto monitor = std::make_shared<ObjectMonitor>(std::cout);
    facade.subscribe(logger);
    facade.subscribe(monitor);
    const int id = facade.receivePoint(examplePoint());
    showTransition(facade, tracks, id, TrackAction::Verify, "Перевірка");
    facade.setStrategy(std::make_unique<AverageSpeed>());
    const double speed = facade.assessSpeed(id, {1.2, 60.0, 60.0});
    std::cout << "Результат оцінки: " << speed << " км/год\n";
    showTransition(facade, tracks, id, TrackAction::Process, "Опрацювання");
    printHistory(tracks, id);
    facade.unsubscribe(logger);
    facade.unsubscribe(monitor);
}
int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const std::string mode = argc >= 2 ? argv[1] : "all";
    if (mode == "--help" || mode == "help") {
        std::cout << "Режими: state, strategy [1|2|3], observer, facade, all\n";
        return 0;
    }
    try {
        if (mode != "all" && mode != "state" && mode != "strategy" &&
            mode != "observer" && mode != "facade")
            throw std::invalid_argument("Невідомий режим. Використайте --help");
        if (argc > 3 || (argc == 3 && mode != "strategy"))
            throw std::invalid_argument("Неприпустимі параметри запуску");
        int choice = 0;
        if (argc == 3) {
            const std::string value = argv[2];
            if (value != "1" && value != "2" && value != "3")
                throw std::invalid_argument("Номер стратегії має бути 1, 2 або 3");
            choice = std::stoi(value);
        }
        const std::string databaseFile = "gps_lab10_" + mode + ".db";
        // Демонстраційна БД відтворюється для кожного режиму окремо.
        std::filesystem::remove(databaseFile);
        auto storage = createStorage(databaseFile);
        initializeStorage(storage);
        seedTestData(storage);
        SQLiteGpsObjectRepository objects(storage);
        TrackPointRepository tracks(storage);
        GpsMonitoringFacade facade(objects, tracks);
        std::cout << "Лабораторна робота №10 | Варіант 21 | GPS-моніторинг\n"
                  << "База даних: " << databaseFile << '\n';
        if (mode == "state" || mode == "all") stateDemo(facade, tracks);
        if (mode == "strategy" || mode == "all") strategyDemo(facade, choice);
        if (mode == "observer" || mode == "all") observerDemo(facade);
        if (mode == "facade" || mode == "all") facadeDemo(facade, tracks);
        std::cout << "\nРоботу завершено.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Помилка: " << error.what() << '\n';
        return 1;
    }
}
