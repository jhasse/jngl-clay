#define CLAY_IMPLEMENTATION
#include "clay.h"

#include <clay_renderer_jngl.hpp>
#include <jngl/init.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

static void handleClayErrors(Clay_ErrorData errorData) {
    jngl::debug(std::string(errorData.errorText.chars, errorData.errorText.length));
}

struct SortableBox {
    int id;
    Clay_Color color;
    std::string stringId;
};

static Clay_TransitionData enterExitSlideUp(Clay_TransitionData initialState,
                                            Clay_TransitionProperty properties) {
    Clay_TransitionData targetState = initialState;
    if (properties & CLAY_TRANSITION_PROPERTY_Y) {
        targetState.boundingBox.y += 20;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR) {
        targetState.overlayColor = { 255, 255, 255, 255 };
    }
    return targetState;
}

class TransitionsExample : public jngl::Scene {
public:
    TransitionsExample() {
        uint64_t totalMemorySize = Clay_MinMemorySize();
        clayMemory.resize(totalMemorySize);
        Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(
            totalMemorySize, clayMemory.data());
        Clay_Initialize(arena,
                        { static_cast<float>(jngl::getScreenWidth()),
                          static_cast<float>(jngl::getScreenHeight()) },
                        { handleClayErrors, 0 });
        Clay_SetMeasureTextFunction(measureText, nullptr);

        charData.resize(100 * 3, '\0');
        for (int i = 0; i < cellCount; i++) {
            std::snprintf(&charData[i * 3], 3, "%02d", i);
            colors.push_back({
                i,
                { static_cast<float>(255 - i), static_cast<float>(255 - i * 4),
                  static_cast<float>(255 - i * 2), 255 },
                std::string(&charData[i * 3], 2),
            });
        }
        maxCount = cellCount;
    }

    void step() override {
        if (jngl::keyPressed('d')) {
            Clay_SetDebugModeEnabled(true);
        }

        const auto w = static_cast<float>(jngl::getScreenWidth());
        const auto h = static_cast<float>(jngl::getScreenHeight());
        const auto mouse = jngl::getMousePos();
        float mouseX = static_cast<float>(mouse.x + jngl::getScreenWidth() / 2.0);
        float mouseY = static_cast<float>(mouse.y + jngl::getScreenHeight() / 2.0);

        instance = this;
        Clay_SetPointerState({ mouseX, mouseY }, jngl::mouseDown());
        Clay_SetLayoutDimensions({ w, h });
        Clay_UpdateScrollContainers(true, { 0, static_cast<float>(jngl::getMouseWheel()) },
                                    1.0f / jngl::getStepsPerSecond());

        Clay_BeginLayout();
        buildLayout();
        renderCommands = Clay_EndLayout(1.0f / jngl::getStepsPerSecond());
    }

    void draw() const override {
        drawClayCommands(renderCommands);
    }

private:
    static void handleShuffle(Clay_ElementId, Clay_PointerData pointerData, void* userData) {
        if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            auto* self = static_cast<TransitionsExample*>(userData);
            auto rng = std::default_random_engine{
                static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())
            };
            std::shuffle(self->colors.begin(),
                         self->colors.begin() + self->cellCount, rng);
        }
    }

    static void handleBlue(Clay_ElementId, Clay_PointerData pointerData, void* userData) {
        if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            auto* self = static_cast<TransitionsExample*>(userData);
            for (int i = 0; i < self->cellCount; i++) {
                int idx = self->colors[i].id;
                self->colors[i].color = { static_cast<float>(255 - idx * 4),
                                          static_cast<float>(255 - idx * 2),
                                          static_cast<float>(255 - idx), 255 };
            }
        }
    }

    static void handlePink(Clay_ElementId, Clay_PointerData pointerData, void* userData) {
        if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            auto* self = static_cast<TransitionsExample*>(userData);
            for (int i = 0; i < self->cellCount; i++) {
                int idx = self->colors[i].id;
                self->colors[i].color = { static_cast<float>(255 - idx),
                                          static_cast<float>(255 - idx * 4),
                                          static_cast<float>(255 - idx * 2), 255 };
            }
        }
    }

    static void handleAdd(Clay_ElementId, Clay_PointerData pointerData, void* userData) {
        if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            auto* self = static_cast<TransitionsExample*>(userData);
            int randomIndex = std::rand() % (self->cellCount + 1);
            int newId = self->maxCount;
            char buf[3];
            std::snprintf(buf, sizeof(buf), "%02d", newId);
            self->colors.insert(self->colors.begin() + randomIndex,
                                { newId,
                                  { static_cast<float>(255 - newId),
                                    static_cast<float>(255 - newId * 4),
                                    static_cast<float>(255 - newId * 2), 255 },
                                  std::string(buf, 2) });
            self->cellCount++;
            self->maxCount++;
        }
    }

    void buildLayout() {
        constexpr Clay_Color cWHITE = { 255, 255, 255, 255 };
        constexpr Clay_Color headerBg = { 174, 143, 204, 255 };
        constexpr Clay_Color headerHoverBg = { 154, 123, 184, 255 };
        constexpr Clay_Color emptyColor = {};

        CLAY(CLAY_ID("OuterContainer"),
             { .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                           .padding = { 16, 16, 16, 16 },
                           .childGap = 12,
                           .layoutDirection = CLAY_TOP_TO_BOTTOM },
               .backgroundColor = cWHITE }) {
            // Header bar
            CLAY_AUTO_ID({
                .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(60) },
                            .padding = { .left = 16 },
                            .childGap = 16,
                            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER } },
                .backgroundColor = headerBg,
                .cornerRadius = { 12, 12, 12, 12 },
            }) {
                // Shuffle button
                CLAY(CLAY_ID("ShuffleButton"),
                     { .layout = { .padding = { 16, 16, 8, 8 } },
                       .backgroundColor = Clay_Hovered() ? headerHoverBg : emptyColor,
                       .cornerRadius = CLAY_CORNER_RADIUS(6),
                       .border = { .color = cWHITE, .width = CLAY_BORDER_OUTSIDE(2) },
                     }) {
                    Clay_OnHover(handleShuffle, this);
                    CLAY_TEXT(CLAY_STRING("Randomise"),
                              CLAY_TEXT_CONFIG({ .textColor = cWHITE, .fontSize = 20 }));
                }
                // Blue button
                CLAY(CLAY_ID("BlueButton"),
                     { .layout = { .padding = { 16, 16, 8, 8 } },
                       .backgroundColor = Clay_Hovered() ? headerHoverBg : emptyColor,
                       .cornerRadius = CLAY_CORNER_RADIUS(6),
                       .border = { .color = cWHITE, .width = CLAY_BORDER_OUTSIDE(2) },
                     }) {
                    Clay_OnHover(handleBlue, this);
                    CLAY_TEXT(CLAY_STRING("Blue"),
                              CLAY_TEXT_CONFIG({ .textColor = cWHITE, .fontSize = 20 }));
                }
                // Pink button
                CLAY(CLAY_ID("PinkButton"),
                     { .layout = { .padding = { 16, 16, 8, 8 } },
                       .backgroundColor = Clay_Hovered() ? headerHoverBg : emptyColor,
                       .cornerRadius = CLAY_CORNER_RADIUS(6),
                       .border = { .color = cWHITE, .width = CLAY_BORDER_OUTSIDE(2) },
                     }) {
                    Clay_OnHover(handlePink, this);
                    CLAY_TEXT(CLAY_STRING("Pink"),
                              CLAY_TEXT_CONFIG({ .textColor = cWHITE, .fontSize = 20 }));
                }
                // Add button
                CLAY(CLAY_ID("AddButton"),
                     { .layout = { .padding = { 16, 16, 8, 8 } },
                       .backgroundColor = Clay_Hovered() ? headerHoverBg : emptyColor,
                       .cornerRadius = CLAY_CORNER_RADIUS(6),
                       .border = { .color = cWHITE, .width = CLAY_BORDER_OUTSIDE(2) },
                     }) {
                    Clay_OnHover(handleAdd, this);
                    CLAY_TEXT(CLAY_STRING("Add Box"),
                              CLAY_TEXT_CONFIG({ .textColor = cWHITE, .fontSize = 20 }));
                }
            }
            // Grid rows
            for (int row = 0; row < 5; row++) {
                CLAY(CLAY_IDI("row", row),
                     { .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                                   .childGap = 12 } }) {
                    for (int col = 0; col < 6; col++) {
                        int index = row * 6 + col;
                        if (index >= cellCount) {
                            break;
                        }
                        Clay_Color boxColor = colors[index].color;
                        Clay_Color darker = { boxColor.r * 0.9f, boxColor.g * 0.9f,
                                              boxColor.b * 0.9f, 255 };
                        CLAY(CLAY_IDI("box", colors[index].id),
                             { .layout = { .sizing = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) },
                                           .childAlignment = { CLAY_ALIGN_X_CENTER,
                                                               CLAY_ALIGN_Y_CENTER } },
                               .backgroundColor = boxColor,
                               .overlayColor = Clay_Hovered()
                                                   ? (Clay_Color){ 140, 140, 140, 80 }
                                                   : (Clay_Color){ 255, 255, 255, 0 },
                               .cornerRadius = { 12, 12, 12, 12 },
                               .border = { .color = darker, .width = CLAY_BORDER_OUTSIDE(3) },
                               .transition = {
                                   .handler = Clay_EaseOut,
                                   .duration = Clay_Hovered() &&
                                                       Clay_GetPointerState().state !=
                                                           CLAY_POINTER_DATA_PRESSED_THIS_FRAME
                                                   ? 0.f
                                                   : 0.5f,
                                   .properties = static_cast<Clay_TransitionProperty>(
                                                 CLAY_TRANSITION_PROPERTY_WIDTH |
                                                 CLAY_TRANSITION_PROPERTY_POSITION |
                                                 CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR |
                                                 CLAY_TRANSITION_PROPERTY_BACKGROUND_COLOR),
                                   .enter = { .setInitialState = enterExitSlideUp },
                                   .exit = { .setFinalState = enterExitSlideUp },
                               },
                             }) {
                            // Cell click handling - encode index in userData
                            Clay_OnHover(handleCellClickStatic,
                                         reinterpret_cast<void*>(static_cast<intptr_t>(index)));
                            Clay_String label = {
                                .isStaticallyAllocated = true,
                                .length = static_cast<int32_t>(colors[index].stringId.size()),
                                .chars = colors[index].stringId.c_str(),
                            };
                            CLAY_TEXT(label,
                                      CLAY_TEXT_CONFIG({
                                          .textColor = colors[index].id > 29
                                                           ? (Clay_Color){ 255, 255, 255, 255 }
                                                           : (Clay_Color){ 154, 123, 184, 255 },
                                          .fontSize = 32,
                                      }));
                        }
                    }
                }
            }
        }
    }

    static void handleCellClickStatic(Clay_ElementId, Clay_PointerData pointerData,
                                      void* userData) {
        if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
            int idx = static_cast<int>(reinterpret_cast<intptr_t>(userData));
            auto* self = instance;
            if (self && idx >= 0 && idx < self->cellCount) {
                for (int i = idx; i < self->cellCount - 1; i++) {
                    self->colors[i] = self->colors[i + 1];
                }
                self->cellCount = std::max(self->cellCount - 1, 0);
            }
        }
    }

    static inline TransitionsExample* instance = nullptr;

    std::vector<uint8_t> clayMemory;
    Clay_RenderCommandArray renderCommands{};
    std::vector<SortableBox> colors;
    std::vector<char> charData;
    int cellCount = 30;
    int maxCount = 0;
};

jngl::AppParameters jnglInit() {
    jngl::AppParameters params;
    params.screenSize = { 1024, 768 };
    params.start = []() { return std::make_shared<TransitionsExample>(); };
    return params;
}
