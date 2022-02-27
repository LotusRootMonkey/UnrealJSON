#pragma once

#include "debug.h"
#include "json.hpp"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealJSONBPLibrary.generated.h"

#if DEBUG_JSONTOOLS
PRAGMA_DISABLE_OPTIMIZATION
#endif

UCLASS()
class UUnrealJSONBPLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


private:
	//enum class T_Type : uint8
	//{
	//	boolType,
	//	byteType,
	//	intType,
	//	int64Type,
	//	floatType,
	//	nameType,
	//	stringType,
	//	textType,
	//	structType,
	//	objectType,
	//	unknownType,
	//};

	enum class Type : uint8
	{
		arrayType,
		mapKeyType,
		mapValueType,
		setType,
		other,
	};

public:
	//UFUNCTION(BlueprintCallable, meta = (DisplayName = "T_TO_JSON", Keywords = "T_TO_JSON"), Category = "JSON Tools")
	//	static void T_TO_JSON(const int32& T, FString& json, bool& success, FString& info);
	//UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T", CompactNodeTitle = "GET/SET"), Category = "JSON Tools")

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void T_TO_JSON(const int32& T, FString& json, bool& success, FString& info, int32 depth = 10);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void JSON_TO_T(const FString& json, FString fieldName, int32& T, bool& success, FString& info, int32 depth = 10);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void AddField(const FString& json, const FString& fieldName, const int32& T, bool& success, FString& info, FString& result, int32 depth = 10);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void UpdateField(const FString& json, const FString& fieldName, const int32& T, bool& success, FString& info, FString& result, int32 depth = 10);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static void DeleteField(const FString& json, const FString& fieldName, bool& success, FString& info, FString& result);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static bool analyticalSeparator(const FString& fieldName, TArray<FString>& fieldNameArray);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static bool generationSeparator(const TArray<FString>& fieldNameArray, FString& fieldName);

private:
	DECLARE_FUNCTION(execT_TO_JSON)
	{
		Stack.Step(Stack.Object, NULL);
		FProperty* property = CastField<FProperty>(Stack.MostRecentProperty);
		void* propertyPtr = Stack.MostRecentPropertyAddress;
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_json);
		P_GET_UBOOL_REF(Z_Param_Out_success);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_info);
		P_GET_PROPERTY(FIntProperty, Z_Param_depth);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_T_TO_JSON(property, propertyPtr, Z_Param_Out_json, Z_Param_Out_success, Z_Param_Out_info, Z_Param_depth);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execJSON_TO_T)
	{
		P_GET_PROPERTY(FStrProperty, Z_Param_json);
		P_GET_PROPERTY(FStrProperty, Z_Param_fieldName);
		Stack.Step(Stack.Object, NULL);
		FProperty* property = CastField<FProperty>(Stack.MostRecentProperty);
		void* propertyPtr = Stack.MostRecentPropertyAddress;
		P_GET_UBOOL_REF(Z_Param_Out_success);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_info);
		P_GET_PROPERTY(FIntProperty, Z_Param_depth);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_JSON_TO_T(Z_Param_json, Z_Param_fieldName, property, propertyPtr, Z_Param_Out_success, Z_Param_Out_info, Z_Param_depth);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execAddField)
	{
		P_GET_PROPERTY(FStrProperty, Z_Param_json);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_fieldName);
		Stack.Step(Stack.Object, NULL);
		FProperty* property = CastField<FProperty>(Stack.MostRecentProperty);
		void* propertyPtr = Stack.MostRecentPropertyAddress;
		P_GET_UBOOL_REF(Z_Param_Out_success);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_info);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_result);
		P_GET_PROPERTY(FIntProperty, Z_Param_depth);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_AddField(Z_Param_json, Z_Param_fieldName, property, propertyPtr, Z_Param_Out_success, Z_Param_Out_info, Z_Param_Out_result, Z_Param_depth);
		P_NATIVE_END;
	}

	DECLARE_FUNCTION(execUpdateField)
	{
		P_GET_PROPERTY(FStrProperty, Z_Param_json);
		P_GET_PROPERTY(FStrProperty, Z_Param_fieldName);
		Stack.Step(Stack.Object, NULL);
		FProperty* property = CastField<FProperty>(Stack.MostRecentProperty);
		void* propertyPtr = Stack.MostRecentPropertyAddress;
		P_GET_UBOOL_REF(Z_Param_Out_success);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_info);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_result);
		P_GET_PROPERTY(FIntProperty, Z_Param_depth);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_UpdateField(Z_Param_json, Z_Param_fieldName, property, propertyPtr, Z_Param_Out_success, Z_Param_Out_info, Z_Param_Out_result, Z_Param_depth);
		P_NATIVE_END;
	}

	static void Generic_T_TO_JSON(FProperty* property, void* propertyPtr, FString& json, bool& success, FString& info, int32 depth = 10);
	static void Generic_JSON_TO_T(const FString& json, FString fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, int32 depth = 10);
	static void Generic_AddField(const FString& json, const FString& fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, FString& result, int32 depth = 10);
	static void Generic_UpdateField(const FString& json, const FString& fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, FString& result, int32 depth = 10);

	static void serialize(FProperty* property, void* propertyPtr, bool& success, FString& info, nlohmann::json& j, int32 depth = 10, UUnrealJSONBPLibrary::Type type = UUnrealJSONBPLibrary::Type::other, nlohmann::json j_mapKey = {}, int32 count = 0);
	static void deserialize(nlohmann::json& j, FProperty* property, void* propertyPtr, bool& success, FString& info, int32 depth = 10, UUnrealJSONBPLibrary::Type type = UUnrealJSONBPLibrary::Type::other, nlohmann::json j_mapKey = {}, int32 count = 0);

	static std::string FString_To_stdstring(const FString& s);
	static FString stdstring_To_FString(const std::string& s);
	static std::string escapeCharacterProcessing(const std::string& s);
	static bool fieldName_check(const nlohmann::json& j, const FString& fieldName);

	static bool pathCheck(nlohmann::json& j, const FString& fieldName, nlohmann::json*& j_Ptr, FString& lastFieldName, bool retain = false);
};

//UENUM(BlueprintType)
//enum class ETestEnum : uint8
//{
//	P1     UMETA(DisplayName = "P1"),
//	P2    UMETA(DisplayName = "P2"),
//	P3     UMETA(DisplayName = "P3"),
//	P4  UMETA(DisplayName = "P4")
//};


#if DEBUG_JSONTOOLS
PRAGMA_ENABLE_OPTIMIZATION
#endif
