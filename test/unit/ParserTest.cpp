#define BOOST_TEST_MAIN
#include "../../include/Piet.h"
#include <boost/test/unit_test.hpp>

using namespace Piet;
using namespace Piet::Parse;

void checkGraphNode(GraphNode *node, Color color, uint32_t size, bool isInitial,
                    bool isTerminal) {
  BOOST_CHECK(node->getColor() == color);
  BOOST_CHECK(node->getSize() == size);
  BOOST_CHECK(node->isInitial() == isInitial);
  BOOST_CHECK(node->isTerminal() == isTerminal);
}

BOOST_AUTO_TEST_CASE(test_parse_1_block_image) {
  {
    // Test with a 1-block image of size 1.
    auto image1Block1Pixel = new Image({{Red}}, 1, 1);
    auto parser1Block1Pixel = new Parser(image1Block1Pixel);
    auto graph1Block1Pixel = parser1Block1Pixel->parse();

    // There are no transitions (blocks can't move into themselves), so the
    // step should be null.
    BOOST_CHECK(graph1Block1Pixel->walk() == nullptr);
  }

  {
    // Test with a 1-block image of an arbitrary but larger size.
    auto image1Block50Pixel =
        new Image({{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                   {Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                   {Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                   {Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                   {Red, Red, Red, Red, Red, Red, Red, Red, Red, Red}},
                  5, 10);
    auto parser1Block50Pixel = new Parser(image1Block50Pixel);
    auto graph1Block50Pixel = parser1Block50Pixel->parse();

    // There are no transitions (blocks can't move into themselves), so the
    // step should be null.
    BOOST_CHECK(graph1Block50Pixel->walk() == nullptr);
  }
}

BOOST_AUTO_TEST_CASE(test_parse_2_block_image) {
  {
    // Test with a 2-block image with both codels being of size 1.
    // The blocks are linked horizontally.
    auto image2Block1PixelHorizontal = new Image({{Red, Blue}}, 1, 2);
    auto parser2Block1PixelHorizontal = new Parser(image2Block1PixelHorizontal);
    auto graph2Block1PixelHorizontal = parser2Block1PixelHorizontal->parse();

    // Confirm that there is an infinite loop.
    auto step2Block1PixelHorizontal = graph2Block1PixelHorizontal->walk();
    checkGraphNode(step2Block1PixelHorizontal->previous, Red, 1, true, false);
    checkGraphNode(step2Block1PixelHorizontal->current, Blue, 1, false, false);

    step2Block1PixelHorizontal = graph2Block1PixelHorizontal->walk();
    checkGraphNode(step2Block1PixelHorizontal->previous, Blue, 1, false, false);
    checkGraphNode(step2Block1PixelHorizontal->current, Red, 1, true, false);

    step2Block1PixelHorizontal = graph2Block1PixelHorizontal->walk();
    checkGraphNode(step2Block1PixelHorizontal->previous, Red, 1, true, false);
    checkGraphNode(step2Block1PixelHorizontal->current, Blue, 1, false, false);
  }

  {
    // Test with a 2-block image with both codels being of size 1.
    // The blocks are linked vertically.
    auto image2Block1PixelVertical = new Image({{Red}, {Blue}}, 2, 1);
    auto parser2Block1PixelVertical = new Parser(image2Block1PixelVertical);
    auto graph2Block1PixelVertical = parser2Block1PixelVertical->parse();

    // Confirm that there is an infinite loop.
    auto step2Block1PixelVertical = graph2Block1PixelVertical->walk();
    checkGraphNode(step2Block1PixelVertical->previous, Red, 1, true, false);
    checkGraphNode(step2Block1PixelVertical->current, Blue, 1, false, false);

    step2Block1PixelVertical = graph2Block1PixelVertical->walk();
    checkGraphNode(step2Block1PixelVertical->previous, Blue, 1, false, false);
    checkGraphNode(step2Block1PixelVertical->current, Red, 1, true, false);

    step2Block1PixelVertical = graph2Block1PixelVertical->walk();
    checkGraphNode(step2Block1PixelVertical->previous, Red, 1, true, false);
    checkGraphNode(step2Block1PixelVertical->current, Blue, 1, false, false);
  }
}

BOOST_AUTO_TEST_CASE(test_termination) {
  {
    // Test with a simple image that only terminates.
    auto matrix = std::vector<std::vector<Color>>{
        {Red, Red, Red, Black, White},     {White, White, Red, Black, White},
        {White, White, Red, Black, White}, {White, Black, White, Black, White},
        {Black, Red, Red, Red, Black},     {White, Black, Black, Black, White},
    };
    auto image = new Image(matrix, 6, 5);
    auto parser = new Parser(image);
    auto graph = parser->parse();

    auto step = graph->walk();
    checkGraphNode(step->previous, Red, 5, true, false);
    checkGraphNode(step->current, Red, 3, false, true);

    step = graph->walk();
    BOOST_CHECK(step == nullptr);
  }
}

BOOST_AUTO_TEST_CASE(test_white_transition) {
  {
    // Test with a more complex image that includes a white background
    auto matrix = std::vector<std::vector<Color>>{
        {LightRed, LightRed, LightRed, LightRed, LightRed, LightRed, LightRed,
         LightRed, LightRed, DarkBlue, Black, White, White, White, White, White,
         White},
        {White, White, White, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, White, White, White},
        {White, Black, White, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, White, Black, White},
        {Black, DarkMagenta, Black, White, White, White, White, White, White,
         DarkBlue, Black, White, White, White, Black, DarkMagenta, Black},
        {Black, DarkMagenta, White, DarkMagenta, Red, LightRed, LightRed,
         LightRed, White, LightYellow, White, LightRed, Red, DarkMagenta, White,
         DarkMagenta, Black},
        {Black, DarkMagenta, Black, White, White, White, White, White, White,
         White, White, White, White, White, Black, DarkMagenta, Black},
        {White, Black, White, White, White, White, White, White, White,
         LightRed, White, White, White, White, White, Black, White},
        {White, White, White, White, White, White, White, White, White,
         LightRed, White, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, White, Red,
         White, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, White,
         DarkMagenta, White, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, Black, White,
         Black, White, White, White, White, White, White},
        {White, White, White, White, White, White, White, Black, DarkMagenta,
         DarkMagenta, DarkMagenta, Black, White, White, White, White, White},
        {White, White, White, White, White, White, White, White, Black, Black,
         Black, White, White, White, White, White, White},
    };
    auto image = new Image(matrix, 17, 17);
    auto parser = new Parser(image);
    auto graph = parser->parse();
    GraphStep *step;

    // Assert the path after the switch without changing the direction.
    // 1. Initial light red to dark blue
    step = graph->walk();
    checkGraphNode(step->previous, LightRed, 9, true, false);
    checkGraphNode(step->current, DarkBlue, 8, false, false);
    BOOST_CHECK(!step->skipTransition);
    // 2. Dark blue to light yellow (SWITCH)
    step = graph->walk();
    checkGraphNode(step->previous, DarkBlue, 8, false, false);
    checkGraphNode(step->current, LightYellow, 1, false, false);
    BOOST_CHECK(!step->skipTransition);
    // 3. Light yellow to middle exit point
    step = graph->walk();
    checkGraphNode(step->previous, LightYellow, 1, false, false);
    checkGraphNode(step->current, LightRed, 2, false, false);
    BOOST_CHECK(step->skipTransition);
    // 4. Light red to red step.
    step = graph->walk();
    checkGraphNode(step->previous, LightRed, 2, false, false);
    checkGraphNode(step->current, Red, 1, false, false);
    BOOST_CHECK(!step->skipTransition);
    // 5. Red to last dark magenta before terminal step.
    step = graph->walk();
    checkGraphNode(step->previous, Red, 1, false, false);
    checkGraphNode(step->current, DarkMagenta, 1, false, false);
    BOOST_CHECK(!step->skipTransition);
    // 6. Dark magenta to terminal dark magenta block.
    step = graph->walk();
    checkGraphNode(step->previous, DarkMagenta, 1, false, false);
    checkGraphNode(step->current, DarkMagenta, 3, false, true);
    BOOST_CHECK(step->skipTransition);
  }
}

// Add test for single codel in top-left corner with white codel to right and
// bottom.