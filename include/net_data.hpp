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
		m_cur_pos = m_buffer;
		m_end = m_buffer + reserve_size;
	}
	~NetEnCode()
	{
		if (m_buffer)
		{
			S_FREE(m_buffer);
		}
		m_buffer = 0;
		m_end = 0;
		m_cur_pos = 0;
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, void>::type AddIntegral(T var)
	{
		if (m_cur_pos + sizeof(T) > m_end)
		{
			__extend(sizeof(T));
		}

		*(T*)m_cur_pos = var;
		m_cur_pos += sizeof(T);
	}



	template<typename T, typename U>
	typename std::enable_if<std::is_integral<T>::value, void>::type AddArray(DataArray<T, U>& array)
	{
		AddIntegral(array.size());

		AddBlob(array.data(), array.size()*array.size_of_data());
	}

	template<typename T, typename U>
	typename std::enable_if<!std::is_integral<T>::value, void>::type AddArray(DataArray<T, U>& array)
	{
		AddIntegral(array.size());

		for (U i = 0; i < array.size(); i++)
		{
			array[i].EnCode(*this);
		}
	}

	void AddString(const char* str, size_t max_str_size)
	{
		size_t str_len = strnlen(str, max_str_size - 1);

		if (m_cur_pos + str_len + sizeof(unsigned short) > m_end)
		{
			__extend(str_len + sizeof(unsigned short));
		}

		AddIntegral((unsigned short)str_len);

		memcpy(m_cur_pos, str, str_len);
		m_cur_pos += str_len;
	}

	void AddBlob(const void* data, size_t data_size)
	{
		if (m_cur_pos + data_size > m_end)
		{
			__extend(data_size);
		}

		memcpy(m_cur_pos, data, data_size);

		m_cur_pos += data_size;
	}

	inline void* Data()
	{
		return m_buffer;
	}

	inline size_t Length()
	{
		return m_cur_pos - m_buffer;
	}

	inline void Clear()
	{
		m_cur_pos = m_buffer;
	}
protected:
	void __extend(size_t min_size)
	{
		size_t cur_size = m_end - m_buffer;
		size_t cur_pos = m_cur_pos - m_buffer;

		if ((std::numeric_limits<size_t>::max)() - cur_size < cur_size / 2)
		{
			if (cur_size == (std::numeric_limits<size_t>::max)())
			{
				char* p = 0;
				*p = 'a';
			}
			else
			{
				cur_size = (std::numeric_limits<size_t>::max)();
			}
		}
		else
		{
			for (;;)
			{
				cur_size += cur_size / 2;
				if (cur_size - (m_cur_pos - m_buffer) >= min_size)
				{
					break;
				}

				if ((std::numeric_limits<size_t>::max)() - cur_size < cur_size / 2)
				{
					char* p = 0;
					*p = 'a';
				}
			}
		}


		m_buffer = (char*)S_REALLOC(m_buffer, cur_size);
		m_cur_pos = m_buffer + cur_pos;
		m_end = m_buffer + cur_size;
	}
private:
	char*		m_buffer;
	char*		m_cur_pos;
	char*		m_end;
};

class NetDeCode
{
public:
	NetDeCode(const void* data, size_t data_len)
	{
		m_buffer = (const char*)data;
		m_cur_pos = m_buffer;
		m_end = m_buffer + data_len;
	}
	~NetDeCode()
	{
		m_buffer = 0;
		m_cur_pos = 0;
		m_end = 0;
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, const char*>::type DelIntegral(T& var)
	{
		if (m_cur_pos + sizeof(T) > m_end)
			return 0;

		var = *(T*)m_cur_pos;
		m_cur_pos += sizeof(T);

		return m_cur_pos;
	}

	template<typename T, typename U>
	typename std::enable_if<std::is_integral<T>::value, const char*>::type DelArray(DataArray<T, U>& array)
	{
		U array_size = 0;
		if (!DelIntegral(array_size))
			return 0;

		array.resize(array_size);

		if (!DelBlob(array.data(), array.size()*array.size_of_data()))
			return 0;

		return m_cur_pos;
	}

	template<typename T, typename U>
	typename std::enable_if<!std::is_integral<T>::value, const char*>::type DelArray(DataArray<T, U>& array)
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

		return m_cur_pos;
	}

	const char* DelString(char* str, size_t max_str_size)
	{
		unsigned short str_len;

		if (!DelIntegral(str_len))
			return 0;

		//if (m_size - m_pos < str_len)
		//	return 0;
		if (m_cur_pos + str_len > m_end)
			return 0;

		if (str_len + 1 > max_str_size)
			return 0;

		memcpy(str, m_cur_pos, str_len);
		str[str_len] = '\0';
		m_cur_pos += str_len;

		return m_cur_pos;
	}

	const char* DelBlob(void* data, size_t data_size)
	{
		if (m_cur_pos + data_size > m_end)
			return 0;

		memcpy(data, m_cur_pos, data_size);
		m_cur_pos += data_size;

		return m_cur_pos;
	}

	inline void Reset(size_t pos = 0)
	{
		if (m_buffer + pos  < m_end)
		{
			m_cur_pos = m_buffer + pos;
		}
	}

	inline const char* DataByPos(size_t pos)
	{
		if (m_buffer + pos < m_end)
		{
			return m_buffer + pos;
		}

		return 0;
	}

	inline size_t CurPos()
	{
		return m_cur_pos - m_buffer;
	}

protected:
private:
	const char*	m_buffer;
	const char* m_cur_pos;
	const char* m_end;
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


