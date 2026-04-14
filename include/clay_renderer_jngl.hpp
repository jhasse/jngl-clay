#pragma once

#include <clay.h>
#include <jngl.hpp>
#include <stack>

inline Clay_Dimensions measureText(Clay_StringSlice text, Clay_TextElementConfig* config,
                                   void* /*userData*/) {
    jngl::setFontSize(config->fontSize);
    std::string str(text.chars, text.length);
    return {
        static_cast<float>(jngl::getTextWidth(str)),
        static_cast<float>(jngl::getLineHeight()),
    };
}

inline void drawClayCommands(const Clay_RenderCommandArray& renderCommands) {
    const auto halfW = jngl::getScreenWidth() / 2.0;
    const auto halfH = jngl::getScreenHeight() / 2.0;
    std::stack<jngl::Rgba> overlayColor;
    std::stack<jngl::Finally> scissorStack;
    auto clayColorToJngl = [&overlayColor](const Clay_Color& col) {
        auto color = jngl::Rgba::u8(static_cast<uint8_t>(col.r), static_cast<uint8_t>(col.g),
                                    static_cast<uint8_t>(col.b), static_cast<uint8_t>(col.a));
        if (!overlayColor.empty()) {
            color = jngl::Rgba(jngl::mix(static_cast<jngl::Rgb>(color),
                                         static_cast<jngl::Rgb>(overlayColor.top()),
                                         overlayColor.top().getAlpha()),
                               jngl::Alpha(color.getAlpha()));
        }
        return color;
    };

    for (int32_t i = 0; i < renderCommands.length; ++i) {
        Clay_RenderCommand* cmd =
            Clay_RenderCommandArray_Get(const_cast<Clay_RenderCommandArray*>(&renderCommands), i);
        const auto& bb = cmd->boundingBox;
        auto mv = jngl::modelview().translate(
            { static_cast<float>(bb.x - halfW), static_cast<float>(bb.y - halfH) });

        switch (cmd->commandType) {
        case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            const auto& rect = cmd->renderData.rectangle;
            const auto& col = rect.backgroundColor;
            auto color = clayColorToJngl(col);
            const auto& cr = rect.cornerRadius;
            if (cr.topLeft > 0 || cr.topRight > 0 || cr.bottomLeft > 0 || cr.bottomRight > 0) {
                jngl::drawRoundedRect(mv, { bb.width, bb.height }, color, cr.topLeft, cr.topRight,
                                      cr.bottomLeft, cr.bottomRight);
            } else {
                jngl::drawRect(mv, { bb.width, bb.height }, color);
            }
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_TEXT: {
            const auto& td = cmd->renderData.text;
            std::string text(td.stringContents.chars, td.stringContents.length);
            jngl::setFontSize(td.fontSize);
            jngl::setFontColor(clayColorToJngl(td.textColor));
            jngl::print(mv.translate({ 0, jngl::getLineHeight() * 0.11 }), text);
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
            auto* sprite = static_cast<jngl::Sprite*>(cmd->renderData.image.imageData);
            if (sprite) {
                sprite->draw(mv.translate({ bb.width / 2.f, bb.height / 2.f }));
            }
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            const auto& bd = cmd->renderData.border;
            auto color = clayColorToJngl(bd.color);
            const auto& cr = bd.cornerRadius;
            // Left border
            if (bd.width.left > 0) {
                jngl::drawRect(
                    jngl::Mat3(mv).translate({ 0, cr.topLeft }),
                    { static_cast<float>(bd.width.left), bb.height - cr.topLeft - cr.bottomLeft },
                    color);
            }
            // Right border
            if (bd.width.right > 0) {
                jngl::drawRect(jngl::Mat3(mv).translate({ bb.width - bd.width.right, cr.topRight }),
                               { static_cast<float>(bd.width.right),
                                 bb.height - cr.topRight - cr.bottomRight },
                               color);
            }
            // Top border
            if (bd.width.top > 0) {
                jngl::drawRect(
                    jngl::Mat3(mv).translate({ cr.topLeft, 0 }),
                    { bb.width - cr.topLeft - cr.topRight, static_cast<float>(bd.width.top) },
                    color);
            }
            // Bottom border
            if (bd.width.bottom > 0) {
                jngl::drawRect(
                    jngl::Mat3(mv).translate({ cr.bottomLeft, bb.height - bd.width.bottom }),
                    { bb.width - cr.bottomLeft - cr.bottomRight,
                      static_cast<float>(bd.width.bottom) },
                    color);
            }
            // Corner arcs
            if (cr.topLeft > 0) {
                jngl::drawRing(jngl::Mat3(mv).translate({ cr.topLeft, cr.topLeft }),
                               cr.topLeft - bd.width.top, cr.topLeft, 180, 270, color);
            }
            if (cr.topRight > 0) {
                jngl::drawRing(jngl::Mat3(mv).translate({ bb.width - cr.topRight, cr.topRight }),
                               cr.topRight - bd.width.top, cr.topRight, 270, 360, color);
            }
            if (cr.bottomLeft > 0) {
                jngl::drawRing(
                    jngl::Mat3(mv).translate({ cr.bottomLeft, bb.height - cr.bottomLeft }),
                    cr.bottomLeft - bd.width.bottom, cr.bottomLeft, 90, 180, color);
            }
            if (cr.bottomRight > 0) {
                jngl::drawRing(jngl::Mat3(mv).translate(
                                   { bb.width - cr.bottomRight, bb.height - cr.bottomRight }),
                               cr.bottomRight - bd.width.bottom, cr.bottomRight, 0, 90, color);
            }
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
            scissorStack.push(jngl::scissor({ bb.x - halfW, bb.y - halfH }, { bb.width, bb.height }));
            break;
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
            scissorStack.pop();
            break;
#ifndef CLAY_MAN // ships with an older clay version unfortunately, so these commands are not
                 // supported there
        case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_START: {
            const auto& oc = cmd->renderData.overlayColor.color;
            overlayColor.push(jngl::Rgba::u8(static_cast<uint8_t>(oc.r), static_cast<uint8_t>(oc.g),
                                             static_cast<uint8_t>(oc.b),
                                             static_cast<uint8_t>(oc.a)));
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_OVERLAY_COLOR_END:
            overlayColor.pop();
            break;
#endif
        default:
            break;
        }
    }
}
