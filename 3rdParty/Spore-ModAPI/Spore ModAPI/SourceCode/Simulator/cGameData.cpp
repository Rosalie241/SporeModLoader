#ifndef MODAPI_DLL_EXPORT
#include <Spore\Simulator\cGameData.h>
#include <Spore\Simulator\NounClassFactories.h>
#include <Spore\App\ScenarioMode.h>

auto_STATIC_METHOD_VOID(Simulator, RegisterNounType, Args(uint32_t nounID, uint32_t typeID, const char* name), Args(nounID, typeID, name));
//auto_STATIC_METHOD_(Simulator, Simulator::NounCreateMap&, GetNounCreateMap);

namespace Simulator
{
	auto_METHOD_VIRTUAL(cGameData, cGameData, bool, Write, Args(ISerializerStream* stream), Args(stream));
	auto_METHOD_VIRTUAL(cGameData, cGameData, bool, Read, Args(ISerializerStream* stream), Args(stream));
	auto_METHOD_VIRTUAL_(cGameData, cGameData, bool, ISimulatorSerializable_func18h);
	auto_METHOD_VIRTUAL(cGameData, cGameData, bool, WriteToXML, Args(XmlSerializer* arg_0), Args(arg_0));
	auto_METHOD_VIRTUAL(cGameData, cGameData, bool, SetDefinitionID, Args(int arg_0, int arg_4, int arg_8), Args(arg_0, arg_4, arg_8));
	auto_METHOD_VIRTUAL_VOID(cGameData, cGameData, SetGameDataOwner, Args(cGameData* pOwner), Args(pOwner));
	auto_METHOD_VIRTUAL_(cGameData, cGameData, bool, IsDestroyed);
	
	auto_METHOD_VIRTUAL_VOID(cGameData, cGameData, SetGameDataOwner2, Args(cGameData* pOwner), Args(pOwner));

	auto_METHOD_VIRTUAL(cGameData, cGameData, bool, cGameData_func3Ch, Args(int arg_0), Args(arg_0));

	auto_METHOD_VIRTUAL_VOID_(cGameData, cGameData, RemoveOwner);
	
	auto_METHOD_VIRTUAL(cGameData, cGameData, int, WriteAsText, Args(int arg_0), Args(arg_0));

	cGameData* cGameData::GetGameDataOwner() {
		return mpGameDataOwner.get();
	}

	void cGameData::SetPoliticalID(uint32_t id) {
		mPoliticalID = id;
	}

	uint32_t cGameData::GetPoliticalID() {
		return mPoliticalID;
	}

	bool cGameData::cGameData_func40h() {
		return true;
	}

	int cGameData::AddRef() {
		return DefaultRefCounted::AddRef();
	}
	int cGameData::Release() {
		return DefaultRefCounted::Release();
	}

	void* cGameData::Cast(uint32_t type) const {
		CLASS_CAST(ISimulatorSerializable);
		CLASS_CAST(Object);
		CLASS_CAST(cGameData);
		return nullptr;
	}

	cGameData::cGameData()
		: mpView(nullptr)
		, mpScenarioMarker(nullptr)
		, mScenarioClassIndex(-1)
		, field_20()
		, mbIsDestroyed()
		, mID(0xFFFFFFFF)
		, mDefinitionID()
		, mpGameDataOwner()
		, mPoliticalID(0xFFFFFFFF)
	{}

	cScenarioClass* cGameData::GetScenarioClass()
	{
		if (mScenarioClassIndex == -1) return nullptr;
		else return ScenarioMode.GetData()->GetClass(mScenarioClassIndex);
	}

	eastl::fixed_string<char16_t, 32> GetSpecializedName(cGameData* object) {
		eastl::fixed_string<char16_t, 32> str;
		CALL(GetAddress(Simulator, GetSpecializedName), void,
			Args(eastl::fixed_string<char16_t, 32>&, cGameData*), Args(str, object));
		return str;
	}
}
#endif