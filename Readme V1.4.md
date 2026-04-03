# TransparentTextRenderer (TTR) v1.4

![TTR Banner](https://img.shields.io/badge/TTR-v1.4-000000?style=for-the-badge&logo=cplusplus&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20Android%20%7C%20macOS-lightgrey?style=for-the-badge)

**Ultra Godlike Edition** — Compile-time SPN string encryption + ImGui stealth layer. Zero plaintext in binary. Zero strings in memory after draw.

---

## Overview

TransparentTextRenderer (TTR) is a high-security C++ header-only library designed to obfuscate string literals at compile-time and securely decrypt them at runtime only when needed. It employs a custom Substitution-Permutation Network (SPN) cipher, mixed boolean arithmetic, and stack canaries to prevent static analysis, memory dumping, and debugger attachment.

### Key Features

*   **Compile-Time Encryption:** All strings are encrypted during compilation using a unique per-callsite seed derived from `__FILE__`, `__LINE__`, `__DATE__`, and `__COUNTER__`.
*   **Zero Plaintext in Binary:** No readable strings exist in the `.rodata` or `.text` sections of the final binary.
*   **Runtime Decryption & Wiping:** Strings are decrypted into a local buffer or thread-local storage, used, and then immediately wiped with a configurable pattern (`0xCC`).
*   **ImGui Stealth Layer:** Specialized macros for ImGui that hide UI labels from static analysis tools like IDA Pro or Ghidra by routing calls through a opaque dispatcher.
*   **Anti-Debug/Anti-Tamper:** Optional integration with `ptrace` (Linux/Android) and `IsDebuggerPresent` (Windows) to detect and trap debuggers.
*   **Cross-Platform:** Supports Windows, Linux, Android, and macOS (Apple).
*   **Architecture Aware:** Optimized for x86, x64, ARM32, and ARM64.

---

## Installation

TTR is a header-only library. Simply include `TTR-Obfuscate V1.4.h` in your project.

```cpp
#include "TTR-Obfuscate V1.4.h"
```

Ensure you have C++17 or higher enabled in your compiler settings.

---

## Usage

### Basic String Obfuscation

Use the `TTR()` macro to obfuscate standard C-strings. The macro returns a `const char*` that is valid until the next call to `TTR()` on the same thread.

```cpp
#include <iostream>
#include "TTR-Obfuscate V1.4.h"

int main() {
    // The string "Hello, Secure World!" is encrypted at compile time.
    std::cout << TTR("Hello, Secure World!") << std::endl;
    
    // Wide character support
    std::wcout << TTR_W(L"Wide string support") << std::endl;

    return 0;
}
```

### Stack-Based Strings (High Security)

For maximum security, use `TTR_S()` which creates a `stack_str` object. The string is decrypted on construction and wiped on destruction. This ensures the plaintext never resides in static memory.

```cpp
void secure_function() {
    // Decrypts to stack, wipes automatically when scope ends
    auto secret = TTR_S("Top Secret Data");
    
    // Use secret.get() to access the const char*
    printf("%s\n", secret.get());
} // Memory wiped here
```

### ImGui Integration

TTR provides two sets of macros for ImGui: **Spoofed** and **Transparent**.

#### Spoofed Macros (`TTR_BUTTON`, `TTR_TEXT`, etc.)

These macros route all ImGui calls through a single dispatcher function. In static analysis tools, all string references will point to a fake signature (`ANDROID_NATIVE_SECURITY_0xDEADF00D`) instead of your actual UI labels.

```cpp
// In your ImGui render loop
if (TTR_BUTTON("Click Me")) {
    // Handle click
}

TTR_TEXT("This label is hidden from static analysis");
```

#### Transparent Macros (`TTR_TBUTTON`, `TTR_TTEXT`, etc.)

These macros decrypt the string locally, call the ImGui function, and then wipe the memory. They do not use the central dispatcher, offering a balance between security and performance.

```cpp
if (TTR_TBUTTON("Secure Button")) {
    // Handle click
}

TTR_TTEXT("This label is decrypted on stack and wiped immediately");
```

---

## Configuration

You can customize TTR behavior by defining macros before including the header.

| Macro | Default | Description |
| :--- | :--- | :--- |
| `TTR_CFG_MAX` | `2048` | Maximum characters per string. |
| `TTR_CFG_ROUNDS` | `6` | Number of SPN cipher rounds (4-16 recommended). |
| `TTR_CFG_CANARY` | `0xDEADF00DCAFEBABEULL` | Stack guard canary value. |
| `TTR_CFG_WIPE_BYTE` | `0xCC` | Byte pattern used to wipe memory. |
| `TTR_ENABLE_ANTIDEBUG` | *Undefined* | Define to enable anti-debug checks. |
| `TTR_DEBUG` | *Undefined* | Define to disable obfuscation for debugging. |

### Example: Custom Configuration

```cpp
#define TTR_CFG_ROUNDS 8
#define TTR_ENABLE_ANTIDEBUG
#include "TTR-Obfuscate V1.4.h"
```

---

## How It Works

1.  **Key Derivation:** A unique 64-bit seed is generated for each macro invocation using `fnv1a` hashing of file path, line number, date, time, and a counter.
2.  **Compile-Time Encryption:** The string is encrypted using a custom SPN cipher with key scheduling derived from the seed. The encrypted data is stored in the binary.
3.  **Runtime Decryption:** When the macro is called, the encrypted data is decrypted using the same key schedule.
4.  **Memory Wiping:** After use, the plaintext buffer is overwritten with `TTR_CFG_WIPE_BYTE` to prevent memory dumps from revealing the string.

---

## Anti-Analysis Techniques

*   **Mixed Boolean Arithmetic (MBA):** XOR and ADD operations are replaced with complex boolean expressions to confuse decompilers.
*   **Control Flow Flattening:** The decryption routine uses a state machine to flatten control flow.
*   **Stack Canaries:** Random canaries are placed on the stack to detect buffer overflows or tampering.
*   **Fake Signatures:** ImGui calls are routed through a dispatcher with a fake signature to mislead reverse engineers.

---

## License

MIT License

Copyright (c) 2026 YourHandle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## Author

[github.com/yourhandle](https://github.com/yourhandle)

For detailed documentation, see `README.md` in the repository.