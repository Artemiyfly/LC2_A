#include <cstdlib>
#include <functional>
#include <utility>
#include <stdexcept>
#include <iostream>
class HashMap {
pitemplate<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
public:
    std::pair<KeyType, ValueType> *data;
    int8_t *status; /// 0 - null, 1 - alive, 2 - dead
    size_t n, cap, full; ///alive, cap (pow of 2!), alive+dead
    Hash hasher;
    size_t raw_find(const KeyType &v) const {
        size_t res = hasher(v) & (cap - 1);
        while (status[res] != 0 && !(data[res].first == v)) {
            //std::cerr << res << ' ';
            ++res;
            if (res >= cap)
                res -= cap;
        }
        return res;
    }
    void transform(size_t new_size) {
        //std::cerr << "NS " << new_size << std::endl;
        HashMap b(begin(), end(), hasher, new_size);
        //b.print_content();
        std::swap(data, b.data);
        std::swap(status, b.status);
        std::swap(n, b.n);
        std::swap(cap, b.cap);
        std::swap(full, b.full);
        //print_content();
    }
public:
     HashMap(const Hash &_hasher = Hash()): hasher(_hasher) {
        n = 0;
        full = 0;
        cap = 4;
        data = (std::pair<KeyType, ValueType>*) malloc(cap * sizeof(std::pair<KeyType, ValueType>));
        status = new int8_t[cap]();
    }
    template<class Iter>
    HashMap(Iter sbegin, Iter send, Hash _hasher = Hash(), size_t cap = 4): n(0), full(0), cap(cap), hasher(_hasher) {
        //data = (std::pair<KeyType, ValueType>*) malloc(cap * sizeof(std::pair<KeyType, ValueType>));
        data = new std::pair<KeyType, ValueType>[cap];
        status = new int8_t[cap]();
        for (int i = 0; i < cap; ++i)
            status[i] = 0;
        std::cerr << cap << '\n';
        while (sbegin != send) {
            insert(*sbegin);
            ++sbegin;
        }
    }
    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> l, Hash hasher = Hash()): HashMap(l.begin(), l.end(), hasher){}
    class iterator {
        HashMap *parent;
        size_t pos;
    public:
        iterator(HashMap *_parent, size_t _pos): parent(_parent), pos(_pos) {}
        iterator() {}
        iterator &operator++() {
            ++pos;
            while (parent->status[pos] != 1 && pos < parent->cap)
                ++pos;
            return *this;
        }
        iterator operator++(int) {
            size_t ppos = pos;
            ++pos;
            while (parent->status[pos] != 1 && pos < parent->cap)
                ++pos;
            return iterator(parent, ppos);
        }
        std::pair<const KeyType&, ValueType&> operator*() {
             return {parent->data[pos].first, parent->data[pos].second};
        }
        std::pair<const KeyType, ValueType> *operator->() {
            return reinterpret_cast<std::pair<const KeyType, ValueType> *>(parent->data + pos);
        }
        bool operator==(const iterator &other) const {
            return parent == other.parent && pos == other.pos;
        }
        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }
    };
    class const_iterator {
    public:
        const HashMap *parent;
        size_t pos;
        const_iterator(const HashMap *_parent, size_t _pos): parent(_parent), pos(_pos) {
            //std::cerr << "FORMED CONST_ITER " << pos << "/" << parent->cap << std::endl;
        }
        const_iterator() {}
        const_iterator &operator++() {
            ++pos;
            while (parent->status[pos] != 1 && pos < parent->cap)
                ++pos;
            return *this;
        }
        const_iterator operator++(int) {
            size_t ppos = pos;
            ++pos;
            while (parent->status[pos] != 1 && pos < parent->cap)
                ++pos;
            return const_iterator(parent, ppos);
        }
        std::pair<const KeyType&, const ValueType&> operator*() {
            return parent->data[pos];
        }
        const std::pair <KeyType, ValueType> *operator->() {

            //std::pair<const KeyType&, const ValueType&> res = std::make_pair(
            //                    const_cast<const KeyType&>(parent->data[pos].first),
            //                    const_cast<const ValueType&>(parent->data[pos].second));
            //std::cerr << "GIVVING " << pos << '/' << parent->cap << " " << res.second << std::endl;
            //return &res;
            return /*reinterpret_cast<std::pair<const KeyType&, const ValueType&> *>*/(parent->data + pos);
            //return parent->data + pos;
            //return &std::make_pair(const_cast<const KeyType&>(parent->data[pos].first),
            //                  const_cast<const ValueType&>(parent->data[pos].second));
        }
        bool operator==(const const_iterator &other) const {
            return parent == other.parent && pos == other.pos;
        }
        bool operator!=(const const_iterator &other) const {
            return !(*this == other);
        }
    };
    iterator begin() {
        int res = 0;
        while (status[res] != 1 && res < cap)
            ++res;
        return iterator(this, res);
    }
    iterator end() {
        return iterator(this, cap);
    }
    const_iterator begin() const {
        int res = 0;
        while (status[res] != 1 && res < cap)
            ++res;
        return const_iterator(this, res);
    }
    const_iterator end() const {
        return const_iterator(this, cap);
    }
    iterator find(const KeyType &key) {
        size_t pos = raw_find(key);
        if (status[pos] == 1)
            return iterator(this, pos);
        else
            return end();
    }
    const_iterator find(const KeyType &key) const {
        size_t pos = raw_find(key);
        if (status[pos] == 1)
            return const_iterator(this, pos);
        else
            return end();
    }
    void insert(const std::pair<KeyType, ValueType>& p) {
        std::cerr << "INS " << p.first << ' ' << p.second << '\n';
        if (full * 2 >= cap)
            transform(cap * 2);
        size_t pos = raw_find(p.first);
        if (status[pos] == 0) {
            status[pos] = 1;
            data[pos] = p;
            std::cerr << pos << '/' << cap << '\n';
            ++full;
            ++n;
        }
    }
    void erase(const KeyType &key) {
        size_t pos = raw_find(key);
        if (status[pos] == 1) {
            status[pos] = 2;
            --n;
            if (n * 4 < cap && cap > 4) {
                transform(cap / 2);
            }
        }
    }
    size_t size() const {
        return n;
    }
    bool empty() const {
        return n == 0;
    }
    Hash hash_function() const {
        return hasher;
    }
    ValueType &operator[](const KeyType &key) {
        size_t pos = raw_find(key);
        if (status[pos] == 0) {
            if (full * 2 >= cap) {
                transform(cap * 2);
                pos = raw_find(key);
            }
            status[pos] = 1;
            data[pos] = {key, ValueType()};
            ++full;
            ++n;
        }
        return data[pos].second;
    }
    const ValueType &at(const KeyType &key) const {
        size_t pos = raw_find(key);
        if (status[key] == 1)
            return data[pos].second;
        else
            throw std::out_of_range("msg");
    }
    void clear() {
        HashMap b;
        std::swap(data, b.data);
        std::swap(status, b.status);
        std::swap(n, b.n);
        std::swap(cap, b.cap);
        std::swap(full, b.full);
    }
    ~HashMap() {
        free(data);
         free(status);
    }
    /*
    void print_content() {
        std::cerr << "n: " << n << " full/cap: " << full << "/" << cap << "\n";
        for (int i = 0; i < cap; i++) {
            std::cerr << (int)status[i] << " " << data[i].first << "\t" << data[i].second << "\n";
        }
    }
    */
};

