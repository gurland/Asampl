#ifndef _MATCHER_IMPL_H
#define _MATCHER_IMPL_H

namespace Matcher {

template< typename M >
bool match_one(const ast_node& node, M&& matcher) {
    static_assert(
            std::is_invocable_r<bool, M>::value ||
            std::is_invocable_r<bool, M, ast_node_type>::value ||
            std::is_invocable_r<bool, M, const std::string&>::value ||
            std::is_invocable_r<bool, M, ast_node_type, const std::string&>::value ||
            std::is_same_v<std::decay_t<M>, ast_node_type>
            );

    if constexpr (std::is_invocable_r<bool, M>::value) {
        return matcher();
    } else if constexpr (std::is_invocable_r<bool, M, ast_node_type>::value) {
        return matcher(node.type);
    } else if constexpr (std::is_invocable_r<bool, M, const std::string&>::value) {
        return matcher(node.value);
    } else if constexpr (std::is_invocable_r<bool, M, ast_node_type, const std::string&>::value) {
        return matcher(node.type, node.value);
    } else if constexpr (std::is_same_v<std::decay_t<M>, ast_node_type>) {
        return node.type == matcher;
    }
}

template< typename M >
bool match_one(const as_tree& tree, M&& matcher) {
    static_assert(
            std::is_invocable_r<bool, M>::value ||
            std::is_invocable_r<bool, M, ast_node_type>::value ||
            std::is_invocable_r<bool, M, const std::string&>::value ||
            std::is_invocable_r<bool, M, ast_node_type, const std::string&>::value ||
            std::is_invocable_r<bool, M, const as_tree&>::value ||
            std::is_same_v<std::decay_t<M>, ast_node_type>
            );

    if constexpr (std::is_invocable_r<bool, M>::value) {
        return matcher();
    } else if constexpr (std::is_invocable_r<bool, M, ast_node_type>::value) {
        return matcher(tree.get_node()->type);
    } else if constexpr (std::is_invocable_r<bool, M, const std::string&>::value) {
        return matcher(tree.get_node()->value);
    } else if constexpr (std::is_invocable_r<bool, M, ast_node_type, const std::string&>::value) {
        return matcher(tree.get_node()->type, tree.get_node()->value);
    } else if constexpr (std::is_invocable_r<bool, M, const as_tree&>::value) {
        return matcher(tree);
    } else if constexpr (std::is_same_v<std::decay_t<M>, ast_node_type>) {
        return tree.get_node()->type == matcher;
    }
}

template< typename M, typename... Ms >
bool match_impl(const ast_children& children, size_t n, M&& matcher, Ms&&... rest) {
    if (n >= children.size()) {
        return false;
    }
    return match_one(*children[n], std::forward<M>(matcher))
        && match_impl(children, n + 1, std::forward<Ms>(rest)...);
}

inline bool match_impl(const ast_children&, size_t) {
    return true;
}

}
#endif /* _MATCHER_IMPL_H */