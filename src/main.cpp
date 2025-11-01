#include <string>
#include "lib/UI.h"

const std::string appTitle = "Main Window";
const std::string fontName = "fonts/OpenSans-Regular.ttf";
constexpr int windowWidth = 1600;
constexpr int windowHeight = 900;

int main() {
    constructUI(appTitle, fontName,windowWidth, windowHeight, "test");

    return 0;
}
