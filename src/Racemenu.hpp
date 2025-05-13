#pragma once
#include "IPluginInterface.hpp"

namespace OverlaySaver {

	enum class OverlayLayers : uint16_t {

		kShaderEmissiveColor = 0,
		kShaderEmissiveMultiple = 1,
		kShaderGlossiness = 2,
		kShaderSpecularStrength = 3,
		kShaderLightingEffect1 = 4,
		kShaderLightingEffect2 = 5,
		kShaderTextureSet = 6,
		kShaderTintColor = 7,
		kShaderAlpha = 8,
		kShaderTexture = 9,

		kControllerStartStop = 20,
		kControllerStartTime = 21,
		kControllerStopTime = 22,
		kControllerFrequency = 23,
		kControllerPhase = 24

	};

	struct RMOverlay {
		float Alpha = 1.0f;
		uint32_t Color = 0xffffff;
		uint16_t TexStrLen = 0;
		std::string TexturePath;
	};

	class Racemenu final {

		public:
		static void Register();

		[[nodiscard]] static inline bool Loaded() {
			return OverlayInterface != nullptr && OverrideInterface != nullptr;
		}

		private:

		static inline SKEE::IOverlayInterface* OverlayInterface = nullptr;
		static inline SKEE::IOverrideInterface* OverrideInterface = nullptr;

		protected:

		static inline int iHead = 0;
		static inline int iHand = 0;
		static inline int iBody = 0;
		static inline int iFeet = 0;

		static constexpr const char* sHead = "[Face Ovl{}]";
		static constexpr const char* sHand = "[Hands Ovl{}]";
		static constexpr const char* sBody = "[Body Ovl{}]";
		static constexpr const char* sFeet = "[Feet Ovl{}]";

		public:

		class OverlayManager {

			public:

			[[nodiscard]] static OverlayManager& GetSingleton() {
				OverlayManager Instance;
				return Instance;
			}

			class Variant : public SKEE::IOverrideInterface::SetVariant, public SKEE::IOverrideInterface::GetVariant {
				public:

				Variant() : _type(Type::None), _u() {}
				Variant(int32_t a_i) : _type(Type::Int), _u{ a_i } {}
				Variant(float a_f) : _type(Type::Float), _u{ ._flt = a_f } {}
				Variant(const char* a_str) : _type(Type::String), _u{ ._str = a_str } {}
				Variant(bool a_b) : _type(Type::Bool), _u{ ._bool = a_b } {}
				Variant(const RE::BGSTextureSet* a_texset) : _type(Type::TextureSet), _u{ ._texset = a_texset } {}

				virtual Type GetType() override {
					return _type;
				}
				virtual int32_t Int() override {
					return _u._int;
				}
				virtual float Float() override {
					return _u._flt;
				}
				virtual const char* String() override {
					return _u._str;
				}
				virtual bool Bool() override {
					return _u._bool;
				}
				virtual RE::BGSTextureSet* TextureSet() override {
					return const_cast<RE::BGSTextureSet*>(_u._texset);
				}

				virtual void Int(const int32_t i) override {
					_type = Type::Int;
					_u._int = i;
				}
				virtual void Float(const float f) override {
					_type = Type::Float;
					_u._flt = f;
				}
				virtual void String(const char* str) override {
					_type = Type::String;
					_u._str = str;
				}
				virtual void Bool(const bool b) override {
					_type = Type::Bool;
					_u._bool = b;
				}
				virtual void TextureSet(const RE::BGSTextureSet* textureSet) override {
					_type = Type::TextureSet;
					_u._texset = textureSet;
				}

				private:
				Type _type;
				union _u_t {
					int32_t                  _int;
					float                    _flt;
					bool                     _bool;
					const char* _str;
					const RE::BGSTextureSet* _texset;
				} _u;
			};

			static float GetNodeOverrideFloat(RE::TESObjectREFR* a_ref, bool a_female, const char* a_node, int a_key, int a_index) {
				Variant result;
				if (OverrideInterface->GetNodeOverride(a_ref, a_female, a_node, static_cast<uint16_t>(a_key), static_cast<uint8_t>(a_index), result))
					return result.Float();
				else
					return 0.f;
			}

			static int GetNodeOverrideInt(RE::TESObjectREFR* a_ref, bool a_female, const char* a_node, int a_key, int a_index) {
				Variant result;
				if (OverrideInterface->GetNodeOverride(a_ref, a_female, a_node, static_cast<uint16_t>(a_key), static_cast<uint8_t>(a_index), result))
					return result.Int();
				else
					return 0;
			}

			static RE::BSFixedString GetNodeOverrideString(RE::TESObjectREFR* a_ref, bool a_female, const char* a_node, int a_key, int a_index) {
				Variant result;
				if (OverrideInterface->GetNodeOverride(a_ref, a_female, a_node, static_cast<uint16_t>(a_key), static_cast<uint8_t>(a_index), result))
					return result.String();
				else
					return "";
			}

		public:
			static void FlipStoredOverlaysAndReapply(RE::Actor* a_Actor);
			static void ClearBodyPart(RE::Actor* a_Actor, int a_numOfSlots, const std::string& a_OvlName);
			static void ClearOverlays(RE::Actor* a_Actor);
			static void GetNumOfSlotsFromRM();
			static void BuildOvlListForSlot(RE::Actor* a_Actor, int a_NumOfSlots, const std::string& a_OvlName, std::vector<RMOverlay>* a_OvlVec);
			static void BuildOverlayList(RE::Actor* a_Actor);
			static void ApplyStoredOverlayOnBodyPart(RE::Actor* a_Actor, int a_NumOfSlots, const std::string& a_OvlName, std::vector<RMOverlay>* a_OvlVec);
			static void ApplyOverlayFromList(RE::Actor* a_Actor);
		};

	};


	
}
