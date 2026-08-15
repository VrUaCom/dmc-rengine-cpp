#include "dmc_rengine/exe/byte_window_receipt.hpp"

#include <cassert>

int main() {
    dmc::rengine::exe::ExeByteWindowReceipt receipt;
    assert(!receipt.valid());
    return 0;
}
