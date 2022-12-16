#ifndef CONCURRENCPP_ATOMIC_WAIT_H
#define CONCURRENCPP_ATOMIC_WAIT_H

#include <atomic>
#include <chrono>

#include <cassert>

namespace concurrencpp::details {
    void CRCPP_API atomic_wait_native(void* atom, int32_t old) noexcept;
    void CRCPP_API atomic_wait_for_native(void* atom, int32_t old, std::chrono::milliseconds ms) noexcept;
    void CRCPP_API atomic_notify_one_native(void* atom) noexcept;
    void CRCPP_API atomic_notify_all_native(void* atom) noexcept;

    enum class atomic_wait_status { ok, timeout };

    template<class type>
    void CRCPP_API atomic_wait(std::atomic<type>& atom, type old, std::memory_order order) noexcept {
        static_assert(std::is_standard_layout_v<std::atomic<type>>, "atomic_wait - std::atom<type> is not standard-layout");
        static_assert(sizeof(type) == sizeof(int32_t), "atomic_wait - <<type>> must be 4 bytes.");

        while (true) {
            const auto val = atom.load(order);
            if (val != old) {
                return;
            }

            atomic_wait_native(&atom, static_cast<int32_t>(old));
        }
    }

    template<class type>
    atomic_wait_status CRCPP_API atomic_wait_for(std::atomic<type>& atom,
                                       type old,
                                       std::chrono::milliseconds ms,
                                       std::memory_order order) noexcept {
        static_assert(std::is_standard_layout_v<std::atomic<type>>, "atomic_wait_for - std::atom<type> is not standard-layout");
        static_assert(sizeof(type) == sizeof(int32_t), "atomic_wait_for - <<type>> must be 4 bytes.");

        const auto deadline = std::chrono::system_clock::now() + ms;

        while (true) {
            const auto val = atom.load(order);
            if (val != old) {
                return atomic_wait_status::ok;
            }

            const auto now = std::chrono::system_clock::now();
            if (now >= deadline) {
                if (val != old) {
                    return atomic_wait_status::ok;
                }

            	return atomic_wait_status::timeout;
            }

            const auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
            assert(time_diff.count() >= 0);
            atomic_wait_for_native(&atom, static_cast<int32_t>(old), time_diff);
        }
    }

    template<class type>
    void CRCPP_API atomic_notify_one(std::atomic<type>& atom) noexcept {
        atomic_notify_one_native(&atom);
    }

    template<class type>
    void CRCPP_API atomic_notify_all(std::atomic<type>& atom) noexcept {
        atomic_notify_all_native(&atom);
    }
}  // namespace concurrencpp::details

#endif