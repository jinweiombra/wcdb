/*
 * Tencent is pleased to support the open source community by making
 * WCDB available.
 *
 * Copyright (C) 2017 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *       https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _WCDB_RECYCLABLE_HPP
#define _WCDB_RECYCLABLE_HPP

#include <WCDB/Assertion.hpp>
#include <WCDB/Console.hpp>
#include <functional>

namespace WCDB {

template<typename T>
class Recyclable {
public:
    typedef std::function<void(T &)> OnRecycled;

    Recyclable() : m_value(nullptr), m_onRecycled(nullptr), m_reference(nullptr)
    {
    }

    Recyclable(const std::nullptr_t &) : Recyclable() {}

    Recyclable(const T &value, const Recyclable::OnRecycled &onRecycled)
    : m_value(value), m_onRecycled(onRecycled), m_reference(new std::atomic<int>(0))
    {
        retain();
    }

    Recyclable(const Recyclable &other)
    : m_value(other.m_value)
    , m_onRecycled(other.m_onRecycled)
    , m_reference(other.m_reference)
    {
        retain();
    }

    Recyclable &operator=(const Recyclable &other)
    {
        other.retain();
        release();
        m_value = other.m_value;
        m_onRecycled = other.m_onRecycled;
        m_reference = other.m_reference;
        return *this;
    }

    Recyclable &operator=(const std::nullptr_t &)
    {
        release();
        m_value = nullptr;
        m_onRecycled = nullptr;
        m_reference = nullptr;
        return *this;
    }

    bool operator==(const Recyclable &other) const
    {
        return m_value == other.m_value;
    }

    bool operator==(const std::nullptr_t &) const { return m_value == nullptr; }
    bool operator!=(const std::nullptr_t &) const { return m_value != nullptr; }

    virtual ~Recyclable() { release(); }

    constexpr const T &operator->() const { return m_value; }
    constexpr T &operator->() { return m_value; }
    T &get() { return m_value; }
    const T &get() const { return m_value; }

protected:
    void retain() const
    {
        if (m_reference) {
            ++(*m_reference);
        }
    }

    void release()
    {
        if (m_reference) {
            WCTInnerAssert((*m_reference) > 0);
            if (--(*m_reference) == 0) {
                if (m_onRecycled) {
                    m_onRecycled(m_value);
                    m_onRecycled = nullptr;
                }
            }
        }
    }

    T m_value;
    mutable std::atomic<int> *m_reference;
    Recyclable::OnRecycled m_onRecycled;
};

} //namespace WCDB

#endif /* _WCDB_RECYCLABLE_HPP */
