
#include <clayman.hpp>
#include <clay_renderer_jngl.hpp>
#include <jngl/init.hpp>

#include <string>

class ClayExample : public jngl::Scene {
public:
    void step() override {
        const auto w = static_cast<uint32_t>(std::lround(jngl::getScreenWidth()));
        const auto h = static_cast<uint32_t>(std::lround(jngl::getScreenHeight()));
        const auto mouse = jngl::getMousePos();
        clayMan.updateClayState(w, h, static_cast<float>(mouse.x + jngl::getScreenWidth() / 2.0),
                                static_cast<float>(mouse.y + jngl::getScreenHeight() / 2.0), 0,
                                jngl::getMouseWheel() * 3, 1.0f / jngl::getStepsPerSecond(),
                                jngl::mouseDown());

        if (clayMan.pointerOver("Button1")) {
            buttonTextConfig.textColor = { 255, 255, 255, 255 };
            if (clayMan.mousePressed()) {
                theStrings[0] = "The Button Has Been Pressed!";
            }
        } else {
            buttonTextConfig.textColor = { 0, 0, 0, 255 };
        }

        clayMan.beginLayout();
        clayMan.element(
        {.id = clayMan.hashID("OuterContainer"),
         .layout =
             {
                 .sizing = clayMan.expandXY(),
                 .childGap = 10,
                 .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
                                    .y = CLAY_ALIGN_Y_CENTER},
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
             },
         .backgroundColor = {10, 10, 10, 255}},
        [&]() {
          clayMan.textElement(theStrings[0], textConfig);
          clayMan.element({.id = clayMan.hashID("ImageContainer"),
                           .layout = {.sizing = clayMan.fixedSize(23, 42)}},
                          [&]() {
                            clayMan.element(
                                {.id = clayMan.hashID("TheImage"),
                                 .layout = {.sizing = clayMan.expandXY()},
                                 .image = {.imageData = &sample_image,
                                           .sourceDimensions = {23, 42}}});
                          });
          clayMan.element({.id = clayMan.hashID("Button1"),
                           .layout = {.padding = clayMan.padAll(6)},
                           .backgroundColor = {150, 150, 150, 150},
                           .cornerRadius = CLAY_CORNER_RADIUS(8)},
                          [&]() {
                            clayMan.textElement("BUTTON",
                                                buttonTextConfig);
                          });
        });
        renderCommands = clayMan.endLayout();
    }

    void draw() const override {
        drawClayCommands(renderCommands);
    }

private:
    ClayMan clayMan{ static_cast<uint32_t>(std::lround(jngl::getScreenWidth())),
                     static_cast<uint32_t>(std::lround(jngl::getScreenHeight())), measureText,
                     nullptr };
    Clay_RenderCommandArray renderCommands{};
    std::vector<std::string> theStrings{ { "This is a string!" } };
    Clay_TextElementConfig textConfig{ .textColor = { 255, 255, 255, 255 }, .fontSize = 16 };
    Clay_TextElementConfig buttonTextConfig{ .textColor = { 0, 0, 0, 255 }, .fontSize = 16 };
    jngl::Sprite sample_image{ "sample" };
};

jngl::AppParameters jnglInit() {
    jngl::AppParameters params;
    params.screenSize = { 800, 600 };
    params.start = []() {
        jngl::setFontColor(0xffffff_rgb);
        return std::make_shared<ClayExample>();
    };
    return params;
}
