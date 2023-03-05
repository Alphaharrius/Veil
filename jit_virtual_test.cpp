#include <windows.h>
#include <vector>
#include <iostream>
#include <cstring>

void test(long long int val) {
    std::cout << "Calling test function:" << val << std::endl;
}

int main() {

    unsigned char code[] = {
            // Load the address of the target into RAX register.
            0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0, // mov rax, imm64
            0x48, 0xB9, 0,0, 0, 0, 0, 0, 0, 0, // mov rcx, imm64
            0x48, 0x83, 0xEC, 0x28, // sub rsp, 40 (Handling of shadow space & alignment)
            0xFF, 0xD0,
            0x48, 0x83, 0xC4, 0x28,
            0xC3 // ret
    };

    *((void **) &code[2]) = (void *) &test;
    *((long long int *) &code[12]) = 12345L;

    // Preparation
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    auto const page_size = sys_info.dwPageSize;
    auto const code_buffer = VirtualAlloc(nullptr, page_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    ::memcpy(code_buffer, code, sizeof(code));

    using func_ptr_t = void (*)();
    auto const func_ptr = reinterpret_cast<func_ptr_t>(code_buffer);
    func_ptr();
    VirtualFree(code_buffer, 0, MEM_RELEASE);

    return 0;
}
