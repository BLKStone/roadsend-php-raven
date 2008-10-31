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

#include <iostream>
#include <unicode/ustream.h>
#include "pHash.h"


namespace boost {
std::size_t hash_value(rphp::hKeyVar const& k) {
    return boost::apply_visitor(rphp::hKeyHasher(), k);
}
}

namespace rphp {

void pHash::insert(const pUString &key, const pVar& data) {
    // TODO check numeric string, set maxIntKey accordingly
    hashData_.insert(h_container(key, data));
}

/*
void pHash::insert(const pBString &key, pVarP data) {
    // TODO check numeric string, set maxIntKey accordingly
    hashData_.insert(h_container(key, data));
}
*/

void pHash::insert(const pInt &key, const pVar& data) {
    if (key > maxIntKey_)
        maxIntKey_ = key+1;
    hashData_.insert(h_container(key, data));
}

void pHash::insertNext(const pVar& data) {
    hashData_.insert(h_container(maxIntKey_++, data));
}

pHash::size_type pHash::remove(const pUString &key) {
    return hashData_.erase(key);
}

pHash::size_type pHash::remove(const pInt &key) {
    return hashData_.erase(key);
}


// query
bool pHash::keyExists(const pUString &key) const {
    stableHash::iterator k = hashData_.find(key);
    return (k != hashData_.end());
}
/*
bool pHash::keyExists(const pBString &key) const {
    stableHash::iterator k = hashData_.find(key);
    return (k != hashData_.end());
}
*/
bool pHash::keyExists(const pInt &key) const {
    stableHash::iterator k = hashData_.find(key);
    return (k != hashData_.end());
}

// lookup
pVar pHash::operator[] ( const pUString &key ) const {
    stableHash::iterator k = hashData_.find(key);
    if (k == hashData_.end())
        return pVar(); // pNull
    else
        return (*k).pData;
}

/*
pVarP pHash::operator[] ( const pBString &key ) const {
    stableHash::iterator k = hashData_.find(key);
    if (k == hashData_.end())
        return pVarP();
    else
        return (*k).pData;
}
*/

pVar pHash::operator[] ( const pInt &key ) const {
    stableHash::iterator k = hashData_.find(key);
    if (k == hashData_.end())
        return pVar(); // pNull
    else
        return (*k).pData;
}

// output
std::ostream& operator << (std::ostream& os, const rphp::pHash& h)
{
    return os << "php_hash:" << std::endl;
}

void pHash::varDump() const {


    std::cout << "array(" << hashData_.size() << ") {" << std::endl;

    const seq_index& ot = get<1>(hashData_);

    hKeyType kType;

    for (seq_index::iterator it = ot.begin(); it!=ot.end(); it++) {
        if ((*it).key.which() == hKeyInt)
            std::cout << "   [" << (*it).key << "] => " << (*it).pData << std::endl;
        else
            std::cout << "   ['" << (*it).key << "'] => " << (*it).pData << std::endl;
    }

    std::cout << "}" << std::endl;

}

} /* namespace rphp */

