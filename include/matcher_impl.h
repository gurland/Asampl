#pragma once

namespace Matcher {

template< typename M >
bool match_one(const AstNode& node, M&& matcher) {
    static_assert(
            std::is_invocable_r<bool, M>::value ||
            std::is_invocable_r<bool, M, AstNodeType>::value ||
            std::is_invocable_r<bool, M, const std::string&>::value ||
            std::is_invocable_r<bool, M, AstNodeType, const std::string&>::value ||
            std::is_same_v<std::decay_t<M>, AstNodeType>
            );

    if constexpr (std::is_invocable_r<bool, M>::value) {
        return matcher();
    } else if constexpr (std::is_invocable_r<bool, M, AstNodeType>::value) {
        return matcher(node.type_);
    } else if constexpr (std::is_invocable_r<bool, M, const std::string&>::value) {
        return matcher(node.value_);
    } else if constexpr (std::is_invocable_r<bool, M, AstNodeType, const std::string&>::value) {
        return matcher(node.type_, node.value_);
    } else if constexpr (std::is_same_v<std::decay_t<M>, AstNodeType>) {
        return node.type_ == matcher;
    }
}

template< typename M >
bool match_one(const Tree& tree, M&& matcher) {
    static_assert(
            std::is_invocable_r<bool, M>::value ||
            std::is_invocable_r<bool, M, AstNodeType>::value ||
            std::is_invocable_r<bool, M, const std::string&>::value ||
            std::is_invocable_r<bool, M, AstNodeType, const std::string&>::value ||
            std::is_invocable_r<bool, M, const Tree&>::value ||
            std::is_same_v<std::decay_t<M>, AstNodeType>
            );

    if constexpr (std::is_invocable_r<bool, M>::value) {
        return matcher();
    } else if constexpr (std::is_invocable_r<bool, M, AstNodeType>::value) {
        return matcher(tree.get_node()->type_);
    } else if constexpr (std::is_invocable_r<bool, M, const std::string&>::value) {
        return matcher(tree.get_node()->value_);
    } else if constexpr (std::is_invocable_r<bool, M, AstNodeType, const std::string&>::value) {
        return matcher(tree.get_node()->type_, tree.get_node()->value_);
    } else if constexpr (std::is_invocable_r<bool, M, const Tree&>::value) {
        return matcher(tree);
    } else if constexpr (std::is_same_v<std::decay_t<M>, AstNodeType>) {
        return tree.get_node()->type_ == matcher;
    }
}

template< typename M, typename... Ms >
bool match_impl(const children_t& children, size_t n, M&& matcher, Ms&&... rest) {
    if (n >= children.size()) {
        return false;
    }
    return match_one(*children[n], std::forward<M>(matcher))
        && match_impl(children, n + 1, std::forward<Ms>(rest)...);
}

inline bool match_impl(const children_t&, size_t) {
    return true;
}

}
