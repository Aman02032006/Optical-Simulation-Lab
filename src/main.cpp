#include <vector>
#include <iostream>
#include <string>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "implot.h"
#include "stb_image.h"

#include "vec3.hpp"
#include "texture_manager.hpp"
#include "scene.hpp"
#include "simulation_engine.hpp"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

static void DrawVec3Control(const std::string &label, vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f)
{
    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);

    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    // 1. Calculate Standard Button Size ("X", "Y", "Z")
    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    float buttonWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImVec2 buttonSize = {buttonWidth, lineHeight};

    // 2. Calculate Widths
    // Cap the total width of the X+Y+Z group to something reasonable (e.g. 350px)
    // so it doesn't stretch across the whole screen on wide monitors.
    float availWidth = ImGui::GetContentRegionAvail().x;
    float maxGroupWidth = 300.0f;
    float actualGroupWidth = min(availWidth, maxGroupWidth);

    float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
    float totalItemWidth = (actualGroupWidth - 2 * itemSpacing) / 3.0f;

    // The drag field takes whatever space is left in the item slot after the button
    float dragFieldWidth = max(1.0f, totalItemWidth - buttonSize.x);

    // Tight spacing between colored button and number field
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

    // --- X AXIS (Red) ---
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    if (ImGui::Button("X", buttonSize))
        values.e[0] = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(dragFieldWidth);
    ImGui::DragScalar("##X", ImGuiDataType_Double, &values.e[0], 0.001f, 0, 0, "%.3f");

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(itemSpacing, 0));
    ImGui::SameLine();

    // --- Y AXIS (Green) ---
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Y", buttonSize))
        values.e[1] = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(dragFieldWidth);
    ImGui::DragScalar("##Y", ImGuiDataType_Double, &values.e[1], 0.001f, 0, 0, "%.3f");

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(itemSpacing, 0));
    ImGui::SameLine();

    // --- Z AXIS (Blue) ---
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    if (ImGui::Button("Z", buttonSize))
        values.e[2] = resetValue;
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(dragFieldWidth);
    ImGui::DragScalar("##Z", ImGuiDataType_Double, &values.e[2], 0.001f, 0, 0, "%.3f");

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

bool DrawFloatControl(const char *label, float *value, bool precise = true, const char *units = "")
{
    ImGui::PushID(label);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 160.0f); // Maintain your consistent label width
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    // 1. Determine Drag Speed
    // Integer mode needs fast dragging (1.0), Precise mode needs slow dragging (0.05)
    float speed = precise ? 0.05f : 1.0f;

    // 2. Construct the Format String (e.g., "%.3f nm" or "%.0f deg")
    char format[32];
    if (precise)
        snprintf(format, sizeof(format), "%%.3f %s", units); // Result: "%.3f nm"
    else
        snprintf(format, sizeof(format), "%%.0f %s", units); // Result: "%.0f nm"

    // 3. Create a unique ID based on the label so ImGui doesn't get confused
    std::string id = std::string("##") + label;

    bool changed = ImGui::DragFloat(id.c_str(), value, speed, 0.0f, 0.0f, format);

    ImGui::Columns(1); // Reset columns
    ImGui::PopID();
    return changed;
}

void SetupStyle()
{
    ImGuiStyle &style = ImGui::GetStyle();

    // Rounding - Makes UI look softer
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 6.0f;

    // Spacing
    style.ItemSpacing = ImVec2(10, 8);
    style.FramePadding = ImVec2(6, 4);

    // Modern Dark Color Palette
    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // Darker Background
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);   // Subtle Headers
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f); // Deep Blue Buttons
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.35f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.20f, 0.25f, 1.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // Input fields
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
}

// --- Helper: Rotate a point around a center ---
ImVec2 RotatePoint(ImVec2 point, ImVec2 center, float angle_rad)
{
    float s = sin(angle_rad);
    float c = cos(angle_rad);
    point.x -= center.x;
    point.y -= center.y;
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;
    return ImVec2(xnew + center.x, ynew + center.y);
}

// --- Helper: GPU Texture Uploader ---
void UpdateGPUTexture(GLuint &texID, const std::vector<double> &data, int w, int h, double maxVal, bool isPhase)
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
            float norm = (float)((val + PI) / (2.0 * PI));
            c = ImPlot::SampleColormap(norm, ImPlotColormap_Twilight);
        }
        else
        {
            float norm = (maxVal > 0) ? (float)(val / maxVal) : 0.0f;
            norm = powf(norm, 0.5f);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

struct ElementType
{
    std::string name;
    std::string typeID;
    std::string iconPath;
    std::string description;
};

const std::vector<ElementType> ELEMENT_REGISTRY = {
    {"Source", "Source", "icons/source.png", "Generates Plane, Gaussian, LG and HG beams"},
    {"Camera", "Camera", "icons/camera.png", "Detects Intensity and phase information"},
    {"Mirror", "Mirror", "icons/mirror.png", "Reflects the optical field"}
    // Add "lens" and "mirror" back here when you have the classes ready
};

int main()
{
    if (!glfwInit())
        return 1;
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "QISS - Optical Workbench", NULL, NULL);
    if (!window)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupStyle();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Icons
    TextureManager icons;
    for (const auto &el : ELEMENT_REGISTRY)
    {
        icons.Load(el.typeID, el.iconPath);
    }

    ImFont *mainFont = io.Fonts->AddFontFromFileTTF("icons/Helvetica.ttf", 18.0f);

    Scene scene;

    std::vector<double> outIntensity, outPhase;
    double outSize = 0.02;
    double outMaxI = 1.0;

    GLuint texIntensity = 0;
    GLuint texPhase = 0;

    ImVec2 canvas_offset = ImVec2(100, 300);
    float canvas_scale = 10000.0f;

    // State Tracking
    int selectedCameraIndex = 0;
    int lastSelectedCameraIndex = -1; // Force update on first frame
    bool needTextureUpdate = false;   // Flag to trigger update

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(viewport->ID, viewport);

        // ------------------------------------------
        // PANEL 1: ELEMENT BROWSER (Left)
        // ------------------------------------------
        ImGui::Begin("Element Browser");
        {
            float window_width = ImGui::GetContentRegionAvail().x;
            float icon_size = 100.0f;

            for (const auto &item : ELEMENT_REGISTRY)
            {
                GLuint iconID = icons.GetID(item.typeID);
                ImGui::PushID(item.name.c_str());

                float cursorX = (window_width - icon_size) * 0.5f;
                ImGui::SetCursorPosX(cursorX);

                if (ImGui::ImageButton(item.name.c_str(), (void *)(intptr_t)iconID, ImVec2(icon_size, icon_size)))
                {
                    // Click logic
                }

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("ADD_ELEMENT", item.typeID.c_str(), item.typeID.size() + 1);
                    ImGui::Text("Adding %s", item.name.c_str());
                    ImGui::Image((void *)(intptr_t)iconID, ImVec2(32, 32));
                    ImGui::EndDragDropSource();
                }

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", item.description.c_str());

                ImGui::SetCursorPosX((window_width - ImGui::CalcTextSize(item.name.c_str()).x) * 0.5f);
                ImGui::Text("%s", item.name.c_str());
                ImGui::Dummy(ImVec2(0, 15));

                ImGui::PopID();
            }
        }
        ImGui::End();

        // ------------------------------------------
        // PANEL 2: SETUP ILLUSTRATION (Center)
        // ------------------------------------------
        ImGui::Begin("Setup Illustration", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 sz = ImGui::GetContentRegionAvail();

            if (sz.x < 1.0f || sz.y < 1.0f)
            {
                ImGui::End();    // Close the window block safely
                goto SkipCanvas; // Skip drawing logic for this frame
            }

            ImVec2 p1 = ImVec2(p0.x + sz.x, p0.y + sz.y);

            // Zoom Logic
            if (ImGui::IsWindowHovered())
            {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f)
                {
                    float zoom_factor = 1.1f;
                    if (wheel < 0)
                        zoom_factor = 1.0f / zoom_factor;
                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 mousePosRel = ImVec2(mousePos.x - p0.x, mousePos.y - p0.y);
                    canvas_offset.x = (canvas_offset.x - mousePosRel.x) * zoom_factor + mousePosRel.x;
                    canvas_offset.y = (canvas_offset.y - mousePosRel.y) * zoom_factor + mousePosRel.y;
                    canvas_scale *= zoom_factor;
                }
            }
            // Pan Logic
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                canvas_offset.x += ImGui::GetIO().MouseDelta.x;
                canvas_offset.y += ImGui::GetIO().MouseDelta.y;
            }

            // Draw Background Grid
            draw_list->AddRectFilled(p0, p1, IM_COL32(30, 30, 30, 255));
            draw_list->PushClipRect(p0, p1, true);

            float GRID_STEP = 50.0f * (canvas_scale / 100.0f);
            if (GRID_STEP < 20.0f)
                GRID_STEP *= 5.0f;

            for (float x = fmodf(canvas_offset.x, GRID_STEP); x < sz.x; x += GRID_STEP)
                draw_list->AddLine(ImVec2(p0.x + x, p0.y), ImVec2(p0.x + x, p1.y), IM_COL32(50, 50, 50, 255));
            for (float y = fmodf(canvas_offset.y, GRID_STEP); y < sz.y; y += GRID_STEP)
                draw_list->AddLine(ImVec2(p0.x, p0.y + y), ImVec2(p1.x, p0.y + y), IM_COL32(50, 50, 50, 255));

            float originX = p0.x + canvas_offset.x;
            float originY = p0.y + canvas_offset.y;
            draw_list->AddLine(ImVec2(originX, p0.y), ImVec2(originX, p1.y), IM_COL32(80, 80, 80, 255));
            draw_list->AddLine(ImVec2(p0.x, originY), ImVec2(p1.x, originY), IM_COL32(80, 80, 80, 255));

            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorScreenPos(p0);
            bool bgClicked = ImGui::InvisibleButton("##CanvasDropZone", sz);

            if (bgClicked)
            {
                scene.ClearSelection();
                std::cout << "Background Clicked -> Clear" << std::endl;
            }

            // --- DROP HANDLING (Updated for your new AddObject) ---
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ADD_ELEMENT"))
                {
                    std::string type = (const char *)payload->Data;
                    ImVec2 mousePos = ImGui::GetMousePos();
                    double dropZ = (mousePos.x - p0.x - canvas_offset.x) / canvas_scale;
                    double dropX = -(mousePos.y - p0.y - canvas_offset.y) / canvas_scale;

                    dropZ = round(dropZ * 100.0) / 100.0;
                    dropX = round(dropX * 100.0) / 100.0;

                    // PASS DEFAULT ORIENTATION (0, 0, 1) pointing forward
                    scene.AddObject(type, vec3(dropX, 0, dropZ), vec3(0, 0, 1));
                }
                ImGui::EndDragDropTarget();
            }

            // Draw Scene Objects
            for (auto &obj : scene.GetObjects())
            {
                vec3 pos = obj->getPosition();
                vec3 orient = obj->uiOrientation;

                // World -> Screen conversion
                float screenX = p0.x + canvas_offset.x + (float)pos.z() * canvas_scale;
                float screenY = p0.y + canvas_offset.y - (float)pos.x() * canvas_scale;
                ImVec2 center(screenX, screenY);

                float angle = -atan2(orient.x(), orient.z());

                // --- NEW SCALING LOGIC ---
                // We define the object as being 0.4 meters (40cm) wide in the real world.
                // As canvas_scale (pixels per meter) changes, this pixel size changes.
                float physicalWidthMeters = 0.02f;
                float iconSize = physicalWidthMeters * canvas_scale;

                // Optional: Don't let it get smaller than 5 pixels so you can always find it
                if (iconSize < 5.0f)
                    iconSize = 5.0f;

                float halfSize = iconSize * 0.5f;
                // -------------------------

                // Calculate Rotated Corners
                ImVec2 p_tl = RotatePoint(ImVec2(screenX - halfSize, screenY - halfSize), center, angle);
                ImVec2 p_tr = RotatePoint(ImVec2(screenX + halfSize, screenY - halfSize), center, angle);
                ImVec2 p_br = RotatePoint(ImVec2(screenX + halfSize, screenY + halfSize), center, angle);
                ImVec2 p_bl = RotatePoint(ImVec2(screenX - halfSize, screenY + halfSize), center, angle);

                GLuint texID = icons.GetID(obj->type);
                if (texID != 0)
                {
                    draw_list->AddImageQuad((void *)(intptr_t)texID, p_tl, p_tr, p_br, p_bl);
                }
                else
                {
                    draw_list->AddCircleFilled(center, halfSize * 0.7f, IM_COL32(255, 0, 255, 255));
                }

                if (obj->isSelected)
                {
                    draw_list->AddQuad(p_tl, p_tr, p_br, p_bl, IM_COL32(255, 200, 0, 255), 2.0f);
                }

                // Interaction
                ImGui::SetCursorScreenPos(ImVec2(screenX - halfSize, screenY - halfSize));
                ImGui::PushID(obj->id);

                // Hitbox must match the new visual size
                if (ImGui::InvisibleButton("##Pick", ImVec2(iconSize, iconSize)))
                {
                    scene.Select(obj->id);
                }

                // ... (Dragging logic remains exactly the same) ...
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    scene.Select(obj->id);
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    double dz = delta.x / canvas_scale;
                    double dx = -delta.y / canvas_scale;

                    vec3 newPos = obj->getPosition() + vec3(dx, 0, dz);
                    if (!ImGui::GetIO().KeyShift)
                    {
                        newPos = vec3(round(newPos.x() * 1000.0) / 1000.0, newPos.y(), round(newPos.z() * 1000.0) / 1000.0);
                    }
                    if (obj->source)
                        obj->source->setPosition(newPos);
                    if (obj->element)
                        obj->element->setPosition(newPos);
                }
                ImGui::PopID();
            }
            draw_list->PopClipRect();
        }
        ImGui::End();

    SkipCanvas:;

        // ------------------------------------------
        // PANEL 3: ELEMENT PARAMETERS (Bottom)
        // ------------------------------------------
        ImGui::Begin("ELEMENT PARAMETERS");
        {
            if (ImGui::Button("SIMULATE", ImVec2(200, 40)))
            {
                std::vector<OpticalElement *> cameras = scene.GetCameras();

                for (auto &cam : cameras)
                    cam->reset();

                auto objects = scene.GetObjects();
                for (auto &obj : objects)
                {
                    if (obj->element)
                    {
                        std::cout << "[Simulation] : ";
                        obj->element->printDetails();
                    }
                }

                SimulationEngine::Run(scene);

                needTextureUpdate = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("CLEAR SETUP", ImVec2(200, 40)))
                scene.Clear();

            ImGui::Separator();

            if (scene.selectedObject)
            {
                auto &obj = scene.selectedObject;
                ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Selected: %s (%s)", obj->name.c_str(), obj->type.c_str());
                ImGui::Spacing();

                // 1. POSITION (Unity Style)
                vec3 currentPos = obj->getPosition();

                // Use the new helper!
                DrawVec3Control("Position", currentPos);

                // Check for changes and update the object
                if (currentPos.x() != obj->getPosition().x() ||
                    currentPos.y() != obj->getPosition().y() ||
                    currentPos.z() != obj->getPosition().z())
                {
                    if (obj->source)
                        obj->source->setPosition(currentPos);
                    if (obj->element)
                        obj->element->setPosition(currentPos);
                }

                // 2. ORIENTATION EDITOR (Angle)
                vec3 currentOrient = obj->uiOrientation;
                if (obj->source)
                    currentOrient = obj->source->getOrientation();
                if (obj->element)
                    currentOrient = obj->element->getOrientation();

                float angleDeg = atan2(currentOrient.x(), currentOrient.z()) * 180.0f / (float)PI;

                // --- NEW: Use Helper for Alignment ---
                // This will align "Rotation" perfectly with "Position" above it
                if (DrawFloatControl("Rotation", &angleDeg, true, "deg"))
                {
                    float angleRad = angleDeg * (float)PI / 180.0f;
                    vec3 newOrient(sin(angleRad), 0, cos(angleRad));

                    obj->uiOrientation = newOrient;
                    if (obj->source)
                        obj->source->setOrientation(newOrient);
                    if (obj->element)
                        obj->element->setOrientation(newOrient);
                }

                // ------------------------------------------
                // 3. TYPE-SPECIFIC PARAMETERS
                // ------------------------------------------

                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s Specific Settings", obj->type.c_str());
                ImGui::Spacing();

                // For Source
                if (obj->type == "Source" && obj->source)
                {
                    auto src = obj->source;

                    const char *types[] = {"Plane Waves", "Gaussian", "Laguerre-Gaussian (LG)", "Hermite-Gaussian (HG)"};
                    int CurrentType = (int)src->getFieldType();

                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 160.0f); // Match label width of helpers
                    ImGui::Text("Field Type");
                    ImGui::NextColumn();
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

                    if (ImGui::Combo("##Type", &CurrentType, types, IM_ARRAYSIZE(types)))
                    {
                        src->setFieldType((FieldType)CurrentType);
                    }
                    ImGui::Columns(1); // Reset

                    if (src->getFieldType() == FieldType::LG || src->getFieldType() == FieldType::HG)
                    {
                        ImGui::Dummy(ImVec2(0, 5));
                        ImGui::Text("Mode Indices");

                        int l = src->getL();
                        int p = src->getP();
                        bool modeChanged = false;

                        // Custom Integer UI to match style
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, 160.0f);
                        ImGui::Text(src->getFieldType() == FieldType::LG ? "Azimuthal (l)" : "Horizontal (m)");
                        ImGui::NextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::InputInt("##l", &l))
                            modeChanged = true;
                        ImGui::NextColumn();

                        ImGui::Text(src->getFieldType() == FieldType::LG ? "Radial (p)" : "Vertical (n)");
                        ImGui::NextColumn();
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        if (ImGui::InputInt("##p", &p))
                            modeChanged = true;
                        ImGui::Columns(1);

                        if (modeChanged)
                        {
                            // Enforce non-negative for p (usually required for physics)
                            if (p < 0)
                                p = 0;
                            src->setBeamMode(l, p);
                        }
                    }

                    float wave_nm = (float)(src->getWavelength() * 1e9);
                    if (DrawFloatControl("Wavelength (nm)", &wave_nm, true, "nm"))
                        src->setWavelength(wave_nm * 1e-9);

                    if (src->getFieldType() != FieldType::PLANE)
                    {
                        float waist_mm = (float)(src->getBeamWaist() * 1000.0);
                        if (DrawFloatControl("Waist (mm)", &waist_mm, true, "mm"))
                            src->setBeamWaist(waist_mm / 1000.0);
                    }

                    ImGui::Dummy(ImVec2(0, 5));
                    ImGui::Separator();
                    ImGui::Text("Polarization");

                    // --- Polarization Controls (Psi & Delta) ---
                    // Converting Radians to Degrees for UI display
                    float psi_deg = (float)(src->getPsi() * 180.0 / 3.14159265359);
                    float delta_deg = (float)(src->getDelta() * 180.0 / 3.14159265359);

                    bool polChanged = false;

                    // Precise: YES, Unit: "deg"
                    if (DrawFloatControl("Psi (Angle)", &psi_deg, true, "deg"))
                        polChanged = true;

                    // Precise: YES, Unit: "deg"
                    if (DrawFloatControl("Delta (Phase)", &delta_deg, true, "deg"))
                        polChanged = true;

                    if (polChanged)
                    {
                        // Convert back to radians for the backend
                        src->setPsi(psi_deg * 3.14159265359 / 180.0);
                        src->setDelta(delta_deg * 3.14159265359 / 180.0);
                    }
                }
            }
            else
            {
                ImGui::TextDisabled("Select an element to edit properties.");
            }
        }
        ImGui::End();

        // ------------------------------------------
        // PANEL 4: SIMULATION OUTPUT (Right)
        // ------------------------------------------
        ImGui::Begin("Simulation Output");

        // 1. Filtering Cameras
        std::vector<OpticalElement *> sensors = scene.GetCameras();
        std::vector<Camera *> cameras;
        for (auto &sensor : sensors)
        {
            Camera *cam = dynamic_cast<Camera *>(sensor);
            if (cam != nullptr)
                cameras.push_back(cam);
        }

        // 2. Selection Dropdown
        static int selectedCameraIndex = 0;

        if (cameras.empty())
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "No Cameras in Scene.");
        }
        else
        {
            // Safety check
            if (selectedCameraIndex >= cameras.size())
                selectedCameraIndex = 0;

            // 2. Selection Dropdown
            // If user changes selection, we trigger an update
            if (ImGui::BeginCombo("Select Sensor", cameras[selectedCameraIndex]->getName().c_str()))
            {
                for (int i = 0; i < cameras.size(); i++)
                {
                    bool is_selected = (selectedCameraIndex == i);
                    if (ImGui::Selectable(cameras[i]->getName().c_str(), is_selected))
                    {
                        selectedCameraIndex = i;
                        needTextureUpdate = true; // <--- TRIGGER UPDATE
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            // --------------------------------------------------------
            // 3. HEAVY LIFTING (Only runs once when needed!)
            // --------------------------------------------------------
            if (needTextureUpdate || selectedCameraIndex != lastSelectedCameraIndex)
            {
                Camera *activeCam = cameras[selectedCameraIndex];
                WaveFront &result = activeCam->getSensedWaveFront();

                if (result.N > 0)
                {
                    int N = result.N;
                    outSize = result.getSize();
                    auto &gridI = result.Intensity();
                    auto &gridP = result.Phase();

                    // Heavy loops (CPU)
                    outIntensity.clear();
                    outIntensity.reserve(N * N);
                    outPhase.clear();
                    outPhase.reserve(N * N);
                    for (int i = 0; i < N; ++i)
                    {
                        outIntensity.insert(outIntensity.end(), gridI[i].begin(), gridI[i].end());
                        outPhase.insert(outPhase.end(), gridP[i].begin(), gridP[i].end());
                    }

                    // Max calc
                    outMaxI = 0;
                    if (!outIntensity.empty())
                        outMaxI = *std::max_element(outIntensity.begin(), outIntensity.end());

                    // Upload to GPU (Slow-ish)
                    UpdateGPUTexture(texIntensity, outIntensity, N, N, outMaxI, false);
                    UpdateGPUTexture(texPhase, outPhase, N, N, 1.0, true);
                }

                // Reset flags
                needTextureUpdate = false;
                lastSelectedCameraIndex = selectedCameraIndex;
            }

            // --------------------------------------------------------
            // 4. DRAWING (Runs every frame, very fast)
            // --------------------------------------------------------
            // We just use the existing texture ID. Zero CPU work.
            if (texIntensity != 0)
            {
                double bounds = outSize / 2.0 * 1000.0;

                ImGui::Text("Intensity");
                ImPlot::PushColormap(ImPlotColormap_Plasma);
                if (ImPlot::BeginPlot("##I", ImVec2(-1, 300), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes("x [mm]", "y [mm]");
                    ImPlot::PlotImage("##I_Tex", (void *)(intptr_t)texIntensity,
                                      ImPlotPoint(-bounds, -bounds), ImPlotPoint(bounds, bounds));
                    ImPlot::EndPlot();
                }
                ImPlot::PopColormap();

                ImGui::Separator();

                ImGui::Text("Phase");
                ImPlot::PushColormap(ImPlotColormap_Twilight);
                if (ImPlot::BeginPlot("##P", ImVec2(-1, 300), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes("x [mm]", "y [mm]");
                    ImPlot::PlotImage("##P_Tex", (void *)(intptr_t)texPhase,
                                      ImPlotPoint(-bounds, -bounds), ImPlotPoint(bounds, bounds));
                    ImPlot::EndPlot();
                }
                ImPlot::PopColormap();
            }
        }
        ImGui::End();

        // ------------------------------------------
        // PANEL 5: INFO (Bottom Right)
        // ------------------------------------------
        ImGui::Begin("Info");
        if (scene.selectedObject)
        {
            if (scene.selectedObject->type == "Source")
                ImGui::TextWrapped("Generates a Gaussian beam. The fundamental mode of a laser.");
            else if (scene.selectedObject->type == "Camera")
                ImGui::TextWrapped("Detects light intensity and phase at a specific plane.");
        }
        ImGui::End();

        // Render
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
    glDeleteTextures(1, &texIntensity);
    glDeleteTextures(1, &texPhase);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}