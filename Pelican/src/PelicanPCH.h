#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <functional>
#include <utility>
#include <filesystem>

// useful for operator""s
using namespace std::string_literals;

#include "Pelican/Util/Defines.h"

#ifdef PELICAN_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

// Configuration

// Whether or not to use validation layers.
#if defined(PELICAN_DEBUG)
constexpr bool PELICAN_VALIDATE = true;
#elif defined(PELICAN_RELEASE)
constexpr bool PELICAN_VALIDATE = false;
#else
constexpr bool PELICAN_VALIDATE = false;
#endif
