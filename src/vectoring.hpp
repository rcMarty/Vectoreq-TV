#pragma once

#include <map>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

double WHEELDIAMETER = 235; // in milimeters
double WHEELTRACK = 1000;   // in milimeters
u_int8_t MAX_STEER_TRAVEL = 37;

struct Wheel
{
    int left;
    int right;
    int middle;
};

class Vectoring
{
private:
    std::map<double, double> turning_circle_radius;
    std::string default_turning_radius_path = "steering-angles.csv";

    double throttle = 0;
    double steer_travel = 0;
    double modifier = 1;

    double regl_R = 0;
    double regl_L = 0;

    // TODO loading torque vectoring table
    // TODO loading z potenciometru na steer_travel
    // idk jak se brzdí motorama ale potřebujeme inputy na rekuperaci

    // Loads csv file with steering radiuses per one degree
    // (max 37 degrees) so far
    bool load_csv(std::string path)
    {
        std::fstream file;
        file.open(path, std::ios::in);

        printf("Loading csv file: %s\n", path.c_str());

        if (!file.is_open())
            return false;

        std::string line, word = "";
        double key, value = 0;

        getline(file, line);

        while (getline(file, line))
        {
            // printf("Line: \"%d\"\n", line.c_str());
            // if (line.empty())
            // {
            //     printf("Empty line\n");
            //     continue;
            // }

            key = std::stod(line.substr(0, line.find(",")));

            value = std::stod(line.substr(line.find(",") + 1, line.length()));

            turning_circle_radius[key] = value;
            // printf("%f %f\n", key, value);
        }

        return true;
    }

    double get_radius(const double steer_degrees)
    {
        return this->turning_circle_radius[round(steer_degrees)];
    }

    // calculate difference between left and right wheel
    double calculate_difference(const double radius)
    {
        double inner = radius / WHEELDIAMETER;
        double outer = (radius + WHEELTRACK) / WHEELDIAMETER;
        // std::cout << inner << "   " << outer << "       " << radius << std::endl;
        double x = (100 * inner) / outer;
        x = x / 100;
        return x;
    }

public:
    Wheel wheel;

    Vectoring(std::string input_path = "")
    {
        if (input_path != "")
            load_csv(input_path);

        else
            load_csv(this->default_turning_radius_path);
    }

    void set_wheel(const Wheel wheel)
    {
        this->wheel = wheel;
    }

    double convert_to_degrees(const double value)
    {
        if (value > wheel.middle)
        {
            return (value - wheel.middle) / ((wheel.right - wheel.middle) * MAX_STEER_TRAVEL);
        }
        else
        {
            return (value - wheel.middle) / ((wheel.middle - wheel.left) * MAX_STEER_TRAVEL * -1);
        }
    }

    // Read gas from a potentiometer on pedal
    bool update_throttle(const double throttle)
    {
        if (throttle > 100 || throttle < 0)
            return false;
        this->throttle = throttle;
        return true;
    }

    // Read steer travel in degrees from a potentiometer on steering wheel
    bool update_steer_travel(const double steer_travel)
    {
        if (steer_travel > MAX_STEER_TRAVEL || steer_travel < -MAX_STEER_TRAVEL)
        {
            this->regl_L = 0;
            this->regl_R = 0;
            std::string text_exception = "steer_travel (" + std::to_string(steer_travel) + ") is greater than max_steer_travel (" + std::to_string(MAX_STEER_TRAVEL) + ")";
            std::__throw_out_of_range(text_exception.c_str());
            return false;
        }

        this->steer_travel = steer_travel;
        return true;
    }

    // Updates additional modifier for torque vectoring
    // Range from 0 to 1
    //  0.5 = 50% of torque vectoring
    bool update_modifier(const double modifier)
    {
        if (modifier > 2 || modifier < 0)
        {
            std::string text_exception = "modifier (" + std::to_string(modifier) + ") is greater than 1 or less than 0";
            std::__throw_out_of_range(text_exception.c_str());
            return false;
        }

        this->modifier = modifier;
        return true;
    }

    // Calculates power for left and right wheel in percent
    void calculate_torque() // const double steer_travel)
    {
        // no torque vectoring for small steer travel
        if (steer_travel < 1 && steer_travel > -1)
            return;

        auto inner_steer_travel = steer_travel;
        bool left = false;

        if (steer_travel < 0)
        {
            left = true;
            inner_steer_travel *= -1;
        }

        auto difference = modifier * calculate_difference(this->get_radius(inner_steer_travel));
        if (left)
        {
            this->regl_L = throttle - difference;
            this->regl_R = throttle + difference;
        }
        else
        {
            this->regl_L = throttle + difference;
            this->regl_R = throttle - difference;
        }

        if (this->regl_L >= 100)
            this->regl_L = 100;

        if (this->regl_R >= 100)
            this->regl_R = 100;
    }

    // print function for debug
    const char *print()
    {
        std::ostringstream oss;
        oss << "Gas: " << throttle << "% "
            << "   steer travel: " << steer_travel << "° "
            << std::endl;

        oss << "R: " << regl_R << "% "
            << " L: " << regl_L << "% "
            << std::endl;
        oss << "_______________________________________________________________________" << std::endl
            << std::endl
            << std::endl;

        return oss.str().c_str();
    }
};
