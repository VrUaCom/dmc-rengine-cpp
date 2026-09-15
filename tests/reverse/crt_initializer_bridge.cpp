// Standalone bridge for the EXE-vs-C++ differential verifier. Not a game ABI.
#include "dmc_rengine/reverse/crt_global_initializers.hpp"
#include <cstring>

namespace {
using namespace dmc::rengine::reverse;
int calls;
int status;
ExitCallback expected;
bool correct_callback;
int record_registration(ExitCallback callback) {
    ++calls;
    correct_callback = callback == expected;
    callback();
    return status;
}
}

extern "C" int initialize_first(unsigned char* bytes, int result) {
    std::array<dmc::rengine::reverse::CrtRecord16, 768> records;
    std::memcpy(records.data(), bytes + 64, sizeof(records));
    calls = 0; status = result; expected = dmc::rengine::reverse::cleanup_cb9ed0;
    correct_callback = false;
    dmc::rengine::reverse::initialize_cb9ed0(records, record_registration);
    std::memcpy(bytes + 64, records.data(), sizeof(records));
    return calls == 1 && correct_callback;
}

extern "C" int initialize_second(unsigned char* bytes, int result) {
    std::array<std::uint32_t, 16> floats;
    std::array<dmc::rengine::reverse::CrtRecord8, 16> records;
    std::memcpy(floats.data(), bytes + 64, sizeof(floats));
    std::memcpy(records.data(), bytes + 64 + 0x80, sizeof(records));
    std::uint8_t flag = bytes[64 + 0x100];
    calls = 0; status = result; expected = dmc::rengine::reverse::cleanup_cf2d90;
    correct_callback = false;
    dmc::rengine::reverse::initialize_cf2d90(floats, records, flag, record_registration);
    std::memcpy(bytes + 64, floats.data(), sizeof(floats));
    std::memcpy(bytes + 64 + 0x80, records.data(), sizeof(records));
    bytes[64 + 0x100] = flag;
    return calls == 1 && correct_callback;
}
