#include "SpeedStrategy.h"
#include <cmath>
#include <stdexcept>
#include <utility>

static double average(const SpeedInput& input) {
    if (!std::isfinite(input.distanceKm) || input.distanceKm < 0 ||
        !std::isfinite(input.timeSeconds) || input.timeSeconds <= 0 ||
        !std::isfinite(input.allowedKmh) || input.allowedKmh <= 0)
        throw std::invalid_argument("Некоректні параметри оцінки швидкості");
    const double speed = input.distanceKm / input.timeSeconds * 3600.0;
    if (!std::isfinite(speed)) throw std::invalid_argument("Некоректне значення швидкості");
    return speed;
}

double AverageSpeed::calculate(const SpeedInput& input) const { return average(input); }
std::string AverageSpeed::name() const { return "Середня швидкість"; }
std::string AverageSpeed::unit() const { return "км/год"; }

double NormalizedSpeed::calculate(const SpeedInput& input) const {
    return average(input) / input.allowedKmh;
}
std::string NormalizedSpeed::name() const { return "Нормована швидкість"; }
std::string NormalizedSpeed::unit() const { return "(безрозмірна)"; }

ToleranceSpeed::ToleranceSpeed(double coefficient) : coefficient_(coefficient) {
    if (!std::isfinite(coefficient_) || coefficient_ < 1.0)
        throw std::invalid_argument("Коефіцієнт має бути не меншим за 1");
}
double ToleranceSpeed::calculate(const SpeedInput& input) const {
    return average(input) / input.allowedKmh / coefficient_;
}
std::string ToleranceSpeed::name() const { return "Оцінка з коефіцієнтом допустимого відхилення"; }
std::string ToleranceSpeed::unit() const { return "(безрозмірна)"; }

SpeedContext::SpeedContext() : strategy_(std::make_unique<AverageSpeed>()) {}
void SpeedContext::setStrategy(std::unique_ptr<SpeedStrategy> strategy) {
    if (!strategy) throw std::invalid_argument("Стратегію не визначено");
    strategy_ = std::move(strategy);
}
double SpeedContext::calculate(const SpeedInput& input) const { return strategy_->calculate(input); }
std::string SpeedContext::name() const { return strategy_->name(); }
std::string SpeedContext::unit() const { return strategy_->unit(); }
