#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "implot.h"

#include "vec3.hpp"
#include "texture_manager.hpp"
#include "scene.hpp"
#include "simulation_engine.hpp"
#include "optical_element.hpp"
#include "utils.hpp"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#ifndef PI
#define PI 3.14159265358979323846
#endif

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;

// --- Helper Functions for UI ---

static void DrawVec3Control(const std::string &label, vec3 &values, float resetValue = 0.0f, float columnWidth = 100.0f)
{
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
    float buttonWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImVec2 buttonSize = {buttonWidth, lineHeight};

    float availWidth = ImGui::GetContentRegionAvail().x;
    float maxGroupWidth = 300.0f;
    float actualGroupWidth = min(availWidth, maxGroupWidth);
    float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
    float totalItemWidth = (actualGroupWidth - 2 * itemSpacing) / 3.0f;
    float dragFieldWidth = max(1.0f, totalItemWidth - buttonSize.x);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));

    // X
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

    // Y
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

    // Z
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
    ImGui::SetColumnWidth(0, 160.0f);
    ImGui::Text("%s", label);
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

    float speed = precise ? 0.05f : 1.0f;
    char format[32];
    if (precise)
        snprintf(format, sizeof(format), "%%.3f %s", units);
    else
        snprintf(format, sizeof(format), "%%.0f %s", units);

    bool changed = ImGui::DragFloat(std::string("##").append(label).c_str(), value, speed, 0.0f, 0.0f, format);
    ImGui::Columns(1);
    ImGui::PopID();
    return changed;
}

void SetupStyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 6.0f;
    style.ItemSpacing = ImVec2(10, 8);
    style.FramePadding = ImVec2(6, 4);

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.35f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
}

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

void UpdateCameraTextures(Camera *cam, GLuint &texIntensity, GLuint &texPhase)
{
    if (!cam)
        return;

    int N = cam->getSensedWaveFront().N;
    if (N <= 0)
        return;

    auto &gridI = cam->getSensedWaveFront().Intensity();
    auto &gridP = cam->getSensedWaveFront().Phase();

    std::vector<double> flatI;
    std::vector<double> flatP;
    flatI.reserve(N * N);
    flatP.reserve(N * N);

    double maxI = 0.0;

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            double valI = gridI[i][j];
            if (valI > maxI)
                maxI = valI;
            flatI.push_back(valI);
            flatP.push_back(gridP[i][j]);
        }
    }

    UpdateGPUTexture(texIntensity, flatI, N, N, maxI, false); // False = Intensity
    UpdateGPUTexture(texPhase, flatP, N, N, 0, true);         // True = Phase
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
    {"Mirror", "Mirror", "icons/mirror.png", "Reflects the optical field"},
    {"Convex Lens", "ConvexLens", "icons/convex lens.png", "Focuses the optical field"},
    {"Concave Lens", "ConcaveLens", "icons/concave lens.png", "Diverges the optical field"},
    {"Iris", "Iris", "icons/iris.png", "Controls the aperture of the optical system"},
    {"Slit", "Slit", "icons/slits.png", "Creates single or multi slits in the optical path"}};

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
        return -1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    TextureManager icons;
    for (const auto &el : ELEMENT_REGISTRY)
        icons.Load(el.typeID, el.iconPath);

    ImFont *mainFont = io.Fonts->AddFontFromFileTTF("icons/Helvetica.ttf", 18.0f);

    Scene scene;
    GLuint texIntensity = 0;
    GLuint texPhase = 0;

    ImVec2 canvas_offset = ImVec2(100, 300);
    float canvas_scale = 10000.0f;

    int selectedCameraIndex = 0;
    int lastSelectedCameraIndex = -1;
    bool needTextureUpdate = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(viewport->ID, viewport);

        // PANEL 1: ELEMENT BROWSER (Left)
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

                ImGui::ImageButton(item.name.c_str(), (void *)(intptr_t)iconID, ImVec2(icon_size, icon_size));

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

        // PANEL 2: SETUP ILLUSTRATION (Center)
        ImGui::Begin("Setup Illustration", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 sz = ImGui::GetContentRegionAvail();

            if (sz.x < 1.0f || sz.y < 1.0f)
            {
                ImGui::End();
                goto SkipCanvas;
            }
            ImVec2 p1 = ImVec2(p0.x + sz.x, p0.y + sz.y);

            if (ImGui::IsWindowHovered())
            {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0.0f)
                {
                    float zoom_factor = (wheel > 0) ? 1.1f : 0.909f;
                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 mousePosRel = ImVec2(mousePos.x - p0.x, mousePos.y - p0.y);
                    canvas_offset.x = (canvas_offset.x - mousePosRel.x) * zoom_factor + mousePosRel.x;
                    canvas_offset.y = (canvas_offset.y - mousePosRel.y) * zoom_factor + mousePosRel.y;
                    canvas_scale *= zoom_factor;
                }
            }
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                canvas_offset.x += ImGui::GetIO().MouseDelta.x;
                canvas_offset.y += ImGui::GetIO().MouseDelta.y;
            }

            draw_list->AddRectFilled(p0, p1, IM_COL32(30, 30, 30, 255));
            draw_list->PushClipRect(p0, p1, true);
            float GRID_STEP = 50.0f * (canvas_scale / 100.0f);
            if (GRID_STEP < 20.0f)
                GRID_STEP *= 5.0f;
            for (float x = fmodf(canvas_offset.x, GRID_STEP); x < sz.x; x += GRID_STEP)
                draw_list->AddLine(ImVec2(p0.x + x, p0.y), ImVec2(p0.x + x, p1.y), IM_COL32(50, 50, 50, 255));
            for (float y = fmodf(canvas_offset.y, GRID_STEP); y < sz.y; y += GRID_STEP)
                draw_list->AddLine(ImVec2(p0.x, p0.y + y), ImVec2(p1.x, p0.y + y), IM_COL32(50, 50, 50, 255));

            ImGui::SetNextItemAllowOverlap();
            ImGui::SetCursorScreenPos(p0);
            if (ImGui::InvisibleButton("##CanvasDropZone", sz))
                scene.ClearSelection();

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ADD_ELEMENT"))
                {
                    std::string type = (const char *)payload->Data;
                    ImVec2 mousePos = ImGui::GetMousePos();
                    double dropZ = (mousePos.x - p0.x - canvas_offset.x) / canvas_scale;
                    double dropX = -(mousePos.y - p0.y - canvas_offset.y) / canvas_scale;
                    scene.AddObject(type, vec3(dropX, 0, dropZ), vec3(0, 0, 1));
                }
                ImGui::EndDragDropTarget();
            }

            for (auto &obj : scene.GetObjects())
            {
                vec3 pos = obj->getPosition();
                vec3 orient = obj->uiOrientation;
                float screenX = p0.x + canvas_offset.x + (float)pos.z() * canvas_scale;
                float screenY = p0.y + canvas_offset.y - (float)pos.x() * canvas_scale;
                ImVec2 center(screenX, screenY);
                float angle = -atan2(orient.x(), orient.z());
                float physicalWidthMeters = 0.02f;
                float iconSize = max(5.0f, physicalWidthMeters * canvas_scale);
                float halfSize = iconSize * 0.5f;

                ImVec2 p_tl = RotatePoint(ImVec2(screenX - halfSize, screenY - halfSize), center, angle);
                ImVec2 p_tr = RotatePoint(ImVec2(screenX + halfSize, screenY - halfSize), center, angle);
                ImVec2 p_br = RotatePoint(ImVec2(screenX + halfSize, screenY + halfSize), center, angle);
                ImVec2 p_bl = RotatePoint(ImVec2(screenX - halfSize, screenY + halfSize), center, angle);

                GLuint texID = icons.GetID(obj->type);
                if (texID != 0)
                    draw_list->AddImageQuad((void *)(intptr_t)texID, p_tl, p_tr, p_br, p_bl);
                else
                    draw_list->AddCircleFilled(center, halfSize * 0.7f, IM_COL32(255, 0, 255, 255));

                if (obj->isSelected)
                    draw_list->AddQuad(p_tl, p_tr, p_br, p_bl, IM_COL32(255, 200, 0, 255), 2.0f);

                ImGui::SetCursorScreenPos(ImVec2(screenX - halfSize, screenY - halfSize));
                ImGui::PushID(obj->id);
                if (ImGui::InvisibleButton("##Pick", ImVec2(iconSize, iconSize)))
                    scene.Select(obj->id);
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    scene.Select(obj->id);
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    double dz = delta.x / canvas_scale;
                    double dx = -delta.y / canvas_scale;
                    vec3 newPos = obj->getPosition() + vec3(dx, 0, dz);
                    if (!ImGui::GetIO().KeyShift)
                        newPos = vec3(round(newPos.x() * 1000.0) / 1000.0, newPos.y(), round(newPos.z() * 1000.0) / 1000.0);
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

        // PANEL 3: ELEMENT PARAMETERS (Bottom)
        ImGui::Begin("ELEMENT PARAMETERS");
        {
            if (ImGui::Button("SIMULATE", ImVec2(200, 40)))
            {
                std::vector<OpticalElement *> cameras = scene.GetCameras();
                for (auto &cam : cameras)
                    cam->reset();
                auto objects = scene.GetObjects();

                SimulationEngine::Run(scene);
                needTextureUpdate = true; // Trigger update from C++ arrays
            }
            ImGui::SameLine();
            if (ImGui::Button("CLEAR SETUP", ImVec2(200, 40)))
                scene.Clear();

            ImGui::Separator();
            if (scene.selectedObject)
            {
                auto &obj = scene.selectedObject;
                ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "Selected: %s", obj->name.c_str());
                ImGui::Spacing();
                vec3 currentPos = obj->getPosition();
                DrawVec3Control("Position", currentPos);
                if (currentPos.x() != obj->getPosition().x() || currentPos.y() != obj->getPosition().y() || currentPos.z() != obj->getPosition().z())
                {
                    if (obj->source)
                        obj->source->setPosition(currentPos);
                    if (obj->element)
                        obj->element->setPosition(currentPos);
                }

                vec3 currentOrient = obj->uiOrientation;
                if (obj->source)
                    currentOrient = obj->source->getOrientation();
                if (obj->element)
                    currentOrient = obj->element->getOrientation();
                float angleDeg = atan2(currentOrient.x(), currentOrient.z()) * 180.0f / (float)PI;
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

                ImGui::Separator();
                ImGui::Spacing();
                if (obj->type == "Source" && obj->source)
                {
                    auto src = obj->source;
                    const char *types[] = {"Plane Waves", "Gaussian", "Laguerre-Gaussian (LG)", "Hermite-Gaussian (HG)"};
                    int CurrentType = (int)src->getFieldType();
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 160.0f);
                    ImGui::Text("Field Type");
                    ImGui::NextColumn();
                    if (ImGui::Combo("##Type", &CurrentType, types, IM_ARRAYSIZE(types)))
                        src->setFieldType((FieldType)CurrentType);
                    ImGui::Columns(1);
                    if (src->getFieldType() == FieldType::LG || src->getFieldType() == FieldType::HG)
                    {
                        int l = src->getL();
                        int p = src->getP();
                        ImGui::InputInt("Azimuthal (l/m)", &l);
                        ImGui::InputInt("Radial (p/n)", &p);
                        if (p < 0)
                            p = 0;
                        src->setBeamMode(l, p);
                    }
                    float wave_nm = (float)(src->getWavelength() * 1e9);
                    if (DrawFloatControl("Wavelength", &wave_nm, true, "nm"))
                        src->setWavelength(wave_nm * 1e-9);
                    if (src->getFieldType() != FieldType::PLANE)
                    {
                        float waist_mm = (float)(src->getBeamWaist() * 1000.0);
                        if (DrawFloatControl("Waist", &waist_mm, true, "mm"))
                            src->setBeamWaist(waist_mm / 1000.0);
                    }
                }
                else if (obj->type == "Mirror" && obj->element)
                {
                    Mirror *mirror = dynamic_cast<Mirror *>(obj->element.get());

                    if (mirror)
                    {
                        float size_mm = (float)(mirror->getSize() * 1000.0);
                        if (DrawFloatControl("Size", &size_mm, true, "mm"))
                            mirror->setSize(size_mm / 1000.0);

                        float reflectivity = (float)mirror->getReflectivity();
                        if (DrawFloatControl("Reflectivity", &reflectivity, true))
                        {
                            if (reflectivity < 0.0f)
                                reflectivity = 0.0f;
                            if (reflectivity > 1.0f)
                                reflectivity = 1.0f;
                            mirror->setReflectivity(reflectivity);
                        }

                        ImGui::Dummy(ImVec2(0, 5));
                        ImGui::Separator();
                        ImGui::Text("Complex Refractive Index");

                        float n = (float)mirror->getRefractiveIndex().real();
                        float k = (float)mirror->getRefractiveIndex().imag();

                        bool ri_changed = false;

                        if (DrawFloatControl("Real (n)", &n, true))
                            ri_changed = true;

                        if (DrawFloatControl("Imag (k)", &k, true))
                            ri_changed = true;

                        if (ri_changed)
                        {
                            k = max(0.0, k);
                            auto new_ri = std::complex<double>(n, k);
                            mirror->setRefractiveIndex(new_ri);
                        }
                    }
                }
                else if (obj->type == "ConvexLens" || obj->type == "ConcaveLens")
                {
                    ConvexLens *cvx = dynamic_cast<ConvexLens *>(obj->element.get());
                    ConcaveLens *ccv = dynamic_cast<ConcaveLens *>(obj->element.get());

                    if (cvx || ccv)
                    {
                        double current_f = cvx ? cvx->getFocalLength() : ccv->getFocalLength();
                        double current_r = cvx ? cvx->getRadius() : ccv->getRadius();
                        double current_n = cvx ? cvx->getRefractiveIndex() : ccv->getRefractiveIndex();

                        float f_mm = (float)(current_f * 1000.0);
                        float diameter_mm = (float)(current_r * 2.0 * 1000.0);
                        float n_val = (float)current_n;

                        if (DrawFloatControl("Focal Length", &f_mm, true, "mm"))
                        {
                            double new_f = f_mm / 1000.0;
                            if (cvx)
                                cvx->setFocalLength(new_f);
                            else
                                ccv->setFocalLength(new_f);
                        }

                        if (DrawFloatControl("Diameter", &diameter_mm, true, "mm"))
                        {
                            double new_r = (diameter_mm / 1000.0) / 2.0;
                            if (cvx)
                                cvx->setRadius(new_r);
                            else
                                ccv->setRadius(new_r);
                        }

                        if (DrawFloatControl("Refractive Index", &n_val, true))
                        {
                            if (n_val < 1.0f)
                                n_val = 1.0f;

                            if (cvx)
                                cvx->setRefractiveIndex((double)n_val);
                            else
                                ccv->setRefractiveIndex((double)n_val);
                        }
                    }
                }
                else if (obj->type == "Iris")
                {
                    if (Iris *iris = dynamic_cast<Iris *>(obj->element.get()))
                    {
                        float size_mm = (float)(iris->getSize() * 1000.0);
                        if (DrawFloatControl("Mount Size", &size_mm, true, "mm"))
                            iris->setSize(size_mm / 1000.0);

                        float radius_mm = (float)(iris->getRadius() * 1000.0);
                        if (DrawFloatControl("Hole Radius", &radius_mm, true, "mm"))
                            iris->setRadius(radius_mm / 1000.0);

                        ImGui::TextDisabled("Hole Diameter: %.3f mm", radius_mm * 2.0f);
                    }
                }
                else if (obj->type == "Slit")
                {
                    if (Slit *slit = dynamic_cast<Slit *>(obj->element.get()))
                    {
                        float size_mm = (float)(slit->getSize() * 1000.0);
                        if (DrawFloatControl("Mount Size", &size_mm, true, "mm"))
                            slit->setSize(size_mm / 1000.0);

                        ImGui::Separator();
                        ImGui::Text("Slit Dimensions");

                        float width_um = (float)(slit->getWidth() * 1000000.0);
                        if (DrawFloatControl("Width", &width_um, true, "um"))
                            slit->setWidth(width_um / 1000000.0);

                        float height_mm = (float)(slit->getHeight() * 1000.0);
                        if (DrawFloatControl("Height", &height_mm, true, "mm"))
                            slit->setHeight(height_mm / 1000.0);

                        ImGui::Separator();
                        ImGui::Text("Slit Parameters");

                        int num = slit->getNumSlits();
                        if (ImGui::InputInt("Count", &num))
                        {
                            if (num < 1)
                                num = 1; // Enforce minimum 1 slit
                            slit->setNumSlits(num);
                        }

                        if (num > 1)
                        {
                            float sep_mm = (float)(slit->getSeparation() * 1000.0);
                            if (DrawFloatControl("Separation", &sep_mm, true, "mm"))
                                slit->setSeparation(sep_mm / 1000.0);
                        }
                    }
                }
            }
        }
        ImGui::End();

        // PANEL 4: SIMULATION OUTPUT (Right)
        ImGui::Begin("Simulation Output");

        std::vector<OpticalElement *> sensors = scene.GetCameras();
        std::vector<Camera *> cameras;
        for (auto &sensor : sensors)
        {
            Camera *cam = dynamic_cast<Camera *>(sensor);
            if (cam != nullptr)
                cameras.push_back(cam);
        }

        if (cameras.empty())
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "No Cameras in Scene.");
        else
        {
            if (selectedCameraIndex >= cameras.size())
                selectedCameraIndex = 0;
            if (ImGui::BeginCombo("Select Sensor", cameras[selectedCameraIndex]->getName().c_str()))
            {
                for (int i = 0; i < cameras.size(); i++)
                {
                    bool is_selected = (selectedCameraIndex == i);
                    if (ImGui::Selectable(cameras[i]->getName().c_str(), is_selected))
                    {
                        selectedCameraIndex = i;
                        needTextureUpdate = true; // Refresh view on switch
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            Camera *activeCam = cameras[selectedCameraIndex];

            if ((needTextureUpdate || selectedCameraIndex != lastSelectedCameraIndex) && activeCam->getSensedWaveFront().N > 0)
            {
                UpdateCameraTextures(activeCam, texIntensity, texPhase);
                needTextureUpdate = false;
                lastSelectedCameraIndex = selectedCameraIndex;
            }

            if (texIntensity != 0 && activeCam->getSensedWaveFront().N > 0)
            {
                float size = (float)(activeCam->getSize() * 1000.0); // Convert to mm
                float half = size / 2.0f;
                ImPlotPoint min_b = {-half, -half};
                ImPlotPoint max_b = {half, half};

                ImPlot::PushColormap(ImPlotColormap_Plasma);

                if (ImPlot::BeginPlot("Intensity (a.u.)", ImVec2(-1, 0), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes("x (mm)", "y (mm)");
                    ImPlot::PlotImage("##Intensity", (void *)(intptr_t)texIntensity, min_b, max_b);
                    ImPlot::EndPlot();
                }

                ImPlot::ColormapScale("##IntensityScale", 0, 1, ImVec2(0, 20));
                ImPlot::PopColormap();
                ImGui::Separator();
                ImGui::Spacing();

                ImPlot::PushColormap(ImPlotColormap_Twilight);

                if (ImPlot::BeginPlot("Phase (rad)", ImVec2(-1, 0), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes("x (mm)", "y (mm)");
                    ImPlot::PlotImage("##Phase", (void *)(intptr_t)texPhase, min_b, max_b);
                    ImPlot::EndPlot();
                }

                ImPlot::ColormapScale("##PhaseScale", -PI, PI, ImVec2(0, 20));
                ImPlot::PopColormap();
            }
            else
                ImGui::TextDisabled("No simulation data available. Click 'SIMULATE'.");
        }
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

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