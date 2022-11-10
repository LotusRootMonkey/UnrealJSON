#pragma once

#include "debug.h"
#include "json.hpp"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealJSONBPLibrary.generated.h"

#if DEBUG_JSONTOOLS
PRAGMA_DISABLE_OPTIMIZATION
#endif

UENUM(BlueprintType)
enum class EJsonType : uint8
{
	array			UMETA(DisplayName = "array"),
	string			UMETA(DisplayName = "string"),
	binary			UMETA(DisplayName = "binary"),
	boolean			UMETA(DisplayName = "boolean"),
	discarded		UMETA(DisplayName = "discarded"),
	null			UMETA(DisplayName = "null"),
	number			UMETA(DisplayName = "number"),
	number_float	UMETA(DisplayName = "number_float"),
	number_integer	UMETA(DisplayName = "number_integer"),
	number_unsigned	UMETA(DisplayName = "number_unsigned"),
	object			UMETA(DisplayName = "object"),
	primitive		UMETA(DisplayName = "primitive"),
	structured		UMETA(DisplayName = "structured"),
};

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
		static void T_TO_JSON(FString mainKey, const int32& T, FString& json, bool& success, FString& info, int32 depth = 10, bool lowercase = false);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void JSON_TO_T(const FString& json, FString fieldName, int32& T, bool& success, FString& info, int32 depth = 10, bool lowercase = false);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void AddField(const FString& json, const FString& fieldName, const int32& T, bool& success, FString& info, FString& result, int32 depth = 10, bool keepJsonObject = false, bool lowercase = false);

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "T"), Category = "JSON Tools")
		static void UpdateField(const FString& json, const FString& fieldName, const int32& T, bool& success, FString& info, FString& result, int32 depth = 10, bool keepJsonObject = false, bool lowercase = false);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static void DeleteField(const FString& json, const FString& fieldName, bool& success, FString& info, FString& result);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static bool analyticalSeparator(const FString& fieldName, TArray<FString>& fieldNameArray);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static bool generationSeparator(const TArray<FString>& fieldNameArray, FString& fieldName);

	UFUNCTION(BlueprintCallable, Category = "JSON Tools")
		static bool matchingType(const FString& json, EJsonType type);

private:
	DECLARE_FUNCTION(execT_TO_JSON)
	{
		P_GET_PROPERTY(FStrProperty, Z_Param_mainKey)
		Stack.Step(Stack.Object, NULL);
		FProperty* property = CastField<FProperty>(Stack.MostRecentProperty);
		void* propertyPtr = Stack.MostRecentPropertyAddress;
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_json);
		P_GET_UBOOL_REF(Z_Param_Out_success);
		P_GET_PROPERTY_REF(FStrProperty, Z_Param_Out_info);
		P_GET_PROPERTY(FIntProperty, Z_Param_depth);
		P_GET_UBOOL(Z_Param_lowercase);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_T_TO_JSON(Z_Param_mainKey, property, propertyPtr, Z_Param_Out_json, Z_Param_Out_success, Z_Param_Out_info, Z_Param_depth, Z_Param_lowercase);
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
		P_GET_UBOOL(Z_Param_lowercase);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_JSON_TO_T(Z_Param_json, Z_Param_fieldName, property, propertyPtr, Z_Param_Out_success, Z_Param_Out_info, Z_Param_depth, Z_Param_lowercase);
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
		P_GET_UBOOL(Z_Param_Out_keepJsonObject);
		P_GET_UBOOL(Z_Param_lowercase);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_AddField(Z_Param_json, Z_Param_fieldName, property, propertyPtr, Z_Param_Out_success, Z_Param_Out_info, Z_Param_Out_result, Z_Param_depth, Z_Param_Out_keepJsonObject, Z_Param_lowercase);
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
		P_GET_UBOOL(Z_Param_Out_keepJsonObject);
		P_GET_UBOOL(Z_Param_lowercase);
		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_UpdateField(Z_Param_json, Z_Param_fieldName, property, propertyPtr, Z_Param_Out_success, Z_Param_Out_info, Z_Param_Out_result, Z_Param_depth, Z_Param_Out_keepJsonObject, Z_Param_lowercase);
		P_NATIVE_END;
	}

	static void Generic_T_TO_JSON(FString mainKey, FProperty* property, void* propertyPtr, FString& json, bool& success, FString& info, int32 depth = 10, bool lowercase = false);
	static void Generic_JSON_TO_T(const FString& json, FString fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, int32 depth = 10, bool lowercase = false);
	static void Generic_AddField(const FString& json, const FString& fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, FString& result, int32 depth = 10, bool keepJsonObject = false, bool lowercase = false);
	static void Generic_UpdateField(const FString& json, const FString& fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, FString& result, int32 depth = 10, bool keepJsonObject = false, bool lowercase = false);

	static void serialize(FProperty* property, void* propertyPtr, bool& success, FString& info, nlohmann::json& j, int32 depth = 10, bool lowercaseID = false, UUnrealJSONBPLibrary::Type type = UUnrealJSONBPLibrary::Type::other, nlohmann::json j_mapKey = {}, int32 count = 0, FString mainKey = FString());
	static void deserialize(nlohmann::json& j, FProperty* property, void* propertyPtr, bool& success, FString& info, int32 depth = 10, bool lowercaseID = false, UUnrealJSONBPLibrary::Type type = UUnrealJSONBPLibrary::Type::other, nlohmann::json j_mapKey = {}, int32 count = 0);

	static std::string FString_To_stdstring(const FString& s);
	static FString stdstring_To_FString(const std::string& s);
	static std::string escapeCharacterProcessing(const std::string& s);
	
	static bool fieldName_check(const nlohmann::json& j, const FString& fieldName, UUnrealJSONBPLibrary::Type& type);

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
