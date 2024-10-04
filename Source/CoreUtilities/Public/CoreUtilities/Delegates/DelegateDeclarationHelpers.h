#pragma once

#define FUNC_DECLARE_DELEGATE( DelegateName, ReturnType, ... ) \
	using DelegateName = Volt::Delegate<ReturnType(__VA_ARGS__)>

#define DECLARE_DELEGATE(DelegateName) FUNC_DECLARE_DELEGATE(DelegateName, void)
#define DECLARE_DELEGATE_OneParam(DelegateName, Param1Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type)
#define DECLARE_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type)
#define DECLARE_DELEGATE_ThreeParams(DelegateName, Param1Type, Param2Type, Param3Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type)
#define DECLARE_DELEGATE_FourParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type)
#define DECLARE_DELEGATE_FiveParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type)
#define DECLARE_DELEGATE_SixParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type)
#define DECLARE_DELEGATE_SevenParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type)
#define DECLARE_DELEGATE_EightParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type, Param8Type) FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type, Param8Type)

#define DECLARE_DELEGATE_RetVal(ReturnValueType, DelegateName) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType)
#define DECLARE_DELEGATE_RetVal_OneParam(ReturnValueType, DelegateName, Param1Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type)
#define DECLARE_DELEGATE_RetVal_TwoParams(ReturnValueType, DelegateName, Param1Type, Param2Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type)
#define DECLARE_DELEGATE_RetVal_ThreeParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type, Param3Type)
#define DECLARE_DELEGATE_RetVal_FourParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type, Param4Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type, Param3Type, Param4Type)
#define DECLARE_DELEGATE_RetVal_FiveParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type)
#define DECLARE_DELEGATE_RetVal_SixParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type)
#define DECLARE_DELEGATE_RetVal_SevenParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type)
#define DECLARE_DELEGATE_RetVal_EightParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type, Param8Type) FUNC_DECLARE_DELEGATE(DelegateName, ReturnValueType, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type, Param6Type, Param7Type, Param8Type)

