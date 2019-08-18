// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_STL_INTERFACES_CONTAINER_INTERFACE_HPP
#define BOOST_STL_INTERFACES_CONTAINER_INTERFACE_HPP

#include <boost/stl_interfaces/reverse_iterator.hpp>

#include <stdexcept>


namespace boost { namespace stl_interfaces { inline namespace v1 {

    /** A CRTP template that one may derive from to make it easier to define
        container types. */
    template<
        typename Derived,
        bool Contiguous = discontiguous
#ifndef BOOST_STL_INTERFACES_DOXYGEN
        ,
        typename E = std::enable_if_t<
            std::is_class<Derived>::value &&
            std::is_same<Derived, std::remove_cv_t<Derived>>::value>
#endif
        >
    struct container_interface;

    namespace detail {
        template<typename T, typename SizeType>
        struct n_iter : iterator_interface<
                            n_iter<T, SizeType>,
                            std::random_access_iterator_tag,
                            T>
        {
            n_iter() : x_(nullptr), n_(0) {}
            n_iter(T const & x, SizeType n) : x_(&x), n_(n) {}

            constexpr std::ptrdiff_t operator-(n_iter other) const noexcept
            {
                return std::ptrdiff_t(n_) - std::ptrdiff_t(other.n_);
            }
            n_iter & operator+=(std::ptrdiff_t offset)
            {
                n_ += offset;
                return *this;
            }

        private:
            friend access;
            constexpr T const *& base_reference() noexcept { return x_; }
            constexpr T const * base_reference() const noexcept { return x_; }

            T const * x_;
            SizeType n_;
        };

        template<typename T, typename SizeType>
        constexpr auto make_n_iter(T const & x, SizeType n) noexcept(
            noexcept(n_iter<T, SizeType>(x, n)))
        {
            using result_type = n_iter<T, SizeType>;
            BOOST_STL_INTERFACES_STATIC_ASSERT_CONCEPT(
                result_type, std::random_access_iterator)
            return result_type(x, SizeType(0));
        }
        template<typename T, typename SizeType>
        constexpr auto make_n_iter_end(T const & x, SizeType n) noexcept(
            noexcept(n_iter<T, SizeType>(x, n)))
        {
            return n_iter<T, SizeType>(x, n);
        }

        template<typename Iter>
        using in_iter = std::is_convertible<
            typename std::iterator_traits<Iter>::iterator_category,
            std::input_iterator_tag>;

        template<typename Derived, typename = void>
        struct clear_impl
        {
            static constexpr void call(Derived & d) noexcept {}
        };
        template<typename Derived>
        struct clear_impl<
            Derived,
            void_t<decltype(std::declval<Derived>().clear())>>
        {
            static constexpr void call(Derived & d) noexcept { d.clear(); }
        };

        template<typename Derived, bool Contiguous>
        void
        derived_container(container_interface<Derived, Contiguous> const &);
    }

    template<
        typename Derived,
        bool Contiguous
#ifndef BOOST_STL_INTERFACES_DOXYGEN
        ,
        typename E
#endif
        >
    struct container_interface
    {
#ifndef BOOST_STL_INTERFACES_DOXYGEN
    private:
        constexpr Derived & derived() noexcept
        {
            return static_cast<Derived &>(*this);
        }
        constexpr const Derived & derived() const noexcept
        {
            return static_cast<Derived const &>(*this);
        }
        constexpr Derived & mutable_derived() const noexcept
        {
            return const_cast<Derived &>(static_cast<Derived const &>(*this));
        }
#endif

    public:
        ~container_interface() { detail::clear_impl<Derived>::call(derived()); }

        template<typename D = Derived>
        constexpr auto empty() noexcept(
            noexcept(std::declval<D &>().begin() == std::declval<D &>().end()))
            -> decltype(
                std::declval<D &>().begin() == std::declval<D &>().end())
        {
            return derived().begin() == derived().end();
        }
        template<typename D = Derived>
        constexpr auto empty() const noexcept(noexcept(
            std::declval<D const &>().begin() ==
            std::declval<D const &>().end()))
            -> decltype(
                std::declval<D const &>().begin() ==
                std::declval<D const &>().end())
        {
            return derived().begin() == derived().end();
        }

        template<
            typename D = Derived,
            bool C = Contiguous,
            typename Enable = std::enable_if_t<C>>
        constexpr auto data() noexcept(noexcept(std::declval<D &>().begin()))
            -> decltype(std::addressof(*std::declval<D &>().begin()))
        {
            return std::addressof(*derived().begin());
        }
        template<
            typename D = Derived,
            bool C = Contiguous,
            typename Enable = std::enable_if_t<C>>
        constexpr auto data() const
            noexcept(noexcept(std::declval<D const &>().begin()))
                -> decltype(std::addressof(*std::declval<D const &>().begin()))
        {
            return std::addressof(*derived().begin());
        }

        template<typename D = Derived>
        constexpr auto size() noexcept(
            noexcept(std::declval<D &>().end() - std::declval<D &>().begin()))
            -> decltype(typename D::size_type(
                std::declval<D &>().end() - std::declval<D &>().begin()))
        {
            return derived().end() - derived().begin();
        }
        template<typename D = Derived>
        constexpr auto size() const noexcept(noexcept(
            std::declval<D const &>().end() -
            std::declval<D const &>().begin()))
            -> decltype(typename D::size_type(
                std::declval<D const &>().end() -
                std::declval<D const &>().begin()))
        {
            return derived().end() - derived().begin();
        }

        template<typename D = Derived>
        constexpr auto front() noexcept(noexcept(*std::declval<D &>().begin()))
            -> decltype(*std::declval<D &>().begin())
        {
            return *derived().begin();
        }
        template<typename D = Derived>
        constexpr auto front() const
            noexcept(noexcept(*std::declval<D const &>().begin()))
                -> decltype(*std::declval<D const &>().begin())
        {
            return *derived().begin();
        }

        template<typename D = Derived>
        constexpr auto push_front(typename D::value_type const & x) noexcept(
            noexcept(std::declval<D &>().emplace_front(x)))
            -> decltype((void)std::declval<D &>().emplace_front(x))
        {
            derived().emplace_front(x);
        }

        template<typename D = Derived>
        constexpr auto push_front(typename D::value_type && x) noexcept(
            noexcept(std::declval<D &>().emplace_front(std::move(x))))
            -> decltype((void)std::declval<D &>().emplace_front(std::move(x)))
        {
            derived().emplace_front(std::move(x));
        }

        template<typename D = Derived>
        constexpr auto pop_front() noexcept -> decltype(
            std::declval<D &>().emplace_front(
                std::declval<typename D::value_type &>()),
            (void)std::declval<D &>().erase(std::declval<D &>().begin()))
        {
            derived().erase(derived().begin());
        }

        template<
            typename D = Derived,
            typename Enable = std::enable_if_t<
                detail::decrementable_sentinel<D>::value &&
                detail::common_range<D>::value>>
        constexpr auto
        back() noexcept(noexcept(*std::prev(std::declval<D &>().end())))
            -> decltype(*std::prev(std::declval<D &>().end()))
        {
            return *std::prev(derived().end());
        }
        template<
            typename D = Derived,
            typename Enable = std::enable_if_t<
                detail::decrementable_sentinel<D>::value &&
                detail::common_range<D>::value>>
        constexpr auto back() const
            noexcept(noexcept(*std::prev(std::declval<D const &>().end())))
                -> decltype(*std::prev(std::declval<D const &>().end()))
        {
            return *std::prev(derived().end());
        }

        template<typename D = Derived>
        constexpr auto push_back(typename D::value_type const & x) noexcept(
            noexcept(std::declval<D &>().emplace_back(x)))
            -> decltype((void)std::declval<D &>().emplace_back(x))
        {
            derived().emplace_back(x);
        }

        template<typename D = Derived>
        constexpr auto push_back(typename D::value_type && x) noexcept(
            noexcept(std::declval<D &>().emplace_back(std::move(x))))
            -> decltype((void)std::declval<D &>().emplace_back(std::move(x)))
        {
            derived().emplace_back(std::move(x));
        }

        template<typename D = Derived>
        constexpr auto pop_back() noexcept -> decltype(
            std::declval<D &>().emplace_back(
                std::declval<typename D::value_type &>()),
            (void)std::declval<D &>().erase(
                std::prev(std::declval<D &>().end())))
        {
            derived().erase(std::prev(derived().end()));
        }

        template<typename D = Derived>
        constexpr auto operator[](typename D::size_type n) noexcept(
            noexcept(std::declval<D &>().begin()[n]))
            -> decltype(std::declval<D &>().begin()[n])
        {
            return derived().begin()[n];
        }
        template<typename D = Derived>
        constexpr auto operator[](typename D::size_type n) const
            noexcept(noexcept(std::declval<D const &>().begin()[n]))
                -> decltype(std::declval<D const &>().begin()[n])
        {
            return derived().begin()[n];
        }

        template<typename D = Derived>
        constexpr auto at(typename D::size_type i)
            -> decltype(std::declval<D &>().size(), std::declval<D &>()[i])
        {
            if (derived().size() <= i) {
                throw std::out_of_range(
                    "Bounds check failed in container_interface::at()");
            }
            return derived()[i];
        }

        template<typename D = Derived>
        constexpr auto at(typename D::size_type i) const -> decltype(
            std::declval<D const &>().size(), std::declval<D const &>()[i])
        {
            if (derived().size() <= i) {
                throw std::out_of_range(
                    "Bounds check failed in container_interface::at()");
            }
            return derived()[i];
        }

        template<typename D = Derived>
        constexpr auto resize(typename D::size_type n) noexcept(
            noexcept(std::declval<D &>().resize(
                n, std::declval<typename D::value_type const &>())))
            -> decltype(std::declval<D &>().resize(
                n, std::declval<typename D::value_type const &>()))
        {
            return derived().resize(n, typename D::value_type());
        }

        template<typename D = Derived>
        constexpr auto begin() const
            noexcept(noexcept(std::declval<D &>().begin()))
        {
            return typename D::const_iterator(mutable_derived().begin());
        }
        template<typename D = Derived>
        constexpr auto end() const noexcept(noexcept(std::declval<D &>().end()))
        {
            return typename D::const_iterator(mutable_derived().end());
        }

        template<typename D = Derived>
        constexpr auto cbegin() const
            noexcept(noexcept(std::declval<D const &>().begin()))
                -> decltype(std::declval<D const &>().begin())
        {
            return derived().begin();
        }
        template<typename D = Derived>
        constexpr auto cend() const
            noexcept(noexcept(std::declval<D const &>().end()))
                -> decltype(std::declval<D const &>().end())
        {
            return derived().end();
        }

        template<
            typename D = Derived,
            typename Enable = std::enable_if_t<detail::common_range<D>::value>>
        constexpr auto rbegin() noexcept(noexcept(
            stl_interfaces::make_reverse_iterator(std::declval<D &>().end())))
        {
            return stl_interfaces::make_reverse_iterator(derived().end());
        }
        template<
            typename D = Derived,
            typename Enable = std::enable_if_t<detail::common_range<D>::value>>
        constexpr auto rend() noexcept(noexcept(
            stl_interfaces::make_reverse_iterator(std::declval<D &>().begin())))
        {
            return stl_interfaces::make_reverse_iterator(derived().begin());
        }

        template<typename D = Derived>
        constexpr auto rbegin() const
            noexcept(noexcept(std::declval<D &>().rbegin()))
        {
            return
                typename D::const_reverse_iterator(mutable_derived().rbegin());
        }
        template<typename D = Derived>
        constexpr auto rend() const
            noexcept(noexcept(std::declval<D &>().rend()))
        {
            return typename D::const_reverse_iterator(mutable_derived().rend());
        }

        template<typename D = Derived>
        constexpr auto crbegin() const
            noexcept(noexcept(std::declval<D const &>().rbegin()))
                -> decltype(std::declval<D const &>().rbegin())
        {
            return derived().rbegin();
        }
        template<typename D = Derived>
        constexpr auto crend() const
            noexcept(noexcept(std::declval<D const &>().rend()))
                -> decltype(std::declval<D const &>().rend())
        {
            return derived().rend();
        }

        template<typename D = Derived>
        constexpr auto insert(
            typename D::const_iterator pos,
            typename D::value_type const &
                x) noexcept(noexcept(std::declval<D &>().emplace(pos, x)))
            -> decltype(std::declval<D &>().emplace(pos, x))
        {
            return derived().emplace(pos, x);
        }

        template<typename D = Derived>
        constexpr auto insert(
            typename D::const_iterator pos,
            typename D::value_type &&
                x) noexcept(noexcept(std::declval<D &>()
                                         .emplace(pos, std::move(x))))
            -> decltype(std::declval<D &>().emplace(pos, std::move(x)))
        {
            return derived().emplace(pos, std::move(x));
        }

        template<typename D = Derived>
        constexpr auto insert(
            typename D::const_iterator pos,
            typename D::size_type n,
            typename D::value_type const & x)
            // If you see an error in this noexcept() expression, that's
            // because this function is not properly constrained.  In other
            // words, Derived does not have a "range" insert like
            // insert(position, first, last).  If that is the case, this
            // function should be removed via SFINAE from overload resolution.
            // However, both the trailing decltype code below and a
            // std::enable_if in the template parameters do not work.  Sorry
            // about that.  See below for details.
            noexcept(noexcept(std::declval<D &>().insert(
                pos, detail::make_n_iter(x, n), detail::make_n_iter_end(x, n))))
        // This causes the compiler to infinitely recurse into this function's
        // declaration, even though the call below does not match the
        // signature of this function.
#if 0
            -> decltype(std::declval<D &>().insert(
                pos, detail::make_n_iter(x, n), detail::make_n_iter_end(x, n)))
#endif
        {
            return derived().insert(
                pos, detail::make_n_iter(x, n), detail::make_n_iter_end(x, n));
        }

        template<typename D = Derived>
        constexpr auto insert(
            typename D::const_iterator pos,
            std::initializer_list<typename D::value_type>
                il) noexcept(noexcept(std::declval<D &>()
                                          .insert(pos, il.begin(), il.end())))
            -> decltype(std::declval<D &>().insert(pos, il.begin(), il.end()))
        {
            return derived().insert(pos, il.begin(), il.end());
        }

        template<typename D = Derived>
        constexpr auto erase(typename D::const_iterator pos) noexcept
            -> decltype(std::declval<D &>().erase(pos, std::next(pos)))
        {
            return derived().erase(pos, std::next(pos));
        }

        template<
            typename InputIterator,
            typename D = Derived,
            typename Enable =
                std::enable_if_t<detail::in_iter<InputIterator>::value>>
        constexpr auto assign(InputIterator first, InputIterator last) noexcept(
            noexcept(std::declval<D &>().insert(
                std::declval<D &>().begin(), first, last)))
            -> decltype(
                std::declval<D &>().clear(),
                (void)std::declval<D &>().insert(
                    std::declval<D &>().begin(), first, last))
        {
            derived().clear();
            derived().insert(derived().begin(), first, last);
        }

        template<typename D = Derived>
        constexpr auto assign(
            typename D::size_type n,
            typename D::value_type const &
                x) noexcept(noexcept(std::declval<D &>()
                                         .insert(
                                             std::declval<D &>().begin(),
                                             detail::make_n_iter(x, n),
                                             detail::make_n_iter_end(x, n))))
            -> decltype(
                std::declval<D &>().clear(),
                (void)std::declval<D &>().insert(
                    std::declval<D &>().begin(),
                    detail::make_n_iter(x, n),
                    detail::make_n_iter_end(x, n)))
        {
            derived().clear();
            derived().insert(
                derived().begin(),
                detail::make_n_iter(x, n),
                detail::make_n_iter_end(x, n));
        }

        template<typename D = Derived>
        constexpr auto
        assign(std::initializer_list<typename D::value_type> il) noexcept(
            noexcept(std::declval<D &>().assign(il.begin(), il.end())))
            -> decltype((void)std::declval<D &>().assign(il.begin(), il.end()))
        {
            derived().assign(il.begin(), il.end());
        }

        template<typename D = Derived>
        constexpr auto
        operator=(std::initializer_list<typename D::value_type> il) noexcept(
            noexcept(std::declval<D &>().assign(il.begin(), il.end())))
            -> decltype(
                std::declval<D &>().assign(il.begin(), il.end()),
                std::declval<D &>())
        {
            derived().assign(il.begin(), il.end());
            return *this;
        }

        template<typename D = Derived>
        constexpr auto clear() noexcept
            -> decltype((void)std::declval<D &>().erase(
                std::declval<D &>().begin(), std::declval<D &>().end()))
        {
            derived().erase(derived().begin(), derived().end());
        }
    };

    /** Implementation of free function `swap()` for all containers derived
        from `container_interface`.  */
    template<typename ContainerInterface>
    constexpr auto swap(
        ContainerInterface & lhs,
        ContainerInterface & rhs) noexcept(noexcept(lhs.swap(rhs)))
        -> decltype(detail::derived_container(lhs), lhs.swap(rhs))
    {
        return lhs.swap(rhs);
    }

    /** Implementation of `operator<=()` for all containers derived from
        `container_interface`.  */
    template<typename ContainerInterface>
    constexpr auto operator<=(
        ContainerInterface lhs,
        ContainerInterface rhs) noexcept(noexcept(lhs < rhs))
        -> decltype(detail::derived_container(lhs), lhs < rhs)
    {
        return !(rhs < lhs);
    }

    /** Implementation of `operator>()` for all containers derived from
        `container_interface`.  */
    template<typename ContainerInterface>
    constexpr auto operator>(
        ContainerInterface lhs,
        ContainerInterface rhs) noexcept(noexcept(lhs < rhs))
        -> decltype(detail::derived_container(lhs), lhs < rhs)
    {
        return rhs < lhs;
    }

    /** Implementation of `operator>=()` for all containers derived from
        `container_interface`.  */
    template<typename ContainerInterface>
    constexpr auto operator>=(
        ContainerInterface lhs,
        ContainerInterface rhs) noexcept(noexcept(lhs < rhs))
        -> decltype(detail::derived_container(lhs), lhs < rhs)
    {
        return !(lhs < rhs);
    }

}}}


#if 201703L < __cplusplus && defined(__cpp_lib_concepts) || BOOST_STL_INTERFACES_DOXYGEN

namespace boost { namespace stl_interfaces { namespace v2 {

    /** A CRTP template that one may derive from to make it easier to define
        container types. */
    template<typename Derived, bool Contiguous = discontiguous>
      requires std::is_class_v<Derived> &&
               std::same_as<Derived, std::remove_cv_t<Derived>>
    struct container_interface;

    namespace detail {
        template<typename Derived, bool Contiguous>
        void
        derived_container(container_interface<Derived, Contiguous> const &);
    }

    // clang-format off
    template<
      typename Derived, bool Contiguous>
      requires std::is_class_v<Derived> &&
               std::same_as<Derived, std::remove_cv_t<Derived>>
    struct container_interface {
    private:
      constexpr Derived& derived() noexcept {
        return static_cast<Derived&>(*this);
      }
      constexpr const Derived& derived() const noexcept {
        return static_cast<const Derived&>(*this);
      }
      constexpr Derived & mutable_derived() const noexcept {
        return const_cast<Derived&>(static_cast<const Derived&>(*this));
      }
      static constexpr void clear_impl(Derived& d) noexcept {}
      static constexpr void clear_impl(Derived& d) noexcept
        requires requires { d.clear()); } {
          d.clear();
        }

    public:
      ~container_interface() { clear_impl(derived()); }

      constexpr bool empty() requires std::forward_range<Derived> {
        return std::ranges::begin(derived()) == std::ranges::end(derived());
      }
      constexpr bool empty() const requires std::forward_range<const Derived> {
        return std::ranges::begin(derived()) == std::ranges::end(derived());
      }

      constexpr auto data() requires std::contiguous_iterator<std::iterator_t<Derived>> {
        return std::to_address(std::ranges::begin(derived()));
      }
      constexpr auto data() const
        requires std::range<const Derived> &&
          std::contiguous_iterator<std::iterator_t<const Derived>> {
            return std::to_address(std::ranges::begin(derived()));
          }

      constexpr auto size() requires std::forward_range<Derived> &&
        std::sized_sentinel_for<std::sentinel_t<Derived>, std::iterator_t<Derived>>
        -> Derived::size_type {
          return Derived::size_type(
            std::ranges::end(derived()) - std::ranges::begin(derived()));
        }
      constexpr auto size() const requires std::forward_range<const Derived> &&
        std::sized_sentinel_for<std::sentinel_t<const Derived>, std::iterator_t<const Derived>>
        -> Derived::size_type {
          return Derived::size_type(
            std::ranges::end(derived()) - std::ranges::begin(derived()));
        }

      constexpr decltype(auto) front() requires std::forward_range<Derived> {
        BOOST_ASSERT(!empty());
        return *std::ranges::begin(derived());
      }
      constexpr decltype(auto) front() const
        requires std::forward_range<const Derived> {
          BOOST_ASSERT(!empty());
          return *std::ranges::begin(derived());
        }

      constexpr void push_front(const Derived::value_type& x)
        requires std::forward_range<Derived> &&
          requires { derived().emplace_front(x); } {
            return derived().emplace_front(x);
          }
      constexpr void push_front(Derived::value_type&& x)
        requires std::forward_range<Derived> &&
          requires { derived().emplace_front(std::move(x)); } {
            return derived().emplace_front(std::move(x));
          }
      constexpr void pop_front() noexcept
          requires std::forward_range<Derived> &&
            requires (Derived::value_type x, Derived::const_iterator position) {
            derived().emplace_front(x); derived().erase(position); } {
              return derived().erase(std::ranges::begin(derived()));
            }

      constexpr decltype(auto) back()
        requires std::bidirectional_range<Derived> && std::common_range<Derived> {
          BOOST_ASSERT(!empty());
          return *std::ranges::prev(std::ranges::begin(derived()));
        }
      constexpr decltype(auto) back() const
        requires std::bidirectional_range<const Derived> && std::common_range<const Derived> {
          BOOST_ASSERT(!empty());
          return *std::ranges::prev(std::ranges::begin(derived()));
        }

      constexpr void push_back(const Derived::value_type& x)
        requires std::bidirectional_range<Derived> && std::common_range<Derived> &&
          requires { derived().emplace_back(x); } {
            return derived().emplace_back(x);
          }
      constexpr void push_back(Derived::value_type&& x)
        requires std::bidirectional_range<Derived> && std::common_range<Derived> &&
          requires { derived().emplace_back(std::move(x)); } {
            return derived().emplace_back(std::move(x));
          }
      constexpr void pop_back() noexcept
          requires std::bidirectional_range<Derived> && std::common_range<Derived> &&
            requires (Derived::value_type x, Derived::const_iterator position) {
            derived().emplace_back(x); derived().erase(position); } {
              return derived().erase(std::ranges::prev(std::ranges::end(derived())));
            }

      template<std::random_access_range Container = Derived>
      constexpr decltype(auto) operator[](Derived::size_type n) {
        return std::ranges::begin(derived())[n];
      }
      template<std::random_access_range Container = const Derived>
      constexpr decltype(auto) operator[](Derived::size_type n) const {
        return std::ranges::begin(derived())[n];
      }

      template<std::random_access_range Container = Derived>
      constexpr decltype(auto) at(Derived::size_type n) {
        if (derived().size() < n)
          throw std::out_of_range("Bounds check failed in container_interface::at()");
        return std::ranges::begin(derived())[n];
      }
      template<std::random_access_range Container = const Derived>
      constexpr decltype(auto) at(Derived::size_type n) const {
        if (derived().size() < n)
          throw std::out_of_range("Bounds check failed in container_interface::at()");
        return std::ranges::begin(derived())[n];
      }

      constexpr void resize(Derived::size_type n)
        requires std::forward_range<Derived> &&
          requires { derived().resize(n, Derived::value_type()); } {
            derived().resize(n, Derived::value_type());
          }

      constexpr auto begin() const requires std::forward_range<Derived> {
        return Derived::const_iterator(std::ranges::begin(mutable_derived()));
      }
      constexpr auto end() const requires std::forward_range<Derived> {
        return Derived::const_iterator(std::ranges::end(mutable_derived()));
      }

      constexpr auto cbegin() const requires std::forward_range<Derived> {
        return std::ranges::begin(derived());
      }
      constexpr auto cend() const requires std::forward_range<Derived> {
        return std::ranges::end(derived());
      }

      constexpr auto rbegin() requires std::bidirectional_range<Derived> {
        return std::reverse_iterator(std::ranges::end(derived()));
      }
      constexpr auto rend() requires std::bidirectional_range<Derived> {
        return std::reverse_iterator(std::ranges::begin(derived()));
      }

      constexpr auto rbegin() const requires std::bidirectional_range<Derived> {
        return Derived::reverse_const_iterator(std::ranges::rbegin(mutable_derived()));
      }
      constexpr auto rend() const requires std::bidirectional_range<Derived> {
        return Derived::reverse_const_iterator(std::ranges::rend(mutable_derived()));
      }

      constexpr auto crbegin() const requires std::bidirectional_range<Derived> {
        return std::ranges::rbegin(derived());
      }
      constexpr auto crend() const requires std::bidirectional_range<Derived> {
        return std::ranges::rend(derived());
      }

      constexpr auto insert(Derived::const_iterator position, const Derived::value_type& x)
        requires std::forward_range<Derived> && requires {
          derived().emplace(position, x); } {
            derived().emplace(position, x);
          }
      constexpr auto insert(Derived::const_iterator position, Derived::value_type&& x)
        requires std::forward_range<Derived> && requires {
          derived().emplace(position, std::move(x)); } {
            derived().emplace(position, std::move(x));
          }
      constexpr auto insert(Derived::const_iterator position, Derived::size_type n,
                            const Derived::value_type& x)
        requires std::forward_range<Derived> && requires {
          derived().insert(position, v1::detail::make_n_iter(x, n),
                           v1::detail::make_n_iter_end(x, n)); } {
            derived().insert(position,  v1::detail::make_n_iter(x, n),
                             v1::detail::make_n_iter_end(x, n));
          }
      constexpr auto insert(Derived::const_iterator position,
                            std::initializer_list<Derived::value_type> il)
        requires std::forward_range<Derived> && requires {
          derived().insert(position, il.begin(), il.end()); } {
            derived().insert(position, il.begin(), il.end());
          }

      constexpr void erase(Derived::const_iterator position)
          requires std::forward_range<Derived> && requires {
            derived().erase(position, std::ranges::next(position)); } {
              derived().erase(position, std::ranges::next(position));
            }

      template<std::forward_iterator Iter>
        constexpr void assign(Iter first, Iter last)
            requires std::forward_range<Derived> && requires {
              derived().clear();
              derived().insert(std::ranges::begin(derived()), first, last); } {
                derived().clear();
                derived().insert(std::ranges::begin(derived()), first, last);
              }
      constexpr void assign(Derived::size_type n, const Derived::value_type& x)
          requires std::forward_range<Derived> && requires {
            derived().clear();
            derived().insert(std::ranges::begin(derived()),
                             v1::detail::make_n_iter(x, n),
                             v1::detail::make_n_iter_end(x, n)); } {
              derived().clear();
              derived().insert(std::ranges::begin(derived()),
                               v1::detail::make_n_iter(x, n),
                               v1::detail::make_n_iter_end(x, n));
            }
      constexpr void assign(std::initializer_list<Derived::value_type> il)
          requires std::forward_range<Derived> && requires {
            derived().clear();
            derived().insert(std::ranges::begin(derived()), il.begin(), il.end()); } {
              derived().clear();
              derived().insert(std::ranges::begin(derived()), il.begin(), il.end());
            }

      constexpr void clear() noexcept
        requires std::forward_range<Derived> && requires {
          derived().erase(std::ranges::begin(derived()), std::ranges::end(derived())); } {
            derived().erase(std::ranges::begin(derived()), std::ranges::end(derived()));
          }

      constexpr decltype(auto) operator=(std::initializer_list<Derived::value_type> il)
        requires std::forward_range<Derived> && requires {
          derived().assign(il.begin(), il.end()); } {
            derived().assign(il.begin(), il.end());
          }
    };
    // clang-format on

}}}

#endif

#endif
