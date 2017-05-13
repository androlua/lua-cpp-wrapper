#pragma once
#include <cassert>
#include "lua/src/lua.hpp"
#include "MacroDefBase.h"

//lua�ű�ʹ�õı���, ���ʹ��utf8, ����ú�. δ����ʱĬ��ΪASCII����
//#define LUA_CODE_UTF8

SHARELIB_BEGIN_NAMESPACE

//----luaջ�����ĸ�����-------------------------------------------------------------

//���ջԪ�������Ƿ����ĸ�����,�����Զ����������Ԫ�أ�ֻ��debugģʽ��Ч
class lua_stack_check
{
public:
    explicit lua_stack_check(lua_State * pL)
#ifndef NDEBUG
        : m_pL(pL)
        , m_nCount(lua_gettop(pL))
#endif
    {
        (void)pL;
    }
    ~lua_stack_check()
    {
#ifndef NDEBUG
        assert(m_nCount == lua_gettop(m_pL));
#endif
    }

#ifndef NDEBUG
private:
    lua_State * m_pL;
    int m_nCount;
#endif
};

//�Զ�����ջԪ��,ȷ����������ĸ�����
class lua_stack_guard
{
public:
    explicit lua_stack_guard(lua_State * pL)
        : m_pL(pL)
        , m_nCount(lua_gettop(pL))
    {
    }

    ~lua_stack_guard()
    {
        int nCount = lua_gettop(m_pL);
        assert(nCount >= m_nCount);
        if (nCount > m_nCount)
        {
            lua_pop(m_pL, (nCount - m_nCount));
        }
    }
private:
    lua_State * m_pL;
    int m_nCount;
};

SHARELIB_END_NAMESPACE
