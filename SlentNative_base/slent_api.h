#pragma once

#ifdef _WIN32
#define SLENT_API __declspec(dllexport)
#else
#define SLENT_API
#endif