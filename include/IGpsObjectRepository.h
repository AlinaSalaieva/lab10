#pragma once
#include <optional>
#include <vector>
#include "GpsObject.h"

// Абстрактний інтерфейс; код клієнта не залежить від конкретної СУБД.
class IGpsObjectRepository {
public:
    virtual ~IGpsObjectRepository() = default;
    virtual int add(const GpsObject& object) = 0;
    virtual std::optional<GpsObject> findById(int id) = 0;
    virtual std::vector<GpsObject> findAll() = 0;
    virtual bool update(const GpsObject& object) = 0;
    virtual bool remove(int id) = 0;
};
