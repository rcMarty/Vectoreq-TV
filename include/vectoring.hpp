#pragma once

#include <map>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <SPIFFS.h>

double WHEELDIAMETER = 235;     // in milimeters
double WHEELTRACK = 1000;       // in milimeters
u_int8_t MAX_STEER_TRAVEL = 37; // in degrees

struct Wheel
{
    int left = 0;
    int right = 4096;
    int middle = 2048;
};

class Vectoring
{
private:
    std::map<double, double> turning_circle_radius;
    std::string default_turning_radius_path = "/steering-angles.csv";

    std::vector<double> modifiers = {0, 0.05, 0.1, 0.15, 0.25, 0.35, 0.5, 1.0, 1.1, 1.2, 1.3};
    int modifier_index = 0;

    double throttle = 0;
    double steer_travel = 0;
    double modifier = 0.05;

    double regl_R = 0;
    double regl_L = 0;

    // TODO loading torque vectoring table
    // TODO loading z potenciometru na steer_travel
    // idk jak se brzdí motorama ale potřebujeme inputy na rekuperaci

    // Loads csv file with steering radiuses per one degree
    // (max 37 degrees) so far
    bool load_csv(std::string path)
    {
        printf("Loading csv file: %s\n", path.c_str());

        if (!SPIFFS.begin(true))
        {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return false;
        }

        File file = SPIFFS.open(path.c_str(), "r");
        if (!file)
        {
            Serial.println("Failed to open file for reading");
            return false;
        }

        printf("Reading csv file: %s\n", path.c_str());

        std::string line, word = "";
        double key, value = 0;

        file.readStringUntil('\n'); // PI4O vOLE KURVA

        while (file.available())
        {
            String rawLine = file.readStringUntil('\n');
            line = rawLine.c_str();

            // printf("Line: \"%s\"\n", line.c_str()); // TODO remove
            //  auto l = line.find(",");
            //  printf("aaaaa\n");
            //  auto substr = line.substr(0, l);
            //  printf("substr: %s\n", substr.c_str());
            //  key = std::stod(substr);
            //  printf("key: %f\n", key);

            key = std::stod(line.substr(0, line.find(",")));
            value = std::stod(line.substr(line.find(",") + 1, line.length()));

            // printf("REEEEEEEEE: %s\n", rawLine.c_str());

            turning_circle_radius[key] = value;
            printf("%f %f\n", key, value);
        }
        file.close();

        return true;
    }

    double get_radius(const double steer_degrees)
    {
        return turning_circle_radius[round(steer_degrees)];
    }

    // calculate difference between left and right wheel
    double calculate_difference(const double radius)
    {

        double inner = radius / WHEELDIAMETER;
        double outer = (radius + WHEELTRACK) / WHEELDIAMETER;

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

    double convert_to_degrees(const int value)
    {
        double oldMin = wheel.left;
        double oldMax = wheel.right;
        double newMin = -MAX_STEER_TRAVEL;
        double newMax = MAX_STEER_TRAVEL;

        // turning right
        if (value > wheel.middle)
        {
            // mapping value to reange form middle to maxsteertravel
            // from middle to max steer value
            oldMin = wheel.middle;
            newMin = 0;
            return (((double)value - oldMin) / (oldMax - oldMin)) * (newMax - newMin) + newMin;
        }
        // turning left
        else if (value < wheel.middle)
        {
            oldMax = wheel.middle;
            newMax = 0;
            return (((double)value - oldMin) / (oldMax - oldMin)) * (newMax - newMin) + newMin;
        }
        else
        {
            return 0;
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

    void next_modifier()
    {
        if (modifier_index < modifiers.size() - 1)
            modifier_index++;
        else
            modifier_index = 0;

        this->modifier = modifiers[modifier_index];
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

        double inner_steer_travel = steer_travel;
        bool left = false;

        if (steer_travel < 0)
        {
            left = true;
            inner_steer_travel *= -1;
        }

        double difference = modifier * 10 * calculate_difference(this->get_radius(inner_steer_travel));
        // printf("Difference: %f\n", calculate_difference(this->get_radius(inner_steer_travel)));

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

        if (this->regl_L <= 5 || this->regl_R <= 5)
        {
            this->regl_L = 0;
            this->regl_R = 0;
        }

        // printf("");
    }

    // print function for debug
    const char *print()
    {
        std::ostringstream oss;
        oss << "Gas: " << throttle << "% "
            << "   steer travel: " << steer_travel << "° "
            << std::endl;

        oss << "Modifier: " << modifier << " "
            << "   Modifier index: " << modifier_index << " "
            << std::endl;

        oss << "R: " << regl_R << "% "
            << " L: " << regl_L << "% "
            << std::endl;
        oss << "_______________________________________________________________________" << std::endl;

        return oss.str().c_str();
    }

    double get_R()
    {
        return regl_R;
    }

    double get_L()
    {
        return regl_L;
    }
};
