//
// buffers_iterator.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_BUFFERS_ITERATOR_HPP
#define ASIO_BUFFERS_ITERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/push_options.hpp"

#include "asio/detail/push_options.hpp"
#include <cstddef>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/add_const.hpp>
#include "asio/detail/pop_options.hpp"

#include "asio/buffer.hpp"

namespace asio {

namespace detail
{
  template <bool IsMutable>
  struct buffers_iterator_types_helper;

  template <>
  struct buffers_iterator_types_helper<false>
  {
    typedef const_buffer buffer_type;
    template <typename ByteType>
    struct byte_type
    {
      typedef typename boost::add_const<ByteType>::type type;
    };
  };

  template <>
  struct buffers_iterator_types_helper<true>
  {
    typedef mutable_buffer buffer_type;
    template <typename ByteType>
    struct byte_type
    {
      typedef ByteType type;
    };
  };

  template <typename BufferSequence, typename ByteType>
  struct buffers_iterator_types
  {
    enum
    {
      is_mutable = boost::is_convertible<
        typename BufferSequence::value_type, mutable_buffer>::value
    };
    typedef buffers_iterator_types_helper<is_mutable> helper;
    typedef typename helper::buffer_type buffer_type;
    typedef typename helper::template byte_type<ByteType>::type byte_type;
  };
}

/// A random access iterator over the bytes in a buffer sequence.
template <typename BufferSequence, typename ByteType = char>
class buffers_iterator
  : public boost::iterator_facade<
        buffers_iterator<BufferSequence, ByteType>,
        typename detail::buffers_iterator_types<
          BufferSequence, ByteType>::byte_type,
        boost::random_access_traversal_tag>
{
private:
  typedef typename detail::buffers_iterator_types<
      BufferSequence, ByteType>::buffer_type buffer_type;
  typedef typename detail::buffers_iterator_types<
      BufferSequence, ByteType>::byte_type byte_type;

public:
  /// Default constructor. Creates an iterator in an undefined state.
  buffers_iterator()
    : current_buffer_(),
      current_buffer_position_(0),
      begin_(),
      current_(),
      end_(),
      position_(0)
  {
  }

  /// Construct an iterator representing the beginning of the buffers' data.
  static buffers_iterator begin(const BufferSequence& buffers)
#if BOOST_WORKAROUND(__GNUC__, == 4) && BOOST_WORKAROUND(__GNUC_MINOR__, == 3)
    __attribute__ ((noinline))
#endif
  {
    buffers_iterator new_iter;
    new_iter.begin_ = buffers.begin();
    new_iter.current_ = buffers.begin();
    new_iter.end_ = buffers.end();
    while (new_iter.current_ != new_iter.end_)
    {
      new_iter.current_buffer_ = *new_iter.current_;
      if (asio::buffer_size(new_iter.current_buffer_) > 0)
        break;
      ++new_iter.current_;
    }
    return new_iter;
  }

  /// Construct an iterator representing the end of the buffers' data.
  static buffers_iterator end(const BufferSequence& buffers)
#if BOOST_WORKAROUND(__GNUC__, == 4) && BOOST_WORKAROUND(__GNUC_MINOR__, == 3)
    __attribute__ ((noinline))
#endif
  {
    buffers_iterator new_iter;
    new_iter.begin_ = buffers.begin();
    new_iter.current_ = buffers.begin();
    new_iter.end_ = buffers.end();
    while (new_iter.current_ != new_iter.end_)
    {
      buffer_type buffer = *new_iter.current_;
      new_iter.position_ += asio::buffer_size(buffer);
      ++new_iter.current_;
    }
    return new_iter;
  }

private:
  friend class boost::iterator_core_access;

  // Dereference the iterator.
  byte_type& dereference() const
  {
    return buffer_cast<byte_type*>(current_buffer_)[current_buffer_position_];
  }

  // Compare two iterators for equality.
  bool equal(const buffers_iterator& other) const
  {
    return position_ == other.position_;
  }

  // Increment the iterator.
  void increment()
  {
    BOOST_ASSERT(current_ != end_ && "iterator out of bounds");
    ++position_;

    // Check if the increment can be satisfied by the current buffer.
    ++current_buffer_position_;
    if (current_buffer_position_ != asio::buffer_size(current_buffer_))
      return;

    // Find the next non-empty buffer.
    ++current_;
    current_buffer_position_ = 0;
    while (current_ != end_)
    {
      current_buffer_ = *current_;
      if (asio::buffer_size(current_buffer_) > 0)
        return;
      ++current_;
    }
  }

  // Decrement the iterator.
  void decrement()
  {
    BOOST_ASSERT(position_ > 0 && "iterator out of bounds");
    --position_;

    // Check if the decrement can be satisfied by the current buffer.
    if (current_buffer_position_ != 0)
    {
      --current_buffer_position_;
      return;
    }

    // Find the previous non-empty buffer.
    typename BufferSequence::const_iterator iter = current_;
    while (iter != begin_)
    {
      --iter;
      buffer_type buffer = *iter;
      std::size_t buffer_size = asio::buffer_size(buffer);
      if (buffer_size > 0)
      {
        current_ = iter;
        current_buffer_ = buffer;
        current_buffer_position_ = buffer_size - 1;
        return;
      }
    }
  }

  // Advance the iterator by the specified distance.
  void advance(std::ptrdiff_t n)
  {
    if (n > 0)
    {
      BOOST_ASSERT(current_ != end_ && "iterator out of bounds");
      for (;;)
      {
        std::ptrdiff_t current_buffer_balance
          = asio::buffer_size(current_buffer_)
          - current_buffer_position_;

        // Check if the advance can be satisfied by the current buffer.
        if (current_buffer_balance > n)
        {
          position_ += n;
          current_buffer_position_ += n;
          return;
        }

        // Update position.
        n -= current_buffer_balance;
        position_ += current_buffer_balance;

        // Move to next buffer. If it is empty then it will be skipped on the
        // next iteration of this loop.
        if (++current_ == end_)
        {
          BOOST_ASSERT(n == 0 && "iterator out of bounds");
          current_buffer_ = buffer_type();
          current_buffer_position_ = 0;
          return;
        }
        current_buffer_ = *current_;
        current_buffer_position_ = 0;
      }
    }
    else if (n < 0)
    {
      std::size_t abs_n = -n;
      BOOST_ASSERT(position_ >= abs_n && "iterator out of bounds");
      for (;;)
      {
        // Check if the advance can be satisfied by the current buffer.
        if (current_buffer_position_ >= abs_n)
        {
          position_ -= abs_n;
          current_buffer_position_ -= abs_n;
          return;
        }

        // Update position.
        abs_n -= current_buffer_position_;
        position_ -= current_buffer_position_;

        // Check if we've reached the beginning of the buffers.
        if (current_ == begin_)
        {
          BOOST_ASSERT(abs_n == 0 && "iterator out of bounds");
          current_buffer_position_ = 0;
          return;
        }

        // Find the previous non-empty buffer.
        typename BufferSequence::const_iterator iter = current_;
        while (iter != begin_)
        {
          --iter;
          buffer_type buffer = *iter;
          std::size_t buffer_size = asio::buffer_size(buffer);
          if (buffer_size > 0)
          {
            current_ = iter;
            current_buffer_ = buffer;
            current_buffer_position_ = buffer_size;
            break;
          }
        }
      }
    }
  }

  // Determine the distance between two iterators.
  std::ptrdiff_t distance_to(const buffers_iterator& other) const
  {
    return other.position_ - position_;
  }

  buffer_type current_buffer_;
  std::size_t current_buffer_position_;
  typename BufferSequence::const_iterator begin_;
  typename BufferSequence::const_iterator current_;
  typename BufferSequence::const_iterator end_;
  std::size_t position_;
};

/// Construct an iterator representing the beginning of the buffers' data.
template <typename BufferSequence>
inline buffers_iterator<BufferSequence> buffers_begin(
    const BufferSequence& buffers)
{
  return buffers_iterator<BufferSequence>::begin(buffers);
}

/// Construct an iterator representing the end of the buffers' data.
template <typename BufferSequence>
inline buffers_iterator<BufferSequence> buffers_end(
    const BufferSequence& buffers)
{
  return buffers_iterator<BufferSequence>::end(buffers);
}

} // namespace asio

#include "asio/detail/pop_options.hpp"

#endif // ASIO_BUFFERS_ITERATOR_HPP
