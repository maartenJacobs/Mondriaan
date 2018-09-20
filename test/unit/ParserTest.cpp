#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "../../include/Piet.h"

using namespace Piet;
using namespace Piet::Parse;

void checkGraphNode(GraphNode *node, Color color, uint32_t size, bool isInitial, bool isTerminal) {
    BOOST_CHECK( node->getColor() == color );
    BOOST_CHECK( node->getSize() == size );
    BOOST_CHECK( node->isInitial() == isInitial );
    BOOST_CHECK( node->isTerminal() == isTerminal );
}

BOOST_AUTO_TEST_CASE( test_parse_1_block_image )
{
    {
        // Test with a 1-block image of size 1.
        auto image1Block1Pixel = new Image(new Color * [1]{ new Color[1]{ Red } }, 1, 1);
        auto parser1Block1Pixel = new Parser(image1Block1Pixel);
        auto graph1Block1Pixel = parser1Block1Pixel->parse();

        // There are no transitions (blocks can't move into themselves), so the
        // step should be null.
        BOOST_CHECK( graph1Block1Pixel->walk() == nullptr );
    }

    {
        // Test with a 1-block image of an arbitrary but larger size.
        auto image1Block50Pixel = new Image(new Color *[5]{
                new Color[10]{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                new Color[10]{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                new Color[10]{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                new Color[10]{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red},
                new Color[10]{Red, Red, Red, Red, Red, Red, Red, Red, Red, Red}}, 5, 10);
        auto parser1Block50Pixel = new Parser(image1Block50Pixel);
        auto graph1Block50Pixel = parser1Block50Pixel->parse();

        // There are no transitions (blocks can't move into themselves), so the
        // step should be null.
        BOOST_CHECK(graph1Block50Pixel->walk() == nullptr);
    }
}

BOOST_AUTO_TEST_CASE( test_parse_2_block_image )
{
    {
        // Test with a 2-block image with both codels being of size 1.
        // The blocks are linked horizontally.
        auto image2Block1PixelHorizontal = new Image(new Color * [1]{ new Color[2]{ Red, Blue } }, 1, 2);
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
        auto image2Block1PixelVertical = new Image(new Color * [2]{ new Color[1]{ Red }, new Color[1]{ Blue } }, 2, 1);
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

BOOST_AUTO_TEST_CASE( test_termination )
{
    {
        // Test with a simple image that only terminates.
        auto matrix = new Color * [6] {
            new Color[5] { Red,     Red,    Red,    Black,  White },
            new Color[5] { White,   White,  Red,    Black,  White },
            new Color[5] { White,   White,  Red,    Black,  White },
            new Color[5] { White,   Black,  White,  Black,  White },
            new Color[5] { Black,   Red,    Red,    Red,    Black },
            new Color[5] { White,   Black,  Black,  Black,  White },
        };
        auto image = new Image(matrix, 6, 5);
        auto parser = new Parser(image);
        auto graph = parser->parse();

        // Assert that there are 3 steps in the graph:
        //  1. The initial red block
        //  2. The white block between the initial and terminal blocks
        //  3. The terminal red block
        auto step = graph->walk();
        checkGraphNode(step->previous, Red, 5, true, false);
        checkGraphNode(step->current, White, 1, false, false);

        step = graph->walk();
        checkGraphNode(step->previous, White, 1, false, false);
        checkGraphNode(step->current, Red, 3, false, true);

        step = graph->walk();
        BOOST_CHECK( step == nullptr );
    }
}

// TODO: Test natural flow from bounds of image
