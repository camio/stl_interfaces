// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/stl_interfaces/iterator_interface.hpp>

#include "ill_formed.hpp"

#include <gtest/gtest.h>

#include <array>
#include <numeric>
#include <type_traits>


namespace bsi = boost::stl_interfaces::v2;

struct basic_random_access_iter : bsi::iterator_interface<
                                      basic_random_access_iter,
                                      std::random_access_iterator_tag,
                                      int>
{
    basic_random_access_iter() {}
    basic_random_access_iter(int * it) : it_(it) {}

    int & operator*() const { return *it_; }
    basic_random_access_iter & operator+=(std::ptrdiff_t i)
    {
        it_ += i;
        return *this;
    }
    friend std::ptrdiff_t operator-(
        basic_random_access_iter lhs, basic_random_access_iter rhs) noexcept
    {
        return lhs.it_ - rhs.it_;
    }

private:
    int * it_;
};


BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    basic_random_access_iter, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    basic_random_access_iter,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int,
    int &,
    int *,
    std::ptrdiff_t)

static_assert(
    boost::stl_interfaces::v1::v1_dtl::
        plus_eq<basic_random_access_iter, std::ptrdiff_t>::value,
    "");

struct basic_adapted_random_access_iter : bsi::iterator_interface<
                                              basic_adapted_random_access_iter,
                                              std::random_access_iterator_tag,
                                              int>
{
    basic_adapted_random_access_iter() {}
    basic_adapted_random_access_iter(int * it) : it_(it) {}

private:
    friend boost::stl_interfaces::access;
    int *& base_reference() noexcept { return it_; }
    int * base_reference() const noexcept { return it_; }

    int * it_;
};

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    basic_adapted_random_access_iter, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    basic_adapted_random_access_iter,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int,
    int &,
    int *,
    std::ptrdiff_t)

template<typename ValueType>
struct adapted_random_access_iter : bsi::iterator_interface<
                                        adapted_random_access_iter<ValueType>,
                                        std::random_access_iterator_tag,
                                        ValueType>
{
    adapted_random_access_iter() {}
    adapted_random_access_iter(ValueType * it) : it_(it) {}

    template<
        typename ValueType2,
        typename Enable = std::enable_if_t<
            std::is_convertible<ValueType2 *, ValueType *>::value>>
    adapted_random_access_iter(adapted_random_access_iter<ValueType2> other) :
        it_(other.it_)
    {}

    template<typename ValueType2>
    friend struct adapted_random_access_iter;

private:
    friend boost::stl_interfaces::access;
    ValueType *& base_reference() noexcept { return it_; }
    ValueType * base_reference() const noexcept { return it_; }

    ValueType * it_;
};

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    adapted_random_access_iter<int>, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    adapted_random_access_iter<int>,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int,
    int &,
    int *,
    std::ptrdiff_t)
BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    adapted_random_access_iter<int const>, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    adapted_random_access_iter<int const>,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int const,
    int const &,
    int const *,
    std::ptrdiff_t)

template<typename ValueType>
struct random_access_iter : bsi::iterator_interface<
                                random_access_iter<ValueType>,
                                std::random_access_iterator_tag,
                                ValueType>
{
    random_access_iter() {}
    random_access_iter(ValueType * it) : it_(it) {}
    template<
        typename ValueType2,
        typename E = std::enable_if_t<std::is_convertible<
            typename ValueType2::value_type *,
            ValueType *>::value>>
    random_access_iter(ValueType2 it) : it_(it.it_)
    {}

    ValueType & operator*() const { return *it_; }
    random_access_iter & operator+=(std::ptrdiff_t i)
    {
        it_ += i;
        return *this;
    }
    friend std::ptrdiff_t
    operator-(random_access_iter lhs, random_access_iter rhs) noexcept
    {
        return lhs.it_ - rhs.it_;
    }

private:
    ValueType * it_;

    template<typename ValueType2>
    friend struct random_access_iter;
};

using random_access = random_access_iter<int>;
using const_random_access = random_access_iter<int const>;

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    random_access, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    random_access,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int,
    int &,
    int *,
    std::ptrdiff_t)

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    const_random_access, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    const_random_access,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int const,
    int const &,
    int const *,
    std::ptrdiff_t)

struct zip_iter : bsi::proxy_iterator_interface<
                      zip_iter,
                      std::random_access_iterator_tag,
                      std::tuple<int, int>,
                      std::tuple<int &, int &>>
{
    zip_iter() : it1_(nullptr), it2_(nullptr) {}
    zip_iter(int * it1, int * it2) : it1_(it1), it2_(it2) {}

    std::tuple<int &, int &> operator*() const
    {
        return std::tuple<int &, int &>{*it1_, *it2_};
    }
    zip_iter & operator+=(std::ptrdiff_t i)
    {
        it1_ += i;
        it2_ += i;
        return *this;
    }
    friend std::ptrdiff_t operator-(zip_iter lhs, zip_iter rhs) noexcept
    {
        return lhs.it1_ - rhs.it1_;
    }

private:
    int * it1_;
    int * it2_;
};

using int_pair = std::tuple<int, int>;
using int_refs_pair = std::tuple<int &, int &>;

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    zip_iter, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    zip_iter,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int_pair,
    int_refs_pair,
    boost::stl_interfaces::proxy_arrow_result<int_refs_pair>,
    std::ptrdiff_t)

struct int_t
{
    int value_;

    bool operator==(int_t other) const { return value_ == other.value_; }
    bool operator!=(int_t other) const { return value_ != other.value_; }
    bool operator<(int_t other) const { return value_ < other.value_; }

    bool operator==(int other) const { return value_ == other; }
    bool operator!=(int other) const { return value_ != other; }
    bool operator<(int other) const { return value_ < other; }

    friend bool operator==(int lhs, int_t rhs) { return lhs == rhs.value_; }
    friend bool operator!=(int lhs, int_t rhs) { return lhs != rhs.value_; }
    friend bool operator<(int lhs, int_t rhs) { return lhs < rhs.value_; }
};

struct udt_zip_iter : bsi::proxy_iterator_interface<
                          udt_zip_iter,
                          std::random_access_iterator_tag,
                          std::tuple<int_t, int>,
                          std::tuple<int_t &, int &>>
{
    udt_zip_iter() : it1_(nullptr), it2_(nullptr) {}
    udt_zip_iter(int_t * it1, int * it2) : it1_(it1), it2_(it2) {}

    std::tuple<int_t &, int &> operator*() const
    {
        return std::tuple<int_t &, int &>{*it1_, *it2_};
    }
    udt_zip_iter & operator+=(std::ptrdiff_t i)
    {
        it1_ += i;
        it2_ += i;
        return *this;
    }
    friend std::ptrdiff_t operator-(udt_zip_iter lhs, udt_zip_iter rhs) noexcept
    {
        return lhs.it1_ - rhs.it1_;
    }

private:
    int_t * it1_;
    int * it2_;
};

using int_t_int_pair = std::tuple<int_t, int>;
using int_t_int_refs_pair = std::tuple<int_t &, int &>;

BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
    udt_zip_iter, bsi::ranges::random_access_iterator)
BOOST_STL_INTERFACES_STATIC_ASSERT_ITERATOR_TRAITS(
    udt_zip_iter,
    std::random_access_iterator_tag,
    std::random_access_iterator_tag,
    int_t_int_pair,
    int_t_int_refs_pair,
    boost::stl_interfaces::proxy_arrow_result<int_t_int_refs_pair>,
    std::ptrdiff_t)

namespace std {
    // Required for std::sort to work with zip_iter.  If zip_iter::reference
    // were not a std::tuple with builtin types as its template parameters, we
    // could have put this in another namespace.
    void swap(zip_iter::reference && lhs, zip_iter::reference && rhs)
    {
        using std::swap;
        swap(std::get<0>(lhs), std::get<0>(rhs));
        swap(std::get<1>(lhs), std::get<1>(rhs));
    }
}

void swap(udt_zip_iter::reference && lhs, udt_zip_iter::reference && rhs)
{
    using std::swap;
    swap(std::get<0>(lhs), std::get<0>(rhs));
    swap(std::get<1>(lhs), std::get<1>(rhs));
}

std::array<int, 10> ints = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}};
std::array<int, 10> ones = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
std::array<std::tuple<int, int>, 10> tuples = {{
    {0, 1},
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 1},
    {7, 1},
    {8, 1},
    {9, 1},
}};

std::array<int_t, 10> udts = {
    {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}}};
std::array<std::tuple<int_t, int>, 10> udt_tuples = {{
    std::tuple<int_t, int>{{0}, 1},
    std::tuple<int_t, int>{{1}, 1},
    std::tuple<int_t, int>{{2}, 1},
    std::tuple<int_t, int>{{3}, 1},
    std::tuple<int_t, int>{{4}, 1},
    std::tuple<int_t, int>{{5}, 1},
    std::tuple<int_t, int>{{6}, 1},
    std::tuple<int_t, int>{{7}, 1},
    std::tuple<int_t, int>{{8}, 1},
    std::tuple<int_t, int>{{9}, 1},
}};

TEST(random_access, basic_coverage)
{
    basic_random_access_iter first(ints.data());
    basic_random_access_iter last(ints.data() + ints.size());

    EXPECT_EQ(*first, 0);
    EXPECT_EQ(*(first + 1), 1);
    EXPECT_EQ(*(first + 2), 2);
    EXPECT_EQ(*(1 + first), 1);
    EXPECT_EQ(*(2 + first), 2);

    EXPECT_EQ(first[0], 0);
    EXPECT_EQ(first[1], 1);
    EXPECT_EQ(first[2], 2);

    EXPECT_EQ(*(last - 1), 9);
    EXPECT_EQ(*(last - 2), 8);
    EXPECT_EQ(*(last - 3), 7);

    EXPECT_EQ(last[-1], 9);
    EXPECT_EQ(last[-2], 8);
    EXPECT_EQ(last[-3], 7);

    EXPECT_EQ(last - first, 10);
    EXPECT_EQ(first, first);
    EXPECT_NE(first, last);
    EXPECT_LT(first, last);
    EXPECT_LE(first, last);
    EXPECT_LE(first, first);
    EXPECT_GT(last, first);
    EXPECT_GE(last, first);
    EXPECT_GE(last, last);

    {
        auto first_copy = first;
        first_copy += 10;
        EXPECT_EQ(first_copy, last);
    }

    {
        auto last_copy = last;
        last_copy -= 10;
        EXPECT_EQ(last_copy, first);
    }
}

TEST(random_access, basic_std_copy)
{
    {
        std::array<int, 10> ints_copy;
        basic_random_access_iter first(ints.data());
        basic_random_access_iter last(ints.data() + ints.size());
        std::copy(first, last, ints_copy.begin());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        std::array<int, 10> ints_copy;
        basic_random_access_iter first(ints.data());
        basic_random_access_iter last(ints.data() + ints.size());
        std::copy(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            ints_copy.begin());
        std::reverse(ints_copy.begin(), ints_copy.end());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        std::array<int, 10> iota_ints;
        basic_random_access_iter first(iota_ints.data());
        basic_random_access_iter last(iota_ints.data() + iota_ints.size());
        std::iota(first, last, 0);
        EXPECT_EQ(iota_ints, ints);
    }

    {
        std::array<int, 10> iota_ints;
        basic_random_access_iter first(iota_ints.data());
        basic_random_access_iter last(iota_ints.data() + iota_ints.size());
        std::iota(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            0);
        std::reverse(iota_ints.begin(), iota_ints.end());
        EXPECT_EQ(iota_ints, ints);
    }

    {
        std::array<int, 10> iota_ints;
        basic_random_access_iter first(iota_ints.data());
        basic_random_access_iter last(iota_ints.data() + iota_ints.size());
        std::iota(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            0);
        std::sort(first, last);
        EXPECT_EQ(iota_ints, ints);
    }
}

TEST(random_access, basic_adapted_coverage)
{
    basic_adapted_random_access_iter first(ints.data());
    basic_adapted_random_access_iter last(ints.data() + ints.size());

    EXPECT_EQ(*first, 0);
    EXPECT_EQ(*(first + 1), 1);
    EXPECT_EQ(*(first + 2), 2);
    EXPECT_EQ(*(1 + first), 1);
    EXPECT_EQ(*(2 + first), 2);

    EXPECT_EQ(first[0], 0);
    EXPECT_EQ(first[1], 1);
    EXPECT_EQ(first[2], 2);

    EXPECT_EQ(*(last - 1), 9);
    EXPECT_EQ(*(last - 2), 8);
    EXPECT_EQ(*(last - 3), 7);

    EXPECT_EQ(last[-1], 9);
    EXPECT_EQ(last[-2], 8);
    EXPECT_EQ(last[-3], 7);

    EXPECT_EQ(last - first, 10);
    EXPECT_EQ(first, first);
    EXPECT_NE(first, last);
    EXPECT_LT(first, last);
    EXPECT_LE(first, last);
    EXPECT_LE(first, first);
    EXPECT_GT(last, first);
    EXPECT_GE(last, first);
    EXPECT_GE(last, last);

    {
        auto first_copy = first;
        first_copy += 10;
        EXPECT_EQ(first_copy, last);
    }

    {
        auto last_copy = last;
        last_copy -= 10;
        EXPECT_EQ(last_copy, first);
    }
}

TEST(random_access, basic_adapted_std_copy)
{
    {
        std::array<int, 10> ints_copy;
        basic_adapted_random_access_iter first(ints.data());
        basic_adapted_random_access_iter last(ints.data() + ints.size());
        std::copy(first, last, ints_copy.begin());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        std::array<int, 10> ints_copy;
        basic_adapted_random_access_iter first(ints.data());
        basic_adapted_random_access_iter last(ints.data() + ints.size());
        std::copy(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            ints_copy.begin());
        std::reverse(ints_copy.begin(), ints_copy.end());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        std::array<int, 10> iota_ints;
        basic_adapted_random_access_iter first(iota_ints.data());
        basic_adapted_random_access_iter last(
            iota_ints.data() + iota_ints.size());
        std::iota(first, last, 0);
        EXPECT_EQ(iota_ints, ints);
    }

    {
        std::array<int, 10> iota_ints;
        basic_adapted_random_access_iter first(iota_ints.data());
        basic_adapted_random_access_iter last(
            iota_ints.data() + iota_ints.size());
        std::iota(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            0);
        std::reverse(iota_ints.begin(), iota_ints.end());
        EXPECT_EQ(iota_ints, ints);
    }

    {
        std::array<int, 10> iota_ints;
        basic_adapted_random_access_iter first(iota_ints.data());
        basic_adapted_random_access_iter last(
            iota_ints.data() + iota_ints.size());
        std::iota(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            0);
        std::sort(first, last);
        EXPECT_EQ(iota_ints, ints);
    }
}

TEST(random_access, mutable_to_const_conversions)
{
    {
        random_access first(ints.data());
        random_access last(ints.data() + ints.size());
        const_random_access first_copy(first);
        const_random_access last_copy(last);
        std::equal(first, last, first_copy, last_copy);
    }

    {
        adapted_random_access_iter<int> first(ints.data());
        adapted_random_access_iter<int> last(ints.data() + ints.size());
        adapted_random_access_iter<int const> first_copy(
            (int const *)ints.data());
        adapted_random_access_iter<int const> last_copy(
            (int const *)ints.data() + ints.size());
        std::equal(first, last, first_copy, last_copy);
    }
}

TEST(random_access, mutable_to_const_comparisons)
{
    {
        random_access first(ints.data());
        random_access last(ints.data() + ints.size());
        const_random_access first_const(first);
        const_random_access last_const(last);

        EXPECT_EQ(first, first_const);
        EXPECT_EQ(first_const, first);
        EXPECT_NE(first, last_const);
        EXPECT_NE(last_const, first);
        EXPECT_LE(first, first_const);
        EXPECT_LE(first_const, first);
        EXPECT_GE(first, first_const);
        EXPECT_GE(first_const, first);
        EXPECT_GT(last_const, first);
        EXPECT_GT(last, first_const);
        EXPECT_LT(first_const, last);
        EXPECT_LT(first, last_const);
    }

    {
        adapted_random_access_iter<int> first(ints.data());
        adapted_random_access_iter<int> last(ints.data() + ints.size());
        adapted_random_access_iter<int const> first_const(first);
        adapted_random_access_iter<int const> last_const(last);

        EXPECT_EQ(first, first_const);
        EXPECT_EQ(first_const, first);
        EXPECT_NE(first, last_const);
        EXPECT_NE(last_const, first);
        EXPECT_LE(first, first_const);
        EXPECT_LE(first_const, first);
        EXPECT_GE(first, first_const);
        EXPECT_GE(first_const, first);
        EXPECT_GT(last_const, first);
        EXPECT_GT(last, first_const);
        EXPECT_LT(first_const, last);
        EXPECT_LT(first, last_const);
    }
}

TEST(random_access, postincrement_preincrement)
{
    {
        random_access first(ints.data());
        random_access last(ints.data() + ints.size());
        while (first != last)
            first++;
    }

    {
        random_access first(ints.data());
        random_access last(ints.data() + ints.size());
        while (first != last)
            last--;
    }

    {
        basic_random_access_iter first(ints.data());
        basic_random_access_iter last(ints.data() + ints.size());
        while (first != last)
            first++;
    }

    {
        basic_random_access_iter first(ints.data());
        basic_random_access_iter last(ints.data() + ints.size());
        while (first != last)
            last--;
    }

    {
        basic_adapted_random_access_iter first(ints.data());
        basic_adapted_random_access_iter last(ints.data() + ints.size());
        while (first != last)
            first++;
    }

    {
        basic_adapted_random_access_iter first(ints.data());
        basic_adapted_random_access_iter last(ints.data() + ints.size());
        while (first != last)
            last--;
    }
}

TEST(random_access, coverage)
{
    random_access first(ints.data());
    random_access last(ints.data() + ints.size());

    EXPECT_EQ(*first, 0);
    EXPECT_EQ(*(first + 1), 1);
    EXPECT_EQ(*(first + 2), 2);
    EXPECT_EQ(*(1 + first), 1);
    EXPECT_EQ(*(2 + first), 2);

    EXPECT_EQ(first[0], 0);
    EXPECT_EQ(first[1], 1);
    EXPECT_EQ(first[2], 2);

    EXPECT_EQ(*(last - 1), 9);
    EXPECT_EQ(*(last - 2), 8);
    EXPECT_EQ(*(last - 3), 7);

    EXPECT_EQ(last[-1], 9);
    EXPECT_EQ(last[-2], 8);
    EXPECT_EQ(last[-3], 7);

    EXPECT_EQ(last - first, 10);
    EXPECT_EQ(first, first);
    EXPECT_NE(first, last);
    EXPECT_LT(first, last);
    EXPECT_LE(first, last);
    EXPECT_LE(first, first);
    EXPECT_GT(last, first);
    EXPECT_GE(last, first);
    EXPECT_GE(last, last);

    {
        auto first_copy = first;
        first_copy += 10;
        EXPECT_EQ(first_copy, last);
    }

    {
        auto last_copy = last;
        last_copy -= 10;
        EXPECT_EQ(last_copy, first);
    }
}

TEST(random_access, std_copy)
{
    random_access first(ints.data());
    random_access last(ints.data() + ints.size());

    {
        std::array<int, 10> ints_copy;
        std::copy(first, last, ints_copy.begin());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        std::array<int, 10> ints_copy;
        std::copy(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            ints_copy.begin());
        std::reverse(ints_copy.begin(), ints_copy.end());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        std::array<int, 10> iota_ints;
        random_access first(iota_ints.data());
        random_access last(iota_ints.data() + iota_ints.size());
        std::iota(first, last, 0);
        EXPECT_EQ(iota_ints, ints);
    }

    {
        std::array<int, 10> iota_ints;
        random_access first(iota_ints.data());
        random_access last(iota_ints.data() + iota_ints.size());
        std::iota(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            0);
        std::reverse(iota_ints.begin(), iota_ints.end());
        EXPECT_EQ(iota_ints, ints);
    }

    {
        std::array<int, 10> iota_ints;
        random_access first(iota_ints.data());
        random_access last(iota_ints.data() + iota_ints.size());
        std::iota(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            0);
        std::sort(first, last);
        EXPECT_EQ(iota_ints, ints);
    }
}

TEST(random_access, const_std_copy)
{
    const_random_access first(ints.data());
    const_random_access last(ints.data() + ints.size());

    {
        std::array<int, 10> ints_copy;
        std::copy(first, last, ints_copy.begin());
        EXPECT_EQ(ints_copy, ints);
    }

    {
        EXPECT_TRUE(std::binary_search(first, last, 3));
        EXPECT_TRUE(std::binary_search(
            std::make_reverse_iterator(last),
            std::make_reverse_iterator(first),
            3,
            std::greater<>{}));
    }
}

TEST(random_access, zip)
{
    {
        zip_iter first(ints.data(), ones.data());
        zip_iter last(ints.data() + ints.size(), ones.data() + ones.size());
        EXPECT_TRUE(std::equal(first, last, tuples.begin(), tuples.end()));
    }

    {
        auto ints_copy = ints;
        std::reverse(ints_copy.begin(), ints_copy.end());
        auto ones_copy = ones;
        zip_iter first(ints_copy.data(), ones_copy.data());
        zip_iter last(
            ints_copy.data() + ints_copy.size(),
            ones_copy.data() + ones_copy.size());
        EXPECT_FALSE(std::equal(first, last, tuples.begin(), tuples.end()));
        std::sort(first, last);
        EXPECT_TRUE(std::equal(first, last, tuples.begin(), tuples.end()));
    }

    {
        udt_zip_iter first(udts.data(), ones.data());
        udt_zip_iter last(udts.data() + udts.size(), ones.data() + ones.size());
        EXPECT_TRUE(
            std::equal(first, last, udt_tuples.begin(), udt_tuples.end()));
    }

    {
        auto udts_copy = udts;
        std::reverse(udts_copy.begin(), udts_copy.end());
        auto ones_copy = ones;
        udt_zip_iter first(udts_copy.data(), ones_copy.data());
        udt_zip_iter last(
            udts_copy.data() + udts_copy.size(),
            ones_copy.data() + ones_copy.size());
        EXPECT_FALSE(
            std::equal(first, last, udt_tuples.begin(), udt_tuples.end()));
        std::sort(first, last);
        EXPECT_TRUE(
            std::equal(first, last, udt_tuples.begin(), udt_tuples.end()));
    }
}
