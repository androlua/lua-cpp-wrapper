#pragma once

#include <string>
#include <codecvt>
#include <cstdlib>
#include <type_traits>
#include "MacroDefBase.h"
#include "lua_wrapper_base.h"

SHARELIB_BEGIN_NAMESPACE

//---------------------------------------------------
/* tableԪ�ص�kay
1. ����lua_ostreamʱ, �� <<lua_table_key_t, �� <<tableԪ��ֵ��luaջ��, ��Ԫ��ֵ��key�㱻ָ����;
   ��ָ��kay�������, ��˳�����δ��롣����ָ��key�벻ָ��key�����ַ�ʽ��Ҫ���ã�lua�ڲ�Ҫ����hash����
   ����ʱ����lua��˳�򲢲��������ڲ�����ʵ˳�򣬵��´�lua�ж�ȡʱ˳��������˳����ͬ��
2. ����lua_istreamʱ, �� >>lua_table_key_t, �� >>����, ����԰�ָ����tableԪ�ض�ȡ������;
*/
struct lua_table_key_t
{
    explicit lua_table_key_t(const char * pKey)
        : m_pKey(pKey)
    {
    }
    const char * m_pKey;
};

//----��luaջ��push����-----------------------------------

/* ���ص� << �����ԭ��:
lua_ostream & operator << (lua_ostream & os, const T & value);
*/
class lua_ostream
{
    SHARELIB_DISABLE_COPY_CLASS(lua_ostream);
public:
    explicit lua_ostream(lua_State * pLua);

    lua_State * get() const;

//----��ֵ����--------------------------
    lua_ostream & operator << (bool value);
    lua_ostream & operator << (char value);
    lua_ostream & operator << (unsigned char value);
    lua_ostream & operator << (wchar_t value);
    lua_ostream & operator << (short value);
    lua_ostream & operator << (unsigned short value);
    lua_ostream & operator << (int value);
    lua_ostream & operator << (unsigned int value);
    lua_ostream & operator << (long value);
    lua_ostream & operator << (unsigned long value);
    lua_ostream & operator << (long long value);
    lua_ostream & operator << (unsigned long long value);
    lua_ostream & operator << (float value);
    lua_ostream & operator << (double value);
    lua_ostream & operator << (long double value);

//----char�ַ���----------------------------
    lua_ostream & operator << (char * value);
    lua_ostream & operator << (const char * value);
    template<class T1, class T2>
    lua_ostream & operator << (const std::basic_string<char, T1, T2> & value)
    {
        return (*this) << (const char *)value.c_str();
    }

//----wchar_t�ַ���----------------------------
    lua_ostream & operator << (wchar_t * value);
    lua_ostream & operator << (const wchar_t * value);
    template<class T1, class T2>
    lua_ostream & operator << (const std::basic_string<wchar_t, T1, T2> & value)
    {
#ifdef LUA_CODE_UTF8
        try
        {
            std::wstring_convert < std::codecvt_utf8_utf16<wchar_t> > cvt;
            return (*this) << cvt.to_bytes(value).c_str();
        }
        catch (...)
        {
            return *this;
        }
#else
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif
        std::string temp;
        temp.assign(value.size() * sizeof(wchar_t), '\0');
        size_t offset = std::wcstombs(&temp[0], value.c_str(), temp.size());
        temp.erase(offset);
        return (*this) << temp.c_str();
#ifdef _MSC_VER
#pragma warning(default:4996)
#endif
#endif
    }

//----ָ��----------------------------
    template<class T>
    lua_ostream & operator << (T * value)
    {
        lua_pushlightuserdata(m_pLua, value);
        check_table_push();
        return *this;
    }

//----table----------------------------
    struct table_begin_t{};
    static const table_begin_t table_begin;
    struct table_end_t{};
    static const table_end_t table_end;

    /*�������ʹ��, ��ʾtable�������ֹ, ��֧��Ƕ��.
    һ����Ե�table����֮��, ջ����������¼����table
    */
    lua_ostream & operator << (table_begin_t);
    lua_ostream & operator << (table_end_t);

    /* �����key��lua��, �ٰ�ֵ�����luaջ��, ջ���ĵ�ֵ��ȡ������
    */
    lua_ostream & operator << (lua_table_key_t key);

    /** ���ڴ���Ƕ�׵�tableʱ�����lua_ostream << table_begin��
    �������¹���һ��lua_ostream�������������ڲ�table��
    ֮���Ƕ�׵���table���뵽���table�С�
    @param[in] subTable ���������һ���������ڴ�table
    */
    void insert_subtable(lua_ostream & subTable);

private:
    /* д������ʵ�֣�
    1. д�����ݵ�ջ��;
    2. check_table_push, ��ջ�ϵ�tableԪ��ѹ��table��;
    3. ������lua_table_key_tʱ, �ַ�����ջ, �´���������ʱ, lua_rawset
    */
    void check_table_push();

    lua_State * const m_pLua;
    int m_tableIndex;
};

//----��lua��ָ����ջλ�ö�ȡ����--------------------------

/* ��lua��ָ����ջλ�ö�ȡ����, һ��lua_istream������eof()Ϊtrue֮ǰ
 ���Զ���֮��㲻���ٶ����������¹���һ���������ָ����ջλ����table��������
 ��ȡtable�ĸ���Ԫ�أ�ֱ��eof()Ϊtrue

 ���ص� >> �����ԭ��:
lua_istream & operator >> (lua_istream & is, T & value); 
*/
class lua_istream
{
    SHARELIB_DISABLE_COPY_CLASS(lua_istream);
public:
    lua_istream(lua_State * pLua, int stackIndex);
    ~lua_istream();
    lua_State * get() const;
    int index() const;

    //�����Ƿ��Ѷ���,ͬһ��lua_istream����,һ�����Ͳ����ٶ�,���Ҫ�ظ���,�������¹���һ��
    bool eof() const;

    //�ϴζ�ȡ�Ƿ�ɹ�, ����Ӱ���´εĶ�ȡ, ������ȡ�Ļ�, �ϴ�ʧ��Ҳ������
    bool bad() const;
    operator void*() const; //����bool�ж�

    //������Զ�ȡ��ֵ�Ƿ���һ��Ƕ�׵���table������Ҫ��������һ��table
    bool isSubTable() const;

//----��ֵ����--------------------------
    lua_istream & operator >> (bool & value);
    lua_istream & operator >> (char & value);
    lua_istream & operator >> (unsigned char & value);
    lua_istream & operator >> (wchar_t & value);
    lua_istream & operator >> (short & value);
    lua_istream & operator >> (unsigned short & value);
    lua_istream & operator >> (int & value);
    lua_istream & operator >> (unsigned int & value);
    lua_istream & operator >> (long & value);
    lua_istream & operator >> (unsigned long & value);
    lua_istream & operator >> (long long & value);
    lua_istream & operator >> (unsigned long long & value);
    lua_istream & operator >> (float & value);
    lua_istream & operator >> (double & value);
    lua_istream & operator >> (long double & value);

//----char�ַ���----------------------------
    template<class T1, class T2>
    lua_istream & operator >> (std::basic_string<char, T1, T2> & value)
    {
        if (!m_isEof)
        {
            m_isOK = (lua_type(m_pLua, get_value_index()) == LUA_TSTRING);
            if (m_isOK)
            {
                value = lua_tostring(m_pLua, get_value_index());
            }
            next();
        }
        return *this;
    }

//----wchar_t�ַ���----------------------------
    template<class T1, class T2>
    lua_istream & operator >> (std::basic_string<wchar_t, T1, T2> & value)
    {
        std::string temp;
        if ((*this) >> temp)
        {
#ifdef LUA_CODE_UTF8
            try
            {
                std::wstring_convert < std::codecvt_utf8_utf16<wchar_t> > cvt;
                value = cvt.from_bytes(temp);
            }
            catch (...)
            {
                value.clear();
            }
#else
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif
            value.assign(temp.size(), '\0');
            size_t offset = std::mbstowcs(&value[0], temp.c_str(), value.size());
            value.erase(offset);
#ifdef _MSC_VER
#pragma warning(default:4996)
#endif
#endif
        }
        return *this;
    }

//----ָ��----------------------------
    template<class T>
    lua_istream & operator >> (T * & value)
    {
        if (!m_isEof)
        {
            m_isOK = (lua_type(m_pLua, get_value_index()) == LUA_TLIGHTUSERDATA);
            if (m_isOK)
            {
                value = (T*)lua_touserdata(m_pLua, get_value_index());
            }
            next();
        }
        return *this;
    }

//----table----------------------------
    /* �� >>key, ���� >>����, �ͱ�ʾ��table��ָ�����ֵ�ֵ��ȡ������
     */
    lua_istream & operator >> (lua_table_key_t key);

    /* ���ֵ��table����������������ȡtableԪ�ص�����������֧��tableǶ�׵������������ȡ��
    tableǶ��ʱ��ǰ��Ķ���֮��ջ��������table����ʱ����һ���µ�lua_istream(pLua, -1); 
    ������µ�lua_istream�Ϳ���������ȡ��table�еı����ˡ�
    ��ȡ�����table�󣬴����������������ջ����
    */
    void cleanup_subtable(lua_istream & subTable);

private:
    /* ��ȡ����ʵ��:
    1. �ж��Ƿ�ͷ
    1. �������
    2. ��get_value_indexȡ�����ʵ�����, ת������;
    3. ��ת����һ���������table����һ����ֵ�Գ�ջ����һ����ֵ����ջ; 
    4. ������lua_table_key_tʱ, lua_rawget��ֵ��ջ���´����ȶ�ȡ�������ջ��
    */
    int get_value_index();
    void next();

    lua_State * const m_pLua;
    const int m_stackIndex;
    const int m_top;
    bool m_isEof;
    bool m_isOK;
    bool m_isTableKey;
};

//----lua����C++����ʱ, ���μ�����ֵ��ת����------------------------------------------------------

/* lua����C++ʱ, ���μ�����ֵ��ת����
1. ����C++����ʱ�Ĵ���: ����from_lua��lua�ж�ȡ���ݴ���, from_lua ת�ӵ� lua_istream , ����ʵ�ֶ�ȡ����
2. ����C++����ʱ�ķ���ֵ: ����to_lua������д�뵽lua��, to_lua ת�ӵ� lua_ostream , ����ʵ���������
3. ֧���Զ������͵�����(�Զ�������Ҫ��Ĭ�Ϲ��캯��, ���Ը��ƻ��ƶ�)��
    ���ڲ������Զ�������, �������� lua_istream �� >> �����;
    ���ڷ���ֵ���Զ�������, �������� lua_ostream �� << �����;
*/
template<class T, 
    bool isEnum = std::is_enum<std::decay_t<T>>::value>
struct lua_io_dispatcher
{
    /** ����push��luaջ��
    @param[in] pL 
    @param[in] value ����
    @return luaջ�����ӵĸ���
    */
    static int to_lua(lua_State * pL, const T & value)
    {
        auto n = lua_gettop(pL);
        lua_ostream os(pL);
        os << value;
        assert(lua_gettop(pL) >= n);
        return lua_gettop(pL) - n;
    }

    /** ��luaջ�϶�ȡ����
    @param[in] pL 
    @param[in] index luaջ�ϵ�����λ��
    @return ת�����C++����
    */
    static T from_lua(lua_State * pL, int index, T defaultValue = T{})
    {
        lua_stack_check checker(pL);
        T temp{};
        lua_istream is(pL, index);
        if (is >> temp)
        {
            return std::move(temp);
        }
        else
        {
            return std::move(defaultValue);
        }
    }
};

template<class T>
struct lua_io_dispatcher<T, true>
{
    using _UType = typename std::underlying_type_t<T>;
    static int to_lua(lua_State * pL, T value)
    {
        return lua_io_dispatcher<_UType>::to_lua(pL, static_cast<_UType>(value));
    }

    static T from_lua(lua_State * pL, int index, T defaultValue = T{})
    {
        lua_stack_check checker(pL);
        return static_cast<T>(lua_io_dispatcher<_UType>::from_lua(pL, index, defaultValue));
    }
};

template<>
struct lua_io_dispatcher<const char*, false>
{
    static int to_lua(lua_State * pL, const char* value)
    {
        lua_ostream os(pL);
        os << value;
        return 1;
    }

    static const char* from_lua(lua_State * pL, int index, const char * defaultValue = "")
    {
        lua_stack_check checker(pL);
        if (lua_type(pL, index) == LUA_TSTRING)
        {
            return lua_tostring(pL, index);
        }
        return defaultValue;
    }
};

template<>
struct lua_io_dispatcher<char*, false>
    : public lua_io_dispatcher<const char*, false>
{
};

namespace Internal
{
    /* Ϊ�˽��wchar_t* ���͵�ģ���ػ�.
    �������������һ������const wchar_t*�Ļ�, �����ڲ�����ʱ��������֮�󣬷��ص�ָ��ͳ���Ұָ�롣
    const char* ������������⣬��Ϊָ����ָ��������lua�ڲ������ţ�û�б����١�
    �������Ҫ����const wchar_t*�ĵط�����Ϊ����StdWstringWrapper������������ʽת��Ϊconst wchar_t*��
    �����ڲ�����ʱ������StdWstringWrapperת��֮���������ӳ���������Ұָ������
    */
    struct StdWstringWrapper
    {
        StdWstringWrapper(std::wstring && stdstr)
        {
            m_str.swap(stdstr);
        }

        operator const wchar_t *() const
        {
            return m_str.c_str();
        }

        operator wchar_t *() const
        {
            return (wchar_t *)m_str.c_str();
        }

        std::wstring m_str;
    };
}

template<>
struct lua_io_dispatcher<const wchar_t*, false>
{
    static int to_lua(lua_State * pL, const wchar_t* value)
    {
        lua_ostream os(pL);
        os << value;
        return 1;
    }

    static Internal::StdWstringWrapper from_lua(lua_State * pL, int index, const wchar_t * defaultValue = L"")
    {
        lua_stack_check checker(pL);
        std::wstring temp;
        lua_istream is(pL, index);
        if (is >> temp)
        {
            return std::move(temp);
        }
        else
        {
            return std::wstring(defaultValue);
        }
    }
};

template<>
struct lua_io_dispatcher<wchar_t*, false>
    : public lua_io_dispatcher<const wchar_t*, false>
{
};

SHARELIB_END_NAMESPACE
