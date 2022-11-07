#include "UnrealJSONBPLibrary.h"
#include "UnrealJSON.h"
#include <sstream>
#include "Kismet/KismetArrayLibrary.h"
#include "UObject/TextProperty.h"
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/BlueprintSetLibrary.h"
#include <regex>

#if DEBUG_JSONTOOLS
PRAGMA_DISABLE_OPTIMIZATION
#endif

class Info final
{
public:
	static Info& getInstance()
	{
		static Info info;
		return info;
	}

	std::string getInfoString()
	{
		return infoString;
	}

	void setInfoString(std::string infoString_T)
	{
		infoString = infoString_T;
	}

private:
	explicit Info() {};
	Info(const Info&) = delete;
	Info& operator=(const Info&) = delete;

	std::string infoString;
};

template<typename BasicJsonType>
class json_sax_acceptor final :nlohmann::json_sax<nlohmann::json>
{
public:

	bool null()
	{
		return true;
	}

	bool boolean(bool /*unused*/)
	{
		return true;
	}

	bool number_integer(number_integer_t /*unused*/)
	{
		return true;
	}

	bool number_unsigned(number_unsigned_t /*unused*/)
	{
		return true;
	}

	bool number_float(number_float_t /*unused*/, const string_t& /*unused*/)
	{
		return true;
	}

	bool string(string_t& /*unused*/)
	{
		return true;
	}

	bool binary(binary_t& /*unused*/)
	{
		return true;
	}

	bool start_object(std::size_t /*unused*/ = static_cast<std::size_t>(-1))
	{
		return true;
	}

	bool key(string_t& /*unused*/)
	{
		return true;
	}

	bool end_object()
	{
		return true;
	}

	bool start_array(std::size_t /*unused*/ = static_cast<std::size_t>(-1))
	{
		return true;
	}

	bool end_array()
	{
		return true;
	}

	bool parse_error(std::size_t /*unused*/, const std::string& /*unused*/, const nlohmann::detail::exception& e/*unused*/)
	{
		Info::getInstance().setInfoString(e.what());
		return false;
	}
};

UUnrealJSONBPLibrary::UUnrealJSONBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UUnrealJSONBPLibrary::DeleteField(const FString& json, const FString& fieldName, bool& success, FString& info, FString& result)
{
	success = true;

	if (json.IsEmpty())
	{
		info = "json IsEmpty";
		success = false;
		return;
	}

	if (fieldName.IsEmpty())
	{
		info = "fieldName IsEmpty";
		success = false;
		return;
	}

	json_sax_acceptor<nlohmann::json> my_sax;
	nlohmann::json j;

	if (nlohmann::json::sax_parse(FString_To_stdstring(json), &my_sax) == false)
	{
		info = stdstring_To_FString(Info::getInstance().getInfoString());
		success = false;
		return;
	}
	else
	{
		j = nlohmann::json::parse(FString_To_stdstring(json));
	}

	//auto r = j.find(FString_To_stdstring(fieldName));
	//if (r == j.end())
	//{
	//	info = "fieldName not exists";
	//	success = false;
	//	return;
	//}

	nlohmann::json* j_Ptr = nullptr;
	FString lastFieldName;

	if (pathCheck(j, fieldName, j_Ptr, lastFieldName, true) == false)
	{
		info = "Field error";
		success = false;
		return;
	}
	else if (j_Ptr == nullptr)
	{
		info = "Field error";
		success = false;
		return;
	}

	UUnrealJSONBPLibrary::Type type_T;
	if (fieldName_check(*j_Ptr, lastFieldName, type_T) == false)
	{
		info = "fieldName not exists";
		success = false;
		return;
	}

	if (type_T == UUnrealJSONBPLibrary::Type::arrayType)
	{
		j_Ptr->erase(FCString::Atoi(*lastFieldName));
	}
	else
	{
		j_Ptr->erase(FString_To_stdstring(lastFieldName));
	}

	std::ostringstream o;
	o << j;

	result = stdstring_To_FString(o.str());
}

bool UUnrealJSONBPLibrary::analyticalSeparator(const FString& fieldName, TArray<FString>& fieldNameArray)
{
	fieldNameArray.Empty();

	bool separator = false;
	FString str;

	for (auto i = 0; i < fieldName.Len(); i++)
	{
		if (separator == false)
		{
			if (fieldName[i] == '#')
			{
				if (i + 1 >= fieldName.Len())
				{
					return false;
				}

				if (fieldName[i + 1] == '-')
				{
					str += "-";
					i++;
				}
				else if (fieldName[i + 1] == '#')
				{
					str += "#";
					i++;
				}
				else
				{
					return false;
				}
			}
			else if (fieldName[i] == '-')
			{
				separator = true;
			}
			else
			{
				str += fieldName[i];
			}
		}
		else
		{
			if (str.IsEmpty())
			{
				return false;
			}
			else
			{
				i--;
				fieldNameArray.Push(str);
				str.Empty();
				separator = false;
			}
		}
	}

	if (str.IsEmpty())
	{
		return false;
	}
	else
	{
		fieldNameArray.Push(str);
	}

	return true;
}

bool UUnrealJSONBPLibrary::generationSeparator(const TArray<FString>& fieldNameArray, FString& fieldName)
{
	if (fieldNameArray.Num() == 0)
	{
		return false;
	}

	fieldName.Empty();

	for (auto i = 0; i < fieldNameArray.Num(); i++)
	{
		for (auto j = 0; j < fieldNameArray[i].Len(); j++)
		{
			if (fieldNameArray[i][j] == '#')
			{
				fieldName += "##";
			}
			else if (fieldNameArray[i][j] == '-')
			{
				fieldName += "#-";
			}
			else
			{
				fieldName += fieldNameArray[i][j];
			}
		}

		fieldName += "-";
	}

	if (fieldName.IsEmpty() == false)
	{
		fieldName = fieldName.Mid(0, fieldName.Len() - 1);
	}

	return true;
}

bool UUnrealJSONBPLibrary::matchingType(const FString& json, EJsonType type)
{
	if (nlohmann::json::accept(FString_To_stdstring(json)) == false)
	{
		return false;
	}

	nlohmann::json j = nlohmann::json::parse(FString_To_stdstring(json));

	switch (type)
	{
	case EJsonType::array:return j.is_array();
	case EJsonType::binary:return j.is_binary();
	case EJsonType::boolean:return j.is_boolean();
	case EJsonType::discarded:return j.is_discarded();
	case EJsonType::null:return j.is_null();
	case EJsonType::number:return j.is_number();
	case EJsonType::number_float:return j.is_number_float();
	case EJsonType::number_integer:return j.is_number_integer();
	case EJsonType::number_unsigned:return j.is_number_unsigned();
	case EJsonType::object:return j.is_object();
	case EJsonType::primitive:return j.is_primitive();
	case EJsonType::string:return j.is_string();
	case EJsonType::structured:return j.is_structured();
	default:
		return false;
	}
}

void UUnrealJSONBPLibrary::Generic_T_TO_JSON(FString mainKey, FProperty* property, void* propertyPtr, FString& json, bool& success, FString& info, int32 depth, bool lowercaseID)
{
	success = true;
	nlohmann::json j;

	if (mainKey.IsEmpty())
	{
		serialize(property, propertyPtr, success, info, j, depth, lowercaseID);
	}
	else
	{
		serialize(property, propertyPtr, success, info, j, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, 0, mainKey);
	}

	std::ostringstream o;
	o << j;

	json = stdstring_To_FString(o.str());
}

void UUnrealJSONBPLibrary::Generic_JSON_TO_T(const FString& json, FString fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, int32 depth, bool lowercaseID)
{
	success = true;
	json_sax_acceptor<nlohmann::json> my_sax;
	if (nlohmann::json::sax_parse(FString_To_stdstring(json), &my_sax) == false)
	{
		info = stdstring_To_FString(Info::getInstance().getInfoString());
		success = false;
		return;
	}
	else
	{
		auto j = nlohmann::json::parse(FString_To_stdstring(json));
		if (fieldName.IsEmpty())
		{
			deserialize(j, property, propertyPtr, success, info, depth, lowercaseID);
		}
		else
		{
			nlohmann::json* j_Ptr = nullptr;
			FString lastFieldName;

			if (pathCheck(j, fieldName, j_Ptr, lastFieldName) == false)
			{
				info = "Field error";
				success = false;
				return;
			}
			else if (j_Ptr == nullptr)
			{
				info = "Field error";
				success = false;
				return;
			}

			deserialize(*j_Ptr, property, propertyPtr, success, info, depth, lowercaseID);

			//auto r = j.find(FString_To_stdstring(fieldName));
			//if (r == j.end())
			//{
			//	info = "Field does not exist";
			//	success = false;
			//	return;
			//}
			//deserialize(r.value(), property, propertyPtr, success, info, depth);
		}
	}
}

void UUnrealJSONBPLibrary::Generic_AddField(const FString& json, const FString& fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, FString& result, int32 depth, bool keepJsonObject, bool lowercaseID)
{
	success = true;

	if (json.IsEmpty())
	{
		info = "json IsEmpty";
		success = false;
		return;
	}

	if (fieldName.IsEmpty())
	{
		info = "fieldName IsEmpty";
		success = false;
		return;
	}

	json_sax_acceptor<nlohmann::json> my_sax;
	nlohmann::json j;

	if (nlohmann::json::sax_parse(FString_To_stdstring(json), &my_sax) == false)
	{
		info = stdstring_To_FString(Info::getInstance().getInfoString());
		success = false;
		return;
	}
	else
	{
		j = nlohmann::json::parse(FString_To_stdstring(json));
	}

	//auto r = j.find(FString_To_stdstring(fieldName));
	//if (r != j.end())
	//{
	//	info = "fieldName already exists";
	//	success = false;
	//	return;
	//}
	nlohmann::json j_T;
	serialize(property, propertyPtr, success, info, j_T, depth, lowercaseID);

	nlohmann::json::iterator it = j_T.begin();

	//j[FString_To_stdstring(fieldName)] = it.value();

	nlohmann::json* j_Ptr = nullptr;
	FString lastFieldName;

	if (pathCheck(j, fieldName, j_Ptr, lastFieldName, true) == false)
	{
		info = "Field error";
		success = false;
		return;
	}
	else if (j_Ptr == nullptr)
	{
		info = "Field error";
		success = false;
		return;
	}

	UUnrealJSONBPLibrary::Type type_T;
	if (fieldName_check(*j_Ptr, lastFieldName, type_T))
	{
		//if ((*j_Ptr)[FString_To_stdstring(lastFieldName)].is_array())
		//{
		//}
		if (j_Ptr->is_array())
		{
		}
		else
		{
			info = "fieldName already exists";
			success = false;
			return;
		}
	}

	if (it.value().is_string() && keepJsonObject)
	{
		json_sax_acceptor<nlohmann::json> my_sax_object;
		nlohmann::json j_object;

		if (nlohmann::json::sax_parse(std::string(it.value()), &my_sax_object) == false)
		{
			info = stdstring_To_FString(Info::getInstance().getInfoString());
			success = false;
			return;
		}
		else
		{
			j_object = nlohmann::json::parse(std::string(it.value()));
		}

		if (type_T == UUnrealJSONBPLibrary::Type::arrayType)
		{
			nlohmann::json::iterator iterator = j_Ptr->begin();
			if (FCString::IsNumeric(*lastFieldName) == false)
			{
				info = "Field error";
				success = false;
				return;
			}
			int index = FCString::Atoi(*lastFieldName);
			if (index > j_Ptr->size())
			{
				info = "Field error";
				success = false;
				return;
			}
			iterator = iterator + index;
			j_Ptr->insert(iterator, j_object);
		}
		else
		{
			(*j_Ptr)[FString_To_stdstring(lastFieldName)] = j_object;
		}
	}
	else
	{
		if (type_T == UUnrealJSONBPLibrary::Type::arrayType)
		{
			nlohmann::json::iterator iterator = j_Ptr->begin();
			if (FCString::IsNumeric(*lastFieldName) == false)
			{
				info = "Field error";
				success = false;
				return;
			}
			int index = FCString::Atoi(*lastFieldName);
			if (index > j_Ptr->size())
			{
				info = "Field error";
				success = false;
				return;
			}
			iterator = iterator + index;
			j_Ptr->insert(iterator, it.value());
		}
		else
		{
			(*j_Ptr)[FString_To_stdstring(lastFieldName)] = it.value();
		}
	}

	std::ostringstream o;
	o << j;

	result = stdstring_To_FString(o.str());
}

void UUnrealJSONBPLibrary::Generic_UpdateField(const FString& json, const FString& fieldName, FProperty* property, void* propertyPtr, bool& success, FString& info, FString& result, int32 depth, bool keepJsonObject, bool lowercaseID)
{
	success = true;

	if (json.IsEmpty())
	{
		info = "json IsEmpty";
		success = false;
		return;
	}

	if (fieldName.IsEmpty())
	{
		info = "fieldName IsEmpty";
		success = false;
		return;
	}

	json_sax_acceptor<nlohmann::json> my_sax;
	nlohmann::json j;

	if (nlohmann::json::sax_parse(FString_To_stdstring(json), &my_sax) == false)
	{
		info = stdstring_To_FString(Info::getInstance().getInfoString());
		success = false;
		return;
	}
	else
	{
		j = nlohmann::json::parse(FString_To_stdstring(json));
	}

	//auto r = j.find(FString_To_stdstring(fieldName));
	//if (r == j.end())
	//{
	//	info = "fieldName not exists";
	//	success = false;
	//	return;
	//}
	nlohmann::json j_T;
	serialize(property, propertyPtr, success, info, j_T, depth, lowercaseID);

	nlohmann::json::iterator it = j_T.begin();

	//j[FString_To_stdstring(fieldName)] = it.value();
	nlohmann::json* j_Ptr = nullptr;
	FString lastFieldName;

	if (pathCheck(j, fieldName, j_Ptr, lastFieldName) == false)
	{
		info = "Field error";
		success = false;
		return;
	}
	else if (j_Ptr == nullptr)
	{
		info = "Field error";
		success = false;
		return;
	}

	if (it.value().is_string() && keepJsonObject)
	{
		json_sax_acceptor<nlohmann::json> my_sax_object;
		nlohmann::json j_object;

		if (nlohmann::json::sax_parse(std::string(it.value()), &my_sax_object) == false)
		{
			info = stdstring_To_FString(Info::getInstance().getInfoString());
			success = false;
			return;
		}
		else
		{
			j_object = nlohmann::json::parse(std::string(it.value()));
		}

		(*j_Ptr) = j_object;
	}
	else
	{
		(*j_Ptr) = it.value();
	}

	std::ostringstream o;
	o << j;

	result = stdstring_To_FString(o.str());
}

void UUnrealJSONBPLibrary::serialize(FProperty* property, void* propertyPtr, bool& success, FString& info, nlohmann::json& j, int32 depth, bool lowercaseID, UUnrealJSONBPLibrary::Type type, nlohmann::json j_mapKey, int32 count, FString mainKey)
{
	if (count >= depth)
	{
		return;
	}

	auto BoolProperty = CastField<FBoolProperty>(property);
	auto ByteProperty = CastField<FByteProperty>(property);
	auto IntProperty = CastField<FIntProperty>(property);
	auto Int64Property = CastField<FInt64Property>(property);
	auto FloatProperty = CastField<FFloatProperty>(property);
	auto NameProperty = CastField<FNameProperty>(property);
	auto StrProperty = CastField<FStrProperty>(property);
	auto TextProperty = CastField<FTextProperty>(property);
	auto StructProperty = CastField<FStructProperty>(property);
	auto ObjectProperty = CastField<FObjectProperty>(property);
	auto ArrayProperty = CastField<FArrayProperty>(property);
	auto EnumProperty = CastField<FEnumProperty>(property);
	auto MapProperty = CastField<FMapProperty>(property);
	auto SetProperty = CastField<FSetProperty>(property);

	if (BoolProperty)
	{
		auto BoolValue = BoolProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(BoolValue);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[BoolValue] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = BoolValue;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[BoolValue] = BoolValue;
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(BoolProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}

				j[name] = BoolValue;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = BoolValue;
			}
		}
	}
	else if (ByteProperty)
	{
		auto ByteValue = ByteProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(ByteValue);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[ByteValue] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = ByteValue;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[ByteValue] = ByteValue;
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(ByteProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = ByteValue;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = ByteValue;
			}
		}
	}
	else if (IntProperty)
	{
		auto IntValue = IntProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(IntValue);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[IntValue] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = IntValue;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[IntValue] = IntValue;
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(IntProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = IntValue;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = IntValue;
			}
		}
	}
	else if (Int64Property)
	{
		auto Int64Value = Int64Property->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(Int64Value);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[Int64Value] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = Int64Value;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[Int64Value] = Int64Value;
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(Int64Property->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = Int64Value;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = Int64Value;
			}
		}
	}
	else if (FloatProperty)
	{
		auto FloatValue = FloatProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(FloatValue);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[FloatValue] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = FloatValue;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[FloatValue] = FloatValue;
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(FloatProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = FloatValue;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = FloatValue;
			}
		}
	}
	else if (NameProperty)
	{
		auto NameValue = NameProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(FString_To_stdstring(NameValue.ToString()));
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[FString_To_stdstring(NameValue.ToString())] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = FString_To_stdstring(NameValue.ToString());
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[FString_To_stdstring(NameValue.ToString())] = FString_To_stdstring(NameValue.ToString());
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(NameProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = FString_To_stdstring(NameValue.ToString());
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = FString_To_stdstring(NameValue.ToString());
			}
		}
	}
	else if (StrProperty)
	{
		auto StrValue = StrProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(FString_To_stdstring(StrValue));
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[FString_To_stdstring(StrValue)] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = FString_To_stdstring(StrValue);
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[FString_To_stdstring(StrValue)] = FString_To_stdstring(StrValue);
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(StrProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = FString_To_stdstring(StrValue);
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = FString_To_stdstring(StrValue);
			}
		}
	}
	else if (TextProperty)
	{
		auto TextValue = TextProperty->GetPropertyValue(propertyPtr);
		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(FString_To_stdstring(TextValue.ToString()));
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[FString_To_stdstring(TextValue.ToString())] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = FString_To_stdstring(TextValue.ToString());
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[FString_To_stdstring(TextValue.ToString())] = FString_To_stdstring(TextValue.ToString());
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(TextProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = FString_To_stdstring(TextValue.ToString());
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = FString_To_stdstring(TextValue.ToString());
			}
		}
	}
	else if (StructProperty)
	{
		nlohmann::json j_T;

		for (TFieldIterator<FProperty> it(StructProperty->Struct); it; ++it)
		{
			auto param = it->ContainerPtrToValuePtr<void>(propertyPtr);

			serialize(*it, param, success, info, j_T, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
		}

		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(j_T);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			std::ostringstream o;
			o << j_T;
			j[std::string(o.str())] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = j_T;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			std::ostringstream o;
			o << j_T;
			j[std::string(o.str())] = std::string(o.str());
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(StructProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = j_T;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = j_T;
			}
		}
	}
	else if (ObjectProperty)
	{
		nlohmann::json j_T;

		auto object = ObjectProperty->GetObjectPropertyValue(propertyPtr);

		if (object == nullptr)
		{
			return;
		}

		for (TFieldIterator<FProperty> it(object->GetClass()); it; ++it)
		{
			auto param = it->ContainerPtrToValuePtr<void>(object);

			serialize(*it, param, success, info, j_T, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
		}

		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(j_T);
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			std::ostringstream o;
			o << j_T;
			j[std::string(o.str())] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = j_T;
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			std::ostringstream o;
			o << j_T;
			j[std::string(o.str())] = std::string(o.str());
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name = FString_To_stdstring(ObjectProperty->GetName());
				if (lowercaseID)
				{
					if (name._Equal("ID"))
					{
						name = "id";
					}
				}
				j[name] = j_T;
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = j_T;
			}
		}
	}
	else if (ArrayProperty)
	{
		nlohmann::json j_T;

		auto length = UKismetArrayLibrary::GenericArray_Length(propertyPtr, ArrayProperty);

		for (auto index = 0; index < UKismetArrayLibrary::GenericArray_Length(propertyPtr, ArrayProperty); index++)
		{
			const int32 itemSize = ArrayProperty->Inner->ElementSize;
			uint8* itemAddr = (uint8*)FMemory::Malloc(itemSize);
			FMemory::Memzero(itemAddr, itemSize);
			UKismetArrayLibrary::GenericArray_Get(propertyPtr, ArrayProperty, index, itemAddr);

			serialize(ArrayProperty->Inner, itemAddr, success, info, j_T, depth, lowercaseID, UUnrealJSONBPLibrary::Type::arrayType, {}, count + 1);
			FMemory::Free(itemAddr);
		}

		if (j_T.is_null())
		{
			j_T = nlohmann::json::array();
		}

		if (mainKey.IsEmpty())
		{
			std::string name = FString_To_stdstring(ArrayProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}
			j[name] = j_T;
		}
		else
		{
			j[FString_To_stdstring(mainKey)] = j_T;
		}
	}
	else if (EnumProperty)
	{
		auto EnumValue = EnumProperty->GetUnderlyingProperty()->GetValueTypeHash(propertyPtr);
		FString className = EnumProperty->GetEnum()->GetName();
		FString name = className + "::" + EnumProperty->GetEnum()->GetNameStringByIndex(EnumValue);

		if (type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			j.push_back(FString_To_stdstring(name));
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapKeyType)
		{
			j[FString_To_stdstring(name)] = j_mapKey["mapValue"];
		}
		else if (type == UUnrealJSONBPLibrary::Type::mapValueType)
		{
			j["mapValue"] = FString_To_stdstring(name);
		}
		else if (type == UUnrealJSONBPLibrary::Type::setType)
		{
			j[FString_To_stdstring(name)] = FString_To_stdstring(name);
		}
		else
		{
			if (mainKey.IsEmpty())
			{
				std::string name_t = FString_To_stdstring(EnumProperty->GetName());
				if (lowercaseID)
				{
					if (name_t._Equal("ID"))
					{
						name_t = "id";
					}
				}
				j[name_t] = FString_To_stdstring(name);
			}
			else
			{
				j[FString_To_stdstring(mainKey)] = FString_To_stdstring(name);
			}
		}
	}
	else if (MapProperty)
	{
		nlohmann::json j_T;

		auto length = UBlueprintMapLibrary::GenericMap_Length(propertyPtr, MapProperty);
		FScriptMapHelper MapHelper(MapProperty, propertyPtr);
		FProperty* keyProp = MapProperty->KeyProp;
		FProperty* valueProp = MapProperty->ValueProp;
		for (int32 i = 0; i < length; ++i)
		{
			if (MapHelper.IsValidIndex(i))
			{
				const int32 keySize = MapProperty->KeyProp->ElementSize;
				uint8* keyAddr = (uint8*)FMemory::Malloc(keySize);
				const int32 ValueSize = MapProperty->ValueProp->ElementSize;
				uint8* ValueAddr = (uint8*)FMemory::Malloc(ValueSize);
				FMemory::Memzero(keyAddr, keySize);
				FMemory::Memzero(ValueAddr, ValueSize);

				keyProp->CopyCompleteValueFromScriptVM(keyAddr, MapHelper.GetKeyPtr(i));
				valueProp->CopyCompleteValueFromScriptVM(ValueAddr, MapHelper.GetValuePtr(i));

				nlohmann::json j_map;
				serialize(valueProp, ValueAddr, success, info, j_map, depth, lowercaseID, UUnrealJSONBPLibrary::Type::mapValueType, {}, count + 1);
				serialize(keyProp, keyAddr, success, info, j_T, depth, lowercaseID, UUnrealJSONBPLibrary::Type::mapKeyType, j_map, count + 1);

				FMemory::Free(keyAddr);
				FMemory::Free(ValueAddr);
			}
		}

		if (mainKey.IsEmpty())
		{
			std::string name = FString_To_stdstring(MapProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}
			j[name] = j_T;
		}
		else
		{
			j[FString_To_stdstring(mainKey)] = j_T;
		}
	}
	else if (SetProperty)
	{
		nlohmann::json j_T;

		auto length = UBlueprintSetLibrary::GenericSet_Length(propertyPtr, SetProperty);
		FScriptSetHelper SetHelper(SetProperty, propertyPtr);
		FProperty* elementProp = SetProperty->ElementProp;
		for (int32 i = 0; i < length; ++i)
		{
			if (SetHelper.IsValidIndex(i))
			{
				const int32 elementSize = SetProperty->ElementSize;
				uint8* elementAddr = (uint8*)FMemory::Malloc(elementSize);
				FMemory::Memzero(elementAddr, elementSize);

				elementProp->CopyCompleteValueFromScriptVM(elementAddr, SetHelper.GetElementPtr(i));

				nlohmann::json j_map;
				serialize(elementProp, elementAddr, success, info, j_map, depth, lowercaseID, UUnrealJSONBPLibrary::Type::setType, {}, count + 1);

				FMemory::Free(elementAddr);
			}
		}

		if (mainKey.IsEmpty())
		{
			std::string name = FString_To_stdstring(SetProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}
			j[name] = j_T;
		}
		else
		{
			j[FString_To_stdstring(mainKey)] = j_T;
		}
	}
	else
	{
		nlohmann::json j_T;
		j_T["unknown type"].push_back(FString_To_stdstring(property->GetName()));
	}
}

void UUnrealJSONBPLibrary::deserialize(nlohmann::json& j, FProperty* property, void* propertyPtr, bool& success, FString& info, int32 depth, bool lowercaseID, UUnrealJSONBPLibrary::Type type, nlohmann::json j_mapKey, int32 count)
{
	UUnrealJSONBPLibrary::Type type_T;

	if (count >= depth)
	{
		return;
	}

	auto BoolProperty = CastField<FBoolProperty>(property);
	auto ByteProperty = CastField<FByteProperty>(property);
	auto IntProperty = CastField<FIntProperty>(property);
	auto Int64Property = CastField<FInt64Property>(property);
	auto FloatProperty = CastField<FFloatProperty>(property);
	auto NameProperty = CastField<FNameProperty>(property);
	auto StrProperty = CastField<FStrProperty>(property);
	auto TextProperty = CastField<FTextProperty>(property);
	auto StructProperty = CastField<FStructProperty>(property);
	auto ObjectProperty = CastField<FObjectProperty>(property);
	auto ArrayProperty = CastField<FArrayProperty>(property);
	auto EnumProperty = CastField<FEnumProperty>(property);
	auto MapProperty = CastField<FMapProperty>(property);
	auto SetProperty = CastField<FSetProperty>(property);

	if (BoolProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::boolean || j.type() == nlohmann::json::value_t::null)
			{
				BoolProperty->SetPropertyValue(propertyPtr, j);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(BoolProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::boolean || j_T.type() == nlohmann::json::value_t::null)
			{
				BoolProperty->SetPropertyValue(propertyPtr, j_T);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (ByteProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::null)
			{
				ByteProperty->SetPropertyValue(propertyPtr, j);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(ByteProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::number_unsigned || j_T.type() == nlohmann::json::value_t::number_integer || j_T.type() == nlohmann::json::value_t::null)
			{
				ByteProperty->SetPropertyValue(propertyPtr, j_T);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (IntProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::null)
			{
				IntProperty->SetPropertyValue(propertyPtr, j);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(IntProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::number_unsigned || j_T.type() == nlohmann::json::value_t::number_integer || j_T.type() == nlohmann::json::value_t::null)
			{
				IntProperty->SetPropertyValue(propertyPtr, j_T);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (Int64Property)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::null)
			{
				Int64Property->SetPropertyValue(propertyPtr, j);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(Int64Property->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::number_unsigned || j_T.type() == nlohmann::json::value_t::number_integer || j_T.type() == nlohmann::json::value_t::null)
			{
				Int64Property->SetPropertyValue(propertyPtr, j_T);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (FloatProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::number_float || j.type() == nlohmann::json::value_t::null)
			{
				FloatProperty->SetPropertyValue(propertyPtr, j);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(FloatProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::number_unsigned || j_T.type() == nlohmann::json::value_t::number_integer || j_T.type() == nlohmann::json::value_t::number_float || j_T.type() == nlohmann::json::value_t::null)
			{
				FloatProperty->SetPropertyValue(propertyPtr, j_T);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (NameProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::string || j.type() == nlohmann::json::value_t::null)
			{
				NameProperty->SetPropertyValue(propertyPtr, FName(stdstring_To_FString(j)));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(NameProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::string || j_T.type() == nlohmann::json::value_t::null)
			{
				NameProperty->SetPropertyValue(propertyPtr, FName(stdstring_To_FString(j_T)));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (StrProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::string || j.type() == nlohmann::json::value_t::null)
			{
				StrProperty->SetPropertyValue(propertyPtr, stdstring_To_FString(j));
			}
			else if (j.type() == nlohmann::json::value_t::object)
			{
				std::ostringstream o;
				o << j;

				StrProperty->SetPropertyValue(propertyPtr, stdstring_To_FString(o.str()));
			}
			else if (j.type() == nlohmann::json::value_t::number_float || j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::boolean)
			{
				std::ostringstream o;
				o << j;

				StrProperty->SetPropertyValue(propertyPtr, stdstring_To_FString(o.str()));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(StrProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::string || j_T.type() == nlohmann::json::value_t::null)
			{
				StrProperty->SetPropertyValue(propertyPtr, stdstring_To_FString(j_T));
			}
			else if (j.type() == nlohmann::json::value_t::object)
			{
				std::ostringstream o;
				o << j;

				StrProperty->SetPropertyValue(propertyPtr, stdstring_To_FString(o.str()));
			}
			else if (j.type() == nlohmann::json::value_t::number_float || j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::boolean)
			{
				std::ostringstream o;
				o << j;

				StrProperty->SetPropertyValue(propertyPtr, stdstring_To_FString(o.str()));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (TextProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::string || j.type() == nlohmann::json::value_t::null)
			{
				TextProperty->SetPropertyValue(propertyPtr, FText::FromString(stdstring_To_FString(j)));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(TextProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::string || j_T.type() == nlohmann::json::value_t::null)
			{
				TextProperty->SetPropertyValue(propertyPtr, FText::FromString(stdstring_To_FString(j_T)));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (StructProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::object || j.type() == nlohmann::json::value_t::null)
			{
				for (TFieldIterator<FProperty> it(StructProperty->Struct); it; ++it)
				{
					auto param = it->ContainerPtrToValuePtr<void>(propertyPtr);
					deserialize(j, *it, param, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(StructProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::object || j_T.type() == nlohmann::json::value_t::null)
			{
				for (TFieldIterator<FProperty> it(StructProperty->Struct); it; ++it)
				{
					auto param = it->ContainerPtrToValuePtr<void>(propertyPtr);
					deserialize(j_T, *it, param, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (ObjectProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::object || j.type() == nlohmann::json::value_t::null)
			{
				auto object = ObjectProperty->GetObjectPropertyValue(propertyPtr);

				if (object == nullptr)
				{
					return;
				}

				for (TFieldIterator<FProperty> it(object->GetClass()); it; ++it)
				{
					auto param = it->ContainerPtrToValuePtr<void>(object);

					deserialize(j, *it, param, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(ObjectProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_O = j[name];
			if (j_O.type() == nlohmann::json::value_t::object || j_O.type() == nlohmann::json::value_t::null)
			{
				auto object = ObjectProperty->GetObjectPropertyValue(propertyPtr);

				if (object == nullptr)
				{
					return;
				}

				for (TFieldIterator<FProperty> it(object->GetClass()); it; ++it)
				{
					auto param = it->ContainerPtrToValuePtr<void>(object);

					deserialize(j_O, *it, param, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (ArrayProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::array || j.type() == nlohmann::json::value_t::null)
			{
				auto length = j.size();
				UKismetArrayLibrary::GenericArray_Clear(propertyPtr, ArrayProperty);

				nlohmann::json j_T;

				for (auto index = 0; index < length; index++)
				{
					j_T.clear();
					j_T = j[index];
					const int32 itemSize = ArrayProperty->Inner->ElementSize;
					uint8* itemAddr = (uint8*)FMemory::Malloc(itemSize);
					FMemory::Memzero(itemAddr, itemSize);
					deserialize(j_T, ArrayProperty->Inner, itemAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::arrayType, {}, count + 1);
					UKismetArrayLibrary::GenericArray_Set(propertyPtr, ArrayProperty, index, itemAddr, true);
					FMemory::Free(itemAddr);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(ArrayProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_A = j[name];
			if (j_A.type() == nlohmann::json::value_t::array || j_A.type() == nlohmann::json::value_t::null)
			{
				auto length = j_A.size();
				UKismetArrayLibrary::GenericArray_Clear(propertyPtr, ArrayProperty);

				nlohmann::json j_T;

				for (auto index = 0; index < length; index++)
				{
					j_T.clear();
					j_T = j_A[index];
					const int32 itemSize = ArrayProperty->Inner->ElementSize;
					uint8* itemAddr = (uint8*)FMemory::Malloc(itemSize);
					FMemory::Memzero(itemAddr, itemSize);
					deserialize(j_T, ArrayProperty->Inner, itemAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::arrayType, {}, count + 1);
					UKismetArrayLibrary::GenericArray_Set(propertyPtr, ArrayProperty, index, itemAddr, true);
					FMemory::Free(itemAddr);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (EnumProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::string || j.type() == nlohmann::json::value_t::null)
			{
				success = false;
				info = "The generated enumeration type currently only supports Int64";
			}
			else if (j.type() == nlohmann::json::value_t::number_integer || j.type() == nlohmann::json::value_t::number_unsigned || j.type() == nlohmann::json::value_t::null)
			{
				EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(propertyPtr, (int64)j);
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(StrProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_T = j[name];
			if (j_T.type() == nlohmann::json::value_t::string || j_T.type() == nlohmann::json::value_t::null)
			{
				TextProperty->SetPropertyValue(propertyPtr, FText::FromString(stdstring_To_FString(j_T)));
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (MapProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::object || j.type() == nlohmann::json::value_t::null)
			{
				UBlueprintMapLibrary::GenericMap_Clear(propertyPtr, MapProperty);

				FScriptMapHelper MapHelper(MapProperty, propertyPtr);
				FProperty* keyProp = MapProperty->KeyProp;
				FProperty* valueProp = MapProperty->ValueProp;

				for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
				{
					json_sax_acceptor<nlohmann::json> my_sax_prop;

					FString key_name_T = stdstring_To_FString(it.key());
					nlohmann::json j_T_prop;
					if (nlohmann::json::sax_parse(it.key(), &my_sax_prop) == false)
					{
						std::string format_str = escapeCharacterProcessing(it.key());
						if (nlohmann::json::sax_parse(format_str, &my_sax_prop) == false)
						{
							success = false;
							info = "type mismatch";
							return;
						}
						else
						{
							j_T_prop = nlohmann::json::parse(format_str);
						}
					}
					else
					{
						j_T_prop = nlohmann::json::parse(it.key());
					}

					const int32 keySize = MapProperty->KeyProp->ElementSize;
					uint8* keyAddr = (uint8*)FMemory::Malloc(keySize);
					const int32 ValueSize = MapProperty->ValueProp->ElementSize;
					uint8* ValueAddr = (uint8*)FMemory::Malloc(ValueSize);
					FMemory::Memzero(keyAddr, keySize);
					FMemory::Memzero(ValueAddr, ValueSize);

					deserialize(j_T_prop, keyProp, keyAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::mapKeyType, {}, count + 1);
					deserialize(it.value(), valueProp, ValueAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::mapValueType, {}, count + 1);

					UBlueprintMapLibrary::GenericMap_Add(propertyPtr, MapProperty, keyAddr, ValueAddr);

					FMemory::Free(keyAddr);
					FMemory::Free(ValueAddr);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(MapProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_M = j[name];
			if (j_M.type() == nlohmann::json::value_t::object || j_M.type() == nlohmann::json::value_t::null)
			{
				UBlueprintMapLibrary::GenericMap_Clear(propertyPtr, MapProperty);

				FScriptMapHelper MapHelper(MapProperty, propertyPtr);
				FProperty* keyProp = MapProperty->KeyProp;
				FProperty* valueProp = MapProperty->ValueProp;

				for (nlohmann::json::iterator it = j_M.begin(); it != j_M.end(); ++it)
				{
					json_sax_acceptor<nlohmann::json> my_sax_prop;
					if (nlohmann::json::sax_parse(it.key(), &my_sax_prop) == false)
					{
						success = false;
						info = "type mismatch";
						return;
					}

					const int32 keySize = MapProperty->KeyProp->ElementSize;
					uint8* keyAddr = (uint8*)FMemory::Malloc(keySize);
					const int32 ValueSize = MapProperty->ValueProp->ElementSize;
					uint8* ValueAddr = (uint8*)FMemory::Malloc(ValueSize);
					FMemory::Memzero(keyAddr, keySize);
					FMemory::Memzero(ValueAddr, ValueSize);

					nlohmann::json j_T_prop = nlohmann::json::parse(it.key());

					deserialize(j_T_prop, keyProp, keyAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);
					deserialize(it.value(), valueProp, ValueAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::other, {}, count + 1);

					UBlueprintMapLibrary::GenericMap_Add(propertyPtr, MapProperty, keyAddr, ValueAddr);

					FMemory::Free(keyAddr);
					FMemory::Free(ValueAddr);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
	else if (SetProperty)
	{
		if (count == 0 || type == UUnrealJSONBPLibrary::Type::mapKeyType || type == UUnrealJSONBPLibrary::Type::mapValueType || type == UUnrealJSONBPLibrary::Type::setType || type == UUnrealJSONBPLibrary::Type::arrayType)
		{
			if (j.type() == nlohmann::json::value_t::object || j.type() == nlohmann::json::value_t::null)
			{
				UBlueprintSetLibrary::GenericSet_Clear(propertyPtr, SetProperty);

				FScriptSetHelper SetHelper(SetProperty, propertyPtr);
				FProperty* elementProp = SetProperty->ElementProp;

				for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it)
				{
					json_sax_acceptor<nlohmann::json> my_sax_prop;

					const int32 elementSize = SetProperty->ElementSize;
					uint8* elementAddr = (uint8*)FMemory::Malloc(elementSize);
					FMemory::Memzero(elementAddr, elementSize);

					deserialize(it.value(), elementProp, elementAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::setType, {}, count + 1);

					UBlueprintSetLibrary::GenericSet_Add(propertyPtr, SetProperty, elementAddr);

					FMemory::Free(elementAddr);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
		else
		{
			std::string name = FString_To_stdstring(SetProperty->GetName());
			if (lowercaseID)
			{
				if (name._Equal("ID"))
				{
					name = "id";
				}
			}

			if (!fieldName_check(j, stdstring_To_FString(name), type_T))
			{
				info = "fieldName not exists";
				success = false;
				return;
			}

			nlohmann::json j_M = j[name];
			if (j_M.type() == nlohmann::json::value_t::object || j_M.type() == nlohmann::json::value_t::null)
			{
				UBlueprintSetLibrary::GenericSet_Clear(propertyPtr, SetProperty);

				FScriptSetHelper SetHelper(SetProperty, propertyPtr);
				FProperty* elementProp = SetProperty->ElementProp;

				for (nlohmann::json::iterator it = j_M.begin(); it != j_M.end(); ++it)
				{
					json_sax_acceptor<nlohmann::json> my_sax_prop;

					const int32 elementSize = SetProperty->ElementSize;
					uint8* elementAddr = (uint8*)FMemory::Malloc(elementSize);
					FMemory::Memzero(elementAddr, elementSize);

					deserialize(it.value(), elementProp, elementAddr, success, info, depth, lowercaseID, UUnrealJSONBPLibrary::Type::setType, {}, count + 1);

					UBlueprintSetLibrary::GenericSet_Add(propertyPtr, SetProperty, elementAddr);

					FMemory::Free(elementAddr);
				}
			}
			else
			{
				success = false;
				info = "type mismatch";
			}
		}
	}
}

std::string UUnrealJSONBPLibrary::FString_To_stdstring(const FString& s)
{
	return std::move(std::string(TCHAR_TO_UTF8(*s)));
}

FString UUnrealJSONBPLibrary::stdstring_To_FString(const std::string& s)
{
	return std::move(FString(UTF8_TO_TCHAR(s.c_str())));
}

std::string UUnrealJSONBPLibrary::escapeCharacterProcessing(const std::string& s)
{
	std::string s_T;

	for (auto i : s)
	{
		switch (i)
		{
		case '\a':
			s_T.append("\\a");
			break;
		case '\b':
			s_T.append("\\b");
			break;
		case '\f':
			s_T.append("\\f");
			break;
		case '\n':
			s_T.append("\\n");
			break;
		case '\r':
			s_T.append("\\r");
			break;
		case '\t':
			s_T.append("\\t");
			break;
		case '\v':
			s_T.append("\\v");
			break;
		case '\\':
			s_T.append("\\\\");
			break;
		case '\?':
			s_T.append("\\?");
			break;
		case '\'':
			s_T.append("\\'");
			break;
		case '\"':
			s_T.append("\\\"");
			break;
		case '\0':
			s_T.append("\\0");
			break;
		default:
			s_T.push_back(i);
			break;
		}
	}

	s_T = '\"' + s_T + '\"';

	return std::move(s_T);
}

bool UUnrealJSONBPLibrary::fieldName_check(const nlohmann::json& j, const FString& fieldName, UUnrealJSONBPLibrary::Type& type)
{
	if (j.is_object())
	{
		type = UUnrealJSONBPLibrary::Type::other;
		auto r = j.find(FString_To_stdstring(fieldName));
		if (r == j.end())
		{
			return false;
		}
	}
	else if (j.is_array())
	{
		type = UUnrealJSONBPLibrary::Type::arrayType;
		if (fieldName.IsNumeric())
		{
			int32 index = FCString::Atoi(*fieldName);
			if (index >= j.size())
			{
				return false;
			}
		}
		else
		{
			return false;
		}
		}

	return true;
	}

bool UUnrealJSONBPLibrary::pathCheck(nlohmann::json& j, const FString& fieldName, nlohmann::json*& j_Ptr, FString& lastFieldName, bool retain)
{
	UUnrealJSONBPLibrary::Type type_T;
	TArray<FString> fieldNameArray;
	if (analyticalSeparator(fieldName, fieldNameArray) == false)
	{
		return false;
	}

	j_Ptr = &j;

	int retain_index = retain ? 1 : 0;

	if (retain && fieldNameArray.Num() > 0)
	{
		lastFieldName = fieldNameArray[fieldNameArray.Num() - 1];
	}

	for (auto i = 0; i < fieldNameArray.Num() - retain_index; i++)
	{
		if (fieldName_check(*j_Ptr, fieldNameArray[i], type_T) == false)
		{
			return false;
		}

		if (j_Ptr->is_object())
		{
			j_Ptr = &((*j_Ptr)[FString_To_stdstring(fieldNameArray[i])]);
		}
		else if (j_Ptr->is_array())
		{
			j_Ptr = &((*j_Ptr)[FCString::Atoi(*fieldNameArray[i])]);
		}
		else
		{
			return false;
		}
	}

	//*j_Ptr = it.value();

	return true;
}

#if DEBUG_JSONTOOLS
PRAGMA_ENABLE_OPTIMIZATION
#endif
