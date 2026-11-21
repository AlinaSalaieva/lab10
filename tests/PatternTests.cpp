#include "TrackPointState.h"
#include "SpeedStrategy.h"
#include "TrackObserver.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

static void check(bool condition, const std::string& name) {
    if (!condition) throw std::runtime_error(name);
    std::cout << "[OK] " << name << '\n';
}
static bool isNearlyEqual(double actual, double expected) { return std::abs(actual - expected) < 1e-9; }
class CountingObserver final : public TrackObserver {
public:
    int received = 0;
    int deviations = 0;
    void update(const TrackEvent& event) override {
        if (event.type == TrackEventType::PointReceived) ++received;
        else ++deviations;
    }
};

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try {
        TrackPointContext state("Отримана");
        check(!state.transition(TrackAction::Process) && state.state() == "Отримана",
              "State: опрацювання без перевірки заборонено");
        check(state.transition(TrackAction::Verify) && state.state() == "Перевірена",
              "State: отримана -> перевірена");
        check(state.transition(TrackAction::Process) && state.state() == "Опрацьована",
              "State: перевірена -> опрацьована");
        check(!state.transition(TrackAction::Invalidate), "State: опрацьована — кінцевий стан");
        TrackPointContext invalid("Отримана");
        check(invalid.transition(TrackAction::Invalidate) && invalid.state() == "Некоректна",
              "State: отримана -> некоректна");
        check(!invalid.transition(TrackAction::Verify) && !invalid.transition(TrackAction::Process),
              "State: некоректна — кінцевий стан");
        TrackPointContext verified("Перевірена");
        check(verified.transition(TrackAction::Invalidate) && verified.state() == "Некоректна",
              "State: перевірена -> некоректна");
        const SpeedInput input{1.2, 60, 60};
        SpeedContext speed;
        check(isNearlyEqual(speed.calculate(input), 72.0), "Strategy: середня швидкість = 72");
        speed.setStrategy(std::make_unique<NormalizedSpeed>());
        check(isNearlyEqual(speed.calculate(input), 1.2), "Strategy: нормована швидкість = 1.2");
        speed.setStrategy(std::make_unique<ToleranceSpeed>(1.1));
        check(isNearlyEqual(speed.calculate(input), 72.0 / 66.0), "Strategy: оцінка з коефіцієнтом 1.1");
        bool rejected = false;
        try { speed.calculate({1.2, 0, 60}); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Strategy: нульовий час відхилено");
        rejected = false;
        try { speed.calculate({1.2, 60, 0}); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Strategy: нульову допустиму швидкість відхилено");
        rejected = false;
        try { ToleranceSpeed invalidCoefficient(0.9); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "Strategy: некоректний коефіцієнт відхилено");
        TrackSubject subject;
        auto a = std::make_shared<CountingObserver>();
        auto b = std::make_shared<CountingObserver>();
        subject.subscribe(a);
        subject.subscribe(b);
        subject.notify({TrackEventType::PointReceived, 1, 1, 72, 0});
        subject.notify({TrackEventType::SpeedDeviation, 1, 1, 72, 60});
        check(a->received == 1 && b->received == 1 && a->deviations == 1 && b->deviations == 1,
              "Observer: два типи подій для двох підписників");
        subject.unsubscribe(b);
        subject.notify({TrackEventType::PointReceived, 2, 1, 72, 0});
        check(a->received == 2 && b->received == 1, "Observer: відключення другого підписника");
        subject.subscribe(b);
        subject.unsubscribe(a);
        subject.notify({TrackEventType::SpeedDeviation, 2, 1, 72, 60});
        check(a->deviations == 1 && b->deviations == 2, "Observer: повторна підписка та незалежне відключення");
        std::weak_ptr<TrackObserver> expired = b;
        b.reset();
        subject.subscribe(a);
        subject.notify({TrackEventType::PointReceived, 3, 1, 72, 0});
        check(expired.expired() && a->received == 3, "Observer: знищений підписник не перешкоджає сповіщенню активного");
        std::ostringstream log, monitor;
        auto logger = std::make_shared<TrackLogger>(log);
        auto objectMonitor = std::make_shared<ObjectMonitor>(monitor);
        subject.subscribe(logger);
        subject.subscribe(objectMonitor);
        subject.notify({TrackEventType::PointReceived, 3, 1, 72, 0});
        subject.notify({TrackEventType::SpeedDeviation, 3, 1, 72, 60});
        check(log.str().find("отримана") != std::string::npos && log.str().find("відхилення") != std::string::npos,
              "Observer: журнал треку обробив обидві події");
        check(monitor.str().find("оновлено") != std::string::npos && monitor.str().find("перевищення") != std::string::npos,
              "Observer: монітор об'єкта обробив обидві події");
        std::cout << "Модульні тести пройдено.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << e.what() << '\n';
        return 1;
    }
}
