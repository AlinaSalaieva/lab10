#include "Database.h"
#include "GpsMonitoringFacade.h"
#include "SQLiteGpsObjectRepository.h"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

static void check(bool condition, const std::string& name) {
    if (!condition) throw std::runtime_error(name);
    std::cout << "[OK] " << name << '\n';
}
// Лише тест: імітуємо відмову SQLite під час запису історії після оновлення стану.
static void executeTestSql(const std::string& file, const char* sql) {
    sqlite3* raw = nullptr;
    const int opened = sqlite3_open(file.c_str(), &raw);
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db(raw, sqlite3_close);
    if (opened != SQLITE_OK) throw std::runtime_error(raw ? sqlite3_errmsg(raw) : "sqlite3_open");
    if (sqlite3_exec(db.get(), sql, nullptr, nullptr, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db.get()));
}
class EventCounter final : public TrackObserver {
public:
    int received = 0;
    int deviations = 0;
    void update(const TrackEvent& e) override {
        if (e.type == TrackEventType::PointReceived) ++received;
        else ++deviations;
    }
};
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const std::string dbFile = "gps_lab10_test.db";
    try {
        std::filesystem::remove(dbFile);
        int persistedId = 0;
        {
            auto storage = createStorage(dbFile);
            initializeStorage(storage);
            SQLiteGpsObjectRepository objects(storage);
            TrackPointRepository tracks(storage);
            GpsMonitoringFacade facade(objects, tracks);
            const int objectId = objects.add({0, "Тестовий транспорт", "Автомобіль", "GPS-TEST", "Активний"});
            const auto a = std::make_shared<EventCounter>();
            const auto b = std::make_shared<EventCounter>();
            facade.subscribe(a);
            facade.subscribe(b);
            const TrackPoint sample{0, objectId, "2026-11-08 09:00:00", 50.45, 30.52, 72, 3};
            const int id = facade.receivePoint(sample);
            persistedId = id;
            check(tracks.findById(id).value().state == "Отримана" && a->received == 1 && b->received == 1,
                  "Facade: точку збережено, обидва підписники сповіщені");
            check(!facade.changeState(id, TrackAction::Process) && tracks.history(id).empty(),
                  "State: заборонений перехід не записано до історії");
            check(facade.changeState(id, TrackAction::Verify), "State: дозволена перевірка");
            auto history = tracks.history(id);
            check(history.size() == 1 && history[0].oldState == "Отримана" && history[0].newState == "Перевірена" &&
                  !history[0].changedAt.empty() && tracks.findById(id).value().state == "Перевірена",
                  "COMMIT: стан та історію збережено разом");
            executeTestSql(dbFile, "CREATE TRIGGER fail_history BEFORE INSERT ON TrackPointStateHistory "
                                   "BEGIN SELECT RAISE(ABORT, 'test rollback'); END;");
            bool failed = false;
            try { facade.changeState(id, TrackAction::Process); }
            catch (const std::exception& e) {
                failed = std::string(e.what()).find("test rollback") != std::string::npos;
            }
            check(failed && tracks.findById(id).value().state == "Перевірена" && tracks.history(id).size() == 1,
                  "ROLLBACK: після помилки INSERT стан та історія не змінилися");
            executeTestSql(dbFile, "DROP TRIGGER fail_history;");
            check(facade.changeState(id, TrackAction::Process) && tracks.history(id).size() == 2 &&
                  tracks.findById(id).value().state == "Опрацьована", "COMMIT: повторна спроба успішна");
            facade.setStrategy(std::make_unique<NormalizedSpeed>());
            check(std::abs(facade.assessSpeed(id, {1.2, 60, 60}) - 1.2) < 1e-9 && a->deviations == 1 && b->deviations == 1,
                  "Facade: стратегія замінена, відхилення виявлено");
            facade.assessSpeed(id, {0.5, 60, 60});
            check(a->deviations == 1 && b->deviations == 1, "Observer: у межах норми подія відхилення відсутня");
            facade.unsubscribe(b);
            facade.assessSpeed(id, {1.2, 60, 60});
            check(a->deviations == 2 && b->deviations == 1, "Observer: відключений підписник не отримав подію");
            const int invalidId = facade.receivePoint(sample);
            check(facade.changeState(invalidId, TrackAction::Invalidate) &&
                  tracks.findById(invalidId).value().state == "Некоректна" && tracks.history(invalidId).size() == 1,
                  "State: некоректний стан збережено з історією");
        }
        {
            auto storage = createStorage(dbFile);
            initializeStorage(storage);
            TrackPointRepository tracks(storage);
            check(tracks.findById(persistedId).value().state == "Опрацьована" && tracks.history(persistedId).size() == 2,
                  "SQLite: стан та історія доступні після повторного відкриття БД");
        }
        std::filesystem::remove(dbFile);
        std::cout << "Інтеграційні тести пройдено.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << e.what() << '\n';
        return 1;
    }
}
