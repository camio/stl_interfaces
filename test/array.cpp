// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/stl_interfaces/container_interface.hpp>

#include "ill_formed.hpp"

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <vector>


// Just like std::array, except for the 0-size specialization, and the fact
// that the base class makes brace-initialization wonky.
template<typename T, std::size_t N>
struct array
    : boost::stl_interfaces::
          container_interface<array<T, N>, boost::stl_interfaces::contiguous>
{
    using value_type = T;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = value_type &;
    using const_reference = value_type const &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = T *;
    using const_iterator = T const *;
    using reverse_iterator = boost::stl_interfaces::reverse_iterator<iterator>;
    using const_reverse_iterator =
        boost::stl_interfaces::reverse_iterator<const_iterator>;

    void fill(T const & x) { std::fill(begin(), end(), x); }

    iterator begin() noexcept { return elements_; }
    iterator end() noexcept { return elements_ + N; }

    size_type max_size() const noexcept { return N; }

    void swap(array & other)
    {
        using std::swap;
        T * element = elements_;
        for (auto & x : other) {
            swap(*element++, x);
        }
    }

    using base_type = boost::stl_interfaces::
        container_interface<array<T, N>, boost::stl_interfaces::contiguous>;
    using base_type::begin;
    using base_type::end;

    T elements_[N];
};

using arr_type = array<int, 5>;


TEST(array, comparisons)
{
    arr_type sm;
    sm[0] = 1;
    sm[1] = 2;
    sm[2] = 3;
    sm[3] = 0;
    sm[4] = 0;
    arr_type md;
    md[0] = 1;
    md[1] = 2;
    md[2] = 3;
    md[3] = 4;
    md[4] = 0;
    arr_type lg;
    lg[0] = 1;
    lg[1] = 2;
    lg[2] = 3;
    lg[3] = 4;
    lg[4] = 5;

    EXPECT_TRUE(sm == sm);
    EXPECT_FALSE(sm == md);
    EXPECT_FALSE(sm == lg);

    EXPECT_FALSE(sm != sm);
    EXPECT_TRUE(sm != md);
    EXPECT_TRUE(sm != lg);

    EXPECT_FALSE(sm < sm);
    EXPECT_TRUE(sm < md);
    EXPECT_TRUE(sm < lg);

    EXPECT_TRUE(sm <= sm);
    EXPECT_TRUE(sm <= md);
    EXPECT_TRUE(sm <= lg);

    EXPECT_FALSE(sm > sm);
    EXPECT_FALSE(sm > md);
    EXPECT_FALSE(sm > lg);

    EXPECT_TRUE(sm >= sm);
    EXPECT_FALSE(sm >= md);
    EXPECT_FALSE(sm >= lg);


    EXPECT_FALSE(md == sm);
    EXPECT_TRUE(md == md);
    EXPECT_FALSE(md == lg);

    EXPECT_FALSE(md < sm);
    EXPECT_FALSE(md < md);
    EXPECT_TRUE(md < lg);

    EXPECT_FALSE(md <= sm);
    EXPECT_TRUE(md <= md);
    EXPECT_TRUE(md <= lg);

    EXPECT_TRUE(md > sm);
    EXPECT_FALSE(md > md);
    EXPECT_FALSE(md > lg);

    EXPECT_TRUE(md >= sm);
    EXPECT_TRUE(md >= md);
    EXPECT_FALSE(md >= lg);


    EXPECT_FALSE(lg == sm);
    EXPECT_FALSE(lg == md);
    EXPECT_TRUE(lg == lg);

    EXPECT_FALSE(lg < sm);
    EXPECT_FALSE(lg < md);
    EXPECT_FALSE(lg < lg);

    EXPECT_FALSE(lg <= sm);
    EXPECT_FALSE(lg <= md);
    EXPECT_TRUE(lg <= lg);

    EXPECT_TRUE(lg > sm);
    EXPECT_TRUE(lg > md);
    EXPECT_FALSE(lg > lg);

    EXPECT_TRUE(lg >= sm);
    EXPECT_TRUE(lg >= md);
    EXPECT_TRUE(lg >= lg);
}

TEST(array, swap)
{
    {
        arr_type v1;
        v1[0] = 3;
        v1[1] = 4;
        v1[2] = 0;
        v1[3] = 0;
        v1[4] = 0;
        arr_type v2;
        v2[0] = 4;
        v2[1] = 3;
        v2[2] = 0;
        v2[3] = 0;
        v2[4] = 0;

        arr_type const v1_copy = v1;
        arr_type const v2_copy = v2;

        static_assert(std::is_same<decltype(v1.swap(v2)), void>::value, "");
        static_assert(std::is_same<decltype(swap(v1, v2)), void>::value, "");

        v1.swap(v2);

        EXPECT_EQ(v1, v2_copy);
        EXPECT_EQ(v2, v1_copy);
    }

    {
        arr_type v1;
        v1[0] = 3;
        v1[1] = 4;
        v1[2] = 0;
        v1[3] = 0;
        v1[4] = 0;
        arr_type v2;
        v2[0] = 4;
        v2[1] = 3;
        v2[2] = 0;
        v2[3] = 0;
        v2[4] = 0;

        arr_type const v1_copy = v1;
        arr_type const v2_copy = v2;

        swap(v1, v2);

        EXPECT_EQ(v1, v2_copy);
        EXPECT_EQ(v2, v1_copy);
    }
}

template<typename Iter>
using writable_iter_t = decltype(
    *std::declval<Iter>() =
        std::declval<typename std::iterator_traits<Iter>::value_type>());

static_assert(
    !ill_formed<writable_iter_t, decltype(std::declval<arr_type &>().begin())>::
        value,
    "");
static_assert(
    !ill_formed<writable_iter_t, decltype(std::declval<arr_type &>().end())>::
        value,
    "");

static_assert(
    ill_formed<
        writable_iter_t,
        decltype(std::declval<arr_type const &>().begin())>::value,
    "");
static_assert(
    ill_formed<
        writable_iter_t,
        decltype(std::declval<arr_type const &>().end())>::value,
    "");
static_assert(
    ill_formed<writable_iter_t, decltype(std::declval<arr_type &>().cbegin())>::
        value,
    "");
static_assert(
    ill_formed<writable_iter_t, decltype(std::declval<arr_type &>().cend())>::
        value,
    "");

static_assert(
    ill_formed<
        writable_iter_t,
        decltype(std::declval<arr_type const &>().rbegin())>::value,
    "");
static_assert(
    ill_formed<
        writable_iter_t,
        decltype(std::declval<arr_type const &>().rend())>::value,
    "");
static_assert(
    ill_formed<
        writable_iter_t,
        decltype(std::declval<arr_type &>().crbegin())>::value,
    "");
static_assert(
    ill_formed<writable_iter_t, decltype(std::declval<arr_type &>().crend())>::
        value,
    "");

TEST(array, iterators)
{
    arr_type v0;
    v0[0] = 3;
    v0[1] = 2;
    v0[2] = 1;
    v0[3] = 0;
    v0[4] = 0;

    {
        arr_type v = v0;

        static_assert(
            std::is_same<decltype(v.begin()), arr_type::iterator>::value, "");
        static_assert(
            std::is_same<decltype(v.end()), arr_type::iterator>::value, "");
        static_assert(
            std::is_same<decltype(v.cbegin()), arr_type::const_iterator>::value,
            "");
        static_assert(
            std::is_same<decltype(v.cend()), arr_type::const_iterator>::value,
            "");
        static_assert(
            std::is_same<decltype(v.rbegin()), arr_type::reverse_iterator>::
                value,
            "");
        static_assert(
            std::is_same<decltype(v.rbegin()), arr_type::reverse_iterator>::
                value,
            "");
        static_assert(
            std::is_same<
                decltype(v.crbegin()),
                arr_type::const_reverse_iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(v.crbegin()),
                arr_type::const_reverse_iterator>::value,
            "");

        std::array<int, 5> const a = {{3, 2, 1, 0, 0}};
        std::array<int, 5> const ra = {{0, 0, 1, 2, 3}};

        EXPECT_TRUE(std::equal(v.begin(), v.end(), a.begin(), a.end()));
        EXPECT_TRUE(std::equal(v.cbegin(), v.cend(), a.begin(), a.end()));

        EXPECT_TRUE(std::equal(v.rbegin(), v.rend(), ra.begin(), ra.end()));
        EXPECT_TRUE(std::equal(v.crbegin(), v.crend(), ra.begin(), ra.end()));

        arr_type v2;
        v2[0] = 8;
        v2[1] = 2;
        v2[2] = 1;
        v2[3] = 0;
        v2[4] = 9;

        *v.begin() = 8;
        *v.rbegin() = 9;
        EXPECT_EQ(v, v2);
    }

    {
        arr_type const v = v0;

        static_assert(
            std::is_same<decltype(v.begin()), arr_type::const_iterator>::value,
            "");
        static_assert(
            std::is_same<decltype(v.end()), arr_type::const_iterator>::value,
            "");
        static_assert(
            std::is_same<decltype(v.cbegin()), arr_type::const_iterator>::value,
            "");
        static_assert(
            std::is_same<decltype(v.cend()), arr_type::const_iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(v.rbegin()),
                arr_type::const_reverse_iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(v.rbegin()),
                arr_type::const_reverse_iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(v.crbegin()),
                arr_type::const_reverse_iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(v.crbegin()),
                arr_type::const_reverse_iterator>::value,
            "");

        std::array<int, 5> const a = {{3, 2, 1, 0, 0}};
        std::array<int, 5> const ra = {{0, 0, 1, 2, 3}};

        EXPECT_TRUE(std::equal(v.begin(), v.end(), a.begin(), a.end()));
        EXPECT_TRUE(std::equal(v.cbegin(), v.cend(), a.begin(), a.end()));

        EXPECT_TRUE(std::equal(v.rbegin(), v.rend(), ra.begin(), ra.end()));
        EXPECT_TRUE(std::equal(v.crbegin(), v.crend(), ra.begin(), ra.end()));
    }
}

template<
    typename Container,
    typename ValueType = typename Container::value_type>
using lvalue_push_front_t = decltype(
    std::declval<Container &>().push_front(std::declval<ValueType const &>()));
template<
    typename Container,
    typename ValueType = typename Container::value_type>
using rvalue_push_front_t = decltype(std::declval<Container &>().push_front(0));
template<typename Container>
using pop_front_t = decltype(std::declval<Container &>().pop_front());

static_assert(ill_formed<lvalue_push_front_t, arr_type>::value, "");
static_assert(ill_formed<rvalue_push_front_t, arr_type>::value, "");
static_assert(ill_formed<pop_front_t, arr_type>::value, "");

using std_deq_int = std::deque<int>;

static_assert(!ill_formed<lvalue_push_front_t, std_deq_int>::value, "");
static_assert(!ill_formed<rvalue_push_front_t, std_deq_int>::value, "");
static_assert(!ill_formed<pop_front_t, std_deq_int>::value, "");


template<
    typename Container,
    typename ValueType = typename Container::value_type>
using lvalue_push_back_t = decltype(
    std::declval<Container &>().push_back(std::declval<ValueType const &>()));
template<
    typename Container,
    typename ValueType = typename Container::value_type>
using rvalue_push_back_t = decltype(std::declval<Container &>().push_back(0));
template<typename Container>
using pop_back_t = decltype(std::declval<Container &>().pop_back());

static_assert(ill_formed<lvalue_push_back_t, arr_type>::value, "");
static_assert(ill_formed<rvalue_push_back_t, arr_type>::value, "");
static_assert(ill_formed<pop_back_t, arr_type>::value, "");

using std_vec_int = std::vector<int>;

static_assert(!ill_formed<lvalue_push_back_t, std_vec_int>::value, "");
static_assert(!ill_formed<rvalue_push_back_t, std_vec_int>::value, "");
static_assert(!ill_formed<pop_back_t, std_vec_int>::value, "");

TEST(array, front_back)
{
    {
        arr_type v;
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
        v[3] = 0;
        v[4] = 0;

        static_assert(std::is_same<decltype(v.front()), int &>::value, "");
        static_assert(std::is_same<decltype(v.back()), int &>::value, "");

        v.front() = 9;
        v.back() = 8;
        EXPECT_EQ(v[0], v.front());
        EXPECT_EQ(v[4], v.back());
    }

    {
        arr_type v0;
        v0[0] = 3;
        v0[1] = 0;
        v0[2] = 2;
        v0[3] = 0;
        v0[4] = 1;

        arr_type const v = v0;
        EXPECT_EQ(v.front(), 3);
        EXPECT_EQ(v.back(), 1);

        static_assert(
            std::is_same<decltype(v.front()), int const &>::value, "");
        static_assert(std::is_same<decltype(v.back()), int const &>::value, "");
    }
}

TEST(array, index_at)
{
    arr_type v0;
    v0[0] = 3;
    v0[1] = 2;
    v0[2] = 1;
    v0[3] = 0;
    v0[4] = 0;

    {
        arr_type v = v0;
        EXPECT_EQ(v[0], 3);
        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v[2], 1);
        EXPECT_NO_THROW(v.at(0));
        EXPECT_NO_THROW(v.at(1));
        EXPECT_NO_THROW(v.at(2));
        EXPECT_THROW(v.at(5), std::out_of_range);

        static_assert(std::is_same<decltype(v[0]), int &>::value, "");
        static_assert(std::is_same<decltype(v.at(0)), int &>::value, "");

        v[0] = 8;
        v.at(1) = 9;
        EXPECT_EQ(v[0], 8);
        EXPECT_EQ(v[1], 9);
    }

    {
        arr_type const v = v0;
        EXPECT_EQ(v[0], 3);
        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v[2], 1);
        EXPECT_NO_THROW(v.at(0));
        EXPECT_NO_THROW(v.at(1));
        EXPECT_NO_THROW(v.at(2));
        EXPECT_THROW(v.at(5), std::out_of_range);

        static_assert(std::is_same<decltype(v[0]), int const &>::value, "");
        static_assert(std::is_same<decltype(v.at(0)), int const &>::value, "");
    }
}

template<typename Container>
using resize_t = decltype(std::declval<Container &>().resize(0));

static_assert(ill_formed<resize_t, arr_type>::value, "");

static_assert(!ill_formed<resize_t, std_vec_int>::value, "");

template<
    typename Container,
    typename ValueType = typename Container::value_type>
using lvalue_insert_t = decltype(std::declval<Container &>().insert(
    std::declval<Container &>().begin(), std::declval<ValueType const &>()));
template<
    typename Container,
    typename ValueType = typename Container::value_type>
using rvalue_insert_t = decltype(std::declval<Container &>().insert(
    std::declval<Container &>().begin(), std::declval<ValueType &&>()));
template<
    typename Container,
    typename ValueType = typename Container::value_type>
using insert_n_t = decltype(std::declval<Container &>().insert(
    std::declval<Container &>().begin(), 2, std::declval<ValueType const &>()));
template<
    typename Container,
    typename ValueType = typename Container::value_type>
using insert_il_t = decltype(std::declval<Container &>().insert(
    std::declval<Container &>().begin(), std::initializer_list<int>{}));

static_assert(ill_formed<lvalue_insert_t, arr_type>::value, "");
static_assert(ill_formed<rvalue_insert_t, arr_type>::value, "");

// TODO: Broken in v1.  Adding the proper constraint ICE's GCC 8, or
// infinitely recurses within the compiler for GCC and Clang, depending on the
// constraint technique used.
#if 0
static_assert(ill_formed<insert_n_t, arr_type>::value, "");
#endif

static_assert(ill_formed<insert_il_t, arr_type>::value, "");

static_assert(!ill_formed<lvalue_insert_t, std_vec_int>::value, "");
static_assert(!ill_formed<rvalue_insert_t, std_vec_int>::value, "");
static_assert(!ill_formed<insert_n_t, std_vec_int>::value, "");
static_assert(!ill_formed<insert_il_t, std_vec_int>::value, "");


template<typename Container>
using erase_t = decltype(
    std::declval<Container &>().erase(std::declval<Container &>().begin()));

static_assert(ill_formed<erase_t, arr_type>::value, "");

static_assert(!ill_formed<erase_t, std_vec_int>::value, "");


template<typename Container>
using assign_t = decltype(std::declval<Container &>().assign(
    std::declval<int *>(), std::declval<int *>()));
template<typename Container>
using assign_n_t = decltype(std::declval<Container &>().assign(5, 5));
template<typename Container>
using assign_il_t =
    decltype(std::declval<Container &>().assign(std::initializer_list<int>{}));
template<typename Container>
using assignment_operator_il_t =
    decltype(std::declval<Container &>() = std::initializer_list<int>{});

static_assert(ill_formed<assign_t, arr_type>::value, "");
static_assert(ill_formed<assign_n_t, arr_type>::value, "");
static_assert(ill_formed<assign_il_t, arr_type>::value, "");
static_assert(ill_formed<assignment_operator_il_t, arr_type>::value, "");

static_assert(!ill_formed<assign_t, std_vec_int>::value, "");
static_assert(!ill_formed<assign_n_t, std_vec_int>::value, "");
static_assert(!ill_formed<assign_il_t, std_vec_int>::value, "");
static_assert(!ill_formed<assignment_operator_il_t, std_vec_int>::value, "");

template<typename Container>
using clear_t = decltype(std::declval<Container &>().clear());

static_assert(ill_formed<clear_t, arr_type>::value, "");

static_assert(!ill_formed<clear_t, std_vec_int>::value, "");
