#pragma once

#include "../Interfaces/IVEngineClient.h"
#include "../Interfaces/IBaseClientDLL.h"
#include "UtlVector.h"

MAKE_SIGNATURE(GetItemSchema, "client.dll", "48 83 EC ? E8 ? ? ? ? 48 83 C0 ? 48 83 C4 ? C3 CC CC CC", 0x0);
MAKE_SIGNATURE(CEconItemSchema_GetAttributeDefinition, "client.dll", "89 54 24 ? 53 48 83 EC ? 48 8B D9 48 8D 54 24 ? 48 81 C1 ? ? ? ? E8 ? ? ? ? 8B D0 3B 83 ? ? ? ? 73 ? 8B 83 ? ? ? ? 83 F8 ? 74 ? 3B D0 7F ? 48 81 C3 ? ? ? ? 44 8B C2 83 FA ? 74 ? 48 8B 03 8B CA", 0x0);
MAKE_SIGNATURE(CAttributeList_SetRuntimeAttributeValue, "client.dll", "48 89 5C 24 10 55 56 57 48 8B EC 48 83 EC 50 44", 0x0);

class CEconItemAttribute
{
public:
	void* pad = 0;
	unsigned int m_iAttributeDefinitionIndex;

	union
	{
		int m_iRawValue32;
		float m_flValue;
	};
	int m_nRefundableCurrency = 0;

	inline CEconItemAttribute(uint16_t iAttributeDefinitionIndex, float flValue)
	{
		m_iAttributeDefinitionIndex = iAttributeDefinitionIndex;
		m_flValue = flValue;
	}
};

class CAttributeList
{
public:
	void* pad;
	CUtlVector<CEconItemAttribute> m_Attributes;
	void* m_pManager;

	inline void AddAttribute(int iIndex, float flValue)
	{
		if (m_Attributes.Count() > 14)
			return;

		CEconItemAttribute attr(iIndex, flValue);

		m_Attributes.AddToTail(attr);
	}

	using GetItemSchemaFN = void* (__fastcall*)();
	using GetAttributeDefinitionFN = void* (__fastcall*)(void*, int);
	using SetRuntimeAttributeValueFN = void(__fastcall*)(CAttributeList*, void*, float);

	void SetAttribute(int index, float value)
	{
		auto schema = reinterpret_cast<GetItemSchemaFN>(S::GetItemSchema())();

		auto attributeDefinition = reinterpret_cast<GetAttributeDefinitionFN>(S::CEconItemSchema_GetAttributeDefinition())(schema, index);
		if (!attributeDefinition)
			return;

		reinterpret_cast<SetRuntimeAttributeValueFN>(S::CAttributeList_SetRuntimeAttributeValue())(this, attributeDefinition, value);
	}
};