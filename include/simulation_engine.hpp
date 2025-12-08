#ifndef SIMULATION_ENGINE_HPP
#define SIMULATION_ENGINE_HPP

#pragma once

#include <vector>
#include <set>
#include "scene.hpp"

class SimulationEngine
{
public:
    static std::vector<OpticalElement *> Run(Scene &scene);

private:
    struct Path
    {
        Source* source = nullptr;
        std::vector<OpticalElement *> Elements;
        bool operator<(const Path &other) const noexcept { return Elements < other.Elements; }
        bool operator==(const Path &other) const noexcept { return Elements == other.Elements; }
    };

    static std::vector<double> FlattenGrid(const std::vector<std::vector<double>> &grid, int N);
};

#endif