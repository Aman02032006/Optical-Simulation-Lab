#ifndef SCENE_HPP
#define SCENE_HPP

#pragma once
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

#include "optical_element.hpp"
#include "source.hpp"
#include "camera.hpp"
#include "mirror.hpp"
#include "lens.hpp"
#include "aperture.hpp"

struct SceneObject
{
    int id;
    std::string name;
    std::string type;

    std::shared_ptr<Source> source = nullptr;
    std::shared_ptr<OpticalElement> element = nullptr;

    bool isSelected = false;
    vec3 uiPosition;
    vec3 uiOrientation;

    vec3 getPosition()
    {
        if (source)
            return source->getPosition();
        if (element)
            return element->getPosition();
        return vec3(0, 0, 0);
    }
};

class Scene
{
private:
    std::vector<std::shared_ptr<SceneObject>> objects;
    int nextID = 1;

public:
    std::shared_ptr<SceneObject> selectedObject = nullptr;

    void AddObject(const std::string &type, vec3 position, vec3 orientation)
    {
        auto obj = std::make_shared<SceneObject>();
        obj->id = nextID++;
        obj->type = type;
        obj->name = type + " " + std::to_string(obj->id);
        obj->uiPosition = position;
        obj->uiOrientation = orientation;

        if (type == "Source")
        {
            obj->source = std::make_shared<Source>(position, orientation, FieldType::GAUSSIAN, 0, 0);
        }
        else if (type == "Camera")
        {
            obj->element = std::make_shared<Camera>(position, orientation, obj->name);
        }
        else if (type == "Mirror")
        {
            obj->element = std::make_shared<Mirror>(position, orientation, obj->name);
        }
        else if (type == "ConvexLens")
        {
            obj->element = std::make_shared<ConvexLens>(position, orientation, obj->name, 0.02, 0.1, 1.5);
        }
        else if (type == "ConcaveLens")
        {
            obj->element = std::make_shared<ConcaveLens>(position, orientation, obj->name, 0.02, 0.1, 1.5);
        }
        else if (type == "Iris")
        {
            obj->element = std::make_shared<Iris>(position, orientation, obj->name, 0.01, 0.02);
        }
        else if (type == "Slit")
        {
            obj->element = std::make_shared<Slit>(position, orientation, obj->name, 0.02, 0.01, 1e-4, 1, 2e-4);
        }

        objects.push_back(obj);
    }

    void ClearSelection()
    {
        selectedObject = nullptr;
        for (auto &obj : objects)
            obj->isSelected = false;
    }

    std::vector<OpticalElement *> GetSimulationElements()
    {
        std::vector<OpticalElement *> list;
        for (auto &obj : objects)
        {
            if (obj->element)
                list.push_back(obj->element.get());
        }
        return list;
    }

    std::vector<Source *> GetActiveSource()
    {
        std::vector<Source *> list;
        for (auto &obj : objects)
        {
            if (obj->source)
                list.push_back(obj->source.get());
        }
        return list;
    }

    std::vector<OpticalElement *> GetCameras()
    {
        std::vector<OpticalElement *> list;
        for (auto &obj : objects)
        {
            if (obj->type == "Camera")
                list.push_back(obj->element.get());
        }
        return list;
    }

    void Clear()
    {
        objects.clear();
        selectedObject = nullptr;
        nextID = 1;
    }

    void Select(int id)
    {
        for (auto &obj : objects)
        {
            obj->isSelected = (obj->id == id);
            if (obj->isSelected)
                selectedObject = obj;
        }
    }

    std::vector<std::shared_ptr<SceneObject>> &GetObjects() { return objects; }
};

#endif