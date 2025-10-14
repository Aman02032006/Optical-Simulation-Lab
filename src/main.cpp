#include "ray.hpp"
#include "vec3.hpp"
#include "wavefront.hpp"
#include "optical_element.hpp"
#include "utils.hpp"
#include <vector>
#include <set>

struct Path
{
    std::vector<OpticalElement *> elements;

    bool operator<(const Path &other) const noexcept
    {
        return elements < other.elements;
    }

    bool operator==(const Path &other) const noexcept
    {
        return elements == other.elements;
    }
};

int main()
{
    std::vector<OpticalElement *> ElementsList;

    std::set<Path> PossiblePaths;

    for (int i = 1; i <= 100; i++)
    {
        ray beam(point3(0, 0, 0), vec3(0, 0, 1));
        std::set<OpticalElement *> interacted_with;
        Path CurrentPath;

        while (beam.isAlive())
        {
            auto min_dist = INF;
            OpticalElement *closest_element = NULL;

            for (auto element : ElementsList)
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
                CurrentPath.elements.push_back(closest_element);
            }
        }

        PossiblePaths.insert(CurrentPath);
    }
}