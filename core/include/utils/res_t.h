#pragma once

#include "defs.h"

namespace utils {
    /** V and E must be different types. */
    template<typename V, typename E> requires (not std::is_same_v<V, E>)
    struct res_t {
        char data[sizeof(V) > sizeof(E) ? sizeof(V) : sizeof(E)];
        bool has_value, owns = true;

        template<typename OV, typename OE> requires (
            not std::is_same_v<OV, OE> && std::is_convertible_v<OV, V> && std::is_convertible_v<OE, E>)
        res_t(res_t<OV, OE> &&other) noexcept : has_value(other.has_value), owns(other.owns) {
            if (has_value) new(tv(data)) V(std::move(other.value()));
            else new(tv(data)) E(std::move(other.error()));

            other.owns = false;
        }

        res_t &&move() {
            return std::move(*this);
        }

        res_t &disown() {
            owns = false;
            return *this;
        }

        res_t &own() {
            owns = true;
            return *this;
        }

        res_t &operator=(res_t &other) {
            this->~res_t();
            has_value = other.has_value;
            owns = other.owns;
            if (has_value)new(tv(data)) V(other.value());
            else new(tv(data)) E(other.error());
            return *this;
        }

        res_t &operator=(res_t &&other) noexcept {
            this->~res_t();
            data = other.data;
            has_value = other.has_value;

            owns = other.owns;
            if (has_value) new(tv(data)) V(std::move(other.value()));
            else new(tv(data)) E(std::move(other.error()));
            other.owns = false;

            return *this;
        }

        res_t(V &&v) : has_value(true) {
            new(tv(data)) V(std::move(v));
        }

        res_t(const V &v) : has_value(true) {
            new(tv(data)) V(v);
        }

        res_t(E &&e) : has_value(false) {
            new(tv(data)) E(std::move(e));
        }

        res_t(const E &e) : has_value(false) {
            new(tv(data)) E(e);
        }

        V &value() {
            return *reinterpret_cast<V *>(data);
        }

        E &error() {
            return *reinterpret_cast<E *>(data);
        }

        V &&value_m() {
            return std::move(*reinterpret_cast<V *>(data));
        }

        E &&error_m() {
            return std::move(*reinterpret_cast<E *>(data));
        }

        operator bool() const {
            return has_value;
        }

        ~res_t() {
            if (owns) {
                if (has_value) delete_value(reinterpret_cast<V *>(data), V) else delete_value(
                    reinterpret_cast<E *>(data), E)
            }
        }
    };
}
