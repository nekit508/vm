#include "compiler/lexer.h"

const char * lexer::to_string(token_kind_t e) {
    switch (e) {
        case null: return "null";
        case ident: return "ident";
        case num: return "num";
        case string: return "string";
        case falsee: return "false";
        case truee: return "true";
        case fun: return "fun";
        case iff: return "iff";
        case asmm: return "asm";
        case l_brace: return "l_brace";
        case r_brace: return "r_brace";
        case l_paren: return "l_paren";
        case r_paren: return "r_paren";
        case l_bracket: return "l_bracket";
        case r_bracket: return "r_bracket";
        case dot: return "dot";
        case comma: return "comma";
        case semicolon: return "semicolon";
        case colon: return "colon";
        case eof: return "eof";
        default: return "unknown";
    }
}

lexer::token_t::token_t(): pos(0), kind(null), literal(0) {
}

lexer::token_t::token_t(const addr_t pos, const token_kind_t kind, vm::utils::str_t &&literal): pos(pos),
                                                                                          kind(kind),
                                                                                          literal(std::move(literal)) {
}

lexer::token_t::token_t(const token_t &other): pos(other.pos),
                                                  kind(other.kind),
                                                  literal(other.literal) {
}

lexer::token_t::token_t(token_t &&other) noexcept: pos(other.pos),
                                                      kind(other.kind),
                                                      literal(std::move(other.literal)) {
}

lexer::token_t & lexer::token_t::operator=(const token_t &other) {
    if (this == &other)
        return *this;
    pos = other.pos;
    kind = other.kind;
    literal = other.literal;
    return *this;
}

lexer::token_t & lexer::token_t::operator=(token_t &&other) noexcept {
    if (this == &other)
        return *this;
    pos = other.pos;
    kind = other.kind;
    literal = std::move(other.literal);
    return *this;
}

bool lexer::token_t::operator==(const token_kind_t &other) const {
    return kind == other;
}

bool lexer::token_t::operator!=(const token_kind_t &other) const {
    return kind != other;
}

bool lexer::context_t::in(const char c, const char *list) {
    for (const char *l = list; *l; l++)
        if (c == *l)
            return true;
    return false;
}

char lexer::context_t::read_one() {
    char out;
    if (!stream.read(&out, 1)) throw eof_e();
    return out;
}

void lexer::context_t::skip_until(const char *list) {
    for (bool exit = false; !exit;) {
        char c = read_one();
        for (const char *p = list; *p; p++)
            if (c == *p) {
                exit = true;
                break;
            }
    }

    stream << 1;
}

void lexer::context_t::skip_while(const char *list) {
    for (bool exit = false; !exit;) {
        char c = read_one();
        exit = true;
        for (const char *p = list; *p; p++)
            if (c == *p) {
                exit = false;
                break;
            }
    }

    stream << 1;
}

vm::utils::str_t lexer::context_t::read_until(const char *list) {
    vm::utils::str_t out;

    for (bool exit = false; !exit;) {
        char c = read_one();
        for (const char *p = list; *p; p++)
            if (c == *p) {
                exit = true;
                out.push(c);
                break;
            }
    }

    stream << 1;

    return out;
}

vm::utils::str_t lexer::context_t::read_while(const char *list) {
    vm::utils::str_t out;

    for (bool exit = false; !exit;) {
        char c = read_one();
        exit = true;
        for (const char *p = list; *p; p++)
            if (c == *p) {
                exit = false;
                out.push(c);
                break;
            }
    }

    stream << 1;

    return out;
}

void lexer::context_t::skip_white_spaces() {
    skip_while(white_space);
}

lexer::token_t lexer::context_t::parse_ident_or_keyword() {
    token_kind_t out_kind = ident;
    vm::utils::str_t literal(std::move(read_while(ident_body)));

    for (const auto &[name, kind] : keywords)
        if (name == literal) {
            out_kind = kind;
            break;
        }

    literal.trim();
    return token_t{stream.pos(), out_kind, std::move(literal)};
}

lexer::token_t lexer::context_t::parse_number() {
    token_kind_t kind = num;
    vm::utils::str_t literal(std::move(read_while(number)));
    literal.trim();
    return token_t{stream.pos(), kind, std::move(literal)};
}

lexer::tokens_t lexer::context_t::parse() {
    tokens_t out{};

    try {
        for (;;) {
            skip_white_spaces();
            char c = read_one();

            if (in(c, ident_start)) {
                stream << 1;
                out.emplace(std::move(parse_ident_or_keyword()));
            } else if (in(c, number)) {
                stream << 1;
                out.emplace(std::move(parse_number()));
            } else {
                // try to parse one-symbol token
                const addr_t pos = stream.pos();
                vm::utils::str_t literal(1);
                literal.emplace(std::move(c));
                bool processed = true;
                switch (c) {
                    case '{':
                        out.emplace(token_t(pos, l_brace, std::move(literal)));
                        break;
                    case '}':
                        out.emplace(token_t(pos, r_brace, std::move(literal)));
                        break;
                    case '(':
                        out.emplace(token_t(pos, l_paren, std::move(literal)));
                        break;
                    case ')':
                        out.emplace(token_t(pos, r_paren, std::move(literal)));
                        break;
                    case '[':
                        out.emplace(token_t(pos, l_bracket, std::move(literal)));
                        break;
                    case ']':
                        out.emplace(token_t(pos, r_bracket, std::move(literal)));
                        break;
                    case '.':
                        out.emplace(token_t(pos, dot, std::move(literal)));
                        break;
                    case ',':
                        out.emplace(token_t(pos, comma, std::move(literal)));
                        break;
                    case ';':
                        out.emplace(token_t(pos, semicolon, std::move(literal)));
                        break;
                    case ':':
                        out.emplace(token_t(pos, colon, std::move(literal)));
                        break;
                    default:
                        processed = false;
                        break;
                }

                // we haven't parsed it
                if (!processed) {

                }
            }
        }
    } catch (const eof_e &) {
        out.emplace(token_t(stream.pos(), eof, vm::utils::str_t(0)));
    }

    return out;
}
