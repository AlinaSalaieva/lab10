#pragma once
#include <string>
#include <stdexcept>
#include <sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>
#include "GpsObject.h"
#include "TrackPoint.h"
#include "TrackPointStateHistory.h"

// ORM-відображення: C++ тип -> таблиця, поле -> стовпець.
inline auto createStorage(const std::string& databaseFile) {
    using namespace sqlite_orm;
    return make_storage(databaseFile,
        make_table("GpsObject",
            make_column("id", &GpsObject::id, primary_key().autoincrement()),
            make_column("name", &GpsObject::name),
            make_column("objectType", &GpsObject::objectType),
            make_column("deviceCode", &GpsObject::deviceCode),
            make_column("status", &GpsObject::status)
        ),
        make_table("TrackPoint",
            make_column("id", &TrackPoint::id, primary_key().autoincrement()),
            make_column("objectId", &TrackPoint::objectId),
            make_column("recordedAt", &TrackPoint::recordedAt),
            make_column("latitude", &TrackPoint::latitude),
            make_column("longitude", &TrackPoint::longitude),
            make_column("speedKmh", &TrackPoint::speedKmh),
            make_column("accuracyMeters", &TrackPoint::accuracyMeters),
            make_column("state", &TrackPoint::state, default_value("Отримана")),
            foreign_key(&TrackPoint::objectId).references(&GpsObject::id)
        ),
        make_table("TrackPointStateHistory",
            make_column("id", &TrackPointStateHistory::id, primary_key().autoincrement()),
            make_column("pointId", &TrackPointStateHistory::pointId),
            make_column("oldState", &TrackPointStateHistory::oldState),
            make_column("newState", &TrackPointStateHistory::newState),
            make_column("changedAt", &TrackPointStateHistory::changedAt),
            foreign_key(&TrackPointStateHistory::pointId).references(&TrackPoint::id)
        )
    );
}
using GpsStorage = decltype(createStorage(""));

// Увімкнення перевірки FK на з'єднанні, яке використовує ORM.
inline void initializeStorage(GpsStorage& storage) {
    // SQLite FK enforcement is a connection-level setting.
    // Enable and verify it whenever sqlite_orm opens its connection.
    storage.on_open = [](sqlite3* db) {
        int enabled = 0;
        const int rc = sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_FKEY, 1, &enabled);
        if (rc != SQLITE_OK || enabled != 1) {
            throw std::runtime_error("Не вдалося увімкнути перевірку зовнішніх ключів SQLite");
        }
    };
    storage.open_forever();
    storage.sync_schema();
}
