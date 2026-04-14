
#include <clayman.hpp>
#include <clay_renderer_jngl.hpp>
#include <jngl/init.hpp>

#include <string>
#include <vector>

class FullExample : public jngl::Scene {
public:
    void step() override {
        if (jngl::keyPressed('d')) {
            Clay_SetDebugModeEnabled(true);
        }

        const auto w = static_cast<uint32_t>(std::lround(jngl::getScreenWidth()));
        const auto h = static_cast<uint32_t>(std::lround(jngl::getScreenHeight()));
        const auto mouse = jngl::getMousePos();
        clayMan.updateClayState(w, h, static_cast<float>(mouse.x + jngl::getScreenWidth() / 2.0),
                                static_cast<float>(mouse.y + jngl::getScreenHeight() / 2.0), 0,
                                jngl::getMouseWheel() * 3, 1.0f / jngl::getStepsPerSecond(),
                                jngl::mouseDown());

        // Actions (before beginLayout, using previous frame's layout)
        if (clayMan.pointerOver("PopupClose") && clayMan.mousePressed()) {
            showPopup = false;
        }
        if (clayMan.pointerOver("PopUpButton") && clayMan.mousePressed()) {
            showPopup = !showPopup;
        }

        for (size_t i = 0; i < documents.size(); i++) {
            if (i != selectedDocumentIndex) {
                std::string docID = "SidebarButton" + std::to_string(i);
                if (!showPopup && clayMan.pointerOver(docID) && clayMan.mousePressed()) {
                    selectedDocumentIndex = static_cast<uint32_t>(i);
                }
            }
        }

        clayMan.beginLayout();
        buildLayout();
        renderCommands = clayMan.endLayout();
    }

    void draw() const override {
        drawClayCommands(renderCommands);
    }

private:
    void renderHeaderButton(const std::string& text) {
        clayMan.element(
            {
                .layout = headerButtonLayoutConfig,
                .backgroundColor = colors.gray_light,
                .cornerRadius = { 5, 5, 5, 5 },
            },
            [&] { clayMan.textElement(text, headerTextConfig); });
    }

    void renderDropdownMenuItem(const std::string& text) {
        clayMan.element({ .layout = dropdownItemLayoutConfig },
                        [&] { clayMan.textElement(text, headerTextConfig); });
    }

    void popupWindow() {
        if (!showPopup) {
            return;
        }
        float popupWidth = clayMan.getWindowWidth() * 0.5f;
        float popupHeight = clayMan.getWindowHeight() * 0.5f;

        clayMan.element(
            {
                .id = clayMan.hashID("Popup"),
                .layout = { .sizing = clayMan.fixedSize(popupWidth, popupHeight),
                            .layoutDirection = CLAY_TOP_TO_BOTTOM },
                .backgroundColor = colors.gray_dark2,
                .cornerRadius = { 8, 8, 8, 8 },
                .floating = { .parentId = clayMan.getClayElementId("OuterContainer").id,
                              .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                                                .parent = CLAY_ATTACH_POINT_CENTER_CENTER },
                              .attachTo = CLAY_ATTACH_TO_PARENT },
                .border = { .color = colors.gray_light, .width = { 4, 4, 4, 4, 0 } },
            },
            [&] {
                // Close button row
                clayMan.element(
                    {
                        .id = clayMan.hashID("PopupCloseContainer"),
                        .layout = { .sizing = clayMan.expandX(),
                                    .padding = clayMan.padAll(4),
                                    .childAlignment = { .x = CLAY_ALIGN_X_RIGHT } },
                    },
                    [&] {
                        clayMan.element(
                            {
                                .id = clayMan.hashID("PopupClose"),
                                .layout = { .sizing = clayMan.fixedSize(24, 24),
                                            .padding = clayMan.padAll(4) },
                                .backgroundColor = clayMan.pointerOver("PopupClose")
                                                       ? colors.gray_light
                                                       : colors.gray_dark2,
                                .cornerRadius = { 4, 4, 4, 4 },
                            },
                            [&] {
                                clayMan.textElement(
                                    "X", Clay_TextElementConfig{ .textColor = colors.white,
                                                                 .fontSize = 16 });
                            });
                    });

                // Popup content
                clayMan.element(
                    { .layout = { .sizing = clayMan.expandXY(), .padding = clayMan.padAll(80) } },
                    [&] { clayMan.textElement("This is a pop-up window!", documentTextConfig); });
            });
    }

    void buildLayout() {
        clayMan.element(
            {
                .id = clayMan.hashID("OuterContainer"),
                .layout = outerLayoutConfig,
                .backgroundColor = colors.gray_dark2,
            },
            [&] {
                // Header bar
                clayMan.element(
                    {
                        .id = clayMan.hashID("HeaderBar"),
                        .layout = headerLayoutConfig,
                        .backgroundColor = colors.gray_med,
                        .cornerRadius = { 8, 8, 8, 8 },
                    },
                    [&] {
                        // File button with dropdown
                        clayMan.element(
                            {
                                .id = clayMan.hashID("FileButton"),
                                .layout = { .padding = clayMan.padXY(16, 8) },
                                .backgroundColor = colors.gray_light,
                                .cornerRadius = { 5, 5, 5, 5 },
                            },
                            [&] {
                                clayMan.textElement("File", headerTextConfig);

                                if (!showPopup && (clayMan.pointerOver("FileButton") ||
                                                   clayMan.pointerOver("FileMenu"))) {
                                    clayMan.element(
                                        {
                                            .id = clayMan.hashID("FileMenu"),
                                            .layout = {.padding = clayMan.padY(8)},
                                            .floating =
                                                {
                                                    .attachPoints =
                                                        {
                                                            .element =
                                                                CLAY_ATTACH_POINT_LEFT_TOP,
                                                            .parent =
                                                                CLAY_ATTACH_POINT_LEFT_BOTTOM,
                                                        },
                                                    .attachTo = CLAY_ATTACH_TO_PARENT,
                                                },
                                        },
                                        [&] {
                                            clayMan.element(
                                                {
                                                    .layout = dropdownLayoutConfig,
                                                    .backgroundColor = colors.gray_dark2,
                                                    .cornerRadius = {8, 8, 8, 8},
                                                },
                                                [&] {
                                                    // PopUp button in dropdown
                                                    clayMan.element(
                                                        {
                                                            .id =
                                                                clayMan.hashID("PopUpButton"),
                                                            .layout = dropdownItemLayoutConfig,
                                                        },
                                                        [&] {
                                                            clayMan.textElement(
                                                                "PopUp", headerTextConfig);
                                                        });
                                                    renderDropdownMenuItem("New");
                                                    renderDropdownMenuItem("Open");
                                                    renderDropdownMenuItem("Close");
                                                });
                                        });
                                }
                            });

                        renderHeaderButton("Edit");

                        // Spacer
                        clayMan.element(
                            {
                                .id = clayMan.hashID("HeaderSpacer"),
                                .layout = { .sizing = clayMan.expandXY() },
                            },
                            [&]() {});

                        renderHeaderButton("Upload");
                        renderHeaderButton("Media");
                        renderHeaderButton("Support");
                    });

                // Lower content area
                clayMan.element(
                    {
                        .id = clayMan.hashID("LowerContent"),
                        .layout = { .sizing = clayMan.expandXY(), .childGap = 16 },
                    },
                    [&] {
                        // Sidebar
                        clayMan.element(
                            {
                                .id = clayMan.hashID("Sidebar"),
                                .layout = { .sizing = clayMan.expandYfixedX(250),
                                            .padding = clayMan.padAll(16),
                                            .childGap = 8,
                                            .layoutDirection = CLAY_TOP_TO_BOTTOM },
                                .backgroundColor = colors.gray_med,
                            },
                            [&] {
                                for (size_t i = 0; i < documents.size(); i++) {
                                    if (i == selectedDocumentIndex) {
                                        clayMan.element(
                                            {
                                                .layout = sidebarButtonLayout,
                                                .backgroundColor = colors.gray_light2,
                                                .cornerRadius = { 8, 8, 8, 8 },
                                            },
                                            [&] {
                                                clayMan.textElement(documents[i].title,
                                                                    sidebarTextConfig);
                                            });
                                    } else {
                                        std::string docID = "SidebarButton" + std::to_string(i);
                                        bool overButton = !showPopup && clayMan.pointerOver(docID);

                                        clayMan.element(
                                            {
                                                .id = clayMan.hashID(docID),
                                                .layout = sidebarButtonLayout,
                                                .backgroundColor = overButton
                                                                       ? colors.gray_light2_alpha
                                                                       : Clay_Color{ 0, 0, 0, 0 },
                                                .cornerRadius = { 8, 8, 8, 8 },
                                            },
                                            [&] {
                                                clayMan.textElement(documents[i].title,
                                                                    sidebarTextConfig);
                                            });
                                    }
                                }
                            });

                        // Main content
                        clayMan.element(
                            {
                                .id = clayMan.hashID("MainContent"),
                                .layout = { .sizing = clayMan.expandXY(),
                                            .padding = clayMan.padAll(16),
                                            .childGap = 16,
                                            .layoutDirection = CLAY_TOP_TO_BOTTOM },
                                .backgroundColor = colors.gray_med,
                                .clip = { .vertical = true },
                            },
                            [&] {
                                if (selectedDocumentIndex >= documents.size()) {
                                    selectedDocumentIndex =
                                        static_cast<uint32_t>(documents.size() - 1);
                                }
                                clayMan.textElement(documents[selectedDocumentIndex].title,
                                                    documentTextConfig);
                                clayMan.textElement(documents[selectedDocumentIndex].contents,
                                                    documentTextConfig);
                            });
                    });

                // Popup window (floating)
                popupWindow();
            });
    }

    ClayMan clayMan{ static_cast<uint32_t>(std::lround(jngl::getScreenWidth())),
                     static_cast<uint32_t>(std::lround(jngl::getScreenHeight())), measureText,
                     nullptr };
    Clay_RenderCommandArray renderCommands{};

    struct Document {
        uint32_t id;
        std::string title;
        std::string contents;
    };

    std::vector<Document> documents = {
        { 0, "Squirrels",
          "The Secret Life of Squirrels: Nature's Clever Acrobats\nSquirrels are often overlooked "
          "creatures, dismissed as mere park inhabitants or backyard nuisances. Yet, beneath their "
          "fluffy tails and twitching noses lies an intricate world of cunning, agility, and "
          "survival tactics that are nothing short of fascinating. As one of the most common "
          "mammals in North America, squirrels have adapted to a wide range of environments from "
          "bustling urban centers to tranquil forests and have developed a variety of unique "
          "behaviors that continue to intrigue scientists and nature enthusiasts alike.\n\nMaster "
          "Tree Climbers\nAt the heart of a squirrel's skill set is its impressive ability to "
          "navigate trees with ease. Whether they're darting from branch to branch or leaping "
          "across wide gaps, squirrels possess an innate talent for acrobatics. Their powerful "
          "hind legs, which are longer than their front legs, give them remarkable jumping power. "
          "With a tail that acts as a counterbalance, squirrels can leap distances of up to ten "
          "times the length of their body, making them some of the best aerial acrobats in the "
          "animal kingdom.\nBut it's not just their agility that makes them exceptional climbers. "
          "Squirrels' sharp, curved claws allow them to grip tree bark with precision, while the "
          "soft pads on their feet provide traction on slippery surfaces. Their ability to run at "
          "high speeds and scale vertical trunks with ease is a testament to the evolutionary "
          "adaptations that have made them so successful in their arboreal habitats.\n\nFood "
          "Hoarders Extraordinaire\nSquirrels are often seen frantically gathering nuts, seeds, "
          "and even fungi in preparation for winter. While this behavior may seem like instinctual "
          "hoarding, it is actually a survival strategy that has been honed over millions of "
          "years. Known as \"scatter hoarding,\" squirrels store their food in a variety of hidden "
          "locations, often burying it deep in the soil or stashing it in hollowed-out tree "
          "trunks.\nInterestingly, squirrels have an incredible memory for the locations of their "
          "caches. Research has shown that they can remember thousands of hiding spots, often "
          "returning to them months later when food is scarce. However, they don't always recover "
          "every stash some forgotten caches eventually sprout into new trees, contributing to "
          "forest regeneration. This unintentional role as forest gardeners highlights the "
          "ecological importance of squirrels in their ecosystems.\n\nThe Great Squirrel Debate: "
          "Urban vs. Wild\nWhile squirrels are most commonly associated with rural or wooded "
          "areas, their adaptability has allowed them to thrive in urban environments as well. In "
          "cities, squirrels have become adept at finding food sources in places like parks, "
          "streets, and even garbage cans. However, their urban counterparts face unique "
          "challenges, including traffic, predators, and the lack of natural shelters. Despite "
          "these obstacles, squirrels in urban areas are often observed using human infrastructure "
          "such as buildings, bridges, and power lines as highways for their acrobatic "
          "escapades.\nThere is, however, a growing concern regarding the impact of urban life on "
          "squirrel populations. Pollution, deforestation, and the loss of natural habitats are "
          "making it more difficult for squirrels to find adequate food and shelter. As a result, "
          "conservationists are focusing on creating squirrel-friendly spaces within cities, with "
          "the goal of ensuring these resourceful creatures continue to thrive in both rural and "
          "urban landscapes.\n\nA Symbol of Resilience\nIn many cultures, squirrels are symbols of "
          "resourcefulness, adaptability, and preparation. Their ability to thrive in a variety of "
          "environments while navigating challenges with agility and grace serves as a reminder of "
          "the resilience inherent in nature. Whether you encounter them in a quiet forest, a city "
          "park, or your own backyard, squirrels are creatures that never fail to amaze with their "
          "endless energy and ingenuity.\nIn the end, squirrels may be small, but they are mighty "
          "in their ability to survive and thrive in a world that is constantly changing. So next "
          "time you spot one hopping across a branch or darting across your lawn, take a moment to "
          "appreciate the remarkable acrobat at work a true marvel of the natural world.\n" },
        { 1, "Lorem Ipsum",
          "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
          "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
          "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure "
          "dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
          "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
          "mollit anim id est laborum." },
        { 2, "Vacuum Instructions",
          "Chapter 3: Getting Started - Unpacking and Setup\n\nCongratulations on your new "
          "SuperClean Pro 5000 vacuum cleaner! In this section, we will guide you through the "
          "simple steps to get your vacuum up and running. Before you begin, please ensure that "
          "you have all the components listed in the \"Package Contents\" section on page 2.\n\n1. "
          "Unboxing Your Vacuum\nCarefully remove the vacuum cleaner from the box. Avoid using "
          "sharp objects that could damage the product. Once removed, place the unit on a flat, "
          "stable surface to proceed with the setup. Inside the box, you should find:\n\n    The "
          "main vacuum unit\n    A telescoping extension wand\n    A set of specialized cleaning "
          "tools (crevice tool, upholstery brush, etc.)\n    A reusable dust bag (if applicable)\n "
          "   A power cord with a 3-prong plug\n    A set of quick-start instructions\n\n2. "
          "Assembling Your Vacuum\nBegin by attaching the extension wand to the main body of the "
          "vacuum cleaner. Line up the connectors and twist the wand into place until you hear a "
          "click. Next, select the desired cleaning tool and firmly attach it to the wand's end, "
          "ensuring it is securely locked in.\n\nFor models that require a dust bag, slide the bag "
          "into the compartment at the back of the vacuum, making sure it is properly aligned with "
          "the internal mechanism. If your vacuum uses a bagless system, ensure the dust container "
          "is correctly seated and locked in place before use.\n\n3. Powering On\nTo start the "
          "vacuum, plug the power cord into a grounded electrical outlet. Once plugged in, locate "
          "the power switch, usually positioned on the side of the handle or body of the unit, "
          "depending on your model. Press the switch to the \"On\" position, and you should hear "
          "the motor begin to hum. If the vacuum does not power on, check that the power cord is "
          "securely plugged in, and ensure there are no blockages in the power switch.\n\nNote: "
          "Before first use, ensure that the vacuum filter (if your model has one) is properly "
          "installed. If unsure, refer to \"Section 5: Maintenance\" for filter installation "
          "instructions." },
        { 3, "To Delete", "This article will get automatically deleted." },
        { 4, "Article 5", "Article 5" }
    };

    uint32_t selectedDocumentIndex = 0;
    bool showPopup = false;

    struct Colors {
        Clay_Color white = { 255, 255, 255, 255 };
        Clay_Color gray_med = { 90, 90, 90, 255 };
        Clay_Color gray_light = { 140, 140, 140, 255 };
        Clay_Color gray_light2 = { 120, 120, 120, 255 };
        Clay_Color gray_light2_alpha = { 120, 120, 120, 120 };
        Clay_Color gray_dark = { 40, 40, 40, 255 };
        Clay_Color gray_dark2 = { 43, 41, 51, 255 };
    } colors;

    Clay_LayoutConfig outerLayoutConfig{
        .sizing = clayMan.expandXY(),
        .padding = clayMan.padAll(16),
        .childGap = 16,
        .layoutDirection = CLAY_TOP_TO_BOTTOM,
    };
    Clay_LayoutConfig headerLayoutConfig{
        .sizing = clayMan.expandXfixedY(60),
        .padding = clayMan.padX(16),
        .childGap = 16,
        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
    };
    Clay_LayoutConfig headerButtonLayoutConfig{ .padding = clayMan.padXY(16, 8) };
    Clay_LayoutConfig dropdownLayoutConfig{ .sizing = clayMan.fixedSize(200, 200),
                                            .layoutDirection = CLAY_TOP_TO_BOTTOM };
    Clay_LayoutConfig dropdownItemLayoutConfig{ .padding = clayMan.padAll(16) };
    Clay_LayoutConfig sidebarButtonLayout{ .sizing = clayMan.expandX(),
                                           .padding = clayMan.padAll(16) };

    Clay_TextElementConfig headerTextConfig{ .textColor = colors.white, .fontSize = 16 };
    Clay_TextElementConfig sidebarTextConfig{ .textColor = colors.white, .fontSize = 20 };
    Clay_TextElementConfig documentTextConfig{ .textColor = colors.white, .fontSize = 24 };
};

jngl::AppParameters jnglInit() {
    jngl::AppParameters params;
    params.screenSize = { 1024, 768 };
    params.start = []() {
        jngl::setFontColor(0xffffff_rgb);
        return std::make_shared<FullExample>();
    };
    return params;
}
