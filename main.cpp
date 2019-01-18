#include <iostream>
#include <cstdio>
#include <cassert>
#include <vector>
#include <stack>
#include <png.h>
#include "include/Piet.h"
#include "include/cxxopts/cxxopts.hpp"

using namespace std;

void print_help(const cxxopts::Options& options) {
    cout << options.help() << endl;
}

bool validate_options(const cxxopts::ParseResult& result) {
    bool valid = true;

    // TODO: add more validation.

    switch (result["output-file"].count()) {
        case 0:
            cout << "Please specify an output file." << endl;
            valid = false;
            break;
        case 1:
            // OK!
            break;
        default:
            break;
    }

    return valid;
}

void compile(std::string inputFile, std::string outputFile, bool outputIR) {
    Piet::Parse::Reader reader;

    auto image = reader.readFromFile(std::move(inputFile));
    auto parser = new Piet::Parse::Parser(image);
    auto graph = parser->parse();
    auto translator = new Piet::Translator(graph);
    translator->translateToExecutable(std::move(outputFile), outputIR);
}

int main(int argc, char **argv) {
    try {
        cxxopts::Options options("Mondriaan", "The unfancy Piet compiler");
        options.add_options()
                ("h,help", "Display available options.")
                ("S,emit-llvm", "Emit optimised LLVM IR code only. Do not compile IR code.")
                ("o,output-file", "Specify output file.", cxxopts::value<std::string>())
                ("input-file", "The Piet file to compile.", cxxopts::value<std::vector<std::string>>());
        options.parse_positional({"input-file"});
        options.positional_help("input-file");
        auto result = options.parse(argc, argv);

        if (result["help"].as<bool>()) {
            print_help(options);
            return 0;
        }

        if (!validate_options(result)) {
            return 1;
        }

        bool outputIR = result["emit-llvm"].count() > 0;
        auto outputFile = result["output-file"].as<std::string>();
        auto inputFile = result["input-file"].as<std::vector<std::string>>()[0];

        compile(inputFile, outputFile, outputIR);
    } catch (cxxopts::OptionParseException& parseExc) {
        cout << parseExc.what() << endl;
        return 1;
    } catch (cxxopts::OptionSpecException& specExc) {
        cout << "Looks like Mondriaan has a bug! Please send the error message to the authors." << endl;
        cout << "Error message: " << specExc.what() << endl;
        return 1;
    }

    return 0;
}
