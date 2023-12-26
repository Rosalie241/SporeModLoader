#ifndef MODAPI_DLL_EXPORT
#include <Spore\Simulator\Cell\cCellResource.h>
#include <Spore\Simulator\Cell\cCellGFX.h>
#include <Spore\Simulator\Cell\cCellGame.h>
#include <Spore\Simulator\Cell\cCellUI.h>
#include <Spore\Simulator\Cell\CellFunctions.h>

namespace Simulator
{
	namespace Cell
	{
		GoalCard* GetGoalCards() {
			return (GoalCard*)GetAddress(Cell, GoalCards_ptr);
		}

		void cCellDataReference_::AddRef() {
			mCounter++;
		}
		void cCellDataReference_::Release() {
			mCounter--;
		}

		auto_STATIC_METHOD(cCellDataReference_, cCellDataReference_*, Create,
			Args(uint32_t instanceID, CellSerializer* serializer), Args(instanceID, serializer));

		CellSerializer* cCellLootTableResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__lootTable);
		}
		CellSerializer* cCellCellResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__cell);
		}
		CellSerializer* cCellPopulateResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__populate);
		}
		CellSerializer* cCellWorldResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__world);
		}
		CellSerializer* cCellLookTableResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__look_table);
		}
		CellSerializer* cCellRandomCreatureResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__random_creature);
		}
		CellSerializer* cCellLookAlgorithmResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__look_algorithm);
		}
		CellSerializer* cCellBackgroundMapResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__backgroundMap);
		}
		CellSerializer* cCellEffectMapResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__effectMap);
		}
		CellSerializer* cCellPowersResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__powers);
		}
		CellSerializer* cCellGlobalsResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__globals);
		}
		CellSerializer* cCellStructureResource::Serializer() {
			return (CellSerializer*)GetAddress(Cell, sSerializer__structure);
		}

		cCellObjectData* GetPlayerCell() {
			return CellGame.mCells.Get(CellGame.mAvatarCellIndex);
		}

		bool cCellObjectData::IsCreature() {
			return mModelKey.instanceID != 0;
		}

		bool cCellObjectData::IsPlayer() {
			return mObjectPoolIndex == CellGame.mAvatarCellIndex;
		}

		const Math::Vector3& cCellObjectData::GetPosition() {
			return mTransform.GetOffset();
		}

		auto_STATIC_METHOD_(Cell, Cell::cCellGlobalsResource*, GetGlobalsData);

		auto_STATIC_METHOD(Cell, uint32_t, GetCurrentAdvectInfo,
			Args(CellStageScale& dst0, float& dstStrength, float& dstVariance, float& dstPeriod),
			Args(dst0, dstStrength, dstVariance, dstPeriod));

		auto_STATIC_METHOD_(Cell, uint32_t, GetNextAdvectID);

		auto_STATIC_METHOD(Cell, cObjectPoolIndex, CreateCellObject,
			Args(cCellQueryLinkedPool* a1, const Math::Vector3& a2, float a3, cCellDataReference<cCellCellResource>* a4, CellStageScale a5, float a6, float a7, bool a8, Math::Quaternion* a9, int a10),
			Args(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));

		auto_STATIC_METHOD_VOID(Cell, MovePlayerToMousePosition,
			Args(float a0), Args(a0));

		auto_STATIC_METHOD(Cell, ScaleDifference, GetScaleDifferenceWithPlayer,
			Args(cCellObjectData* a0), Args(a0));

		auto_STATIC_METHOD(Cell, bool, ShouldNotAttack,
			Args(cCellObjectData* a0, cCellObjectData* a1), Args(a0, a1));

		auto_STATIC_METHOD(Cell, int, GetDamageAmount,
			Args(cCellObjectData* a0, cCellObjectData* a1), Args(a0, a1));

		auto_STATIC_METHOD(Cell, int, FindCellsInRadius,
			Args(cCellQueryLinkedPool* a0, const Math::Vector3& a1, float a2, cObjectPoolIndex* a3, int a4, void* a5),
			Args(a0, a1, a2, a3, a4, a5));

		auto_STATIC_METHOD_VOID(Cell, PlayAnimation,
			Args(cCellObjectData* cell, cCellObjectData* otherCell, CellAnimations targetAnim, CellAnimations currentAnim),
			Args(cell, otherCell, targetAnim, currentAnim));

		ResourceKey GetModelKeyForCellResource(cCellDataReference<cCellCellResource>* cellResource)
		{
			ResourceKey result;
			STATIC_CALL(GetAddress(Cell, GetModelKeyForCellResource), void,
				Args(ResourceKey&, cCellDataReference<cCellCellResource>*), Args(result, cellResource));
			return result;
		}

		//// cCellGame ////

		cCellGame* cCellGame::Get()
		{
			return *(cCellGame**)GetAddress(cCellGame, _ptr);
		}

		auto_STATIC_METHOD_VOID_(cCellGame, Initialize);


		//// cCellGFX ////

		cCellGFX* cCellGFX::Get()
		{
			return *(cCellGFX**)GetAddress(cCellGFX, _ptr);
		}

		auto_STATIC_METHOD_VOID_(cCellGFX, PreloadResources);

		auto_STATIC_METHOD_VOID(cCellGFX, PreloadCellResource, 
			Args(cCellGFX::IsPreloadedMap_t a1, cCellDataReference<cCellCellResource>* a2),
			Args(a1, a2));

		auto_STATIC_METHOD_VOID(cCellGFX, PreloadPopulateResource,
			Args(cCellGFX::IsPreloadedMap_t a1, cCellDataReference<cCellPopulateResource>* a2),
			Args(a1, a2));

		auto_STATIC_METHOD_VOID(cCellGFX, PreloadLootTableResource,
			Args(cCellGFX::IsPreloadedMap_t a1, cCellDataReference<cCellLootTableResource>* a2),
			Args(a1, a2));

		auto_STATIC_METHOD_VOID(cCellGFX, PreloadCreature, Args(uint32_t a1), Args(a1));

		auto_STATIC_METHOD_VOID(cCellGFX, AddPreloadedEffect, Args(uint32_t a1, uint32_t a2), Args(a1, a2));

		auto_STATIC_METHOD_VOID(cCellGFX, AddPreloadedModel, Args(uint32_t a1, bool a2), Args(a1, a2));

		auto_METHOD_VOID(cCellGFX, AddPreloadedTexture, Args(Graphics::Texture* a1), Args(a1));

		auto_METHOD_VOID(cCellGFX, AddPreloadedModel2, Args(Graphics::Model* a1), Args(a1));

		auto_STATIC_METHOD_VOID_(cCellGFX, Initialize);
		auto_STATIC_METHOD_VOID_(cCellGFX, StartDisplay);

		auto_STATIC_METHOD_VOID(cCellGFX, CreateEffect,
			Args(Swarm::IEffectsWorld* a1, uint32_t a2, uint32_t a3, IVisualEffectPtr& a4),
			Args(a1, a2, a3, a4));

		auto_STATIC_METHOD_VOID_(cCellGFX, LoadEffectMap);

		auto_STATIC_METHOD(cCellGFX, Swarm::IVisualEffect* , InstanceEffectOnCell,
			Args(cCellObjectData* cell, uint32_t effectID),
			Args(cell, effectID));

		Math::BoundingBox& cCellGFX::GetVisibleBackgroundBBox() {
			return *(Math::BoundingBox*)GetAddress(cCellGFX, sVisibleBackgroundBBox_ptr);
		}

		Graphics::cFrustumCull& cCellGFX::GetFrustumCull() {
			return *(Graphics::cFrustumCull*)GetAddress(cCellGFX, sFrustumCull_ptr);
		}

		//// cCellUI ////

		cCellUI* cCellUI::Get()
		{
			return *(cCellUI**)GetAddress(cCellUI, _ptr);
		}

		auto_STATIC_METHOD_VOID_(cCellUI, Load);

		auto_STATIC_METHOD_VOID(cCellUI, ShowHealthRollover,
			Args(cCellObjectData* cell, int initialHealth), Args(cell, initialHealth));
	}
}

#endif
