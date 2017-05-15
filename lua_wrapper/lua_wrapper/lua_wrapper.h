#pragma once

/* 
version: 0.1
*/

#include <cstdint>
#include <initializer_list>
#include "MacroDefBase.h"
#include "lua_iostream.h"
#include "MetaUtility.h"

SHARELIB_BEGIN_NAMESPACE

//----lua����C++��ת�Ӹ���---------------------------------------------
namespace Internal
{
//----��鹩lua���õ�C���������Ƿ�Ϸ�-----------------------------
    template<class ...T>
    struct CheckCFuncArgValid;

    template<>
    struct CheckCFuncArgValid<>
    {
    };

    template<class T, class ...Rest>
    struct CheckCFuncArgValid<T, Rest...>
        : public CheckCFuncArgValid<Rest...>
    {
        //���ܰ�����ֵ���õĲ���
        static_assert(!std::is_lvalue_reference<T>::value 
        || std::is_const<std::remove_reference_t<T>>::value,
        "unsupport left value reference parameter");
    };

    template<class ...T>
    struct CheckCFuncArgValid<std::tuple<T...> >
        : public CheckCFuncArgValid<T...>
    {
    };

//----����ͨ����, ��Ա����, ��Ա���� ����ֵ�Ƿ�Ϊvoid, ���ֵ���------------------------------------------

    /** lua���÷ַ���
    @Tparam[in] callId: ����ֵ, ��ʾ��������
    @Tparam[in] returnVoid: ����ֵ, ��ʾ���������Ƿ�Ϊvoid
    @Tparam[in] _CallableType: CallableTypeHelper������
    @Tparam[in] _IndexType: �������к�
    */
    template<CallableIdType callId, bool returnVoid, class _CallableType, class _IndexType>
    struct luaCFunctionDispatcher;

    //ģ���ػ�, ����ָ��, ����void
    template<class _CallableType, size_t ... index>
    struct luaCFunctionDispatcher<CallableIdType::POINTER_TO_FUNCTION, true, _CallableType, ArgIndex<index...> >
    {
        template<class _PfType>
        static int Apply(lua_State * pLua, _PfType pf)
        {
            (void)pLua;//����0��0����ֵʱ�ľ���
            pf(lua_io_dispatcher<
                std::decay_t<std::tuple_element<index, typename _CallableType::arg_tuple_t>::type >
                >::from_lua(pLua, index + 1)...);
            return 0;
        }
    };

    //ģ���ػ�, ����ָ��, �з���ֵ
    template<class _CallableType, size_t ... index>
    struct luaCFunctionDispatcher<CallableIdType::POINTER_TO_FUNCTION, false, _CallableType, ArgIndex<index...> >
    {
        template<class _PfType>
        static int Apply(lua_State * pLua, _PfType pf)
        {
            using result_type = std::decay_t<typename _CallableType::result_t>;
            return lua_io_dispatcher<result_type>::to_lua(
                pLua,
                pf(lua_io_dispatcher<
                    std::decay_t<std::tuple_element<index, typename _CallableType::arg_tuple_t>::type >
                    >::from_lua(pLua, index + 1)...)
                );
        }
    };

    //ģ���ػ�, ��Ա����ָ��, ����void
    template<class _CallableType, size_t ... index>
    struct luaCFunctionDispatcher<CallableIdType::POINTER_TO_MEMBER_FUNCTION, true, _CallableType, ArgIndex<index...> >
    {
        template<class _PfType>
        static int Apply(lua_State * pLua, _PfType pmf)
        {
            using class_type = std::decay_t<typename _CallableType::class_t>;
            class_type * pThis = lua_io_dispatcher<class_type*>::from_lua(pLua, 1);
            assert(pThis);
            if (pThis)
            {
                (pThis->*pmf)(lua_io_dispatcher <
                    std::decay_t<std::tuple_element<index, typename _CallableType::arg_tuple_t>::type >
                    >::from_lua(pLua, index + 2)...);
            }
            return 0;
        }
    };

    //ģ���ػ�, ��Ա����ָ��, �з���ֵ
    template<class _CallableType, size_t ... index>
    struct luaCFunctionDispatcher<CallableIdType::POINTER_TO_MEMBER_FUNCTION, false, _CallableType, ArgIndex<index...> >
    {
        template<class _PfType>
        static int Apply(lua_State * pLua, _PfType pmf)
        {
            using result_type = std::decay_t<typename _CallableType::result_t>;
            using class_type = std::decay_t<typename _CallableType::class_t>;
            class_type * pThis = lua_io_dispatcher<class_type*>::from_lua(pLua, 1);
            assert(pThis);
            if (pThis)
            {
                return lua_io_dispatcher<result_type>::to_lua(
                    pLua,
                    (pThis->*pmf)(lua_io_dispatcher<
                        std::decay_t<std::tuple_element<index, typename _CallableType::arg_tuple_t>::type>
                        >::from_lua(pLua, index + 2)...)
                    );
            }
            else
            {
                return 0;
            }
        }
    };

    //ģ���ػ�, ��Ա����ָ��
    template<class _CallableType>
    struct luaCFunctionDispatcher<CallableIdType::POINTER_TO_MEMBER_DATA, false, _CallableType, ArgIndex<> >
    {
        template<class _PfType>
        static int Apply(lua_State * pLua, _PfType pmd)
        {
            using result_type = std::decay_t<typename _CallableType::result_t>;
            using class_type = std::decay_t<typename _CallableType::class_t>;
            class_type * pThis = lua_io_dispatcher<class_type*>::from_lua(pLua, 1);
            assert(pThis);
            if (pThis)
            {
                return lua_io_dispatcher<result_type>::to_lua(
                    pLua,
                    pThis->*pmd
                    );
            }
            else
            {
                return 0;
            }
        }
    };

//----lua C���������亯�����----------------------------------------

    //lua����C��������,���е�C++���ö�������ת����ȥ
    template<class _FuncType>
    int MainLuaCFunctionCall(lua_State * pLua)
    {
        //upvalue�е�һ��ֵ�̶�Ϊ��ʵִ�еĵ���ָ��
        void * ppf = lua_touserdata(pLua, lua_upvalueindex(1));
        assert(ppf);
        assert(*(_FuncType*)ppf);
        if (ppf)
        {
            using call_t = typename CallableTypeHelper< std::decay_t<_FuncType> >;
            CheckCFuncArgValid<call_t::arg_tuple_t> checkParam;
            (void)checkParam;
            return luaCFunctionDispatcher<
                call_t::callable_id,
                std::is_void<call_t::result_t>::value,
                call_t,
                call_t::arg_index_t>::Apply(pLua, *(_FuncType*)ppf);
        }
        return 0;
    }
}

/** ��ջ��ѹ��C++����(֧�ֺ���ָ��,��Ա����ָ��,��Ա����ָ��).
lua����C++�ĳ�Ա�������Ա����ʱ, ��һ���������봫C++����ָ��, ʣ�µĲ�����ó�Ա�����Ĳ�����ͬ, ��Ա����������û����������.
���C++���õĲ����б�����Ĭ��ֵ, Ĭ��ֵ������Ч.
@param[in,out] pLua lua״ָ̬��
@param[in] pf C++����ָ��, Ŀǰ��֧�ֲ����е���ֵ���õĵ�������
*/
template<class _FuncType>
void push_cpp_callable_to_lua(lua_State * pLua, _FuncType pf)
{
    //_FuncType ��ֵ, �Զ��Ѻ���ת��ָ������ָ��
    assert(pf);
    ::luaL_checkstack(pLua, 1, "too many upvalues");
    void * ppf = lua_newuserdata(pLua, sizeof(pf));
    assert(ppf);
    //�ѵ���ָ��д��upvalue
    std::memcpy(ppf, &pf, sizeof(pf));
    ::lua_pushcclosure(pLua, Internal::MainLuaCFunctionCall<_FuncType>, 1);
}


//----lua����C++��ӳ��ʵ�ֺ�---------------------------------------------
/* ��������ʵ����һ������, �������lua����C++��ӳ��, ����lua_State��, ���øú���,
�Ͱ���ЩC++������lua����������.
*/

/** ����һ��ע�ắ��
@param[in] registerFuncName: ע�ắ��������, ��������
@param[in] pLibName: ������, const char *
*/
#define BEGIN_LUA_CPP_MAP_IMPLEMENT(registerFuncName, pLibName) \
void registerFuncName(lua_State * pLua) \
{ \
    const char * pLib = pLibName; \
    assert(pLua); \
    assert(pLib); \
    shr::lua_stack_check stackChecker(pLua); \
    ::lua_newtable(pLua); 

/** ���һ��C++���õ�lua, ������һ���ַ�����������
@param[in] luaFuncName: lua�ű��е���ʱ���õ�����, const char *
@param[in] cppCallable: C++����ָ��, ��push_cpp_callable_to_luaʵ��,�������Ʋ��ոú���ע��
*/
#define ENTRY_LUA_CPP_MAP_IMPLEMENT(luaFuncName, cppCallable) \
    shr::push_cpp_callable_to_lua(pLua, cppCallable); \
    ::lua_setfield(pLua, -2, luaFuncName);

/** ����ע�ắ��
*/
#define END_LUA_CPP_MAP_IMPLEMENT() \
    ::lua_setglobal(pLua, pLib); \
}

//----lua_State�ķ�װ��---------------------------------------------------------

class lua_state_wrapper
{
    SHARELIB_DISABLE_COPY_CLASS(lua_state_wrapper);

    lua_State * m_pLuaState;
public:

    lua_state_wrapper();
    ~lua_state_wrapper();
    bool create();
    void close();
    void attach(lua_State * pState);
    lua_State * detach();

	lua_State* get_raw_state();
	operator lua_State*();

/* һ��Ĳ������̣�
�����ñ�����C�����ȣ���ִ�нű�������ȡ����
*/

//----ִ�нű�ǰ�Ĳ�����---------------------------------

    //��lua��д��ȫ�ֱ���
    template<class T>
    void set_variable(const char * pName, T value)
    {
        assert(m_pLuaState);
        if (m_pLuaState && pName)
        {
            lua_stack_check check(m_pLuaState);
            lua_io_dispatcher<std::decay_t<T>>::to_lua(m_pLuaState, value);
            lua_setglobal(m_pLuaState, pName);
        }
    }

    //��lua�з���һ�������ڴ�, �����ڴ��ַ, ջ���ֲ���
    void * alloc_user_data(const char * pName, size_t size);

//----ִ�нű�----------------------------

    //��������,���ز�ִ��, ���ص�ʱ�����: ��������
    bool do_lua_file(const char * pFileName);
    bool do_lua_string(const char * pString);

    //��������,ֻ���ز�ִ��,������Զ��ִ�У�run(),�������͵�ִ�нű��������ɻ���.
    bool load_lua_file(const char * pFileName);
    bool load_lua_string(const char * pString);

    /* ִ��.
    �������ǰѼ��ص�lua�ű�ת���lua����,��˶��ִ�е�lua����������ͬ��. ����, һ��ȫ�ֱ�����ʼΪ0��
    ��һ��ִ�а�����1����ô�ڶ���ִ��ʱ������1�������ǳ�ʼֵ0.
    */
    bool run();

    // ��ȡ����ʧ�ܵĴ�����Ϣ,ע�⣺��ʧ�ܵ�ʱ��ŵ���
    std::string get_error_msg();

//----ִ�нű���Ĳ���-----------------------------

    //��ȡջ�����ݵĸ���
    int get_stack_count();

    // ��ȡջ��Ԫ�صĴ�С���ַ��������ȣ� table��������userdata��size
    size_t get_size(int index);

    //��lua�ж�ȡȫ�ֱ���
    template<class T>
    T get_variable(const char * pName, T defaultValue = T{})
    {
        assert(m_pLuaState);
        if (m_pLuaState && pName)
        {
            lua_stack_guard guard(m_pLuaState);
            lua_getglobal(m_pLuaState, pName);
            return lua_io_dispatcher<std::decay_t<T>>::from_lua(m_pLuaState, -1, defaultValue);
        }
        return defaultValue;
    }
};

SHARELIB_END_NAMESPACE

