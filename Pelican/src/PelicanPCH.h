#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <functional>

// useful for operator""s
using namespace std::string_literals;

#include "Pelican/Util/Defines.h"

// Configuration

// Whether or not to use validation layers.
#if defined(PELICAN_DEBUG)
constexpr bool PELICAN_VALIDATE = true;
#elif defined(PELICAN_RELEASE)
constexpr bool PELICAN_VALIDATE = false;
#else
constexpr bool PELICAN_VALIDATE = false;
#endif
