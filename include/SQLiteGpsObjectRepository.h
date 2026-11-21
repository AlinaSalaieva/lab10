#pragma once
#include "Database.h"
#include "IGpsObjectRepository.h"

class SQLiteGpsObjectRepository final : public IGpsObjectRepository {
public:
    explicit SQLiteGpsObjectRepository(GpsStorage& storage);
    int add(const GpsObject& object) override;
    std::optional<GpsObject> findById(int id) override;
    std::vector<GpsObject> findAll() override;
    bool update(const GpsObject& object) override;
    bool remove(int id) override;
private:
    GpsStorage& storage_;
};
