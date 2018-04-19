#pragma once
#include "smemory.hpp"

#define NEAR_LARGE(m,n) ((m+n-1)/n*n)

template<typename T, typename POD = std::is_pod<T>::type>
class DataArray
{

};

template <typename T>
class DataArray<T, std::true_type>
{
public:
    DataArray(void)
    {
        m_size = 0;
        m_capacity = 0;
        m_array = 0;
        reserve(m_grow_size);
    }

    ~DataArray()
    {
        if (m_array)
        {
            S_FREE(m_array);
            m_array = 0;
        }
    }

    DataArray(const DataArray& src)
    {
        m_capacity = NEAR_LARGE(src.m_capacity, m_grow_size);

        m_size = src.m_size;
        m_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
        memcpy(m_array, src.m_array, sizeof(T)*m_size);
    }

    DataArray& operator= (const DataArray& src)
    {
        if (m_capacity < src.m_size)
        {
            m_capacity = NEAR_LARGE(src.m_capacity, m_grow_size);

            if (m_array)
            {
                S_FREE(m_array);
            }
            m_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
        }

        m_size = src.m_size;

        memcpy(m_array, src.m_array, sizeof(T)*m_size);

        return *this;
    }

    T& operator[](size_t idx)
    {
        if (idx < m_size)
        {
            return m_array[idx];
        }
        else
        {
            char* p = 0;
            *p = 'a';
            return m_array[0];
        }
    }

    void resize(size_t new_size)
    {
        if (new_size > m_capacity)
        {
            m_capacity = NEAR_LARGE(new_size, m_grow_size);

            m_array = (T*)S_REALLOC(m_array, sizeof(T)*m_capacity);
        }
        m_size = new_size;
    }

    void reserve(size_t new_size)
    {
        if (new_size > m_capacity)
        {
            m_capacity = NEAR_LARGE(new_size, m_grow_size);

            m_array = (T*)S_REALLOC(m_array, sizeof(T)*m_capacity);
        }
    }

    void push_back(const T& val)
    {
        if (m_size < m_capacity)
        {
            memcpy(m_array + m_size, &val, sizeof(T));
            m_size++;
        }
        else
        {
            m_capacity = NEAR_LARGE(m_capacity + m_capacity / 2, m_grow_size);
            m_array = (T*)S_REALLOC(m_array, sizeof(T)*m_capacity);
            memcpy(m_array + m_size, &val, sizeof(T));
            m_size++;
        }
    }

    inline size_t size(void)
    {
        return m_size;
    }

    inline size_t capacity(void)
    {
        return m_capacity;
    }

private:
    size_t  m_size;
    size_t  m_capacity;
    T*      m_array;
    static const size_t m_grow_size = 1024 / sizeof(T);
};

template <typename T>
class DataArray<T, std::false_type>
{
public:
    DataArray(void)
    {
        m_size = 0;
        m_capacity = 0;
        m_array = 0;
        reserve(m_grow_size);
    }

    ~DataArray()
    {
        if (m_array)
        {
            for (size_t i = 0; i < m_size; i++)
            {
                (m_array + i)->~T();
            }
            m_size = 0;

            S_FREE(m_array);
            m_array = 0;
        }
    }

    DataArray(const DataArray& src)
    {
        m_capacity = NEAR_LARGE(src.m_capacity, m_grow_size);
        m_size = src.m_size;

        m_array = S_MALLOC(sizeof(T)*m_capacity);
        for (size_t i = 0; i < m_size; i++)
        {
            new(m_array + i)T(src.m_array[i]);
        }
    }

    DataArray& operator= (const DataArray& src)
    {
        for (size_t i = 0; i < m_size; i++)
        {
            (m_array + i)->~T();
        }
        m_size = 0;

        if (m_capacity < src.m_size)
        {
            m_capacity = NEAR_LARGE(src.m_capacity, m_grow_size);

            if (m_array)
            {
                S_FREE(m_array);
            }

            m_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
        }

        m_size = src.m_size;

        for (size_t i = 0; i < m_size; i++)
        {
            new(m_array + i)T(src.m_array[i]);
        }

        return *this;
    }

    T& operator[](size_t idx)
    {
        if (idx < m_size)
        {
            return m_array[idx];
        }
        else
        {
            char* p = 0;
            *p = 'a';
            return m_array[0];
        }
    }

    void resize(size_t new_size)
    {
        if (new_size < m_size)
        {
            while (m_size > new_size)
            {
                (m_array + m_size - 1)->~T();
                m_size--;
            }
        }
        else if (new_size <= m_capacity)
        {
            while (m_size < new_size)
            {
                new(m_array + m_size)T();
                m_size++;
            }
        }
        else
        {
            m_capacity = NEAR_LARGE(new_size, m_grow_size);
            T* new_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
            for (size_t i = 0; i < m_size; i++)
            {
                new(new_array + i)T(m_array[i]);
                (m_array + i)->~T();
            }

            S_FREE(m_array);
            m_array = new_array;

            for (size_t i = m_size; i < m_capacity; i++)
            {
                new(m_array + i)T();
            }
            m_size = m_capacity;
        }
    }

    void reserve(size_t new_size)
    {
        if (new_size > m_capacity)
        {
            m_capacity = NEAR_LARGE(new_size, m_grow_size);
            T* new_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
            for (size_t i = 0; i < m_size; i++)
            {
                new(new_array + i)T(m_array[i]);
                (m_array + i)->~T();
            }

            S_FREE(m_array);
            m_array = new_array;
        }
    }

    void push_back(const T& val)
    {
        if (m_size < m_capacity)
        {
            new(m_array + m_size)T(val);
            m_size++;
        }
        else
        {
            m_capacity = NEAR_LARGE(m_capacity + m_capacity / 2, m_grow_size);
            T* new_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
            for (size_t i = 0; i < m_size; i++)
            {
                new(new_array + i)T(m_array[i]);
                (m_array + i)->~T();
            }
            new(new_array + m_size)T(val);
            ++m_size;

            S_FREE(m_array);
            m_array = new_array;
        }
    }

    inline size_t size(void)
    {
        return m_size;
    }

    inline size_t capacity(void)
    {
        return m_capacity;
    }

protected:
private:
    size_t  m_size;
    size_t  m_capacity;
    T*      m_array;
    static const size_t m_grow_size = 1024 / sizeof(T);
};

class CNetData
{
public:
    CNetData();
    virtual ~CNetData();

    void Prepare(char *pNetData, int iSize);
    void Reset();

    char * GetData();
    int GetDataLen();

    int AddByte(unsigned char byByte);
    int DelByte(unsigned char &byByte);

    int AddChar(char chChar);
    int DelChar(char &chChar);

    int AddWord(unsigned short wWord);
    int DelWord(unsigned short &wWord);

    int AddShort(short shShort);
    int DelShort(short &shShort);

    int AddDword(unsigned int dwDword);
    int DelDword(unsigned int &dwDword);

    int AddInt(int iInt);
    int DelInt(int &iInt);

    int AddInt64(signed long long iInt64);
    int DelInt64(signed long long &iInt64);

    int AddQword(unsigned long long qwQword);
    int DelQword(unsigned long long &qwQword);

    int AddString(const char *pszString, int iSize);
    int DelString(char *pszOut, int iSize);

    int AddBlob(const char *pszData, int iSize);
    int DelBlob(char *pszOut, int iSize);

protected:
    char * m_pBuf;
    int m_iSize;
    int m_iPos;
};

