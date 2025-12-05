#pragma once

#include "defs.h"

namespace vm::utils {
    /** V and E must be different types. */
    template<typename V, typename E> requires (not __is_same(V, E))
    struct res_t {
        /*void *data;*/
        char data[sizeof(V) > sizeof(E) ? sizeof(V) : sizeof(E)];
        bool hv, owns = true;

        res_t(res_t &other) : hv(other.hv), owns(other.owns) {
            if (hv) new(tv(data)) V(other.value());
            else new(tv(data)) E(other.error());
        }

        res_t(res_t &&other) noexcept : hv(other.hv), owns(other.owns) {
            if (hv) new(tv(data)) V(std::move(other.value()));
            else new(tv(data)) E(std::move(other.error()));

            other.owns = false;
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
            hv = other.hv;
            owns = other.owns;
            if (hv)new(tv(data)) V(other.value());
            else new(tv(data)) E(other.error());
            return *this;
        }

        res_t &operator=(res_t &&other) noexcept {
            this->~res_t();
            data = other.data;
            hv = other.hv;

            owns = other.owns;
            if (hv) new(tv(data)) V(std::move(other.value()));
            else new(tv(data)) E(std::move(other.error()));
            other.owns = false;

            return *this;
        }

        res_t(V &&v) : hv(true) {
            new(tv(data)) V(std::move(v));
        }

        res_t(const V &v) : hv(true) {
            new(tv(data)) V(v);
        }

        res_t(E &&e) : hv(false) {
            new(tv(data)) E(std::move(e));
        }

        res_t(const E &e) : hv(false) {
            new(tv(data)) E(e);
        }

        V &value() {
            return *reinterpret_cast<V *>(data);
        }

        E &error() {
            return *reinterpret_cast<E *>(data);
        }

        operator bool() const {
            return hv;
        }

        ~res_t() {
            if (owns) {
                if (hv) delete_value(reinterpret_cast<V *>(data), V) else delete_value(reinterpret_cast<E *>(data), E)
            }
        }
    };
}
