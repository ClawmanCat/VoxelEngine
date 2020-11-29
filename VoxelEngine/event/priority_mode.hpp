#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::events {
    enum class priority_mode {
        // Handlers are ordered per event, e.g. given handlers HighP and LowP the execution order is:
        // HighP(Event1), LowP(Event1), HighP(Event2), LowP(Event2), assuming Events 1 and 2 are of the same type.
        PER_EVENT,
        // Handlers are ordered per event class, e.g. given handlers HighP and LowP the execution order is:
        // HighP(Event1), HighP(Event2), LowP(Event1), LowP(Event2), assuming Events 1 and 2 are of the same type.
        PER_EVENT_CLASS
    };
}