#include "io.h"
#include <string>


/*
 *
 */
std::string ioHandler(const std::string& io, const std::string& command) {
    return "val " +  io + " " + command;
}
