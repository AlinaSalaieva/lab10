#pragma once
#include <memory>
#include <string>

struct SpeedInput {
    double distanceKm;
    double timeSeconds;
    double allowedKmh;
};

class SpeedStrategy {
public:
    virtual ~SpeedStrategy() = default;
    virtual double calculate(const SpeedInput& input) const = 0;
    virtual std::string name() const = 0;
    virtual std::string unit() const = 0;
};
class AverageSpeed final : public SpeedStrategy {
public:
    double calculate(const SpeedInput& input) const override;
    std::string name() const override;
    std::string unit() const override;
};
class NormalizedSpeed final : public SpeedStrategy {
public:
    double calculate(const SpeedInput& input) const override;
    std::string name() const override;
    std::string unit() const override;
};
class ToleranceSpeed final : public SpeedStrategy {
public:
    explicit ToleranceSpeed(double coefficient = 1.1);
    double calculate(const SpeedInput& input) const override;
    std::string name() const override;
    std::string unit() const override;
private:
    double coefficient_;
};

class SpeedContext {
public:
    SpeedContext();
    void setStrategy(std::unique_ptr<SpeedStrategy> strategy);
    double calculate(const SpeedInput& input) const;
    std::string name() const;
    std::string unit() const;
private:
    std::unique_ptr<SpeedStrategy> strategy_;
};
