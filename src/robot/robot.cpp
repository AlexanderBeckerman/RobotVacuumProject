#include "robot.h"
#include "config.h"
#include <cmath>
#include <algorithm>
#include <iostream>

void Robot::move(const Direction direction)
{
    // This function will move the robot in the given direction
    auto curr_location = this->location;
    if (curr_location.isChargingStation())
        this->battery_sensor.stopCharging();

    LOG(INFO) << "Moving: " << direction << "" << std::endl; 
    LOG(INFO) << "Current location in grid: " << curr_location << "" << std::endl;
    this->location = curr_location + direction;
    LOG(INFO) << "Battery level: " << this->battery_sensor.batteryLevel() << "" << std::endl;
    LOG(INFO) << "New location: " << this->location << "" << std::endl;
    this->battery_sensor.decreaseCharge();
}

void Robot::step()
{
    Direction direction = algorithm.nextMove();
    if (direction == Direction::STAY)
    {
        if (this->location.isChargingStation())
            this->battery_sensor.chargeBattery();
        else{
            LOG(INFO) << " Staying at location:" << this->location << "" << std::endl;
            this->clean();
        }
    } else {
        this->move(direction);
    }

    this->curr_steps++;
    addToPath();
}

void Robot::clean()
{
    Tile &t = this->dirt_sensor.getCurrentTile();
    LOG(INFO) << "Dirt level before clean: " << t.getDirtLevel() << " | ";
    LOG(INFO) << "Cleaning: " << this->location << " | ";
    t.setDirtLevel(t.getDirtLevel() - 1);
    this->config.setAmountToClean(this->config.getAmountToClean() - 1);
    LOG(INFO) << "Dirt level after clean: " << t.getDirtLevel() << " | ";
    // TODO(Ohad): log + output. printing for now
    LOG(INFO) << "Battery level: " << this->battery_sensor.batteryLevel() << "" << std::endl;
    this->battery_sensor.decreaseCharge();

}

void Robot::start(){
    // This function will start the robot and make it clean the map
    while (canContinue())
    {
        this->step();
    }
    logOutput();
}

void Robot::printLayout()
{
    LOG(INFO) << "--- LAYOUT INFORMATION (INSIDE ROBOT) ---" << std::endl;
    for (int i = 0; i < static_cast<int>(this->config.getLayout()->size()); i++)
    {
        for (int j = 0; j < static_cast<int>(this->config.getLayout()->at(i).size()); j++)
        {
            LOG(INFO) << (*this->config.getLayout())[i][j].getDirtLevel() << " ";
        }
        LOG(INFO) << "" << std::endl;
    }
    LOG(INFO) << "-----------" << std::endl;

}

bool Robot::canContinue()
{
    bool cleaned_all = this->location.isChargingStation() && this->config.getAmountToClean() == 0;
    bool stuck = !this->location.isChargingStation() && this->battery_sensor.batteryLevel() <= 0;
    bool still_have_steps = this->curr_steps < this->config.getMaxSteps();
    if (cleaned_all){
        LOG(INFO) << "Cleaned all and at charging station, exiting..." << std::endl;
    }
    else if(stuck){
        LOG(INFO) << "Stuck and battery is empty, exiting..." << std::endl;
    }
    else if (!still_have_steps){
        LOG(INFO) << "Reached max steps allowed, exiting..." << std::endl;
    }
    return still_have_steps && !cleaned_all && !stuck;
}

void Robot::addToPath(){
    Coordinate point = (Coordinate)this->config.getChargingStation() + this->location;
    std::string s = std::to_string(-point.y) + " " + std::to_string(point.x) + " " + std::to_string(this->battery_sensor.batteryLevel());
    this->path.push_back(s);
}

void Robot::logOutput() const
{
    std::ofstream moves; // moves file for the visualization (also contains battery data)
    std::ofstream output;
    moves.open("../../../output/moves.txt");
    output.open(this->config.output_path);
    Coordinate point = (Coordinate)this->config.getChargingStation();
    moves << -point.y << " " << point.x << " " << this->config.getMaxBatterySteps() <<"\n";
    output << -point.y << " " << point.x << "\n";
    for (auto &d : this->path)
    {
        std::istringstream iss(d);
        std::string y, x, battery;
        iss >> y >> x >> battery;
        moves << y << " " <<  x << " " << battery << "\n";
        output << y << " " << x << "\n";
    }
    output << "Total number of steps performed: " << this->curr_steps << "\n";
    output << "Amount of dirt left: " << this->config.getAmountToClean() << "\n";
    if (exit_cond == 0){
        output << "Success! no dirt left and robot is at the docking station." << "\n";
    }
    else if(exit_cond == 1){
        output << "Battery is empty and the robot is stuck!" << "\n";
    }
    moves.close();
    output.close();

}

