/* ***** BEGIN LICENSE BLOCK *****
 * Roadsend PHP Compiler Runtime Libraries
 *
 * Copyright (c) 2008 Shannon Weyrick <weyrick@roadsend.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 * ***** END LICENSE BLOCK ***** */

#ifndef RPHP_PUSTRING_H_
#define RPHP_PUSTRING_H_

#include <string>
#include <cstring>
#include <exception>

#include <boost/shared_array.hpp>
#include <boost/cstdint.hpp>

#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/unistr.h>

namespace rphp {

// a lightweight unicode string class based on libicu's UChar
// this is little more than a buffer that's easily passed around
// inside of pVar. for real functionality, use UnicodeString
// which can be efficiently created from this object, both
// read-only, or "checked out" and "checked in" if you need to mutate it.
// pUString is copy-on-write
class pUString {
public:
    static const unsigned int defaultBufferSize = 32;
    typedef boost::shared_array<UChar> arrayType;

private:
    arrayType data_;
    int32_t strLen_;
    int32_t bufSize_;

    void detach() {
        UChar* tmp = data_.get();
        if( !( tmp == 0 || data_.unique() ) ) {
            tmp = new UChar[bufSize_];
            memcpy(tmp, data_.get(), bufSize_ * 2);
            data_.reset(tmp);
        }
    }

public:
    pUString(void): strLen_(0),
                    bufSize_(defaultBufferSize),
                    data_(new UChar[bufSize_])
    {
        
    }

    pUString(const char * s): strLen_(0), bufSize_(0), data_(0) {
        strLen_ = bufSize_ = strlen(s);
        if (strLen_) {
            data_.reset(new UChar[bufSize_]);
            u_uastrncpy(data_.get(), s, strLen_);
        }
    }

    pUString(const char * s, int32_t len): strLen_(len), bufSize_(len), data_(0) {
        if (strLen_ >= 1) {
            data_.reset(new UChar[bufSize_]);
            u_uastrncpy(data_.get(), s, strLen_);
        }
        else {
            strLen_ = bufSize_ = 0;
        }
    }

    pUString(const std::string& s): strLen_(s.length()), bufSize_(s.length()), data_(0) {
        if (strLen_) {
            data_.reset(new UChar[bufSize_]);
            u_uastrncpy(data_.get(), s.data(), strLen_);
        }
    }

    pUString(const UnicodeString& s): strLen_(s.length()),
                                      bufSize_(s.length()),
                                      data_(0)
    {
        if (strLen_) {
            data_.reset(new UChar[bufSize_]);
            memcpy(data_.get(), s.getBuffer(), bufSize_ * U_SIZEOF_UCHAR);
        }        
    }

    // size
    int32_t size(void) const { return strLen_; }
    int32_t length(void) const { return strLen_; }

    // empty predicate
    bool empty(void) const { return (bool)strLen_; }

    // buffer viewer
    const UChar* getBuffer(void) const { return data_.get(); }

    // return ICU's hash code
    std::size_t hashCode(void) const;

    // if you want to work with a UnicodeString starting with a copy of the data
    // in this buffer, use this
    UnicodeString copyAsICUString(void) const;

    // if you want to work with a UnicodeString interface to this buffer
    // and you don't need to mutate it, use this
    const UnicodeString readonlyICUString(void) const;

    // if you want to work with a UnicodeString interface to this buffer
    // that you can mutate, use this.
    // if you checkout however, you _must_ check in to gurantee that the buffer
    // is updated properly
    UnicodeString checkoutICUString(void);
    
    // use after checking out a unicode string
    void checkinICUString(const UnicodeString& s);
    
};
    
} /* namespace rphp */


#endif /* RPHP_PUSTRING_H_ */