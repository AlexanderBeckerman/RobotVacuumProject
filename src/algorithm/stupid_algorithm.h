#pragma once
#include "config.h"
#include "expanding_map.h"
#include "relative_point.h"
#include "sensors.h"
#include <stack>
#include <vector>

class StupidAlgorithm {
    // State.
    DirtSensor &dirt_sensor;
    WallsSensor &wall_sensor;
    BatteryMeter &battery_sensor;
    ExpandingMap map;
    RelativePoint robot_location;
    std::stack<Direction> path;
    int steps_since_charge = 0;

    // Helper functions.
    bool shouldMove(const Direction d);
    Direction returnToChargingStation();
    bool notEnoughBattery();
    void setLocation(const Direction d);
    void updateMapFloor();
    void updateMapWall(const Direction d);
    std::vector<Direction> getPossibleDirections();
    Direction
    selectDirection(const std::vector<Direction> &possible_directions);

  public:
    StupidAlgorithm(DirtSensor &ds, WallsSensor &ws,
                    BatteryMeter &bs)
        : dirt_sensor(ds), wall_sensor(ws), battery_sensor(bs),
          map(ExpandingMap()), robot_location(RelativePoint{0, 0}) {
        // Add the charging station tile.
        this->map.addChargingStationTile(this->robot_location);
    }

    StupidAlgorithm(const StupidAlgorithm &other) = delete;

    Direction nextMove();

    friend class AlgorithmTest;
    friend class ExpandingMapTest;
};