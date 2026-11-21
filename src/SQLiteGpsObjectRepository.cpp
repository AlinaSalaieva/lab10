#include "SQLiteGpsObjectRepository.h"

SQLiteGpsObjectRepository::SQLiteGpsObjectRepository(GpsStorage& storage)
    : storage_(storage) {}

int SQLiteGpsObjectRepository::add(const GpsObject& object) {
    return storage_.insert(object);
}

std::optional<GpsObject> SQLiteGpsObjectRepository::findById(int id) {
    return storage_.get_optional<GpsObject>(id);
}

std::vector<GpsObject> SQLiteGpsObjectRepository::findAll() {
    return storage_.get_all<GpsObject>();
}

bool SQLiteGpsObjectRepository::update(const GpsObject& object) {
    if (!findById(object.id)) return false;
    storage_.update(object);
    return true;
}

bool SQLiteGpsObjectRepository::remove(int id) {
    if (!findById(id)) return false;
    storage_.remove<GpsObject>(id);
    return true;
}
