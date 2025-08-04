#include <cstddef>
#include <iterator>
#include <memory>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <type_traits>
/*
   std::construct_at：原地构造对象（调用构造函数）
   std::destroy_at：原地销毁对象（调用析构函数）
   std::move_if_noexcept 会根据类型的移动构造函数是否声明为 noexcept（不抛异常）来决定：
   如果移动构造 noexcept，则返回 std::move(obj) —— 使用移动语义。
   否则，返回 obj 的 const 左值引用 —— 使用拷贝语义，避免抛异常。
*/
template <class _Tp, class _Alloc = std::allocator<_Tp>>
struct Vector
{
public:
    using value_type = _Tp;
    using allocator_type = _Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t; // 表示指针之间的偏移量（元素个数差），是有符号类型
    using pointer = _Tp *;
    using const_pointer = _Tp const *;
    using reference = _Tp &;
    using const_reference = _Tp const &;
    using _iterator = _Tp *;
    using const_iterator = _Tp const *;
    using reverse_iterator = std::reverse_iterator<_iterator>; // 反向迭代器
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    _Tp *_M_data;
    std::size_t _M_size;
    std::size_t _M_cap;
    [[no_unique_address]] _Alloc _M_alloc;

public:
    Vector() noexcept : _M_data(nullptr), _M_size(0), _M_cap(0)
    {
    }

    Vector(std::initializer_list<_Tp> _list, _Alloc const &alloc = _Alloc()) : Vector(_list.begin(), _list.end(), alloc) {}

    Vector(std::size_t _n, _Alloc const &alloc = _Alloc())
        requires std::default_initializable<_Tp>
        : _M_alloc(alloc)
    {
        _M_data = _M_alloc.allocate(_n);
        _M_size = _M_cap = _n;
        for (std::size_t _i = 0; _i != _n; ++_i)
            std::construct_at(&_M_data[_i]);
    }

    Vector(std::size_t _n, const_reference val, _Alloc const &alloc = _Alloc()) : _M_alloc(alloc)
    {
        _M_data = _M_alloc.allocate(_n);
        _M_size = _M_cap = _n;
        for (std::size_t _i = 0; _i != _n; ++_i)
            std::construct_at(&_M_data[_i], val);
    }

    template <typename _Input>
        requires std::random_access_iterator<_Input>
    Vector(_Input _first, _Input _last, _Alloc const &alloc = _Alloc()) : _M_alloc(alloc)
    {
        std::size_t diff = _last - _first;
        _M_data = _M_alloc.allocate(diff);
        _M_cap = _M_size = diff;
        for (size_t _i = 0; _i != diff; ++_i)
        {
            std::construct_at(&_M_data[_i], *_first);
            ++_first;
        }
    }

    Vector(Vector const &_other) : _M_alloc(_other._M_alloc)
    {
        _M_cap = _M_size = _other._M_size;
        if (_M_size != 0)
        {
            _M_data = _M_alloc.allocate(_M_size);
            for (std::size_t _i = 0; _i != _M_size; ++_i)
            {
                std::construct_at(&_M_data[_i], std::as_const(_other._M_data[_i]));
            }
        }
        else
        {
            _M_data = nullptr;
        }
    }

    Vector(Vector const &_other, _Alloc const &_alloc) : _M_alloc(_alloc)
    {
        _M_cap = _M_size = _other._M_size;
        if (_M_size != 0)
        {
            _M_data = _M_alloc.allocate(_M_size);
            for (std::size_t _i = 0; _i != _M_size; ++_i)
            {
                std::construct_at(&_M_data[_i], std::as_const(_other._M_data[_i]));
            }
        }
        else
        {
            _M_data = nullptr;
        }
    }

    Vector(Vector &&_other) noexcept : _M_alloc(std::move(_other._M_alloc))
    {
        _M_data = _other._M_data;
        _M_size = _other._M_size;
        _M_cap = _other._M_cap;
        _other._M_data = nullptr;
        _other._M_size = 0;
        _other._M_cap = 0;
    }

    Vector(Vector &&_other, _Alloc const &alloc) noexcept : _M_alloc(alloc)
    {
        _M_data = _other._M_data;
        _M_size = _other._M_size;
        _M_cap = _other._M_cap;
        _other._M_data = nullptr;
        _other._M_size = 0;
        _other._M_cap = 0;
    }

public:
    Vector &operator=(Vector const &_other) noexcept
    {
        if (&_other == this) [[unlikely]]
            return *this;
        reserve(_other._M_size);
        _M_size = _other._M_size;
        for (std::size_t _i = 0; _i != _M_size; ++_i)
        {
            std::construct_at(&_M_data[_i], std::as_const(_other._M_data[_i]));
        }
        return *this;
    }

    Vector &operator=(Vector &&_other) noexcept
    {
        if (&_other == this) [[unlikely]]
            return *this;
        for (std::size_t _i = 0; _i != _M_size; ++_i)
        {
            std::destroy_at(&_M_data[_i]);
        }
        if (_M_cap != 0)
            _M_alloc.deallocate(_M_data, _M_cap);
        _M_data = _other._M_data;
        _M_size = _other._M_size;
        _M_cap = _other._M_cap;
        _other._M_data = nullptr;
        _other._M_size = 0;
        _other._M_cap = 0;
        return *this;
    }

    Vector &operator=(std::initializer_list<_Tp> _list)
    {
        assign(_list.begin(), _list.end());
        return *this;
    }

    const_reference operator[](std::size_t _i) const noexcept
    {
        return _M_data[_i];
    }

    reference operator[](std::size_t _i) noexcept
    {
        return _M_data[_i];
    }

public:
    void clear() noexcept
    {
        for (std::size_t _i = 0; _i != _M_size; _i++)
        {
            std::destroy_at(&_M_data[_i]);
        }
        _M_size = 0; // 不清理分配的内存
    }

    void resize(std::size_t _n)
        requires std::default_initializable<_Tp>
    {
        if (_n == _M_size)
            return;
        if (_n < _M_size)
        {
            for (std::size_t _i = _n; _i != _M_size; ++_i)
            {
                std::destroy_at(&_M_data[_i]);
            }
            _M_size = _n;
        }
        else if (_n > _M_size)
        {
            reserve(_n);
            for (std::size_t _i = _M_size; _i != _n; ++_i)
            {
                std::construct_at(&_M_data[_i]);
            }
        }

        _M_size = _n;
    }

    void resize(std::size_t _n, const_reference val)
    {
        if (_n == _M_size)
            return;
        if (_n < _M_size)
        {
            for (std::size_t _i = _n; _i != _M_size; ++_i)
            {
                std::destroy_at(&_M_data[_i]);
            }
        }
        else if (_n > _M_size)
        {
            reserve(_n);
            for (std::size_t _i = _M_size; _i != _n; ++_i)
            {
                std::construct_at(&_M_data[_i], val);
            }
        }
        _M_size = _n;
    }

    void shrink_to_fit()
    {
        auto _old_data = _M_data;
        auto _old_cap = _M_cap;
        _M_cap = _M_size;
        if (_M_size == 0)
        {
            _M_data = nullptr;
        }
        else
        {
            _M_data = _M_alloc.allocate(_M_size);
        }

        if (_old_cap != 0) [[likely]]
        {
            if (_M_data)
                for (std::size_t _i = 0; _i != _M_size; ++_i)
                {
                    std::construct_at(&_M_data[_i], std::move_if_noexcept(_old_data[_i]));
                    std::destroy_at(&_old_data[_i]);
                }
            _M_alloc.deallocate(_old_data, _old_cap);
        }
    }

    void reserve(std::size_t _n)
    {
        if (_n <= _M_cap)
            return;
        _n = std::max(_n, _M_cap * 2);
        if (_n == 0)
        {
            _M_alloc.deallocate(_M_data, _M_cap);
            _M_data = nullptr;
            _M_cap = 0;
        }
        else
        {
            auto _old_data = _M_data;
            auto _old_cap = _M_cap;
            _M_data = _M_alloc.allocate(_n);
            _M_cap = _n;
            if (_old_cap == 0)
                return;

            for (std::size_t _i = 0; _i != _M_size; ++_i)
            {
                std::construct_at(&_M_data[_i], std::move_if_noexcept(_old_data[_i]));
            }

            for (std::size_t _i = 0; _i != _M_size; ++_i)
            {
                std::destroy_at(&_old_data[_i]);
            }

            _M_alloc.deallocate(_old_data, _old_cap);
        }
    }

    void swap(Vector &_other) noexcept
    {
        std::swap(_M_data, _other._M_data);
        std::swap(_M_cap, _other._M_cap);
        std::swap(_M_size, _other._M_size);
        std::swap(_M_alloc, _other._M_alloc);
    }

    void push_back(const_reference val)
    {
        if (_M_size + 1 >= _M_cap) [[unlikely]]
            reserve(_M_size + 1);
        std::construct_at(&_M_data[_M_size], val);
        _M_size = _M_size + 1;
    }

    void push_back(_Tp &&val)
    {
        if (_M_size + 1 >= _M_cap) [[unlikely]]
            reserve(_M_size + 1);
        std::construct_at(&_M_data[_M_size], std::move(val));
        _M_size = _M_size + 1;
    }

    template <class... Args>
    reference emplace_back(Args &&...args)
    {
        if (_M_size + 1 >= _M_cap) [[unlikely]]
            reserve(_M_size + 1);
        pointer _p = &_M_data[_M_size];
        std::construct_at(_p, std::forward<Args>(args)...);
        _M_size++;
        return *_p;
    }

    pointer erase(const_pointer _it) noexcept(std ::is_nothrow_move_assignable_v<_Tp>)
    {
        std::size_t _i = _it - _M_data;
        for (std::size_t _j = _i + 1; _j != _M_size; ++_j)
        {
            _M_data[_j - 1] = std::move(_M_data[_j]);
        }
        _M_size--;
        std::destroy_at(&_M_data[_M_size]);
        return const_cast<pointer>(_it);
    }

    pointer erase(const_pointer _first, const_pointer _last) noexcept(std ::is_nothrow_move_assignable_v<_Tp>)
    {
        std::size_t diff = _last - _first;
        for (std::size_t _j = _last - _M_data; _j != _M_size; ++_j)
        {
            _M_data[_j - diff] = _M_data[_j];
        }
        _M_size -= diff;
        for (std::size_t _j = _M_size; _j != (_M_size + diff); ++_j)
        {
            std::destroy_at(&_M_data[_j]);
        }
        return const_cast<pointer>(_first);
    }

    void assign(std::size_t _n, const_reference val)
    {
        if (_n != _M_cap)
        {
            clear();
            reserve(_n);
        }

        _M_size = _n;
        for (std::size_t _j = 0; _j != _M_size; ++_j)
        {
            std::construct_at(&_M_data[_j], val);
        }
    }

    template <typename _InputIt>
        requires std::random_access_iterator<_InputIt>
    void assign(_InputIt _first, _InputIt _end)
    {
        std::size_t diff = _end - _first;
        if (diff != _M_cap)
        {
            clear();
            reserve(diff);
        }

        _M_size = diff;
        for (std::size_t _i = 0; _i != _M_size; ++_i)
        {
            std::construct_at(_M_data + _i, *_first);
            ++_first;
        }
    }

    void assign(std::initializer_list<_Tp> _list)
    {
        assign(_list.begin(), _list.end());
    }

    template <class... Args>
    pointer emplace(const_pointer _it, Args &&...args)
    {
        std::size_t diff = _it - _M_data;
        reserve(_M_size + 1);
        for (std::size_t _i = _M_size; _i != diff; --_i)
        {
            std::construct_at(&_M_data[_i], std::move(_M_data[_i - 1]));
            std::destroy_at(&_M_data[_i - 1]);
        }
        _M_size++;
        std::construct_at(&_M_data[diff], std::forward<Args>(args)...);
        return _M_data + diff;
    }

    pointer insert(const_pointer _it, _Tp &&val)
    {
        std::size_t diff = _it - _M_data;
        reserve(_M_size + 1);
        for (std::size_t _i = _M_size; _i != diff; --_i)
        {
            std::construct_at(&_M_data[_i], std::move(_M_data[_i - 1]));
            std::destroy_at(&_M_data[_i - 1]);
        }
        _M_size++;
        std::construct_at(&_M_data[diff], std::move(val));
        return _M_data + diff;
    }

    pointer insert(const_pointer _it, const_reference val)
    {
        std::size_t diff = _it - _M_data;
        reserve(_M_size + 1);
        for (std::size_t _i = _M_size; _i != diff; --_i)
        {
            std::construct_at(&_M_data[_i], std::move(_M_data[_i - 1]));
            std::destroy_at(&_M_data[_i - 1]);
        }
        _M_size++;
        std::construct_at(&_M_data[diff], val);
        return _M_data + diff;
    }

    pointer insert(const_pointer _it, std::size_t _n, const_reference val)
    {
        std::size_t diff = _it - _M_data;
        if (_n == 0) [[unlikely]]
            return const_cast<pointer>(_it);
        reserve(_M_size + _n);
        for (std::size_t _i = _M_size; _i != diff; --_i)
        {
            std::construct_at(&_M_data[_i + _n - 1], std::move_if_noexcept(_M_data[_i - 1]));
            std::destroy_at(&_M_data[_i - 1]);
        }
        _M_size += _n;
        for (std::size_t _i = diff; _i != diff + _n; ++_i)
        {
            std::construct_at(&_M_data[_i], val);
        }
        return _M_data + diff;
    }

    template <typename _InputIt>
        requires std::random_access_iterator<_InputIt>
    pointer insert(const_pointer _it, _InputIt _first, _InputIt _last)
    {
        std::size_t diff = _last - _first;
        if (diff == 0) [[unlikely]]
            return const_cast<pointer>(_it);
        reserve(_M_size + diff);
        std::size_t _j = _it - _M_data;
        for (std::size_t _i = _M_size; _i != _j; --_i)
        {
            std::construct_at(&_M_data[_i + diff - 1], _M_data[_i - 1]);
            std::destroy_at(&_M_data[_i - 1]);
        }
        _M_size += diff;
        for (std::size_t _i = _j; _i != _j + diff; ++_i)
        {
            std::construct_at(&_M_data[_i], *_first);
            ++_first;
        }
        return _M_data + _j;
    }

    pointer insert(_Tp const *__it, std::initializer_list<_Tp> __ilist)
    {
        return insert(__it, __ilist.begin(), __ilist.end());
    }

public:
    reverse_iterator rbegin() noexcept
    {
        return std::make_reverse_iterator(_M_data + _M_size);
    }

    reverse_iterator rend() noexcept
    {
        return std::make_reverse_iterator(_M_data);
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return std::make_reverse_iterator(_M_data + _M_size);
    }

    const_reverse_iterator rend() const noexcept
    {
        return std::make_reverse_iterator(_M_data);
    }

public:
    const_reference at(std::size_t _i) const
    {
        if (_i >= _M_size) [[unlikely]]
            throw std::out_of_range("Vector::at");
        return _M_data[_i];
    }

    reference at(std::size_t _i)
    {
        if (_i >= _M_size) [[unlikely]]
            throw std::out_of_range("Vector::at");
        return _M_data[_i];
    }

public:
    std::size_t capacity() const noexcept
    {
        return _M_cap;
    }

    std::size_t size() const noexcept
    {
        return _M_size;
    }

    const_reference front() const noexcept
    {
        return *_M_data;
    }

    _Tp &front() noexcept
    {
        return *_M_data;
    }

    const_reference back() const noexcept
    {
        return _M_data[_M_size - 1];
    }

    _Tp &back()
    {
        return _M_data[_M_size - 1];
    }

    pointer data() noexcept
    {
        return _M_data;
    }

    const_pointer data() const noexcept
    {
        return _M_data;
    }

    const_pointer cdata() const noexcept
    {
        return _M_data;
    }

    _iterator begin() noexcept
    {
        return _M_data;
    }

    const_iterator begin() const noexcept
    {
        return _M_data;
    }

    const_iterator cbegin() const noexcept
    {
        return _M_data;
    }

    _iterator end() noexcept
    {
        return _M_data + _M_size;
    }

    const_iterator end() const noexcept
    {
        return _M_data + _M_size;
    }

    const_iterator cend() const noexcept
    {
        return _M_data + _M_size;
    }

    bool empty() const noexcept
    {
        return _M_size == 0;
    }

    static constexpr std::size_t max_size() noexcept
    {
        return std::numeric_limits<std::size_t>::max() / sizeof(_Tp);
    }

    ~Vector()
    {
        for (std::size_t _i = 0; _i != _M_size; ++_i)
        {
            std::destroy_at(&_M_data[_i]);
        }
        if (_M_cap != 0)
        {
            _M_alloc.deallocate(_M_data, _M_cap);
        }
    }

    _Alloc get_allocator() const noexcept
    {
        return _M_alloc;
    }
};
