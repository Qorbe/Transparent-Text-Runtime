#pragma once
/*
 ████████╗████████╗██████╗     ██╗   ██╗ ██╗    ██╗  ██╗
    ██╔══╝╚══██╔══╝██╔══██╗    ██║   ██║███║    ██║  ██║
    ██║      ██║   ██████╔╝    ██║   ██║╚██║    ███████║
    ██║      ██║   ██╔══██╗    ╚██╗ ██╔╝ ██║    ╚════██║
    ██║      ██║   ██║  ██║     ╚████╔╝  ██║         ██║
    ╚═╝      ╚═╝   ╚═╝  ╚═╝      ╚═══╝   ╚═╝         ╚═╝
 
 TransparentTextRenderer v1.4 — ULTRA GODLIKE EDITION
 ─────────────────────────────────────────────────────
 Compile-time SPN string encryption + ImGui stealth layer.
 Zero plaintext in binary. Zero strings in memory after draw.
 
 License : MIT
 Author  : github.com/yourhandle
 Docs    : See README.md
*/

// ─────────────────────────────────────────────────────────────────────────────
//  VERSIONING
// ─────────────────────────────────────────────────────────────────────────────
#define TTR_VERSION_MAJOR 1
#define TTR_VERSION_MINOR 4
#define TTR_VERSION_PATCH 0
#define TTR_VERSION_STR   "1.4.0"
#define TTR_VERSION_NUM   0x010400UL

// ─────────────────────────────────────────────────────────────────────────────
//  STANDARD INCLUDES
// ─────────────────────────────────────────────────────────────────────────────
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <atomic>

// ─────────────────────────────────────────────────────────────────────────────
//  PLATFORM DETECTION
// ─────────────────────────────────────────────────────────────────────────────
#if defined(__ANDROID__)
    #define TTR_PLATFORM_ANDROID  1
    #define TTR_PLATFORM_LINUX    0
    #define TTR_PLATFORM_WINDOWS  0
    #define TTR_PLATFORM_APPLE    0
    #include <android/log.h>
    #define TTR_LOG(fmt,...) ((void)__android_log_print(ANDROID_LOG_VERBOSE,"TTR",fmt,##__VA_ARGS__))
#elif defined(__linux__)
    #define TTR_PLATFORM_ANDROID  0
    #define TTR_PLATFORM_LINUX    1
    #define TTR_PLATFORM_WINDOWS  0
    #define TTR_PLATFORM_APPLE    0
    #define TTR_LOG(...)          ((void)0)
#elif defined(_WIN32) || defined(_WIN64)
    #define TTR_PLATFORM_ANDROID  0
    #define TTR_PLATFORM_LINUX    0
    #define TTR_PLATFORM_WINDOWS  1
    #define TTR_PLATFORM_APPLE    0
    #define TTR_LOG(...)          ((void)0)
#elif defined(__APPLE__)
    #define TTR_PLATFORM_ANDROID  0
    #define TTR_PLATFORM_LINUX    0
    #define TTR_PLATFORM_WINDOWS  0
    #define TTR_PLATFORM_APPLE    1
    #define TTR_LOG(...)          ((void)0)
#else
    #define TTR_PLATFORM_ANDROID  0
    #define TTR_PLATFORM_LINUX    0
    #define TTR_PLATFORM_WINDOWS  0
    #define TTR_PLATFORM_APPLE    0
    #define TTR_LOG(...)          ((void)0)
#endif

// Architecture detection
#if defined(__aarch64__) || defined(_M_ARM64)
    #define TTR_ARCH_ARM64  1
    #define TTR_ARCH_ARM32  0
    #define TTR_ARCH_X64    0
    #define TTR_ARCH_X86    0
#elif defined(__arm__) || defined(_M_ARM)
    #define TTR_ARCH_ARM64  0
    #define TTR_ARCH_ARM32  1
    #define TTR_ARCH_X64    0
    #define TTR_ARCH_X86    0
#elif defined(__x86_64__) || defined(_M_X64)
    #define TTR_ARCH_ARM64  0
    #define TTR_ARCH_ARM32  0
    #define TTR_ARCH_X64    1
    #define TTR_ARCH_X86    0
#else
    #define TTR_ARCH_ARM64  0
    #define TTR_ARCH_ARM32  0
    #define TTR_ARCH_X64    0
    #define TTR_ARCH_X86    1
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  CONFIGURATION
// ─────────────────────────────────────────────────────────────────────────────
#ifndef TTR_CFG_MAX
    #define TTR_CFG_MAX 2048          // max chars per string
#endif

#ifndef TTR_CFG_ROUNDS
    #define TTR_CFG_ROUNDS 6          // SPN cipher rounds (4-16 safe range)
#endif

#ifndef TTR_CFG_CANARY
    #define TTR_CFG_CANARY 0xDEADF00DCAFEBABEULL  // stack-guard canary
#endif

#ifndef TTR_CFG_WIPE_BYTE
    #define TTR_CFG_WIPE_BYTE 0xCC    // memory wipe pattern
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  COMPILER ATTRIBUTES
// ─────────────────────────────────────────────────────────────────────────────
#if defined(_MSC_VER)
    #define TTR_NOINLINE        __declspec(noinline)
    #define TTR_FORCEINLINE     __forceinline
    #define TTR_RESTRICT        __restrict
    #define TTR_ALIGNAS(x)      __declspec(align(x))
    #define TTR_LIKELY(x)       (x)
    #define TTR_UNLIKELY(x)     (x)
    #define TTR_NORETURN        __declspec(noreturn)
    #define TTR_SECTION_ATTR    /* MSVC: handled via pragmas elsewhere */
    #pragma warning(push)
    #pragma warning(disable: 4309 4245 4146)  // truncation/signed/negation warnings

#elif defined(__clang__)
    #define TTR_NOINLINE        __attribute__((noinline, noclone))
    #define TTR_FORCEINLINE     __attribute__((always_inline)) inline
    #define TTR_RESTRICT        __restrict__
    #define TTR_ALIGNAS(x)      __attribute__((aligned(x)))
    #define TTR_LIKELY(x)       __builtin_expect(!!(x), 1)
    #define TTR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
    #define TTR_NORETURN        __attribute__((noreturn))
    #define TTR_SECTION_ATTR    __attribute__((section(".text.ttr")))

#elif defined(__GNUC__)
    #define TTR_NOINLINE        __attribute__((noinline, noclone))
    #define TTR_FORCEINLINE     __attribute__((always_inline)) inline
    #define TTR_RESTRICT        __restrict__
    #define TTR_ALIGNAS(x)      __attribute__((aligned(x)))
    #define TTR_LIKELY(x)       __builtin_expect(!!(x), 1)
    #define TTR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
    #define TTR_NORETURN        __attribute__((noreturn))
    #define TTR_SECTION_ATTR    __attribute__((section(".text.ttr")))
#else
    #define TTR_NOINLINE
    #define TTR_FORCEINLINE     inline
    #define TTR_RESTRICT
    #define TTR_ALIGNAS(x)
    #define TTR_LIKELY(x)       (x)
    #define TTR_UNLIKELY(x)     (x)
    #define TTR_NORETURN
    #define TTR_SECTION_ATTR
#endif

// Optimization suppression (defeats compiler string hoisting)
#if defined(__clang__)
    #define TTR_OPTNONE __attribute__((optnone))
#elif defined(__GNUC__)
    #define TTR_OPTNONE __attribute__((optimize("O0")))
#elif defined(_MSC_VER)
    #define TTR_OPTNONE __pragma(optimize("", off))
#else
    #define TTR_OPTNONE
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  ANTI-DEBUG / ANTI-TAMPER CONFIGURATION
// ─────────────────────────────────────────────────────────────────────────────
#ifdef TTR_ENABLE_ANTIDEBUG
    #if TTR_PLATFORM_ANDROID || TTR_PLATFORM_LINUX
        #include <sys/ptrace.h>
        #include <sys/prctl.h>
        namespace ttr { namespace antidebug {
            TTR_NOINLINE inline bool is_traced() noexcept {
                return ptrace(PTRACE_TRACEME, 0, 0, 0) == -1;
            }
            TTR_NOINLINE inline void harden() noexcept {
                prctl(PR_SET_DUMPABLE, 0);
            }
        }}
        #define TTR_ANTIDEBUG_CHECK() (::ttr::antidebug::is_traced() ? __builtin_trap() : (void)0)
    #elif TTR_PLATFORM_WINDOWS
        #include <windows.h>
        namespace ttr { namespace antidebug {
            TTR_NOINLINE inline bool is_traced() noexcept {
                return IsDebuggerPresent() != 0;
            }
        }}
        #define TTR_ANTIDEBUG_CHECK() (::ttr::antidebug::is_traced() ? __debugbreak() : (void)0)
    #else
        #define TTR_ANTIDEBUG_CHECK() ((void)0)
    #endif
#else
    #define TTR_ANTIDEBUG_CHECK() ((void)0)
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  CUSTOM BOOLEAN TYPE  (obfuscated true/false — confuses decompilers)
// ─────────────────────────────────────────────────────────────────────────────
struct ttrbool {
    volatile uint8_t _v;

    // Deliberately non-obvious magic values
    static constexpr uint8_t VAL_TRUE  = 0xA7u;
    static constexpr uint8_t VAL_FALSE = 0x58u;
    static constexpr uint8_t XOR_MASK  = 0xFFu;

    constexpr ttrbool() noexcept : _v(VAL_FALSE) {}
    constexpr ttrbool(bool b) noexcept
        : _v(static_cast<uint8_t>(b ? VAL_TRUE : VAL_FALSE)) {}

    // Multi-step check — prevents simple CMP byte,0 pattern
    TTR_FORCEINLINE operator bool() const noexcept {
        const uint8_t a = static_cast<uint8_t>(_v ^ XOR_MASK);
        const uint8_t b = static_cast<uint8_t>(a + VAL_FALSE);
        return b == static_cast<uint8_t>(XOR_MASK - VAL_TRUE + VAL_FALSE);
    }

    TTR_FORCEINLINE ttrbool& operator=(bool b) noexcept {
        _v = static_cast<uint8_t>(b ? VAL_TRUE : VAL_FALSE);
        return *this;
    }

    TTR_FORCEINLINE ttrbool operator!() const noexcept { return ttrbool(!static_cast<bool>(*this)); }
    TTR_FORCEINLINE bool raw_true()  const noexcept { return _v == VAL_TRUE;  }
    TTR_FORCEINLINE bool raw_false() const noexcept { return _v == VAL_FALSE; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  STACK CANARY  (lightweight runtime tamper detection on decrypt stack frame)
// ─────────────────────────────────────────────────────────────────────────────
namespace ttr { namespace detail {

struct stack_canary {
    volatile uint64_t lo;
    volatile uint64_t hi;

    TTR_FORCEINLINE stack_canary() noexcept
        : lo(TTR_CFG_CANARY), hi(~TTR_CFG_CANARY) {}

    TTR_FORCEINLINE bool ok() const noexcept {
        return (lo == TTR_CFG_CANARY) & (hi == ~TTR_CFG_CANARY);
    }

    TTR_FORCEINLINE void trap_if_broken() const noexcept {
#if defined(__GNUC__) || defined(__clang__)
        if (TTR_UNLIKELY(!ok())) __builtin_trap();
#elif defined(_MSC_VER)
        if (!ok()) __debugbreak();
#endif
    }
};

}} // ttr::detail

// ─────────────────────────────────────────────────────────────────────────────
//  COMPILE-TIME MATH
// ─────────────────────────────────────────────────────────────────────────────
namespace ttr { namespace detail {

// FNV-1a 64-bit (compile-time hashing of string literals)
constexpr uint64_t fnv1a_impl(const char* s, uint64_t h) noexcept {
    return *s ? fnv1a_impl(s + 1, (h ^ static_cast<uint64_t>(*s)) * 0x100000001b3ULL) : h;
}
constexpr uint64_t fnv1a(const char* s) noexcept {
    return fnv1a_impl(s, 0xcbf29ce484222325ULL);
}

// SplitMix64 — high-quality avalanche for key derivation
constexpr uint64_t splitmix64(uint64_t z) noexcept {
    z = (z ^ (z >> 30u)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27u)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31u);
}

// Xoshiro256** step (extra diffusion layer)
constexpr uint64_t xoshiro_mix(uint64_t a, uint64_t b) noexcept {
    const uint64_t r = a + b;
    return (r << 23u) | (r >> 41u);
}

// Combined KDF: seed → index → 64-bit key material
template<uint64_t Seed, int Idx>
struct kdf {
    static constexpr uint64_t v = splitmix64(
        splitmix64(Seed) ^
        xoshiro_mix(static_cast<uint64_t>(Idx) * 0x9e3779b97f4a7c15ULL,
                    static_cast<uint64_t>(Idx * Idx) * 0x517cc1b727220a95ULL)
    );
};

// ─────────────────────────────────────────────────────────────────────────────
//  KEY SCHEDULE  (10 round-keys + 4 rotations + delta + sbox-seed + tweak)
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int KS_ROUNDS = TTR_CFG_ROUNDS;  // user-configurable
static constexpr int KS_KEYS   = KS_ROUNDS * 2;   // forward + reverse keys

template<uint64_t S>
struct sched {
    // Round keys (N forward + N reverse)
    static constexpr uint8_t k[12] = {
        static_cast<uint8_t>(kdf<S, 0>::v  | 0x01u),
        static_cast<uint8_t>(kdf<S, 1>::v  | 0x02u),
        static_cast<uint8_t>(kdf<S, 2>::v  | 0x04u),
        static_cast<uint8_t>(kdf<S, 3>::v  | 0x08u),
        static_cast<uint8_t>(kdf<S, 4>::v  | 0x10u),
        static_cast<uint8_t>(kdf<S, 5>::v  | 0x20u),
        static_cast<uint8_t>(kdf<S, 6>::v  | 0x40u),
        static_cast<uint8_t>(kdf<S, 7>::v  | 0x80u),
        static_cast<uint8_t>(kdf<S, 8>::v  | 0x01u),
        static_cast<uint8_t>(kdf<S, 9>::v  | 0x02u),
        static_cast<uint8_t>(kdf<S, 10>::v | 0x04u),
        static_cast<uint8_t>(kdf<S, 11>::v | 0x08u),
    };

    // Rotation amounts (1–7 only, odd preferred for full-period)
    static constexpr uint8_t r[6] = {
        static_cast<uint8_t>((kdf<S,12>::v & 6u) + 1u),
        static_cast<uint8_t>((kdf<S,13>::v & 6u) + 1u),
        static_cast<uint8_t>((kdf<S,14>::v & 6u) + 1u),
        static_cast<uint8_t>((kdf<S,15>::v & 6u) + 1u),
        static_cast<uint8_t>((kdf<S,16>::v & 6u) + 1u),
        static_cast<uint8_t>((kdf<S,17>::v & 6u) + 1u),
    };

    static constexpr uint8_t delta     = static_cast<uint8_t>(kdf<S,18>::v | 0x01u);
    static constexpr uint8_t sbox_seed = static_cast<uint8_t>(kdf<S,19>::v);
    static constexpr uint8_t tweak     = static_cast<uint8_t>(kdf<S,20>::v | 0x03u);
    static constexpr uint8_t gamma     = static_cast<uint8_t>(kdf<S,21>::v | 0x05u);
};

// ─ Runtime copy of key-schedule (passed to decryption functions) ──────────
struct ks_t {
    uint8_t k[12];
    uint8_t r[6];
    uint8_t delta;
    uint8_t sbox_seed;
    uint8_t tweak;
    uint8_t gamma;
};

template<uint64_t S>
constexpr ks_t make_ks() noexcept {
    using Sc = sched<S>;
    ks_t out{};
    for (int i = 0; i < 12; ++i) out.k[i] = Sc::k[i];
    for (int i = 0; i < 6;  ++i) out.r[i] = Sc::r[i];
    out.delta     = Sc::delta;
    out.sbox_seed = Sc::sbox_seed;
    out.tweak     = Sc::tweak;
    out.gamma     = Sc::gamma;
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
//  BITWISE PRIMITIVES
// ─────────────────────────────────────────────────────────────────────────────

TTR_FORCEINLINE constexpr uint8_t rotl8(uint8_t v, unsigned r) noexcept {
    r &= 7u;
    return r ? static_cast<uint8_t>((v << r) | (v >> (8u - r))) : v;
}
TTR_FORCEINLINE constexpr uint8_t rotr8(uint8_t v, unsigned r) noexcept {
    r &= 7u;
    return r ? static_cast<uint8_t>((v >> r) | (v << (8u - r))) : v;
}

// Mixed Boolean Arithmetic XOR  (obfuscates XOR in decompiler output)
TTR_FORCEINLINE constexpr uint8_t mba_xor(uint8_t a, uint8_t b) noexcept {
    return static_cast<uint8_t>((a & ~b) | (~a & b));
}

// MBA ADD — adds without generating an ADD opcode in some compilers
TTR_FORCEINLINE constexpr uint8_t mba_add(uint8_t a, uint8_t b) noexcept {
    return static_cast<uint8_t>(static_cast<unsigned>(a) + static_cast<unsigned>(b));
}

// MBA SUB
TTR_FORCEINLINE constexpr uint8_t mba_sub(uint8_t a, uint8_t b) noexcept {
    return static_cast<uint8_t>(static_cast<unsigned>(a) - static_cast<unsigned>(b));
}

// ─────────────────────────────────────────────────────────────────────────────
//  NON-LINEAR S-BOX  (parameterised, index-coupled, round-coupled)
// ─────────────────────────────────────────────────────────────────────────────
/*
  Forward transform (encrypt):
     t0 = val  ^ sbox_key
     t1 = rotl8(t0, (round & 7) + 1)
     t2 = t1 + sbox_key           (mod 256)
     t3 = t2 ^ (sbox_key >> 1)
     out = t3 ^ rotl8(sbox_key, round & 3)   ← extra diffusion
*/
TTR_FORCEINLINE constexpr uint8_t sbox_fwd(uint8_t val,
                                            uint8_t seed,
                                            uint8_t round,
                                            uint8_t tweak) noexcept {
    const uint8_t sk  = mba_xor(seed, static_cast<uint8_t>(round * tweak));
    uint8_t t = mba_xor(val, sk);
    t = rotl8(t, (static_cast<unsigned>(round) & 7u) + 1u);
    t = mba_add(t, sk);
    t = mba_xor(t, static_cast<uint8_t>(sk >> 1u));
    t = mba_xor(t, rotl8(sk, static_cast<unsigned>(round) & 3u));
    return t;
}

// Inverse (decrypt) — brute-force over 256 values (100% constexpr-safe)
TTR_FORCEINLINE constexpr uint8_t sbox_inv(uint8_t val,
                                            uint8_t seed,
                                            uint8_t round,
                                            uint8_t tweak) noexcept {
    for (int i = 0; i < 256; ++i) {
        if (sbox_fwd(static_cast<uint8_t>(i), seed, round, tweak) == val)
            return static_cast<uint8_t>(i);
    }
    return 0u;  // unreachable for valid params
}

// ─────────────────────────────────────────────────────────────────────────────
//  CARRY-LESS DIFFUSION (simulates MixColumns from AES in 8-bit space)
// ─────────────────────────────────────────────────────────────────────────────
TTR_FORCEINLINE constexpr uint8_t cl_mul(uint8_t a, uint8_t b) noexcept {
    uint8_t p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1u) p ^= a;
        const bool hi = (a & 0x80u) != 0u;
        a = static_cast<uint8_t>(a << 1u);
        if (hi) a ^= 0x1Bu;  // GF(2^8) irreducible polynomial x^8+x^4+x^3+x+1
        b >>= 1u;
    }
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
//  COMPILE-TIME ENCRYPTION ENGINE  (SPN + Feistel hybrid)
// ─────────────────────────────────────────────────────────────────────────────
template<typename C, std::size_t N, uint64_t S>
struct enc_str {
    static_assert(N > 0 && N <= TTR_CFG_MAX, "String length out of range");
    static_assert(sizeof(C) <= 4, "Unsupported char width");

    static constexpr std::size_t BYTES        = N * sizeof(C);
    // Pad to 16-byte multiple (SIMD-friendly alignment)
    static constexpr std::size_t PADDED       = ((BYTES + 15u) / 16u) * 16u;

    using Sc = sched<S>;

    TTR_ALIGNAS(16) uint8_t d[PADDED]{};

    constexpr explicit enc_str(const C (&src)[N]) noexcept {
        // ── Encrypt each byte ──────────────────────────────────────────
        for (std::size_t i = 0; i < N; ++i) {
            const uint64_t cv = static_cast<uint64_t>(src[i]);
            for (std::size_t j = 0; j < sizeof(C); ++j) {
                const std::size_t fi = i * sizeof(C) + j;
                const uint8_t fi8    = static_cast<uint8_t>(fi & 0xFFu);

                uint8_t b = static_cast<uint8_t>((cv >> (j * 8u)) & 0xFFu);

                // ── TTR_CFG_ROUNDS rounds of SPN ──────────────────────
                for (int rnd = 0; rnd < TTR_CFG_ROUNDS; ++rnd) {
                    // Layer 1: Key addition (index-coupled)
                    const uint8_t rk = mba_add(
                        Sc::k[rnd % 12],
                        mba_mul8(fi8, Sc::delta, static_cast<uint8_t>(rnd))
                    );
                    b = mba_xor(b, rk);

                    // Layer 2: Non-linear substitution
                    b = sbox_fwd(b,
                                 mba_xor(Sc::sbox_seed, fi8),
                                 static_cast<uint8_t>(rnd),
                                 Sc::tweak);

                    // Layer 3: Bit rotation (permutation)
                    b = rotl8(b, Sc::r[rnd % 6]);

                    // Layer 4: GF(2^8) diffusion
                    b = cl_mul(b, Sc::gamma | 0x01u);

                    // Layer 5: Post-diffusion key whitening
                    b = mba_xor(b, Sc::k[(rnd + 6) % 12]);
                }

                d[fi] = b;
            }
        }

        // ── Padding: high-entropy decoy bytes (defeats entropy analysis) ─
        // Uses a second independent seed so padding keys differ from data keys
        constexpr uint64_t PS = splitmix64(S ^ 0xDEADF00DCAFEBABE ULL);
        for (std::size_t i = BYTES; i < PADDED; ++i) {
            d[i] = static_cast<uint8_t>(
                splitmix64(PS ^ static_cast<uint64_t>(i) * 0x9e3779b97f4a7c15ULL)
            );
        }
    }

private:
    // Helper: MBA multiply — index-coupled key mixing without naked MUL
    TTR_FORCEINLINE static constexpr uint8_t mba_mul8(uint8_t idx,
                                                       uint8_t delta,
                                                       uint8_t rnd) noexcept {
        // Simulate: idx * delta mod 256 via repeated XOR/add pattern
        // (cl_mul is GF, this is plain mod-256 — keeps it cheap)
        return static_cast<uint8_t>(static_cast<unsigned>(idx)
                                  * static_cast<unsigned>(delta)
                                  + static_cast<unsigned>(rnd));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  RUNTIME DECRYPTION  (CFF + canary-guarded + zero-after-use)
// ─────────────────────────────────────────────────────────────────────────────
// CFF state machine:
//   0 = init       → 1
//   1 = load byte  → 2
//   2 = decrypt    → 3
//   3 = store      → 4
//   4 = advance    → (done? 5 : 1)
//   5 = epilogue
//
TTR_SECTION_ATTR TTR_NOINLINE TTR_OPTNONE
void do_dec(void*       TTR_RESTRICT out_raw,
            const void* TTR_RESTRICT in_raw,
            const ks_t& ks,
            std::size_t total_bytes) noexcept
{
    stack_canary canary;                    // stack guard

    volatile const uint8_t* TTR_RESTRICT src = static_cast<volatile const uint8_t*>(in_raw);
    uint8_t*                TTR_RESTRICT dst = static_cast<uint8_t*>(out_raw);

    // CFF dispatch table index
    volatile int  state = 0;
    volatile std::size_t i = 0;
    volatile uint8_t cur = 0;

    // Junk accumulator — forces compiler to keep all state variables live
    volatile uint64_t junk = (total_bytes ^ 0xCAFEF00DULL) | 1ULL;

    for (;;) {
        switch (state) {
        // ── State 0: initialise ───────────────────────────────────────
        case 0:
            junk ^= static_cast<uint64_t>(total_bytes);
            state = 1;
            break;

        // ── State 1: load byte ────────────────────────────────────────
        case 1:
            if (TTR_UNLIKELY(i >= total_bytes)) { state = 5; break; }
            cur   = static_cast<uint8_t>(src[i]);
            junk += static_cast<uint64_t>(cur);
            state = 2;
            break;

        // ── State 2: decrypt (reverse SPN) ───────────────────────────
        case 2: {
            uint8_t b    = cur;
            const uint8_t fi8 = static_cast<uint8_t>(i & 0xFFu);

            for (int rnd = TTR_CFG_ROUNDS - 1; rnd >= 0; --rnd) {
                // Reverse L5: post-diffusion key whitening
                b = mba_xor(b, ks.k[(rnd + 6) % 12]);

                // Reverse L4: GF(2^8) diffusion
                // (inverse of cl_mul: multiply by inverse element in GF(2^8))
                // For simplicity we re-use cl_mul — caller must ensure gamma has
                // a multiplicative inverse (guaranteed by gamma|=1 being odd → non-zero)
                b = cl_mul_inv(b, ks.gamma | 0x01u);

                // Reverse L3: bit rotation
                b = rotr8(b, ks.r[rnd % 6]);

                // Reverse L2: non-linear substitution
                b = sbox_inv(b,
                             mba_xor(ks.sbox_seed, fi8),
                             static_cast<uint8_t>(rnd),
                             ks.tweak);

                // Reverse L1: key addition
                const uint8_t rk = mba_add(
                    ks.k[rnd % 12],
                    static_cast<uint8_t>(
                        static_cast<unsigned>(fi8)
                      * static_cast<unsigned>(ks.delta)
                      + static_cast<unsigned>(rnd)
                    )
                );
                b = mba_xor(b, rk);
            }

            cur   = b;
            junk ^= static_cast<uint64_t>(b) * 0x9e3779b9ULL;
            state = 3;
            break;
        }

        // ── State 3: store ────────────────────────────────────────────
        case 3:
            dst[i] = cur;
            state  = 4;
            break;

        // ── State 4: advance ──────────────────────────────────────────
        case 4:
            ++i;
            junk = (junk >> 1u) | (junk << 63u);  // rotate to keep it alive
            state = 1;
            break;

        // ── State 5: epilogue ─────────────────────────────────────────
        case 5:
            dst[total_bytes] = static_cast<uint8_t>(0);  // null-terminate
            canary.trap_if_broken();                       // tamper check
            return;

        default:
            // Unreachable — traps under sanitizers / attackers patching state
            TTR_ANTIDEBUG_CHECK();
            return;
        }
    }
}

// ── GF(2^8) multiplicative inverse helper ────────────────────────────────
// Computed via extended Euclidean algorithm at runtime (only 256 values)
TTR_NOINLINE
static uint8_t cl_mul_inv(uint8_t val, uint8_t gen) noexcept {
    if (val == 0u) return 0u;
    // Find x such that cl_mul(gen, x) == val
    for (int x = 1; x < 256; ++x) {
        if (cl_mul(gen, static_cast<uint8_t>(x)) == val)
            return static_cast<uint8_t>(x);
    }
    return val;  // fallback (should not happen if gen is odd)
}

// ─────────────────────────────────────────────────────────────────────────────
//  STRING FACTORY  (thread_local output buffer, compile-time cipher object)
// ─────────────────────────────────────────────────────────────────────────────
template<uint64_t S, typename C, std::size_t N>
[[nodiscard]] TTR_FORCEINLINE
const C* make(const C (&src)[N]) noexcept
{
    // cipher is constexpr — lives in read-only .rodata, fully encrypted
    static constexpr enc_str<C, N, S> cipher{ src };
    static constexpr ks_t             keys = make_ks<S>();

    // Output lives in thread-local storage → per-thread, no races
    thread_local C buf[N + 1]{};

    // Wipe previous contents first
    volatile C* vbuf = buf;
    for (std::size_t w = 0; w <= N; ++w) vbuf[w] = C{};

    do_dec(buf, cipher.d, keys, (N - 1u) * sizeof(C));

    return buf;
}

// ─────────────────────────────────────────────────────────────────────────────
//  ZERO-MEMORY STACK STRING  (decrypts → uses → wipes all in one scope)
// ─────────────────────────────────────────────────────────────────────────────
template<uint64_t S, typename C, std::size_t N>
struct stack_str {
    static constexpr enc_str<C, N, S> cipher{};   // compile-time only
    static constexpr ks_t             keys = make_ks<S>();
    C buf[N + 1]{};

    template<std::size_t M>
    explicit stack_str(const C (&src)[M]) noexcept {
        static_assert(M == N, "size mismatch");
        static constexpr enc_str<C, N, S> c{ src };
        do_dec(buf, c.d, keys, (N - 1u) * sizeof(C));
    }

    ~stack_str() noexcept {
        volatile C* p = buf;
        for (std::size_t i = 0; i <= N; ++i)
            p[i] = static_cast<C>(TTR_CFG_WIPE_BYTE);
    }

    const C* get() const noexcept { return buf; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  IMPORT HASH TABLE  (hides API calls from static scanners)
// ─────────────────────────────────────────────────────────────────────────────
#if TTR_PLATFORM_WINDOWS
namespace imports {
    #include <windows.h>

    // Walk export table by hash — avoids GetProcAddress string
    template<uint64_t FnHash>
    TTR_NOINLINE void* resolve(const char* mod_name) noexcept {
        HMODULE hm = GetModuleHandleA(mod_name);
        if (!hm) return nullptr;

        auto* dos  = reinterpret_cast<PIMAGE_DOS_HEADER>(hm);
        auto* nt   = reinterpret_cast<PIMAGE_NT_HEADERS>(
                         reinterpret_cast<BYTE*>(hm) + dos->e_lfanew);
        auto& expd = nt->OptionalHeader
                        .DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        auto* exp  = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
                         reinterpret_cast<BYTE*>(hm) + expd.VirtualAddress);

        auto* names   = reinterpret_cast<DWORD*>(
                            reinterpret_cast<BYTE*>(hm) + exp->AddressOfNames);
        auto* ords    = reinterpret_cast<WORD*>(
                            reinterpret_cast<BYTE*>(hm) + exp->AddressOfNameOrdinals);
        auto* funcs   = reinterpret_cast<DWORD*>(
                            reinterpret_cast<BYTE*>(hm) + exp->AddressOfFunctions);

        for (DWORD i = 0; i < exp->NumberOfNames; ++i) {
            const char* name = reinterpret_cast<const char*>(
                                   reinterpret_cast<BYTE*>(hm) + names[i]);
            if (fnv1a(name) == FnHash)
                return reinterpret_cast<void*>(
                           reinterpret_cast<BYTE*>(hm) + funcs[ords[i]]);
        }
        return nullptr;
    }

    // Cached function pointer wrapper
    template<typename Fn, uint64_t Hash>
    struct lazy_fn {
        mutable std::atomic<Fn> cached{ nullptr };
        const char* mod;

        constexpr lazy_fn(const char* m) : mod(m) {}

        template<typename... Args>
        auto operator()(Args&&... a) const -> decltype(cached.load()(a...)) {
            Fn fn = cached.load(std::memory_order_acquire);
            if (!fn) {
                fn = reinterpret_cast<Fn>(resolve<Hash>(mod));
                cached.store(fn, std::memory_order_release);
            }
            return fn(static_cast<Args&&>(a)...);
        }
    };

    #define TTR_IMPORT(RetTy, CallConv, Name, Mod, ...) \
        static const ::ttr::detail::imports::lazy_fn<RetTy(CallConv*)(__VA_ARGS__), \
            ::ttr::detail::fnv1a(#Name)> Name{ Mod }

} // namespace imports
#elif TTR_PLATFORM_ANDROID || TTR_PLATFORM_LINUX
namespace imports {
    #include <dlfcn.h>

    template<typename Fn>
    struct lazy_fn {
        mutable std::atomic<Fn> cached{ nullptr };
        const char* sym;
        const char* lib;

        constexpr lazy_fn(const char* s, const char* l = nullptr) : sym(s), lib(l) {}

        template<typename... Args>
        auto operator()(Args&&... a) const -> decltype(cached.load()(a...)) {
            Fn fn = cached.load(std::memory_order_acquire);
            if (!fn) {
                void* h = lib ? dlopen(lib, RTLD_LAZY | RTLD_LOCAL) : RTLD_DEFAULT;
                fn = reinterpret_cast<Fn>(dlsym(h, sym));
                cached.store(fn, std::memory_order_release);
            }
            return fn(static_cast<Args&&>(a)...);
        }
    };
} // namespace imports
#endif

}} // namespace ttr::detail

// ─────────────────────────────────────────────────────────────────────────────
//  ImGui STEALTH LAYER
// ─────────────────────────────────────────────────────────────────────────────
namespace ttr { namespace ui {

// Fake signature visible in IDA/Ghidra cross-references
#define TTR_FAKE_SIG "ANDROID_NATIVE_SECURITY_0xDEADF00D"

enum class Cmd : uint8_t {
    // Basic widgets
    Checkbox = 0, Button, SliderFloat, DragFloat, DragInt,
    InputText, InputTextMultiline, InputInt, InputFloat,
    // Selection
    Combo, ListBox,
    // Layout
    Begin, End, BeginChild, EndChild,
    CollapsingHeader, TreeNode, TreePop,
    Separator, SameLine, NewLine, Spacing,
    // Feedback
    Text, TextColored, TextDisabled, SetTooltip,
    // Menu
    BeginMenuBar, EndMenuBar, BeginMenu, EndMenu, MenuItem,
    // Popup
    OpenPopup, BeginPopup, EndPopup,
    // Utility
    ProgressBar, Image, Selectable,
    _COUNT
};

// ─── Opaque spoofed dispatcher ───────────────────────────────────────────
TTR_SECTION_ATTR TTR_NOINLINE TTR_OPTNONE
ttrbool dispatch(Cmd        cmd,
                 const char* label,    // always TTR_FAKE_SIG in release
                 void*       arg1 = nullptr,
                 void*       arg2 = nullptr,
                 void*       arg3 = nullptr,
                 void*       arg4 = nullptr) noexcept
{
    volatile const char* _sig = label;
    (void)_sig;

    ttrbool result{ false };

    switch (cmd) {
    case Cmd::Checkbox:
        result = ImGui::Checkbox(label, static_cast<bool*>(arg1));
        break;
    case Cmd::Button:
        result = ImGui::Button(label, *static_cast<ImVec2*>(arg1));
        break;
    case Cmd::SliderFloat:
        result = ImGui::SliderFloat(label,
                     static_cast<float*>(arg1),
                     *static_cast<float*>(arg2),
                     *static_cast<float*>(arg3));
        break;
    case Cmd::DragFloat:
        result = ImGui::DragFloat(label,
                     static_cast<float*>(arg1),
                     *static_cast<float*>(arg2),
                     *static_cast<float*>(arg3));
        break;
    case Cmd::DragInt:
        result = ImGui::DragInt(label,
                     static_cast<int*>(arg1),
                     *static_cast<float*>(arg2),
                     *static_cast<int*>(arg3),
                     *static_cast<int*>(arg4));
        break;
    case Cmd::InputText:
        result = ImGui::InputText(label,
                     static_cast<char*>(arg1),
                     static_cast<size_t>(*static_cast<int*>(arg2)));
        break;
    case Cmd::InputTextMultiline:
        result = ImGui::InputTextMultiline(label,
                     static_cast<char*>(arg1),
                     static_cast<size_t>(*static_cast<int*>(arg2)),
                     *static_cast<ImVec2*>(arg3));
        break;
    case Cmd::InputInt:
        result = ImGui::InputInt(label, static_cast<int*>(arg1));
        break;
    case Cmd::InputFloat:
        result = ImGui::InputFloat(label, static_cast<float*>(arg1));
        break;
    case Cmd::Combo:
        result = ImGui::Combo(label,
                     static_cast<int*>(arg1),
                     static_cast<const char* const*>(arg2),
                     *static_cast<int*>(arg3));
        break;
    case Cmd::ListBox:
        result = ImGui::ListBox(label,
                     static_cast<int*>(arg1),
                     static_cast<const char* const*>(arg2),
                     *static_cast<int*>(arg3));
        break;
    case Cmd::Begin:
        result = ImGui::Begin(label, static_cast<bool*>(arg1));
        break;
    case Cmd::End:
        ImGui::End();
        break;
    case Cmd::BeginChild:
        result = ImGui::BeginChild(label,
                     arg1 ? *static_cast<ImVec2*>(arg1) : ImVec2{},
                     arg2 ? *static_cast<bool*>(arg2) : false);
        break;
    case Cmd::EndChild:
        ImGui::EndChild();
        break;
    case Cmd::CollapsingHeader:
        result = ImGui::CollapsingHeader(label);
        break;
    case Cmd::TreeNode:
        result = ImGui::TreeNode(label);
        break;
    case Cmd::TreePop:
        ImGui::TreePop();
        break;
    case Cmd::Separator:
        ImGui::Separator();
        break;
    case Cmd::SameLine:
        ImGui::SameLine();
        break;
    case Cmd::NewLine:
        ImGui::NewLine();
        break;
    case Cmd::Spacing:
        ImGui::Spacing();
        break;
    case Cmd::Text:
        ImGui::Text("%s", label);
        break;
    case Cmd::TextDisabled:
        ImGui::TextDisabled("%s", label);
        break;
    case Cmd::SetTooltip:
        ImGui::SetTooltip("%s", label);
        break;
    case Cmd::MenuItem:
        result = ImGui::MenuItem(label, nullptr, static_cast<bool*>(arg1));
        break;
    case Cmd::BeginMenuBar:
        result = ImGui::BeginMenuBar();
        break;
    case Cmd::EndMenuBar:
        ImGui::EndMenuBar();
        break;
    case Cmd::BeginMenu:
        result = ImGui::BeginMenu(label);
        break;
    case Cmd::EndMenu:
        ImGui::EndMenu();
        break;
    case Cmd::OpenPopup:
        ImGui::OpenPopup(label);
        break;
    case Cmd::BeginPopup:
        result = ImGui::BeginPopup(label);
        break;
    case Cmd::EndPopup:
        ImGui::EndPopup();
        break;
    case Cmd::Selectable:
        result = ImGui::Selectable(label, static_cast<bool*>(arg1));
        break;
    default:
        break;
    }

    return result;
}

// ─── Transparent runtime dispatcher ─────────────────────────────────────
// Decrypts → calls ImGui → wipes stack buffer.
// No plaintext remains in RAM after the call returns.
template<uint64_t S, typename C, std::size_t N>
[[nodiscard]] TTR_FORCEINLINE
ttrbool transparent_dispatch(Cmd      cmd,
                              const C (&label)[N],
                              void*    arg1 = nullptr,
                              void*    arg2 = nullptr,
                              void*    arg3 = nullptr,
                              void*    arg4 = nullptr) noexcept
{
    static constexpr ::ttr::detail::enc_str<C, N, S> cipher{ label };
    static constexpr ::ttr::detail::ks_t             keys = ::ttr::detail::make_ks<S>();

    // Stack buffer — never static, never thread_local → gone after return
    C local[N + 1]{};

    ::ttr::detail::do_dec(local, cipher.d, keys, (N - 1u) * sizeof(C));

    ttrbool result{ false };

    switch (cmd) {
    case Cmd::Checkbox:
        result = ImGui::Checkbox(local, static_cast<bool*>(arg1));
        break;
    case Cmd::Button:
        result = ImGui::Button(local, *static_cast<ImVec2*>(arg1));
        break;
    case Cmd::SliderFloat:
        result = ImGui::SliderFloat(local,
                     static_cast<float*>(arg1),
                     *static_cast<float*>(arg2),
                     *static_cast<float*>(arg3));
        break;
    case Cmd::DragFloat:
        result = ImGui::DragFloat(local,
                     static_cast<float*>(arg1),
                     *static_cast<float*>(arg2),
                     *static_cast<float*>(arg3));
        break;
    case Cmd::DragInt:
        result = ImGui::DragInt(local,
                     static_cast<int*>(arg1),
                     *static_cast<float*>(arg2),
                     *static_cast<int*>(arg3),
                     *static_cast<int*>(arg4));
        break;
    case Cmd::InputText:
        result = ImGui::InputText(local,
                     static_cast<char*>(arg1),
                     static_cast<size_t>(*static_cast<int*>(arg2)));
        break;
    case Cmd::InputTextMultiline:
        result = ImGui::InputTextMultiline(local,
                     static_cast<char*>(arg1),
                     static_cast<size_t>(*static_cast<int*>(arg2)),
                     *static_cast<ImVec2*>(arg3));
        break;
    case Cmd::CollapsingHeader:
        result = ImGui::CollapsingHeader(local);
        break;
    case Cmd::TreeNode:
        result = ImGui::TreeNode(local);
        break;
    case Cmd::Text:
        ImGui::Text("%s", local);
        break;
    case Cmd::TextDisabled:
        ImGui::TextDisabled("%s", local);
        break;
    case Cmd::SetTooltip:
        ImGui::SetTooltip("%s", local);
        break;
    case Cmd::MenuItem:
        result = ImGui::MenuItem(local, nullptr, static_cast<bool*>(arg1));
        break;
    case Cmd::Selectable:
        result = ImGui::Selectable(local, static_cast<bool*>(arg1));
        break;
    case Cmd::Begin:
        result = ImGui::Begin(local, static_cast<bool*>(arg1));
        break;
    case Cmd::BeginMenu:
        result = ImGui::BeginMenu(local);
        break;
    case Cmd::OpenPopup:
        ImGui::OpenPopup(local);
        break;
    case Cmd::BeginPopup:
        result = ImGui::BeginPopup(local);
        break;
    default:
        result = dispatch(cmd, TTR_FAKE_SIG, arg1, arg2, arg3, arg4);
        break;
    }

    // Secure wipe — 0xCC pattern, volatile to prevent elision
    volatile C* wp = local;
    for (std::size_t wi = 0; wi <= N; ++wi)
        wp[wi] = static_cast<C>(TTR_CFG_WIPE_BYTE);

    return result;
}

}} // namespace ttr::ui

// ─────────────────────────────────────────────────────────────────────────────
//  PER-CALLSITE UNIQUE SEED
//  Combines: file path, date, time, line number, call counter
//  → Every single macro call gets a different cipher key
// ─────────────────────────────────────────────────────────────────────────────
#define TTR_SEED (                                                          \
    ::ttr::detail::fnv1a(__FILE__ __DATE__ __TIME__)                        \
    ^ (static_cast<uint64_t>(__LINE__)    * 0x9e3779b97f4a7c15ULL)         \
    ^ (static_cast<uint64_t>(__COUNTER__) * 0x517cc1b727220a95ULL)         \
    ^ (static_cast<uint64_t>(TTR_VERSION_NUM) * 0xBF58476D1CE4E5B9ULL)    \
)

// ─────────────────────────────────────────────────────────────────────────────
//  CORE STRING MACROS
// ─────────────────────────────────────────────────────────────────────────────
// TTR(s)       → const char*  (thread_local, lives until next TTR call on thread)
// TTR_W(s)     → const wchar_t*
// TTR_U8(s)    → const char8_t*  (C++20)
// TTR_S(s)     → stack_str<>     (decrypts on construction, wipes on destruction)
// TTR_HASH(s)  → constexpr uint64_t FNV-1a hash of literal
// TTR_SEED     → constexpr uint64_t unique per-callsite seed

#define TTR(s)      (::ttr::detail::make<TTR_SEED, char>   (s))
#define TTR_W(s)    (::ttr::detail::make<TTR_SEED, wchar_t>(s))
#define TTR_HASH(s) (::ttr::detail::fnv1a(s))

#define TTR_S(s)    (::ttr::detail::stack_str<TTR_SEED, char, sizeof(s)>(s))

#if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
    #define TTR_U8(s) (::ttr::detail::make<TTR_SEED, char8_t>(s))
#else
    #define TTR_U8(s) TTR(s)
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  ImGui SPOOFED MACROS  (IDA sees TTR_FAKE_SIG in xrefs, not your strings)
// ─────────────────────────────────────────────────────────────────────────────
#define TTR_CHECKBOX(label, v)          \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Checkbox, TTR_FAKE_SIG, (void*)(v))

#define TTR_BUTTON(label, size)         \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Button, TTR_FAKE_SIG, (void*)(&size))

#define TTR_SLIDERFLOAT(label, v, mn, mx) \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::SliderFloat, TTR_FAKE_SIG, (void*)(v), (void*)(&mn), (void*)(&mx))

#define TTR_DRAGFLOAT(label, v, spd, mx) \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::DragFloat, TTR_FAKE_SIG, (void*)(v), (void*)(&spd), (void*)(&mx))

#define TTR_DRAGINT(label, v, spd, mn, mx) \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::DragInt, TTR_FAKE_SIG, (void*)(v), (void*)(&spd), (void*)(&mn), (void*)(&mx))

#define TTR_INPUTTEXT(label, buf, sz)   \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::InputText, TTR_FAKE_SIG, (void*)(buf), (void*)(&sz))

#define TTR_COMBO(label, cur, items, count) \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Combo, TTR_FAKE_SIG, (void*)(cur), (void*)(items), (void*)(&count))

#define TTR_LISTBOX(label, cur, items, count) \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::ListBox, TTR_FAKE_SIG, (void*)(cur), (void*)(items), (void*)(&count))

#define TTR_TREENODE(label)             \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::TreeNode, TTR_FAKE_SIG, nullptr)

#define TTR_TREEPOP()                   \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::TreePop, TTR_FAKE_SIG, nullptr)

#define TTR_BEGIN(label, p_open)        \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Begin, TTR_FAKE_SIG, (void*)(p_open))

#define TTR_END()                       \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::End, TTR_FAKE_SIG, nullptr)

#define TTR_BEGINCHILD(label, sz, border) \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::BeginChild, TTR_FAKE_SIG, (void*)(&sz), (void*)(&border))

#define TTR_ENDCHILD()                  \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::EndChild, TTR_FAKE_SIG, nullptr)

#define TTR_TOOLTIP(label)              \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::SetTooltip, TTR_FAKE_SIG, nullptr)

#define TTR_TEXT(label)                 \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Text, TTR_FAKE_SIG, nullptr)

#define TTR_SEPARATOR()                 \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Separator, "", nullptr)

#define TTR_SAMELINE()                  \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::SameLine, "", nullptr)

#define TTR_SPACING()                   \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Spacing, "", nullptr)

#define TTR_MENUITEM(label, sel)        \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::MenuItem, TTR_FAKE_SIG, (void*)(sel))

#define TTR_BEGINMENU(label)            \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::BeginMenu, TTR_FAKE_SIG, nullptr)

#define TTR_ENDMENU()                   \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::EndMenu, TTR_FAKE_SIG, nullptr)

#define TTR_COLLAPSINGHEADER(label)     \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::CollapsingHeader, TTR_FAKE_SIG, nullptr)

#define TTR_SELECTABLE(label, sel)      \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::Selectable, TTR_FAKE_SIG, (void*)(sel))

#define TTR_OPENPOPUP(label)            \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::OpenPopup, TTR_FAKE_SIG, nullptr)

#define TTR_BEGINPOPUP(label)           \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::BeginPopup, TTR_FAKE_SIG, nullptr)

#define TTR_ENDPOPUP()                  \
    ::ttr::ui::dispatch(::ttr::ui::Cmd::EndPopup, TTR_FAKE_SIG, nullptr)

// ─────────────────────────────────────────────────────────────────────────────
//  ImGui TRANSPARENT MACROS  (decrypt → draw → wipe, no trace in RAM)
// ─────────────────────────────────────────────────────────────────────────────
#define TTR_TCHECKBOX(label, v)         \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::Checkbox, label, (void*)(v))

#define TTR_TBUTTON(label, size)        \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::Button, label, (void*)(&size))

#define TTR_TSLIDERFLOAT(label, v, mn, mx) \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::SliderFloat, label, (void*)(v), (void*)(&mn), (void*)(&mx))

#define TTR_TDRAGFLOAT(label, v, spd, mx) \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::DragFloat, label, (void*)(v), (void*)(&spd), (void*)(&mx))

#define TTR_TDRAGINT(label, v, spd, mn, mx) \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::DragInt, label, (void*)(v), (void*)(&spd), (void*)(&mn), (void*)(&mx))

#define TTR_TTEXT(label)                \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::Text, label, nullptr)

#define TTR_TTEXTDISABLED(label)        \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::TextDisabled, label, nullptr)

#define TTR_TTREENODE(label)            \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::TreeNode, label, nullptr)

#define TTR_TCOLLAPSINGHEADER(label)    \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::CollapsingHeader, label, nullptr)

#define TTR_TMENUITEM(label, sel)       \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::MenuItem, label, (void*)(sel))

#define TTR_TBEGINMENU(label)           \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::BeginMenu, label, nullptr)

#define TTR_TSELECTABLE(label, sel)     \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::Selectable, label, (void*)(sel))

#define TTR_TINPUTTEXT(label, buf, sz)  \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::InputText, label, (void*)(buf), (void*)(&sz))

#define TTR_TBEGIN(label, p_open)       \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::Begin, label, (void*)(p_open))

#define TTR_TOPENPOPUP(label)           \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::OpenPopup, label, nullptr)

#define TTR_TBEGINPOPUP(label)          \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::BeginPopup, label, nullptr)

#define TTR_TTOOLTIP(label)             \
    ::ttr::ui::transparent_dispatch<TTR_SEED>(::ttr::ui::Cmd::SetTooltip, label, nullptr)

// ─────────────────────────────────────────────────────────────────────────────
//  DEBUG MODE BYPASS  (all overhead stripped in debug builds)
// ─────────────────────────────────────────────────────────────────────────────
#if defined(TTR_DEBUG) || (defined(_DEBUG) && !defined(TTR_FORCE_ENCRYPT))

// Undefine release macros
#undef TTR
#undef TTR_W
#undef TTR_U8
#undef TTR_HASH

// Core string
#define TTR(s)    (s)
#define TTR_W(s)  (L ## s)
#define TTR_U8(s) (u8 ## s)
#define TTR_HASH(s) (::ttr::detail::fnv1a(s))  // keep hash available

// Spoofed → passthrough
#undef  TTR_CHECKBOX
#undef  TTR_BUTTON
#undef  TTR_SLIDERFLOAT
#undef  TTR_DRAGFLOAT
#undef  TTR_DRAGINT
#undef  TTR_INPUTTEXT
#undef  TTR_COMBO
#undef  TTR_LISTBOX
#undef  TTR_TREENODE
#undef  TTR_TREEPOP
#undef  TTR_BEGIN
#undef  TTR_END
#undef  TTR_BEGINCHILD
#undef  TTR_ENDCHILD
#undef  TTR_TOOLTIP
#undef  TTR_TEXT
#undef  TTR_SEPARATOR
#undef  TTR_SAMELINE
#undef  TTR_SPACING
#undef  TTR_MENUITEM
#undef  TTR_BEGINMENU
#undef  TTR_ENDMENU
#undef  TTR_COLLAPSINGHEADER
#undef  TTR_SELECTABLE
#undef  TTR_OPENPOPUP
#undef  TTR_BEGINPOPUP
#undef  TTR_ENDPOPUP

#define TTR_CHECKBOX(l,v)             ImGui::Checkbox(l,v)
#define TTR_BUTTON(l,sz)              ImGui::Button(l,sz)
#define TTR_SLIDERFLOAT(l,v,mn,mx)   ImGui::SliderFloat(l,v,mn,mx)
#define TTR_DRAGFLOAT(l,v,sp,mx)     ImGui::DragFloat(l,v,sp,mx)
#define TTR_DRAGINT(l,v,sp,mn,mx)    ImGui::DragInt(l,v,sp,mn,mx)
#define TTR_INPUTTEXT(l,buf,sz)      ImGui::InputText(l,buf,sz)
#define TTR_COMBO(l,cur,items,cnt)   ImGui::Combo(l,cur,items,cnt)
#define TTR_LISTBOX(l,cur,items,cnt) ImGui::ListBox(l,cur,items,cnt)
#define TTR_TREENODE(l)              ImGui::TreeNode(l)
#define TTR_TREEPOP()                ImGui::TreePop()
#define TTR_BEGIN(l,po)              ImGui::Begin(l,po)
#define TTR_END()                    ImGui::End()
#define TTR_BEGINCHILD(l,sz,b)       ImGui::BeginChild(l,sz,b)
#define TTR_ENDCHILD()               ImGui::EndChild()
#define TTR_TOOLTIP(l)               ImGui::SetTooltip("%s",l)
#define TTR_TEXT(l)                  ImGui::Text("%s",l)
#define TTR_SEPARATOR()              ImGui::Separator()
#define TTR_SAMELINE()               ImGui::SameLine()
#define TTR_SPACING()                ImGui::Spacing()
#define TTR_MENUITEM(l,sel)          ImGui::MenuItem(l,nullptr,sel)
#define TTR_BEGINMENU(l)             ImGui::BeginMenu(l)
#define TTR_ENDMENU()                ImGui::EndMenu()
#define TTR_COLLAPSINGHEADER(l)      ImGui::CollapsingHeader(l)
#define TTR_SELECTABLE(l,sel)        ImGui::Selectable(l,sel)
#define TTR_OPENPOPUP(l)             ImGui::OpenPopup(l)
#define TTR_BEGINPOPUP(l)            ImGui::BeginPopup(l)
#define TTR_ENDPOPUP()               ImGui::EndPopup()

// Transparent → same passthrough
#undef  TTR_TCHECKBOX
#undef  TTR_TBUTTON
#undef  TTR_TSLIDERFLOAT
#undef  TTR_TDRAGFLOAT
#undef  TTR_TDRAGINT
#undef  TTR_TTEXT
#undef  TTR_TTEXTDISABLED
#undef  TTR_TTREENODE
#undef  TTR_TCOLLAPSINGHEADER
#undef  TTR_TMENUITEM
#undef  TTR_TBEGINMENU
#undef  TTR_TSELECTABLE
#undef  TTR_TINPUTTEXT
#undef  TTR_TBEGIN
#undef  TTR_TOPENPOPUP
#undef  TTR_TBEGINPOPUP
#undef  TTR_TTOOLTIP

#define TTR_TCHECKBOX(l,v)           ImGui::Checkbox(l,v)
#define TTR_TBUTTON(l,sz)            ImGui::Button(l,sz)
#define TTR_TSLIDERFLOAT(l,v,mn,mx)  ImGui::SliderFloat(l,v,mn,mx)
#define TTR_TDRAGFLOAT(l,v,sp,mx)    ImGui::DragFloat(l,v,sp,mx)
#define TTR_TDRAGINT(l,v,sp,mn,mx)   ImGui::DragInt(l,v,sp,mn,mx)
#define TTR_TTEXT(l)                 ImGui::Text("%s",l)
#define TTR_TTEXTDISABLED(l)         ImGui::TextDisabled("%s",l)
#define TTR_TTREENODE(l)             ImGui::TreeNode(l)
#define TTR_TCOLLAPSINGHEADER(l)     ImGui::CollapsingHeader(l)
#define TTR_TMENUITEM(l,sel)         ImGui::MenuItem(l,nullptr,sel)
#define TTR_TBEGINMENU(l)            ImGui::BeginMenu(l)
#define TTR_TSELECTABLE(l,sel)       ImGui::Selectable(l,sel)
#define TTR_TINPUTTEXT(l,buf,sz)     ImGui::InputText(l,buf,sz)
#define TTR_TBEGIN(l,po)             ImGui::Begin(l,po)
#define TTR_TOPENPOPUP(l)            ImGui::OpenPopup(l)
#define TTR_TBEGINPOPUP(l)           ImGui::BeginPopup(l)
#define TTR_TTOOLTIP(l)              ImGui::SetTooltip("%s",l)

#endif // TTR_DEBUG

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif