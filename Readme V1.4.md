# TransparentTextRuntime (TTR) v1.4 — ULTRA GODLIKE EDITION

[](https://t.me/+M8d0qE5RuQFhZTI8)
[](https://t.me/+M8d0qE5RuQFhZTI8)

## Overview

TTR v1.4 is a high-performance, compile-time string obfuscation and stealth UI library designed for C++20. It eliminates plaintext strings from your binary and ensures that sensitive data remains in memory only for the duration of a single function call.

By combining a **Substitution-Permutation Network (SPN)** cipher with **Control Flow Flattening (CFF)** and **Mixed Boolean Arithmetic (MBA)**, TTR provides industrial-grade protection against static analysis (IDA/Ghidra) and dynamic memory forensics.

-----

## Core Security Features

### 1\. Multi-Layer Encryption Engine

  * Every string literal is encrypted at compile-time using a unique 64-bit seed generated per call-site.
  * **SPN Cipher**: Uses S-Boxes, bit rotations, and GF(2^8) diffusion (MixColumns-style).
  * **Unique Seeds**: Seeds are derived from `__FILE__`, `__DATE__`, `__TIME__`, `__LINE__`, and `__COUNTER__`, ensuring identical strings have different ciphertexts.
  * **Padding**: High-entropy decoy bytes are added to defeat entropy-based automated scanners.

### 2\. Stealth ImGui Layer

  * **Spoofed Dispatcher**: Replaces all labels with a generic signature (`ANDROID_NATIVE_SECURITY_0xDEADF00D`) in binary cross-references.
  * **Transparent Dispatcher**: Decrypts labels on the stack, passes them to ImGui, and immediately overwrites the buffer with `0xCC` wipe patterns after the draw call.

### 3\. Anti-Analysis Protections

  * **Control Flow Flattening**: The decryption routine is wrapped in a state-machine loop to prevent linear disassembly.
  * **Stack Canaries**: Includes runtime detection of memory tampering during the decryption process.
  * **MBA Primitives**: Replaces standard logic (XOR, ADD) with complex arithmetic identities to confuse decompilers.
  * **Import Hashing**: (Windows) Resolves system APIs via FNV-1a hashes to hide the Import Address Table (IAT).

-----

## Technical Specifications

| Feature | Specification |
| :--- | :--- |
| **Max String Length** | 2048 Characters (Default) |
| **Cipher Rounds** | 6 Rounds (Configurable) |
| **Hash Algorithm** | FNV-1a 64-bit |
| **KDF** | SplitMix64 + Xoshiro256\*\* Mix |
| **Memory Policy** | Zero plaintext in .data / Zero lingering RAM |

-----

## Quick Start

### Basic String Encryption

Use the `TTR()` macro for general usage or `TTR_S()` for stack-only lifetime.

```cpp
// Returns a thread_local const char* (decrypted on demand)
printf(TTR("Access Granted\n"));

// Decrypts on construction, wipes on destruction
{
    auto secure = TTR_S("Sensitive API Key");
    use_key(secure.get());
} 
```

### Stealth UI Integration

Replace standard ImGui calls with TTR equivalents to hide your menu labels from static analysis.

```cpp
if (TTR_TBEGIN("Main Menu", &is_open)) {
    if (TTR_TBUTTON("Enable Cheat", size)) {
        // Business logic here
    }
    TTR_TEND();
}
```

-----

## Architecture Support

  * **X86 / X64**
  * **ARM32 / ARM64**
  * **Platforms**: Windows, Linux, Android, Apple
  * **Compiler Support**: Clang, GCC, MSVC (with optimization suppression support)

-----

**Telegram**: [Join Channel](https://t.me/+M8d0qE5RuQFhZTI8)
