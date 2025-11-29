// Standard Imports
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>
#include <cmath>

// Graphics Headers
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

// Project Headers
#include "optical_element.hpp"
#include "ray.hpp"
#include "source.hpp"
#include "utils.hpp"
#include "vec3.hpp"
#include "wavefront.hpp"
#include "camera.hpp"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

struct Path
{
    std::vector<OpticalElement *> elements;
    bool operator<(const Path &other) const noexcept { return elements < other.elements; }
    bool operator==(const Path &other) const noexcept { return elements == other.elements; }
};

std::vector<double> FlattenGrid(const std::vector<std::vector<double>> &grid, int N)
{
    std::vector<double> flat;
    flat.reserve(N * N);
    for (int i = 0; i < N; i++)
    {
        flat.insert(flat.end(), grid[i].begin(), grid[i].end());
    }
    return flat;
}

void RunSimulation(std::vector<OpticalElement *> Elements, Source &source, Camera &camera, std::vector<double> &outIntensity, std::vector<double> &outPhase, double &outSize)
{
    std::set<Path> PossiblePaths;

    for (int i = 1; i <= 100; i++)
    {
        ray beam(source.getPosition(), source.getOrientation());
        std::set<OpticalElement *> interacted_with;
        Path CurrentPath;

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
                CurrentPath.elements.push_back(closest_element);
            }
            else
                beam.kill();
        }

        PossiblePaths.insert(CurrentPath);
    }
    std::cout << "[main] : Paths calculated" << std::endl;

    for (auto Path : PossiblePaths)
    {
        auto E_field = source.E;
        for (auto element : Path.elements)
        {
            double dist = element->hit(E_field.getNormal());
            if (dist != -999.0)
            {
                E_field.propagate(dist);
                element->interact_wavefront(E_field);
            }
        }
    }

    std::cout << "[main] : Paths Traversed" << std::endl;

    WaveFront &outField = camera.getSensedWaveFront();

    outIntensity = FlattenGrid(outField.Intensity(), outField.N);
    outPhase = FlattenGrid(outField.Phase(), outField.N);
    outSize = outField.getSize();

    double MaxIntensity = 0;
    if (!outIntensity.empty())
        MaxIntensity = *std::max_element(outIntensity.begin(), outIntensity.end());

    for (size_t i = 0; i < outIntensity.size(); i++)
    {
        if (outIntensity[i] < MaxIntensity * 0.0001)
            outPhase[i] = 0;
    }

    std::cout << "[main] : Simulation finished" << std::endl;
}

void UpdateTexture(GLuint &texID, const std::vector<double> &data, int w, int h, double maxVal, bool isPhase)
{
    if (data.empty())
        return;

    std::vector<unsigned char> pixels;
    pixels.reserve(w * h * 4);

    for (double val : data)
    {
        ImVec4 c;
        if (isPhase)
        {
            // Map -PI..PI to 0..1
            double norm = (val + PI) / (2.0 * PI);
            c = ImPlot::SampleColormap(norm, ImPlotColormap_Twilight);
        }
        else
        {
            // Normalize Intensity
            double norm = (maxVal > 0) ? (val / maxVal) : 0.0;
            // Apply gamma correction for better visibility of faint rings
            norm = pow(norm, 0.5);
            c = ImPlot::SampleColormap(norm, ImPlotColormap_Plasma);
        }
        pixels.push_back((unsigned char)(c.x * 255));
        pixels.push_back((unsigned char)(c.y * 255));
        pixels.push_back((unsigned char)(c.z * 255));
        pixels.push_back(255);
    }

    if (texID == 0)
        glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{
    // 1. Window Setup (Boilerplate)
    if (!glfwInit())
        return 1;
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Quantum Imaging Simulation Software", NULL, NULL);
    if (!window)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync (Limit to 60 FPS)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    std::cout << "[main] : Window Initialized" << std::endl;

    // ------------------------------------------------------------
    // 2. PERSISTENT STATE (Data lives here, outside the loop)
    // ------------------------------------------------------------

    // Simulation Parameters (Variables you might want to change)
    float source_wavelength_nm = 633.0f;
    float source_waist_mm = 1.0f;

    // Output Data Containers
    std::vector<double> flatIntensity, flatPhase;
    double sensorSize = 0.02;

    GLuint texIntensity = 0;
    GLuint texPhase = 0;

    // Objects
    Camera camera(point3(0, 0, 10), vec3(0, 0, 1), "Sensor", 0.02);

    std::vector<OpticalElement *> Elements;
    Elements.push_back(&camera);

    std::cout << "[main] : Elements List prepared" << std::endl;
    double maxI;

    // Perform INITIAL Simulation (So the screen isn't empty at start)
    {
        Source source(point3(0, 0, 0), vec3(0, 0, 1), FieldType::GAUSSIAN, 0.0, 0.0, source_wavelength_nm * 1e-9, source_waist_mm / 1000.0);
        RunSimulation(Elements, source, camera, flatIntensity, flatPhase, sensorSize);

        int N = (int)sqrt(flatIntensity.size());
        maxI = *std::max_element(flatIntensity.begin(), flatIntensity.end());
        UpdateTexture(texIntensity, flatIntensity, N, N, maxI, false);
        UpdateTexture(texPhase, flatPhase, N, N, 1.0, true);

        std::cout << "[main] : Initial Simulation run" << std::endl;
    }

    // ------------------------------------------------------------
    // 3. RENDER LOOP (Runs 60 times per second)
    // ------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Standard ImGui Frame Start
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- GUI LAYOUT ---
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

        // A. CONTROLS SECTION
        ImGui::Text("Simulation Controls");
        ImGui::SliderFloat("Wavelength (nm)", &source_wavelength_nm, 400.0f, 800.0f);
        ImGui::SliderFloat("Beam Waist (mm)", &source_waist_mm, 0.1f, 5.0f);

        // B. THE "RUN" BUTTON
        if (ImGui::Button("Run Simulation", ImVec2(150, 30)))
        {
            std::cout << "[main] : Wavelength set to " << source_wavelength_nm << " nm\n";
            std::cout << "[main] : Waist radius set to " << source_waist_mm << " mm" << std::endl;

            // This block ONLY runs when you click the button.
            camera.reset();

            Source source(point3(0, 0, 0), vec3(0, 0, 1), FieldType::GAUSSIAN, 0.0, 0.0, source_wavelength_nm * 1e-9, source_waist_mm / 1000.0);
            RunSimulation(Elements, source, camera, flatIntensity, flatPhase, sensorSize);

            int N = (int)sqrt(flatIntensity.size());
            maxI = 0;
            if (!flatIntensity.empty())
                maxI = *std::max_element(flatIntensity.begin(), flatIntensity.end());

            UpdateTexture(texIntensity, flatIntensity, N, N, maxI, false);
            UpdateTexture(texPhase, flatPhase, N, N, 1.0, true);

            std::cout << "[main] : Simulation Re-run" << std::endl;

            ImPlot::SetNextAxesToFit();
        }

        ImGui::Separator();

        // C. PLOTTING SECTION
        if (texIntensity != 0)
        {
            double bounds = sensorSize / 2.0 * 1000.0; // mm

            if (ImGui::BeginTable("Plots", 2))
            {
                // ----------------------------------------------------
                // LEFT COLUMN: INTENSITY
                // ----------------------------------------------------
                ImGui::TableNextColumn();

                // 1. Push the colormap so the Scale knows what colors to draw
                ImPlot::PushColormap(ImPlotColormap_Plasma);

                // 2. Draw Plot (Width = Full - 70px for the bar)
                if (ImPlot::BeginPlot("Intensity", ImVec2(-70, -1), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes("x [mm]", "y [mm]");
                    ImPlot::PlotImage("##I", (void *)(intptr_t)texIntensity, ImPlotPoint(-bounds, -bounds), ImPlotPoint(bounds, bounds));
                    ImPlot::EndPlot();
                }

                // 3. Draw Color Bar (Side-by-side)
                ImGui::SameLine();
                // Format: Label, Min, Max, Size(Width, Height)
                ImPlot::ColormapScale("##IScale", 0, maxI, ImVec2(60, -1), "%.2e");

                ImPlot::PopColormap();

                // ----------------------------------------------------
                // RIGHT COLUMN: PHASE
                // ----------------------------------------------------
                ImGui::TableNextColumn();

                ImPlot::PushColormap(ImPlotColormap_Twilight);

                // 2. Draw Plot (Width = Full - 70px)
                if (ImPlot::BeginPlot("Phase", ImVec2(-70, -1), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes("x [mm]", "y [mm]");
                    ImPlot::PlotImage("##P", (void *)(intptr_t)texPhase,
                                      ImPlotPoint(-bounds, -bounds), ImPlotPoint(bounds, bounds));
                    ImPlot::EndPlot();
                }

                // 3. Draw Color Bar
                ImGui::SameLine();
                ImPlot::ColormapScale("##PScale", -PI, PI, ImVec2(60, -1), "%.2f rad");

                ImPlot::PopColormap();

                ImGui::EndTable();
            }
        }
        else
        {
            // Debug info if data is missing
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Waiting for Data... (Size mismatch or Empty)");
            ImGui::Text("Vector Size: %d", (int)flatIntensity.size());
            ImGui::Text("Expected: %d", 512 * 512);
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}