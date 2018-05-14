#pragma once
#include "smemory.hpp"

template< typename T, typename U = size_t, bool is_pod = std::is_pod<T>::value, bool is_unsigned = std::is_unsigned<U>::value >
class DataArray
{

};

template <typename T, typename U>
class DataArray<T, U, true, true>
{
public:
    DataArray(void)
    {
        m_size = 0;
        m_capacity = 0;
        m_array = 0;
        reserve(2);
    }

    ~DataArray()
    {
        if (m_array)
        {
            S_FREE(m_array);
            m_array = 0;
            m_size = 0;
            m_capacity = 0;
        }
    }

    DataArray(const DataArray& src)
    {
        m_capacity = src.m_capacity;

        m_size = src.m_size;
        m_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
        memcpy(m_array, src.m_array, sizeof(T)*m_size);
    }

    DataArray& operator= (const DataArray& src)
    {
        if (m_capacity < src.m_size)
        {
            m_capacity = src.m_capacity;

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

    T& operator[](U idx)
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

    void resize(U new_size)
    {
        if (new_size > m_capacity)
        {
            m_capacity = new_size;

            m_array = (T*)S_REALLOC(m_array, sizeof(T)*m_capacity);
        }
        m_size = new_size;
    }

    void reserve(U new_size)
    {
        if (new_size > m_capacity)
        {
            m_capacity = new_size;

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
            if ((std::numeric_limits<U>::max)() - m_capacity < m_capacity/2)
            {
                if (m_capacity == (std::numeric_limits<U>::max)())
                {
                    char* p = 0;
                    *p = 'a';
                }
                else
                {
                    m_capacity = (std::numeric_limits<U>::max)();
                }
            }
            else
            {
                m_capacity += m_capacity / 2;
            }


            m_array = (T*)S_REALLOC(m_array, sizeof(T)*m_capacity);
            memcpy(m_array + m_size, &val, sizeof(T));
            m_size++;
        }
    }

    inline void clear()
    {
        m_size = 0;
    }

    inline U size(void)
    {
        return m_size;
    }

	inline size_t size_of_data(void)
	{
		return sizeof(T);
	}

    inline U capacity(void)
    {
        return m_capacity;
    }

	inline T* data(void)
	{
		return m_array;
	}

private:
    U   m_size;
    U   m_capacity;
    T*  m_array;
};

template <typename T, typename U>
class DataArray<T, U, false, true>
{
public:
    DataArray(void)
    {
        m_size = 0;
        m_capacity = 0;
        m_array = 0;
        reserve(2);
    }

    ~DataArray()
    {
        if (m_array)
        {
            for (U i = 0; i < m_size; i++)
            {
                (m_array + i)->~T();
            }
            m_size = 0;

            S_FREE(m_array);
            m_array = 0;

            m_capacity = 0;
        }
    }

    DataArray(const DataArray& src)
    {
        m_capacity = src.m_capacity;
        m_size = src.m_size;

        m_array = S_MALLOC(sizeof(T)*m_capacity);
        for (U i = 0; i < m_size; i++)
        {
            new(m_array + i)T(src.m_array[i]);
        }
    }

    DataArray& operator= (const DataArray& src)
    {
        for (U i = 0; i < m_size; i++)
        {
            (m_array + i)->~T();
        }
        m_size = 0;

        if (m_capacity < src.m_size)
        {
            m_capacity = src.m_capacity;

            if (m_array)
            {
                S_FREE(m_array);
            }

            m_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
        }

        m_size = src.m_size;

        for (U i = 0; i < m_size; i++)
        {
            new(m_array + i)T(src.m_array[i]);
        }

        return *this;
    }

    T& operator[](U idx)
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

    void resize(U new_size)
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
            m_capacity = new_size;

            T* new_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
            for (U i = 0; i < m_size; i++)
            {
                new(new_array + i)T(m_array[i]);
                (m_array + i)->~T();
            }

            S_FREE(m_array);
            m_array = new_array;

            for (U i = m_size; i < m_capacity; i++)
            {
                new(m_array + i)T();
            }
            m_size = m_capacity;
        }
    }

    void reserve(U new_size)
    {
        if (new_size > m_capacity)
        {
            m_capacity = new_size;

            T* new_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
            for (U i = 0; i < m_size; i++)
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
            if ((std::numeric_limits<U>::max)() - m_capacity < m_capacity / 2)
            {
                if (m_capacity == (std::numeric_limits<U>::max)())
                {
                    char* p = 0;
                    *p = 'a';
                }
                else
                {
                    m_capacity = (std::numeric_limits<U>::max)();
                }
            }
            else
            {
                m_capacity += m_capacity / 2;
            }

            T* new_array = (T*)S_MALLOC(sizeof(T)*m_capacity);
            for (U i = 0; i < m_size; i++)
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

    void clear()
    {
        if (m_array)
        {
            for (U i = 0; i < m_size; i++)
            {
                (m_array + i)->~T();
            }
            m_size = 0;
        }
    }

    inline U size(void)
    {
        return m_size;
    }

	inline size_t size_of_data(void)
	{
		return sizeof(T);
	}

    inline U capacity(void)
    {
        return m_capacity;
    }

	inline T* data(void)
	{
		return m_array;
	}

protected:
private:
    U   m_size;
    U   m_capacity;
    T*  m_array;
};


class NetEnCode
{
public:
	NetEnCode(size_t reserve_size)
	{
		m_buffer = (char*)S_MALLOC(reserve_size);
		m_pos = 0;
		m_size = reserve_size;
	}
	~NetEnCode()
	{
		if (m_buffer)
		{
			S_FREE(m_buffer);
		}
		m_buffer = 0;
		m_size = 0;
		m_pos = 0;
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, size_t>::type AddIntegral(T var)
	{
		if (m_size - m_pos < sizeof(T))
		{
			if ((std::numeric_limits<size_t>::max)() - m_size < m_size / 2)
			{
				if (m_size == (std::numeric_limits<size_t>::max)())
				{
					char* p = 0;
					*p = 'a';
				}
				else
				{
					m_size = (std::numeric_limits<size_t>::max)();
				}
			}
			else
			{
				for (;;)
				{
					m_size += m_size / 2;
					if (m_size - m_pos >= sizeof(T))
					{
						break;
					}

					if ((std::numeric_limits<size_t>::max)() - m_size < m_size / 2)
					{
						char* p = 0;
						*p = 'a';
					}
				}
			}

			m_buffer = (char*)S_REALLOC(m_buffer, m_size);
		}

		*(T*)(m_buffer + m_pos) = var;
		m_pos += sizeof(T);

		return m_pos;
	}

	template<typename T, typename U>
	typename std::enable_if<std::is_integral<T>::value, size_t>::type AddArray(DataArray<T, U>& array)
	{
		AddIntegral(array.size());

		AddBlob(array.data(), array.size()*array.size_of_data());

		return m_pos;
	}

	template<typename T, typename U>
	typename std::enable_if<!std::is_integral<T>::value, size_t>::type AddArray(DataArray<T, U>& array)
	{
		AddIntegral(array.size());

		for (U i = 0; i < array.size(); i++)
		{
			array[i].EnCode(*this);
		}

		return m_pos;
	}

	size_t AddString(const char* str, size_t max_str_size)
	{
		size_t str_len = strnlen(str, max_str_size - 1);

		if (m_size - m_pos < str_len + sizeof(unsigned short))
		{
			if ((std::numeric_limits<size_t>::max)() - m_size < m_size / 2)
			{
				if (m_size == (std::numeric_limits<size_t>::max)())
				{
					char* p = 0;
					*p = 'a';
				}
				else
				{
					m_size = (std::numeric_limits<size_t>::max)();
				}
			}
			else
			{
				for (;;)
				{
					m_size += m_size / 2;
					if (m_size - m_pos >= str_len + sizeof(unsigned short))
					{
						break;
					}

					if ((std::numeric_limits<size_t>::max)() - m_size < m_size / 2)
					{
						char* p = 0;
						*p = 'a';
					}
				}
			}

			m_buffer = (char*)S_REALLOC(m_buffer, m_size);
		}

		if (str_len > (std::numeric_limits<unsigned short>::max)())
		{
			str_len = (std::numeric_limits<unsigned short>::max)();
		}

		AddIntegral((unsigned short)str_len);

		memcpy(m_buffer + m_pos, str, str_len);
		m_pos += str_len;

		return m_pos;
	}

	size_t AddBlob(const void* data, size_t data_size)
	{
		if (m_size - m_pos < data_size)
		{
			if ((std::numeric_limits<size_t>::max)() - m_size < m_size / 2)
			{
				if (m_size == (std::numeric_limits<size_t>::max)())
				{
					char* p = 0;
					*p = 'a';
				}
				else
				{
					m_size = (std::numeric_limits<size_t>::max)();
				}
			}
			else
			{
				for (;;)
				{
					m_size += m_size / 2;
					if (m_size - m_pos >= data_size)
					{
						break;
					}

					if ((std::numeric_limits<size_t>::max)() - m_size < m_size / 2)
					{
						char* p = 0;
						*p = 'a';
					}
				}
			}

			m_buffer = (char*)S_REALLOC(m_buffer, m_size);
		}

		memcpy(m_buffer + m_pos, data, data_size);

		m_pos += data_size;

		return m_pos;
	}

	inline void* Data()
	{
		return m_buffer;
	}

	inline size_t Length()
	{
		return m_pos;
	}

	inline void Clear()
	{
		m_pos = 0;
	}
protected:
private:
	char*		m_buffer;
	size_t		m_size;
	size_t      m_pos;
};

class NetDeCode
{
public:
	NetDeCode(const void* data, size_t data_len)
	{
		m_buffer = (const char*)data;
		m_size = data_len;
		m_pos = 0;
	}
	~NetDeCode()
	{
		m_buffer = 0;
		m_size = 0;
		m_pos = 0;
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, size_t>::type DelIntegral(T& var)
	{
		if (m_size - m_pos < sizeof(T))
			return 0;
		var = *(T*)(m_buffer + m_pos);
		m_pos += sizeof(T);

		return m_pos;
	}

	template<typename T, typename U>
	typename std::enable_if<std::is_integral<T>::value, size_t>::type DelArray(DataArray<T, U>& array)
	{
		U array_size = 0;
		if (!DelIntegral(array_size))
			return 0;

		array.resize(array_size);

		if (!DelBlob(array.data(), array.size()*array.size_of_data()))
			return 0;

		return m_pos;
	}

	template<typename T, typename U>
	typename std::enable_if<!std::is_integral<T>::value, size_t>::type DelArray(DataArray<T, U>& array)
	{
		U array_size = 0;
		if (!DelIntegral(array_size))
			return 0;

		array.resize(array_size);

		for (U i = 0; i < array_size; i++)
		{
			if (!array[i].DeCode(*this))
			{
				return 0;
			}
		}

		return m_pos;
	}

	size_t DelString(char* str, size_t max_str_size)
	{
		unsigned short str_len;

		if (!DelIntegral(str_len))
			return 0;

		if (m_size - m_pos < str_len)
			return 0;

		if (str_len + 1 > max_str_size)
			return 0;

		memcpy(str, m_buffer + m_pos, str_len);
		str[str_len] = '\0';
		m_pos += str_len;

		return m_pos;
	}

	size_t DelBlob(void* data, size_t data_size)
	{
		if (m_size - m_pos < data_size)
			return 0;

		memcpy(data, m_buffer + m_pos, data_size);
		m_pos += data_size;

		return m_pos;
	}

	inline void Reset(size_t pos = 0)
	{
		if (pos < m_size)
		{
			m_pos = pos;
		}
	}

	inline const void* DataByPos(size_t pos)
	{
		if (pos < m_size)
		{
			return m_buffer + pos;
		}
		return 0;
	}

	inline size_t CurPos()
	{
		return m_pos;
	}

protected:
private:
	const char*	m_buffer;
	size_t		m_size;
	size_t      m_pos;
};


struct protocol_base 
{
    const unsigned short module_id;
    const unsigned short protocol_id;
	protocol_base( unsigned short m_id,
        unsigned short p_id):
        module_id(m_id), protocol_id(p_id){}

	virtual bool EnCode(NetEnCode& net_data) = 0;
};


