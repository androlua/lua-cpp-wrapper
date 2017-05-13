#pragma once

#include <tuple>
#include "MacroDefBase.h"

SHARELIB_BEGIN_NAMESPACE

//----��ָ�������ͳһת��Ϊ����-------------------------------------------------------------------------------
namespace Internal
{
    template<class T, bool isPointer, bool isLValue>
    struct ReferenceTypeHelper
    {
        using reference_type = typename std::remove_pointer<typename std::remove_reference<T>::type>::type &;
    };

    template<class T>
    struct ReferenceTypeHelper<T, false, false>
    {
        using reference_type = typename std::remove_pointer<typename std::remove_reference<T>::type>::type &&;
    };

    template<class T>
    struct ToReferenceImpl
    {
        static const bool s_bPointer = std::is_pointer<typename std::remove_reference<T>::type>::value;

        using type = typename ReferenceTypeHelper<
            T,
            s_bPointer,
            std::is_lvalue_reference<T>::value>::reference_type;

        template<bool isPointer = s_bPointer>
        static type ToReference(T && arg)
        {
            return static_cast<type>(arg);
        }

        template<>
        static type ToReference<true>(T && arg)
        {
            return static_cast<type>(*arg);
        }
    };
}

//��ָ�������ͳһת��Ϊ����
template<class T>
typename Internal::ToReferenceImpl<T>::type to_reference(T && arg)
{
    return Internal::ToReferenceImpl<T>::ToReference(std::forward<T>(arg));
}

//----���ݿɱ�������ɲ�������-----------------------------------------------------------------------

template<size_t ... Index>
struct ArgIndex
{};

namespace Internal
{
    template<class T, class ... Rest>
    struct MakeArgIndexHelper
    {
        using type = T;
    };

    template<size_t ... Index, class T, class ... Rest>
    struct MakeArgIndexHelper<ArgIndex<Index...>, T, Rest ... > :
        public MakeArgIndexHelper<ArgIndex<sizeof...(Rest), Index...>, Rest...>
    {};
}

//���ݿɱ�������ɲ�������,��0��ʼ
template<class ... T>
struct MakeArgIndex
{
    using type = typename Internal::MakeArgIndexHelper<ArgIndex<>, T... >::type;
};

//����tuple���ɲ�������,��0��ʼ
template<class T>
struct MakeTupleIndex;

template<class ... T>
struct MakeTupleIndex<std::tuple<T...> >
{
    using type = typename Internal::MakeArgIndexHelper<ArgIndex<>, T... >::type;
};

//----�ж����Ƿ��г�Ա����------------------------------------------------------------------------

namespace Internal
{
    struct WrapInt
    {
        WrapInt(int){};
    };

    template<class T>
    struct Identity
    {
        using type = T;

        const T & operator()(const T & _Left) const
        {	// apply identity operator to operand
            return (_Left);
        }
    };
}

/* �ж����Ƿ��г�Ա
ʹ�÷���: �����������

template<class _ClassType>
struct HasMember...
HAS_MEMBER_TYPE_IMPL(membername)

֮��Ϳ��� HasMember...<className>::value ���ж�һ�����Ƿ���ĳ��Ա
ʵ��ϸ��: ������̬����������ģ��
1. ���صĺ���,�������ذ汾�������ɶ����ƴ���,�����Ƿ��ľͱ��벻��;
2. ģ����ѡ���Ա���,��������,ֻ��ѡ��һ���Ϸ��İ汾����ɶ����ƴ���,�Ƿ��İ汾�Ͳ������,Ҳ�Ͳ��ᵼ�±��벻��;
*/

//�Ƿ��г�Ա(����,��̬����), ��bug, ������ģ��ʱ����true,����
//#define HAS_MEMBER_IMPL(memberName) \
//{ \
//private: \
//    template<class T>static auto DeclFunc(int, decltype(typename T::memberName) * = 0)->std::true_type; \
//    template<class T>static auto DeclFunc(shr::Internal::WrapInt)->std::false_type; \
//public: \
//    static const bool value = decltype(DeclFunc<_ClassType>(0))::value; \
//};

//�Ƿ��г�Ա(����)
#define HAS_MEMBER_TYPE_IMPL(memberType) \
{ \
private: \
    template<class T>static auto DeclFunc(int, typename T::memberType * = 0)->std::true_type; \
    template<class T>static auto DeclFunc(shr::Internal::WrapInt)->std::false_type; \
public: \
    static const bool value = decltype(DeclFunc<_ClassType>(0))::value; \
};

//----�������͸���----------------------------------------------------------------------------

namespace Internal
{
// ����ĺ��������庯����������, �Ǵ�STLԴ���и���������

#ifdef _M_IX86

#ifdef _M_CEE
#ifndef NON_MEMBER_CALL_MACRO
#define NON_MEMBER_CALL_MACRO(FUNC, CONST_OPT) \
	FUNC(__cdecl, CONST_OPT) \
	FUNC(__stdcall, CONST_OPT) \
	FUNC(__clrcall, CONST_OPT)
#endif // !NON_MEMBER_CALL_MACRO
#ifndef MEMBER_CALL_MACRO
#define MEMBER_CALL_MACRO(FUNC, CONST_OPT, CV_OPT) \
	FUNC(__thiscall, CONST_OPT, CV_OPT) \
	FUNC(__cdecl, CONST_OPT, CV_OPT) \
	FUNC(__stdcall, CONST_OPT, CV_OPT) \
	FUNC(__clrcall, CONST_OPT, CV_OPT)
#endif // !MEMBER_CALL_MACRO

#else /* _M_CEE */
#ifndef NON_MEMBER_CALL_MACRO
#define NON_MEMBER_CALL_MACRO(FUNC, CONST_OPT) \
	FUNC(__cdecl, CONST_OPT) \
	FUNC(__stdcall, CONST_OPT) \
	FUNC(__fastcall, CONST_OPT)
#endif // !NON_MEMBER_CALL_MACRO
#ifndef MEMBER_CALL_MACRO
#define MEMBER_CALL_MACRO(FUNC, CONST_OPT, CV_OPT) \
	FUNC(__thiscall, CONST_OPT, CV_OPT) \
	FUNC(__cdecl, CONST_OPT, CV_OPT) \
	FUNC(__stdcall, CONST_OPT, CV_OPT) \
	FUNC(__fastcall, CONST_OPT, CV_OPT)
#endif // !MEMBER_CALL_MACRO
#endif /* _M_CEE */

#else /* _M_IX86 */

#ifdef _M_CEE
#ifndef NON_MEMBER_CALL_MACRO
#define NON_MEMBER_CALL_MACRO(FUNC, CONST_OPT) \
	FUNC(__cdecl, CONST_OPT) \
	FUNC(__clrcall, CONST_OPT)
#endif // !NON_MEMBER_CALL_MACRO
#ifndef MEMBER_CALL_MACRO
#define MEMBER_CALL_MACRO(FUNC, CONST_OPT, CV_OPT) \
	FUNC(__cdecl, CONST_OPT, CV_OPT) \
	FUNC(__clrcall, CONST_OPT, CV_OPT)
#endif // !MEMBER_CALL_MACRO

#else /* _M_CEE */
#ifndef NON_MEMBER_CALL_MACRO
#define NON_MEMBER_CALL_MACRO(FUNC, CONST_OPT) \
	FUNC(__cdecl, CONST_OPT)
#endif // !NON_MEMBER_CALL_MACRO
#ifndef MEMBER_CALL_MACRO
#define MEMBER_CALL_MACRO(FUNC, CONST_OPT, CV_OPT) \
	FUNC(__cdecl, CONST_OPT, CV_OPT)
#endif // !MEMBER_CALL_MACRO
#endif /* _M_CEE */
#endif /* _M_IX86 */


//���涨��ĺ�, ��������ģ���ػ�

/* ָ����������const,volatile���Բ�����Ҫר���ػ�,ֻҪ�Ա���Ӧ�� std::remove_cv_t ����.
   ���������ר���ػ�, ���ɵ��ػ��汾��̫����, ����Ҳ���������һЩ������
*/

#ifndef CALLABLE_MEMBER_CALL_CV
#define CALLABLE_MEMBER_CALL_CV(FUNC, CONST_OPT) \
	MEMBER_CALL_MACRO(FUNC, CONST_OPT, ) \
	MEMBER_CALL_MACRO(FUNC, CONST_OPT, const) \
	MEMBER_CALL_MACRO(FUNC, CONST_OPT, volatile) \
	MEMBER_CALL_MACRO(FUNC, CONST_OPT, const volatile)
#endif // !CALLABLE_MEMBER_CALL_CV

}

//����ֵ
enum class CallableIdType
{
    FUNCTION,
    POINTER_TO_FUNCTION,
    POINTER_TO_MEMBER_FUNCTION,
    POINTER_TO_MEMBER_DATA,
};

/* �������͸���
�����˺���,����ָ��,��Ա����ָ��,��Աָ��,��������û�и��ǣ����������¼������ͱ�����
result_t, ����ֵ���ͣ�
arg_tuple_t, �����󶨳�tuple������;
arg_index_t, ���������к�����, ArgIndex<...>
class_t, (ֻ�г�Ա����ָ��ͳ�Աָ��Ŷ���), ������
callable_id, ֵ,CallableIdType�ж��������ֵ
 */
template<class T>
struct CallableTypeHelper;

//���������ػ�
#define FUNCTION_HELPER(CALL_OPT, NO_USE) \
template<class _RetType, class... _ArgType> \
struct CallableTypeHelper<_RetType CALL_OPT (_ArgType...)> \
{ \
    using result_t = _RetType; \
    using arg_tuple_t = std::tuple<_ArgType...>; \
    using arg_index_t = typename MakeArgIndex<_ArgType...>::type; \
    static const CallableIdType callable_id = CallableIdType::FUNCTION; \
};
NON_MEMBER_CALL_MACRO(FUNCTION_HELPER, )
#undef FUNCTION_HELPER

//����ָ�������ػ�
#define POINTER_TO_FUNCTION_HELPER(CALL_OPT, NO_USE) \
template<class _RetType, class... _ArgType> \
struct CallableTypeHelper<_RetType(CALL_OPT * )(_ArgType...)> \
{ \
    using result_t = _RetType; \
    using arg_tuple_t = std::tuple<_ArgType...>; \
    using arg_index_t = typename MakeArgIndex<_ArgType...>::type; \
    static const CallableIdType callable_id = CallableIdType::POINTER_TO_FUNCTION; \
};
NON_MEMBER_CALL_MACRO(POINTER_TO_FUNCTION_HELPER, )
#undef POINTER_TO_FUNCTION_HELPER

//��Ա����ָ�������ػ�
#define POINTER_TO_MEMBER_FUNCTION_HELPER(CALL_OPT, NO_USE, CV_OPT) \
template<class _RetType, class _ClassType, class... _ArgType> \
struct CallableTypeHelper<_RetType(CALL_OPT _ClassType::* )(_ArgType...) CV_OPT> \
{ \
    using class_t = _ClassType; \
    using result_t = _RetType; \
    using arg_tuple_t = std::tuple<_ArgType...>; \
    using arg_index_t = typename MakeArgIndex<_ArgType...>::type; \
    static const CallableIdType callable_id = CallableIdType::POINTER_TO_MEMBER_FUNCTION; \
};
CALLABLE_MEMBER_CALL_CV(POINTER_TO_MEMBER_FUNCTION_HELPER, )
#undef POINTER_TO_MEMBER_FUNCTION_HELPER

//��Աָ�������ػ�
template<class _RetType, class _ClassType>
struct CallableTypeHelper<_RetType _ClassType::* >
{
    using class_t = _ClassType;
    using result_t = _RetType;
    using arg_tuple_t = std::tuple<>;
    using arg_index_t = typename MakeArgIndex<>::type;
    static const CallableIdType callable_id = CallableIdType::POINTER_TO_MEMBER_DATA;
};

SHARELIB_END_NAMESPACE
