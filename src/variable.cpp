#include "sdkgenny/namespace.hpp"
#include <climits>

#include <sdkgenny/struct.hpp>

#include <sdkgenny/variable.hpp>

namespace sdkgenny {
Variable* Variable::append() {
    auto struct_ = owner<Struct>();
    uintptr_t highest_offset{};
    Variable* highest_var{};

    for (auto&& var : struct_->get_all<Variable>()) {
        if (var->offset() >= highest_offset && var != this) {
            highest_offset = var->offset();
            highest_var = var;
        }
    }

    if (highest_var != nullptr) {
        // Both bitfields of the same type.
        if (is_bitfield() && highest_var->is_bitfield() && m_type == highest_var->type()) {
            uintptr_t highest_bit = 0;
            auto bf = struct_->bitfield(highest_var->offset(), this);

            for (auto&& [bit_offset, bit_var] : bf) {
                if (bit_offset >= highest_bit && bit_var != this) {
                    highest_bit = bit_offset;
                    highest_var = bit_var;
                }
            }

            auto end_bit = highest_var->bit_offset() + highest_var->bit_size();

            if (end_bit + m_bit_size <= m_type->size() * CHAR_BIT) {
                // Squeeze into the remainign bits.
                m_offset = highest_var->offset();
            } else {
                // Not enough room, so start where the previous bitfield ended.
                m_offset = highest_var->end();
            }
        } else {
            m_offset = highest_var->end();
        }
    } else if (auto parents = struct_->parents(); !parents.empty()) {
        size_t size{};

        for (auto&& parent : parents) {
            size += parent->size();
        }

        m_offset = size;
    } else {
        m_offset = 0;
    }

    return this;
}

Type* Variable::type() {
    if(m_type == nullptr) {
        m_type = m_owner->topmost_owner<Namespace>()->resolve_type(m_type_name);
    }

    return m_type;
}

size_t Variable::size() {
    if (type() == nullptr) {
        return 0;
    }

    return type()->size();
}

Variable* Variable::bit_append() {
    auto struct_ = owner<Struct>();
    uintptr_t highest_bit{};
    Variable* highest_var{};
    auto bf = struct_->bitfield(m_offset, this);

    for (auto&& [bit_offset, bit_var] : bf) {
        if (bit_offset >= highest_bit && bit_var != this) {
            highest_bit = bit_offset;
            highest_var = bit_var;
        }
    }

    if (highest_var != nullptr) {
        auto end_bit = highest_var->bit_offset() + highest_var->bit_size();

        m_bit_offset = end_bit;
    } else {
        m_bit_offset = 0;
    }

    return this;
}

void Variable::generate(std::ostream& os) {
    generate_comment(os);
    generate_metadata(os);
    type()->generate_typename_for(os, this);
    os << " " << usable_name();
    type()->generate_variable_postamble(os);

    if (m_bit_size != 0) {
        os << " : " << std::dec << m_bit_size;
    }

    os << "; // 0x" << std::hex << m_offset << "\n";
}
} // namespace sdkgenny