//
//  LuaUtils.h
//  Chilli Source
//  Created by Scott Downie on 13/08/2014.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2014 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifndef _CHILLISOURCE_LUA_BASE_LUAUTILS_H_
#define _CHILLISOURCE_LUA_BASE_LUAUTILS_H_

#include <ChilliSource/ChilliSource.h>

extern "C"
{
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <tuple>

namespace ChilliSource
{
    namespace Lua
    {
        namespace LuaUtils
        {
            //---------------------------------------------------------
            /// Push the value to the Lua VM stack. This is specialised
            /// for each supported value type and will assert if
            /// unsupported type is used.
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            /// @param Value
            //---------------------------------------------------------
            template <typename TArg> void PushValueToVM(lua_State* in_vm, TArg&& in_arg)
            {
                static_assert(std::is_pointer<TArg>::value, "Lua type not supported");
            }
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, bool&& in_arg);
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, s32&& in_arg);
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, u32&& in_arg);
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, f32&& in_arg);
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, f64&& in_arg);
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, std::string&& in_arg);
            //---------------------------------------------------------
            //---------------------------------------------------------
            void PushValueToVM(lua_State* in_vm, const char* in_arg);
            //---------------------------------------------------------
            /// Pop the value from the index of the Lua VM stack. This is specialised
            /// for each supported value type and will assert if
            /// unsupported type is used.
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            /// @param Index in the stack
            ///
            /// @return Value
            //---------------------------------------------------------
            template <typename TResult> TResult ReadValueFromVM(lua_State* in_vm, s32 in_index)
            {
                static_assert(std::is_pointer<TResult>::value, "Lua type not supported");
                return TResult();
            }
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> bool ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> s32 ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> u32 ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> f32 ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> f64 ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> std::string ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            //---------------------------------------------------------
            template <> const char* ReadValueFromVM(lua_State* in_vm, s32 in_index);
            //---------------------------------------------------------
            /// Specialised function for pushing no values to the
            /// Lua stack.
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            //---------------------------------------------------------
            void PushAllToVM(lua_State* in_vm);
            //---------------------------------------------------------
            /// Helper function for pushing values of different types
            /// to the Lua stack using template recursion
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            /// @param Value
            /// @param Rest of values
            //---------------------------------------------------------
            template <typename TValue, typename... TValues> void PushAllToVM(lua_State* in_vm, TValue&& in_value, TValues&&... in_restValues)
            {
                PushValueToVM(in_vm, std::forward<TValue>(in_value));
                PushAllToVM(in_vm, std::forward<TValues>(in_restValues)...);
            }
            //---------------------------------------------------------
            /// Performs recursive popping of the Lua VM stack
            /// into a tuple
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <size_t, typename... TResults> struct Popper
            {
                using type = std::tuple<TResults...>;
                
                //---------------------------------------------------------
                /// Creates a tuple that contains a single element
                /// of template type with the value of the given index
                /// in the Lua stack
                ///
                /// @author S Downie
                ///
                /// @param Lua VM
                /// @param Index in Lua stack
                ///
                /// @return Tuple
                //---------------------------------------------------------
                template <typename TElementType> static std::tuple<TElementType> ReadValue(lua_State* in_vm, s32 in_index)
                {
                    return std::make_tuple(ReadValueFromVM<TElementType>(in_vm, in_index));
                }
                //---------------------------------------------------------
                /// Creates a tuple from the values starting at the
                /// given index in the Lua stack.
                ///
                /// @author S Downie
                ///
                /// @param Lua VM
                /// @param Index in Lua stack to start from
                ///
                /// @return Tuple
                //---------------------------------------------------------
                template <typename TElementType1, typename TElementType2, typename... TElementTypeRest>
                static std::tuple<TElementType1, TElementType2, TElementTypeRest...> ReadValue(lua_State* in_vm, s32 in_index)
                {
                    std::tuple<TElementType1> head = std::make_tuple(ReadValueFromVM<TElementType1>(in_vm, in_index));
                    return std::tuple_cat(head, ReadValue<TElementType2, TElementTypeRest...>(in_vm, in_index + 1));
                }
                //---------------------------------------------------------
                /// Entry function that builds a tuple from
                /// the contents of the VM stack. The results
                /// are built in reverse order with the top of the
                /// stack the last element in the tuple
                ///
                /// @author S Downie
                ///
                /// @param Lua VM
                ///
                /// @return Tuple
                //---------------------------------------------------------
                static type PopRecursive(lua_State* in_vm)
                {
                    auto ret = ReadValue<TResults...>(in_vm, 1);
                    lua_pop(in_vm, sizeof...(TResults));
                    return ret;
                }
            };
            //---------------------------------------------------------
            /// Special case for one results which will specify one
            /// return type
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <typename TResult> struct Popper<1, TResult>
            {
                using type = TResult;
                
                //---------------------------------------------------------
                /// Special case for single return result
                ///
                /// @author S Downie
                ///
                /// @param Lua VM
                ///
                /// @return Single type
                //---------------------------------------------------------
                static type PopRecursive(lua_State* in_vm)
                {
                    auto ret = ReadValueFromVM<TResult>(in_vm, -1);
                    lua_pop(in_vm, 1);
                    return ret;
                }
            };
            //---------------------------------------------------------
            /// Special case for zero results which will specify void
            /// return type
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <typename... TResults> struct Popper<0, TResults...>
            {
                using type = void;
                
                //---------------------------------------------------------
                /// Special case for no return results which peforms
                /// no action
                ///
                /// @author S Downie
                ///
                /// @param Lua VM
                ///
                /// @return Void
                //---------------------------------------------------------
                static type PopRecursive(lua_State* in_vm) {}
            };
            //---------------------------------------------------------
            /// Helper function for popping all the values from the
            /// Lua VM stack into a tuple or void.
            ///
            /// NOTE: The results are built in reverse order with the
            /// top of the stack the last element in the tuple
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            ///
            /// @return Tuple or void depending on num results
            //---------------------------------------------------------
            template <typename... TResults> typename Popper<sizeof...(TResults), TResults...>::type PopAllFromVM(lua_State* in_vm)
            {
                return Popper<sizeof...(TResults), TResults...>::PopRecursive(in_vm);
            }
            //---------------------------------------------------------
            /// Holds a list of indices accessible at template
            /// compile time
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <size_t... TIndices> struct Indices {};
            //---------------------------------------------------------
            /// Builds a list of indices accessible at template
            /// compile time
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <size_t TIndex, size_t... TIndices> struct IndicesBuilder : IndicesBuilder<TIndex-1, TIndex-1, TIndices...> {};
            //---------------------------------------------------------
            /// Holds the type of given index
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <size_t... TIndices> struct IndicesBuilder<0, TIndices...>
            {
                using type = Indices<TIndices...>;
            };
            //---------------------------------------------------------
            /// Unpack the arguments in the tuple at each given index
            /// and call the given function
            ///
            /// @author S Downie
            ///
            /// @param Function
            /// @param Arguments stored as tuple
            /// @param Indices to unpack
            ///
            /// @return Result of executing function
            //---------------------------------------------------------
            template <typename TResult, typename... TArgs, size_t... TIndex>
            TResult LiftToParamPack(const std::function<TResult(TArgs...)>& in_function, const std::tuple<TArgs...>& in_args, Indices<TIndex...>)
            {
                return in_function(std::get<TIndex>(in_args)...);
            }
            //---------------------------------------------------------
            /// Call the given function with each element of the
            /// tuple as a unique argument. This requires template
            /// magic to lift each value from the tuple into a param
            /// pack in order to match the signature of the function
            ///
            /// @author S Downie
            ///
            /// @param Function
            /// @param Arguments stored as tuple
            ///
            /// @return Result of executing function
            //---------------------------------------------------------
            template <typename TResult, typename... TArgs>
            TResult LiftToParamPack(const std::function<TResult(TArgs...)>& in_function, const std::tuple<TArgs...>& in_args)
            {
                return LiftToParamPack(in_function, in_args, typename IndicesBuilder<sizeof...(TArgs)>::type());
            }
            //---------------------------------------------------------
            /// Holds the number of values of the template param.
            /// (Specialised for 1)
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <typename TValue> struct NumValues
            {
                static const u32 value = 1;
            };
            //---------------------------------------------------------
            /// Holds the number of values of the template param.
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <typename... TValues> struct NumValues<std::tuple<TValues...>>
            {
                static const u32 value = sizeof...(TValues);
            };
            //---------------------------------------------------------
            /// Holds the number of values of the template param.
            /// (Specialised for void)
            ///
            /// @author S Downie
            //---------------------------------------------------------
            template <> struct NumValues<void>
            {
                static const u32 value = 0;
            };
            //---------------------------------------------------------
            /// Helper function for reading all the values from the
            /// Lua VM stack into a tuple or void.
            ///
            /// NOTE: The results are built in reverse order with the
            /// top of the stack the last element in the tuple
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            ///
            /// @return Tuple or void depending on num results
            //---------------------------------------------------------
            template <typename... TResults, size_t... TIndex> std::tuple<TResults...> ReadAllFromVM(lua_State* in_luaVM, Indices<TIndex...>)
            {
                return std::tuple<TResults...>{ReadValueFromVM<TResults>(in_luaVM, TIndex + 1)...};
            }
            //---------------------------------------------------------
            /// Helper function for reading all the values from the
            /// Lua VM stack into a tuple or void.
            ///
            /// NOTE: The results are built in reverse order with the
            /// top of the stack the last element in the tuple
            ///
            /// @author S Downie
            ///
            /// @param Lua VM
            ///
            /// @return Tuple or void depending on num results
            //---------------------------------------------------------
            template <typename... TResults> std::tuple<TResults...> ReadAllFromVM(lua_State* in_luaVM)
            {
                return ReadAllFromVM<TResults...>(in_luaVM, typename IndicesBuilder<sizeof...(TResults)>::type());
            }
        }
    }
}

#endif
