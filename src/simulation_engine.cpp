#include "simulation_engine.hpp"
#include "ray.hpp"
#include "wavefront.hpp"
#include "utils.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

std::vector<double> SimulationEngine::FlattenGrid(const std::vector<std::vector<double>> &grid, int N)
{
    std::vector<double> flat;

    if (grid.empty() || grid.size() != N)
        return std::vector<double>(N * N, 0.0);

    flat.reserve(N * N);
    for (int i = 0; i < N; i++)
    {
        if (grid[i].size() != N)
            flat.insert(flat.end(), N, 0.0);
        else
            flat.insert(flat.end(), grid[i].begin(), grid[i].end());
    }
    return flat;
}

std::vector<OpticalElement *> SimulationEngine::Run(Scene &scene)
{
    std::vector<Source *> Sources = scene.GetActiveSource();
    std::vector<OpticalElement *> Elements = scene.GetSimulationElements();

    if (Sources.empty())
    {
        std::cout << "[Simulation] : No Source found in the scene." << std::endl;
        return scene.GetCameras();
    }
    else
        std::cout << "[Simulation] : " << Sources.size() << " sources found in the scene." << std::endl;

    std::set<Path> PossiblePaths;

    for (auto &Src : Sources)
    {
        for (int i = 1; i <= 100; i++)
        {
            ray beam(Src->getPosition(), Src->getOrientation());
            std::set<OpticalElement *> interacted_with;
            Path CurrentPath;
            CurrentPath.source = Src;

            while (beam.isAlive())
            {
                auto min_dist = INF;
                OpticalElement *closest_element = NULL;

                for (auto element : Elements)
                {
                    if (interacted_with.find(element) != interacted_with.end())
                        continue;
                    auto dist = element->hit(beam);

                    if (dist != -999 && dist <= min_dist)
                    {
                        closest_element = element;
                        min_dist = dist;
                    }
                }

                if (closest_element != NULL)
                {
                    beam.propagate(min_dist);
                    closest_element->interact_ray(beam);
                    interacted_with.insert(closest_element);
                    CurrentPath.Elements.push_back(closest_element);
                }
                else
                    beam.kill();
            }

            PossiblePaths.insert(CurrentPath);
        }
    }
    std::cout << "[Simulation] : Paths calculated" << std::endl;

    for (auto &Src : Sources)
        Src->E.initialize();

    for (auto Path : PossiblePaths)
    {
        auto E_field = Path.source->E;
        for (auto element : Path.Elements)
        {
            double dist = element->hit(E_field.getNormal());
            if (dist != -999.0)
            {
                E_field.propagate(dist);
                element->interact_wavefront(E_field);
            }
        }
    }

    std::cout << "[Simulation] : Paths Traversed" << std::endl;
    std::cout << "[Simulation] : Simulation finished" << std::endl;

    return scene.GetCameras();
}