#include "../include/Piet.h"

namespace Piet {
    ColorTransition *ColorTransition::determineTransition(Parse::GraphNode *previous, Parse::GraphNode *current) {
        array<array<Color, 6>, 3> colorTable{
                array<Color, 6>{ LightRed, LightYellow,    LightGreen, LightCyan,  LightBlue,  LightMagenta },
                { Red,      Yellow,         Green,      Cyan,       Blue,       Magenta },
                { DarkRed,  DarkYellow,     DarkGreen,  DarkCyan,   DarkBlue,   DarkMagenta }
        };

        Color previousColor = previous->getColor();
        Color currentColor = current->getColor();

        if (currentColor == Black || currentColor == White) {
            return nullptr;
        }

        // Find the position of each color in the table.
        Parse::Position *previousPosition = nullptr;
        Parse::Position *currentPosition = nullptr;

        for (uint8_t row = 0; row < 3; row++) {
            for (uint8_t column = 0; column < 6; column++) {
                Color colorAt = colorTable[row][column];
                if (colorAt == previousColor) {
                    previousPosition = new Parse::Position{row, column};
                }
                if (colorAt == currentColor) {
                    currentPosition = new Parse::Position{row, column};
                }
            }
        }

        assert(previousPosition != nullptr);
        assert(currentPosition != nullptr);

        // Determine the hue change.
        if (currentPosition->column < previousPosition->column) {
            currentPosition->column += 6;
        }
        auto hueChange = (HueChange)(currentPosition->column - previousPosition->column);

        // Determine the lightness change.
        if (currentPosition->row < previousPosition->row) {
            currentPosition->row += 3;
        }
        auto lightnessChange = (LightnessChange)(currentPosition->row - previousPosition->row);

        return new ColorTransition(hueChange, lightnessChange);
    }

    HueChange ColorTransition::getHueChange() {
        return hueChange;
    }

    LightnessChange ColorTransition::getLightnessChange() {
        return lightnessChange;
    }
}
