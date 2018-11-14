#pragma once

#include "Shared.hpp"

class Settings {
    public:
        static void set_touch();
        static Handler on_event(int val);
        static void render();
};