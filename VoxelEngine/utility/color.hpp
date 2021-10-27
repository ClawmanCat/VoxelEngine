#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/image.hpp>


namespace ve {
    using color = RGBA8;


    constexpr vec3f normalize_color(const color& clr) {
        return vec3f { clr } / 255.0f;
    }


    // Color names match those defined by CSS.
    // Since CSS defines GREEN as [0, 128, 0] rather than [0, 255, 0], three additional colors are provided:
    // PURE_RED, PURE_GREEN and PURE_BLUE, which have 255 in their appropriate channel and 0 everywhere else.
    struct colors {
        constexpr static color PURE_RED                 = { 255, 0,   0,   255 };
        constexpr static color PURE_GREEN               = { 0,   255, 0,   255 };
        constexpr static color PURE_BLUE                = { 0,   0,   255, 255 };

        constexpr static color LIGHT_SALMON             = { 255, 160, 122, 255 };
        constexpr static color SALMON                   = { 250, 128, 114, 255 };
        constexpr static color DARK_SALMON              = { 233, 150, 122, 255 };
        constexpr static color LIGHT_CORAL              = { 240, 128, 128, 255 };
        constexpr static color INDIAN_RED               = { 205, 92,  92,  255 };
        constexpr static color CRIMSON                  = { 220, 20,  60,  255 };
        constexpr static color FIREBRICK                = { 178, 34,  34,  255 };
        constexpr static color RED                      = { 255, 0,   0,   255 };
        constexpr static color DARK_RED                 = { 139, 0,   0,   255 };
        constexpr static color CORAL                    = { 255, 127, 80,  255 };
        constexpr static color TOMATO                   = { 255, 99,  71,  255 };
        constexpr static color ORANGE_RED               = { 255, 69,  0,   255 };
        constexpr static color GOLD                     = { 255, 215, 0,   255 };
        constexpr static color ORANGE                   = { 255, 165, 0,   255 };
        constexpr static color DARK_ORANGE              = { 255, 140, 0,   255 };
        constexpr static color LIGHT_YELLOW             = { 255, 255, 224, 255 };
        constexpr static color LEMON_CHIFFON            = { 255, 250, 205, 255 };
        constexpr static color LIGHT_GOLDENROD_YELLOW   = { 250, 250, 210, 255 };
        constexpr static color PAPAYA_WHIP              = { 255, 239, 213, 255 };
        constexpr static color MOCCASIN                 = { 255, 228, 181, 255 };
        constexpr static color PEACH_PUFF               = { 255, 218, 185, 255 };
        constexpr static color PALE_GOLDENROD           = { 238, 232, 170, 255 };
        constexpr static color KHAKI                    = { 240, 230, 140, 255 };
        constexpr static color DARK_KHAKI               = { 189, 183, 107, 255 };
        constexpr static color YELLOW                   = { 255, 255, 0,   255 };
        constexpr static color LAWN_GREEN               = { 124, 252, 0,   255 };
        constexpr static color CHARTREUSE               = { 127, 255, 0,   255 };
        constexpr static color LIME_GREEN               = { 50,  205, 50,  255 };
        constexpr static color LIME                     = { 0,   255, 0,   255 };
        constexpr static color FOREST_GREEN             = { 34,  139, 34,  255 };
        constexpr static color GREEN                    = { 0,   255, 0,   255 };
        constexpr static color DARK_GREEN               = { 0,   100, 0,   255 };
        constexpr static color GREEN_YELLOW             = { 173, 255, 47,  255 };
        constexpr static color YELLOW_GREEN             = { 154, 205, 50,  255 };
        constexpr static color SPRING_GREEN             = { 0,   255, 127, 255 };
        constexpr static color MEDIUM_SPRING_GREEN      = { 0,   250, 154, 255 };
        constexpr static color LIGHT_GREEN              = { 144, 238, 144, 255 };
        constexpr static color PALE_GREEN               = { 152, 251, 152, 255 };
        constexpr static color DARK_SEA_GREEN           = { 143, 188, 143, 255 };
        constexpr static color MEDIUM_SEA_GREEN         = { 60,  179, 113, 255 };
        constexpr static color SEA_GREEN                = { 46,  139, 87,  255 };
        constexpr static color OLIVE                    = { 128, 128, 0,   255 };
        constexpr static color DARK_OLIVE_GREEN         = { 85,  107, 47,  255 };
        constexpr static color OLIVE_DRAB               = { 107, 142, 35,  255 };
        constexpr static color LIGHT_CYAN               = { 224, 255, 255, 255 };
        constexpr static color CYAN                     = { 0,   255, 255, 255 };
        constexpr static color AQUA                     = { 0,   255, 255, 255 };
        constexpr static color AQUAMARINE               = { 127, 255, 212, 255 };
        constexpr static color MEDIUM_AQUAMARINE        = { 102, 205, 170, 255 };
        constexpr static color PALE_TURQUOISE           = { 175, 238, 238, 255 };
        constexpr static color TURQUOISE                = { 64,  224, 208, 255 };
        constexpr static color MEDIUM_TURQUOISE         = { 72,  209, 204, 255 };
        constexpr static color DARK_TURQUOISE           = { 0,   206, 209, 255 };
        constexpr static color LIGHT_SEA_GREEN          = { 32,  178, 170, 255 };
        constexpr static color CADET_BLUE               = { 95,  158, 160, 255 };
        constexpr static color DARK_CYAN                = { 0,   139, 139, 255 };
        constexpr static color TEAL                     = { 0,   128, 128, 255 };
        constexpr static color POWDER_BLUE              = { 176, 224, 230, 255 };
        constexpr static color LIGHT_BLUE               = { 173, 216, 230, 255 };
        constexpr static color LIGHT_SKY_BLUE           = { 135, 206, 250, 255 };
        constexpr static color SKY_BLUE                 = { 135, 206, 235, 255 };
        constexpr static color DEEP_SKY_BLUE            = { 0,   191, 255, 255 };
        constexpr static color LIGHT_STEEL_BLUE         = { 176, 196, 222, 255 };
        constexpr static color DODGER_BLUE              = { 30,  144, 255, 255 };
        constexpr static color CORNFLOWER_BLUE          = { 100, 149, 237, 255 };
        constexpr static color STEEL_BLUE               = { 70,  130, 180, 255 };
        constexpr static color ROYAL_BLUE               = { 65,  105, 225, 255 };
        constexpr static color BLUE                     = { 0,   0,   255, 255 };
        constexpr static color MEDIUM_BLUE              = { 0,   0,   205, 255 };
        constexpr static color DARK_BLUE                = { 0,   0,   139, 255 };
        constexpr static color NAVY                     = { 0,   0,   128, 255 };
        constexpr static color MIDNIGHT_BLUE            = { 25,  25,  112, 255 };
        constexpr static color MEDIUM_SLATE_BLUE        = { 123, 104, 238, 255 };
        constexpr static color SLATE_BLUE               = { 106, 90,  205, 255 };
        constexpr static color DARK_SLATE_BLUE          = { 72,  61,  139, 255 };
        constexpr static color LAVENDER                 = { 230, 230, 250, 255 };
        constexpr static color THISTLE                  = { 216, 191, 216, 255 };
        constexpr static color PLUM                     = { 221, 160, 221, 255 };
        constexpr static color VIOLET                   = { 238, 130, 238, 255 };
        constexpr static color ORCHID                   = { 218, 112, 214, 255 };
        constexpr static color FUCHSIA                  = { 255, 0,   255, 255 };
        constexpr static color MAGENTA                  = { 255, 0,   255, 255 };
        constexpr static color MEDIUM_ORCHID            = { 186, 85,  211, 255 };
        constexpr static color MEDIUM_PURPLE            = { 147, 112, 219, 255 };
        constexpr static color BLUE_VIOLET              = { 138, 43,  226, 255 };
        constexpr static color DARK_VIOLET              = { 148, 0,   211, 255 };
        constexpr static color DARK_ORCHID              = { 153, 50,  204, 255 };
        constexpr static color DARK_MAGENTA             = { 139, 0,   139, 255 };
        constexpr static color PURPLE                   = { 128, 0,   128, 255 };
        constexpr static color INDIGO                   = { 75,  0,   130, 255 };
        constexpr static color PINK                     = { 255, 192, 203, 255 };
        constexpr static color LIGHT_PINK               = { 255, 182, 193, 255 };
        constexpr static color HOT_PINK                 = { 255, 105, 180, 255 };
        constexpr static color DEEP_PINK                = { 255, 20,  147, 255 };
        constexpr static color PALE_VIOLET_RED          = { 219, 112, 147, 255 };
        constexpr static color MEDIUM_VIOLET_RED        = { 199, 21,  133, 255 };
        constexpr static color WHITE                    = { 255, 255, 255, 255 };
        constexpr static color SNOW                     = { 255, 250, 250, 255 };
        constexpr static color HONEYDEW                 = { 240, 255, 240, 255 };
        constexpr static color MINT_CREAM               = { 245, 255, 250, 255 };
        constexpr static color AZURE                    = { 240, 255, 255, 255 };
        constexpr static color ALICE_BLUE               = { 240, 248, 255, 255 };
        constexpr static color GHOST_WHITE              = { 248, 248, 255, 255 };
        constexpr static color WHITE_SMOKE              = { 245, 245, 245, 255 };
        constexpr static color SEASHELL                 = { 255, 245, 238, 255 };
        constexpr static color BEIGE                    = { 245, 245, 220, 255 };
        constexpr static color OLD_LACE                 = { 253, 245, 230, 255 };
        constexpr static color FLORAL_WHITE             = { 255, 250, 240, 255 };
        constexpr static color IVORY                    = { 255, 255, 240, 255 };
        constexpr static color ANTIQUE_WHITE            = { 250, 235, 215, 255 };
        constexpr static color LINEN                    = { 250, 240, 230, 255 };
        constexpr static color LAVENDER_BLUSH           = { 255, 240, 245, 255 };
        constexpr static color MISTY_ROSE               = { 255, 228, 225, 255 };
        constexpr static color GAINSBORO                = { 220, 220, 220, 255 };
        constexpr static color LIGHT_GRAY               = { 211, 211, 211, 255 };
        constexpr static color SILVER                   = { 192, 192, 192, 255 };
        constexpr static color DARK_GRAY                = { 169, 169, 169, 255 };
        constexpr static color GRAY                     = { 128, 128, 128, 255 };
        constexpr static color DIM_GRAY                 = { 105, 105, 105, 255 };
        constexpr static color LIGHT_SLATE_GRAY         = { 119, 136, 153, 255 };
        constexpr static color SLATE_GRAY               = { 112, 128, 144, 255 };
        constexpr static color DARK_SLATE_GRAY          = { 47,  79,  79,  255 };
        constexpr static color BLACK                    = { 0,   0,   0,   255 };
        constexpr static color CORN_SILK                = { 255, 248, 220, 255 };
        constexpr static color BLANCHED_ALMOND          = { 255, 235, 205, 255 };
        constexpr static color BISQUE                   = { 255, 228, 196, 255 };
        constexpr static color NAVAJO_WHITE             = { 255, 222, 173, 255 };
        constexpr static color WHEAT                    = { 245, 222, 179, 255 };
        constexpr static color BURLY_WOOD               = { 222, 184, 135, 255 };
        constexpr static color TAN                      = { 210, 180, 140, 255 };
        constexpr static color ROSY_BROWN               = { 188, 143, 143, 255 };
        constexpr static color SANDY_BROWN              = { 244, 164, 96,  255 };
        constexpr static color GOLDENROD                = { 218, 165, 32,  255 };
        constexpr static color PERU                     = { 205, 133, 63,  255 };
        constexpr static color CHOCOLATE                = { 210, 105, 30,  255 };
        constexpr static color SADDLE_BROWN             = { 139, 69,  19,  255 };
        constexpr static color SIENNA                   = { 160, 82,  45,  255 };
        constexpr static color BROWN                    = { 165, 42,  42,  255 };
        constexpr static color MAROON                   = { 128, 0,   0,   255 };
    };
}