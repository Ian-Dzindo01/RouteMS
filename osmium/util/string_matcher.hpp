#ifndef OSMIUM_UTIL_STRING_MATCHER_HPP
#define OSMIUM_UTIL_STRING_MATCHER_HPP

/*

This file is part of Osmium (https://osmcode.org/libosmium).

Copyright 2013-2023 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <algorithm>
#include <cstring>
#include <iosfwd>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#ifdef __has_include
# if __has_include(<variant>)
#  include <variant>
#  ifdef __cpp_lib_variant
#   define OSMIUM_USE_STD_VARIANT
#  endif
# endif
#endif

#ifndef OSMIUM_USE_STD_VARIANT
# include <boost/variant.hpp>
#endif


// std::regex isn't implemented properly in glibc++ (before the version
// delivered with GCC 4.9) and libc++ before the version 3.6, so the use is
// disabled by these checks. Checks for GLIBC were based on
// https://stackoverflow.com/questions/12530406/is-gcc-4-8-or-earlier-buggy-about-regular-expressions
// Checks for libc++ are simply based on compiler defines. This is probably
// not optimal but seems to work for now.
#if defined(__GLIBCXX__)
# if ((__cplusplus >= 201402L) || \
        defined(_GLIBCXX_REGEX_DFS_QUANTIFIERS_LIMIT) || \
        defined(_GLIBCXX_REGEX_STATE_LIMIT))
#  define OSMIUM_WITH_REGEX
# else
#  pragma message("Disabling regex functionality. See source code for info.")
# endif
#elif defined(__clang__)
# if ((__clang_major__ > 3) || \
      (__clang_minor__ == 3 && __clang_minor__ > 5))
#  define OSMIUM_WITH_REGEX
# else
#  pragma message("Disabling regex functionality")
# endif
#endif

namespace osmium {

    /**
     * Implements various string matching functions.
     */
    class StringMatcher {

    public:

        // Parent class for all matcher classes. Used for enable_if check.
        class matcher {
        };

        /**
         * Never matches.
         */
        class always_false : public matcher {

        public:

            static bool match(const char* /*test_string*/) noexcept {
                return false;
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "always_false";
            }

        }; // class always_false

        /**
         * Always matches.
         */
        class always_true : public matcher {

        public:

            static bool match(const char* /*test_string*/) noexcept {
                return true;
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "always_true";
            }

        }; // class always_true

        /**
         * Matches if the test string is equal to the stored string.
         */
        class equal : public matcher {

            std::string m_str;

        public:

            explicit equal(std::string str) :
                m_str(std::move(str)) {
            }

            explicit equal(const char* str) :
                m_str(str) {
            }

            bool match(const char* test_string) const noexcept {
                return !std::strcmp(m_str.c_str(), test_string);
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "equal[" << m_str << ']';
            }

        }; // class equal

        /**
         * Matches if the test string starts with the stored string.
         */
        class prefix : public matcher {

            std::string m_str;

        public:

            explicit prefix(std::string str) :
                m_str(std::move(str)) {
            }

            explicit prefix(const char* str) :
                m_str(str) {
            }

            bool match(const char* test_string) const noexcept {
                return m_str.compare(0, std::string::npos, test_string, 0, m_str.size()) == 0;
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "prefix[" << m_str << ']';
            }

        }; // class prefix

        /**
         * Matches if the test string is a substring of the stored string.
         */
        class substring : public matcher {

            std::string m_str;

        public:

            explicit substring(std::string str) :
                m_str(std::move(str)) {
            }

            explicit substring(const char* str) :
                m_str(str) {
            }

            bool match(const char* test_string) const noexcept {
                return std::strstr(test_string, m_str.c_str()) != nullptr;
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "substring[" << m_str << ']';
            }

        }; // class substring

#ifdef OSMIUM_WITH_REGEX
        /**
         * Matches if the test string matches the regular expression.
         */
        class regex : public matcher {

            std::regex m_regex;

        public:

            explicit regex(std::regex regex) :
                m_regex(std::move(regex)) {
            }

            bool match(const char* test_string) const noexcept {
                return std::regex_search(test_string, m_regex);
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "regex";
            }

        }; // class regex
#endif

        /**
         * Matches if the test string is equal to any of the stored strings.
         */
        class list : public matcher {

            std::vector<std::string> m_strings;

        public:

            explicit list() = default;

            explicit list(std::vector<std::string> strings) :
                m_strings(std::move(strings)) {
            }

            list& add_string(const char* str) {
                m_strings.emplace_back(str);
                return *this;
            }

            list& add_string(const std::string& str) {
                m_strings.push_back(str);
                return *this;
            }

            bool match(const char* test_string) const noexcept {
                return std::any_of(m_strings.cbegin(), m_strings.cend(),
                                   [&test_string](const std::string& s){
                    return s == test_string;
                });
            }

            template <typename TChar, typename TTraits>
            void print(std::basic_ostream<TChar, TTraits>& out) const {
                out << "list[";
                for (const auto& s : m_strings) {
                    out << '[' << s << ']';
                }
                out << ']';
            }

        }; // class list

    private:

        using matcher_type =
#ifdef OSMIUM_USE_STD_VARIANT
            std::variant
#else
            boost::variant
#endif
                <always_false,
                 always_true,
                 equal,
                 prefix,
                 substring,
#ifdef OSMIUM_WITH_REGEX
                 regex,
#endif
                 list>;

        matcher_type m_matcher;

        class match_visitor
#ifndef OSMIUM_USE_STD_VARIANT
        : public boost::static_visitor<bool>
#endif
        {

            const char* m_str;

        public:

            explicit match_visitor(const char* str) noexcept :
                m_str(str) {
            }

            template <typename TMatcher>
            bool operator()(const TMatcher& t) const noexcept {
                return t.match(m_str);
            }

        }; // class match_visitor

        template <typename TChar, typename TTraits>
        class print_visitor
#ifndef OSMIUM_USE_STD_VARIANT
        : public boost::static_visitor<void>
#endif
        {

            std::basic_ostream<TChar, TTraits>* m_out;

        public:

            explicit print_visitor(std::basic_ostream<TChar, TTraits>& out) :
                m_out(&out) {
            }

            template <typename TMatcher>
            void operator()(const TMatcher& t) const noexcept {
                t.print(*m_out);
            }

        }; // class print_visitor

    public:

        /**
         * Create a string matcher that will never match.
         */
        StringMatcher() :
            m_matcher(always_false{}) {
        }

        /**
         * Create a string matcher that will always or never match based on
         * the argument.
         * Shortcut for
         * @code StringMatcher{StringMatcher::always_true}; @endcode
         * or
         * @code StringMatcher{StringMatcher::always_false}; @endcode
         */
        // cppcheck-suppress noExplicitConstructor
        StringMatcher(bool result) : // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
            m_matcher(always_false{}) {
            if (result) {
                m_matcher = always_true{};
            }
        }

        /**
         * Create a string matcher that will match the specified string.
         * Shortcut for
         * @code StringMatcher{StringMatcher::equal{str}}; @endcode
         */
        // cppcheck-suppress noExplicitConstructor
        StringMatcher(const char* str) : // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
            m_matcher(equal{str}) {
        }

        /**
         * Create a string matcher that will match the specified string.
         * Shortcut for
         * @code StringMatcher{StringMatcher::equal{str}}; @endcode
         */
        // cppcheck-suppress noExplicitConstructor
        StringMatcher(const std::string& str) : // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
            m_matcher(equal{str}) {
        }

#ifdef OSMIUM_WITH_REGEX
        /**
         * Create a string matcher that will match the specified regex.
         * Shortcut for
         * @code StringMatcher{StringMatcher::regex{aregex}}; @endcode
         */
        // cppcheck-suppress noExplicitConstructor
        StringMatcher(const std::regex& aregex) : // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
            m_matcher(regex{aregex}) {
        }
#endif

        /**
         * Create a string matcher that will match if any of the strings
         * match.
         * Shortcut for
         * @code StringMatcher{StringMatcher::list{strings}}; @endcode
         */
        // cppcheck-suppress noExplicitConstructor
        StringMatcher(const std::vector<std::string>& strings) : // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
            m_matcher(list{strings}) {
        }

        /**
         * Create a string matcher.
         *
         * @tparam TMatcher Must be one of the matcher classes
         *                  osmium::StringMatcher::always_false, always_true,
         *                  equal, prefix, substring, regex or list.
         */
        // cppcheck-suppress noExplicitConstructor
        template <typename TMatcher, typename X = typename std::enable_if<
            std::is_base_of<matcher, TMatcher>::value, void>::type>
        StringMatcher(TMatcher&& matcher) : // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
            m_matcher(std::forward<TMatcher>(matcher)) {
        }

        /**
         * Match the specified string.
         */
        bool operator()(const char* str) const noexcept {
#ifdef OSMIUM_USE_STD_VARIANT
            return std::visit(match_visitor{str}, m_matcher);
#else
            return boost::apply_visitor(match_visitor{str}, m_matcher);
#endif
        }

        /**
         * Match the specified string.
         */
        bool operator()(const std::string& str) const noexcept {
            return operator()(str.c_str());
        }

        template <typename TChar, typename TTraits>
        void print(std::basic_ostream<TChar, TTraits>& out) const {
#ifdef OSMIUM_USE_STD_VARIANT
            std::visit(print_visitor<TChar, TTraits>{out}, m_matcher);
#else
            boost::apply_visitor(print_visitor<TChar, TTraits>{out}, m_matcher);
#endif
        }

    }; // class StringMatcher

    template <typename TChar, typename TTraits>
    inline std::basic_ostream<TChar, TTraits>& operator<<(std::basic_ostream<TChar, TTraits>& out, const StringMatcher& matcher) {
        matcher.print(out);
        return out;
    }

} // namespace osmium

#undef OSMIUM_USE_STD_VARIANT

#endif // OSMIUM_UTIL_STRING_MATCHER_HPP
