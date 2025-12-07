#pragma once
#include "bits/move.h"

namespace utils {
    template<typename T>
    struct seq_node_t {
        typedef T * rT;

        seq_node_t *prev = nullptr, *next = nullptr;

        T value;

        seq_node_t(const T &value) : value(value) {
        }

        seq_node_t(T &&value) : value(std::move(value)) {
        }

        operator rT() {
            return &value;
        }

        seq_node_t(seq_node_t &&other) = delete;

        seq_node_t &operator=(seq_node_t &&other) = delete;

        seq_node_t(const seq_node_t &other) = delete;

        seq_node_t &operator=(const seq_node_t &other) = delete;

        ~seq_node_t() {
        }
    };

    template<typename T>
    struct seq_iter_t {
        typedef seq_node_t<T> *node_t;
        typedef T * rT;

        node_t start_node, end_node, current_node;

        seq_iter_t(const node_t start_node, const node_t end_node)
            : start_node(start_node),
              end_node(end_node),
              current_node(start_node) {
        }

        node_t next() {
            return current_node = current_node->next;
        }

        node_t cur() {
            return current_node;
        }

        bool has_next() {
            return current_node != nullptr;
        }

        operator rT() {
            return &current_node->value;
        }
    };

    template<typename T>
    struct seq_t {
        typedef seq_node_t<T> *node_t;
        typedef seq_iter_t<T> iter;

        node_t start_node, end_node;

        seq_iter_t<T> iterator() {
            return seq_iter_t<T>(start_node, end_node);
        }
    };

    void f() {
        seq_t<char> seq;

        for (seq_t<char>::iter iter = seq.iterator(); iter.has_next(); iter.next()) {
            char *c = *iter.cur();
        }
    }
}
