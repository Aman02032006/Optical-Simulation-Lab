#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#pragma once

#include <string>
#include <map>
#include <iostream>
#include <vector>

// Graphics Headers (Same as main.cpp)
#include <glad/glad.h>
#include "stb_image.h"  // We will assume this is available
#include <GLFW/glfw3.h> // OR <GLFW/glfw3.h> if you aren't using GLAD

struct Texture
{
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

class TextureManager
{
private:
    std::map<std::string, Texture> textures;

public:
    // Load an image from file (PNG/JPG)
    bool Load(const std::string &name, const std::string &filepath)
    {
        int w, h, channels;
        // Load raw pixels
        unsigned char *data = stbi_load(filepath.c_str(), &w, &h, &channels, 4); // Force 4 channels (RGBA)

        if (!data)
        {
            std::cerr << "[TextureManager] Failed to load image : " << filepath << std::endl;
            std::cerr << "[TextureManager] Reason : " << stbi_failure_reason() << std::endl;
            return false;
        }

        // Create OpenGL Texture
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        // Setup Filtering (Linear makes icons look smooth when resized)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Clamp to edge prevents weird artifacts at the border
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Upload pixels to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Cleanup RAM (It's on VRAM now)
        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        textures[name] = {id, w, h};
        return true;
    }

    // Get texture ID by name (returns 0 if missing)
    GLuint GetID(const std::string &name)
    {
        if (textures.find(name) != textures.end())
            return textures[name].id;
        return 0;
    }
};

#endif