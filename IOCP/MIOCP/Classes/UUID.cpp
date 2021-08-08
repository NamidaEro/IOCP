#include "UUID.h"

#include <random>
#include <sstream>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<int> distrib(0, 15);

std::string CUUID::generate_uuid()
{
    std::stringstream ss;

    ss << std::hex;
    for (int i = 0; i < 8; ++i) {
        ss << distrib(gen);
    }

    ss << '-';
    for (int i = 0; i < 4; ++i) {
        ss << distrib(gen);
    }

    ss << '-';
    for (int i = 0; i < 4; ++i) {
        ss << distrib(gen);
    }

    ss << '-';
    for (int i = 0; i < 12; ++i) {
        ss << distrib(gen);
    }

    return ss.str();
}
