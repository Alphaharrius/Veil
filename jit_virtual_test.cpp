#include <windows.h>
#include <vector>
#include <string>
#include <stdint.h>

class Base {
public:
    virtual int magic_number() = 0;
};

class Derived : public Base {
public:
    int magic_number() override {
        return 16654321;
    }
};

typedef int (Base::*MFP)();

int main() {
    Base *obj = new Derived();




    // Machine code
    const std::vector<unsigned char> code = {};

    // Preparation
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    auto const page_size = sys_info.dwPageSize;
    auto const code_buffer = VirtualAlloc(nullptr, page_size, MEM_COMMIT, PAGE_READWRITE);
    ::memcpy(code_buffer, code.data(), code.size());
    DWORD _;
    VirtualProtect(code_buffer, code.size(), PAGE_EXECUTE_READWRITE, &_);
    auto const func_ptr = reinterpret_cast<std::int32_t (*)()>(code_buffer);
    auto const result = func_ptr();
    VirtualFree(code_buffer, 0, MEM_RELEASE);

    return 0;
}
