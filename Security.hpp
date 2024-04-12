//
// Created by Dottik on 6/4/2024.
//
#pragma once

#include <lapi.h>
#include "oxorany/oxorany.h"

typedef int64_t (*Validator)(int64_t testAgainst, struct lua_State *testWith);

namespace RBX::Lua {
    struct ExtraSpace {
        struct Shared {
            int32_t threadCount;
            void *scriptContext;
            void *scriptVmState;
            char field_18[0x8];
            void *__intrusive_set_AllThreads;
        };
        uint8_t __CUSTOM__THREADMARK;
        [[maybe_unused]] char _1[7];
        [[maybe_unused]] char _8[8];
        [[maybe_unused]] char _10[8];
        struct RBX::Lua::ExtraSpace::Shared *sharedExtraSpace;
        [[maybe_unused]] char _20[8];
        Validator *CapabilitiesValidator;
        uint32_t identity;
        [[maybe_unused]] char _38[9];
        [[maybe_unused]] char _40[8];
        uint32_t capabilities;
        [[maybe_unused]] char _50[9];
        [[maybe_unused]] char _58[8];
        [[maybe_unused]] char _60[8];
        [[maybe_unused]] char _68[8];
        [[maybe_unused]] char _70[8];
        [[maybe_unused]] char _78[8];
        [[maybe_unused]] char _80[8];
        [[maybe_unused]] char _88[8];
        [[maybe_unused]] char _90[1];
        [[maybe_unused]] char _91[1];
        [[maybe_unused]] char _92[1];
        uint8_t taskStatus;
        uint8_t executor_thread_mark;
    };
}

namespace RBX {

    enum Identity {
        One_Four = (3),
        Two = (0),
        Five = (1),
        Three_Six = (0xB),
        Eight_Seven = (0x3F),
        Nine = (0xC)
    };

    namespace Security {
        int64_t deobfuscate_identity(RBX::Identity identity);

        int64_t to_obfuscated_identity(int64_t identity);

        // Marks a lua_State as ours for the purposes of Check Caller, etc.
        void MarkThread(lua_State *L);

        namespace Bypasses {
            void set_thread_security(lua_State *L, RBX::Identity identity);

            // Sets capabilities on Lua closures, returns false if the operation fails (i.e: The closure is a C closure).
            bool set_luaclosure_security(Closure *cl, RBX::Identity identity);
        }
    }

}
