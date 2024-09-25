#pragma once

#include <CoreUtilities/Math/Hash.h>

#include <glm/glm.hpp>
#include <fbxsdk.h>

namespace Volt
{
	namespace FbxUtility
	{
		glm::vec3 ToVec3(const FbxVector4& v)
		{
			return { v[0], v[1], v[2] };
		}

		glm::vec2 ToVec2(const FbxVector2& v)
		{
			return { v[0], v[1] };
		}

		glm::mat4 ToMatrix(const FbxAMatrix& aMatrix)
		{
			glm::mat4 result;

			for (int i = 0; i < 4; i++)
			{
				// The FBX Matrix might need a Transpose!
				fbxsdk::FbxVector4 row = aMatrix.GetRow(i);
				result[i][0] = static_cast<float>(row[0]);
				result[i][1] = static_cast<float>(row[1]);
				result[i][2] = static_cast<float>(row[2]);
				result[i][3] = static_cast<float>(row[3]);
			}

			return result;
		}

		glm::quat ToQuat(const fbxsdk::FbxQuaternion& aQuat)
		{
			glm::quat result;

			result.x = static_cast<float>(aQuat[0]);
			result.y = static_cast<float>(aQuat[1]);
			result.z = static_cast<float>(aQuat[2]);
			result.w = static_cast<float>(aQuat[3]);

			return result;
		}

		uint32_t GetFramesPerSecond(FbxTime::EMode aMode)
		{
			switch (aMode)
			{
				case FbxTime::eDefaultMode: return 0;
				case FbxTime::eFrames120: return 120;
				case FbxTime::eFrames100: return 100;
				case FbxTime::eFrames60: return 60;
				case FbxTime::eFrames50: return 50;
				case FbxTime::eFrames48: return 48;
				case FbxTime::eFrames30: return 30;
				case FbxTime::eFrames30Drop: return 30;
				case FbxTime::eNTSCDropFrame: return 30;
				case FbxTime::eNTSCFullFrame: return 30;
				case FbxTime::ePAL: return 24;
				case FbxTime::eFrames24: return 24;
				case FbxTime::eFrames1000: return 1000;
				case FbxTime::eFilmFullFrame: return 0;
				case FbxTime::eCustom: return 0;
				case FbxTime::eFrames96: return 96;
				case FbxTime::eFrames72: return 72;
				case FbxTime::eFrames59dot94: return 60;
				case FbxTime::eFrames119dot88: return 120;
			}

			return 0;
		}
	}

	struct FbxSDKDeleter
	{
		template<typename T>
		void operator()(T* object) const
		{
			object->Destroy();
		}
	};

	struct FbxVertex
	{
		int32_t material;
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 texCoords;

		glm::uvec4 influences;
		glm::vec4 weights;

		inline size_t GetHash() const
		{
			size_t result = 0;

			result = std::hash<int32_t>()(material);
			result = Math::HashCombine(result, std::hash<float>()(position.x));
			result = Math::HashCombine(result, std::hash<float>()(position.y));
			result = Math::HashCombine(result, std::hash<float>()(position.z));

			result = Math::HashCombine(result, std::hash<float>()(normal.x));
			result = Math::HashCombine(result, std::hash<float>()(normal.y));
			result = Math::HashCombine(result, std::hash<float>()(normal.z));

			result = Math::HashCombine(result, std::hash<float>()(tangent.x));
			result = Math::HashCombine(result, std::hash<float>()(tangent.y));
			result = Math::HashCombine(result, std::hash<float>()(tangent.z));

			result = Math::HashCombine(result, std::hash<float>()(texCoords.x));
			result = Math::HashCombine(result, std::hash<float>()(texCoords.y));

			result = Math::HashCombine(result, std::hash<uint32_t>()(influences.x));
			result = Math::HashCombine(result, std::hash<uint32_t>()(influences.y));
			result = Math::HashCombine(result, std::hash<uint32_t>()(influences.z));
			result = Math::HashCombine(result, std::hash<uint32_t>()(influences.w));

			result = Math::HashCombine(result, std::hash<float>()(weights.x));
			result = Math::HashCombine(result, std::hash<float>()(weights.y));
			result = Math::HashCombine(result, std::hash<float>()(weights.z));
			result = Math::HashCombine(result, std::hash<float>()(weights.w));

			return result;
		}
	};

	struct FbxRestPose
	{
		glm::vec3 translation;
		glm::vec3 scale;
		glm::quat rotation;
	};

	struct FbxJoint
	{
		glm::mat4 inverseBindPose = glm::identity<glm::mat4>();
		FbxRestPose restPose;

		std::string name;
		std::string namespaceName;
		int32_t parentIndex;
	};

	struct FbxSkeletonContainer
	{
		Vector<FbxJoint> joints;

		VT_INLINE int32_t GetJointIndexFromName(const std::string& name) const
		{
			for (size_t i = 0; i < joints.size(); i++)
			{
				if (joints[i].name == name)
				{
					return static_cast<int32_t>(i);
				}
			}

			return -1;
		}
	};

	namespace ElementType
	{
		constexpr uint32_t Material = 0;
		constexpr uint32_t Position = 1;
		constexpr uint32_t Normal = 2;
		constexpr uint32_t UV = 3;
		constexpr uint32_t Tangent = 4;
		constexpr uint32_t AnimationData = 5;
		constexpr uint32_t Count = 6;
	}

	struct FatIndex
	{
		int32_t elements[ElementType::Count];

		FatIndex()
		{
			for (int i = 0; i < ElementType::Count; i++)
			{
				elements[i] = -1;
			}
		}
	};

	template<typename Functor>
	inline void VisitNodes(FbxScene* scene, Functor& functor)
	{
		auto visitor = [&](FbxNode* parent, FbxNode* node, auto visitor) -> bool
		{
			if (node)
			{
				const int childCount = node->GetChildCount();
				functor(parent, node);

				for (int i = 0; i < childCount; i++)
				{
					visitor(node, node->GetChild(i), visitor);
				}
			}

			return false;
		};

		visitor(nullptr, scene->GetRootNode(), visitor);
	}

	template<typename Attribute, typename Functor>
	inline void VisitNodeAttributesOfType(FbxScene* scene, Functor& functor, bool primaryAttribsOnly = false)
	{
		auto selector = [&](FbxNodeAttribute* attrib, FbxNode* node)
		{
			if (attrib && attrib->Is<Attribute>())
			{
				functor(static_cast<Attribute*>(attrib), node);
			}

			return true;
		};

		auto visitor = [&](FbxNode* parent, FbxNode* node)
		{
			if (primaryAttribsOnly)
			{
				selector(node->GetNodeAttribute(), node);
			}
			else
			{
				const int attribCount = node->GetNodeAttributeCount();
				for (int i = 0; i < attribCount; i++)
				{
					selector(node->GetNodeAttributeByIndex(i), node);
				}
			}

			return true;
		};

		VisitNodes(scene, visitor);
	}

	// From: https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	inline bool AlmostEqual(float lhs, float rhs)
	{
		const float diff = fabs(lhs - rhs);
		lhs = fabs(lhs);
		rhs = fabs(rhs);

		float largest = (rhs > lhs) ? rhs : lhs;
		return (diff <= largest * FLT_EPSILON);
	}

	inline bool AlmostEqual(const glm::vec3& lhs, const glm::vec3& rhs)
	{
		return
			AlmostEqual(lhs.x, rhs.x) &&
			AlmostEqual(lhs.y, rhs.y) &&
			AlmostEqual(lhs.z, rhs.z);
	}

	inline bool AlmostEqual(const glm::vec4& lhs, const glm::vec4& rhs)
	{
		return
			AlmostEqual(lhs.x, rhs.x) &&
			AlmostEqual(lhs.y, rhs.y) &&
			AlmostEqual(lhs.z, rhs.z) &&
			AlmostEqual(lhs.w, rhs.w);
	}

	inline bool AlmostEqual(const glm::vec2& lhs, const glm::vec2& rhs)
	{
		return
			AlmostEqual(lhs.x, rhs.x) &&
			AlmostEqual(lhs.y, rhs.y);
	}

	inline bool AlmostEqual(const FbxVertex& lhs, const FbxVertex& rhs)
	{
		return
			lhs.material == rhs.material &&
			lhs.influences == rhs.influences &&
			AlmostEqual(lhs.weights, rhs.weights) &&
			AlmostEqual(lhs.position, rhs.position) &&
			AlmostEqual(lhs.normal, rhs.normal) &&
			AlmostEqual(lhs.tangent, rhs.tangent) &&
			AlmostEqual(lhs.texCoords, rhs.texCoords);
	}

	inline std::string GetJointName(const std::string& name)
	{
		if (const size_t pos = name.find_last_of(':'); pos != std::string::npos)
		{
			return name.substr(pos + 1);
		}
		return name;
	}

	inline std::string GetFbxNodePath(FbxNode* node)
	{
		VT_PROFILE_FUNCTION();

		constexpr std::string_view rootName = "<root>";

		std::string result;
		if (node != nullptr && node->GetParent() != nullptr)
		{
			result = rootName;
		}

		while (node != nullptr && node->GetParent() != nullptr)
		{
			std::string nodeDesc;

			const char* const nodeName = node->GetName();
			if (nodeName && nodeName[0])
			{
				nodeDesc = nodeName;
			}
			else
			{
				int nodeIndex = -1;
				const FbxNode* const parentNode = node->GetParent();
				if (parentNode)
				{
					const int childCount = parentNode->GetChildCount();
					for (int i = 0; i < childCount; i++)
					{
						if (parentNode->GetChild(i) == node)
						{
							nodeIndex = i;
							break;
						}
					}
				}

				if (nodeIndex >= 0)
				{
					nodeDesc = std::format("<node at index {}>", nodeIndex);
				}
				else
				{
					nodeDesc = "<anonymous node>";
				}
			}

			if (result.empty())
			{
				result.swap(nodeDesc);
			}
			else
			{
				result = nodeDesc + "/" + result;
			}

			node = node->GetParent();
		}

		if (result.empty())
		{
			result = "<null>";
		}

		return result;
	}

	inline void SetupIOSettings(FbxIOSettings& ioSettings, const MeshSourceImportConfig& importConfig, const std::filesystem::path& filepath)
	{
		if (!importConfig.password.empty())
		{
			ioSettings.SetStringProp(IMP_FBX_PASSWORD, importConfig.password.c_str());
			ioSettings.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
		}

		// General
		ioSettings.SetBoolProp(IMP_PRESETS, true);
		ioSettings.SetBoolProp(IMP_STATISTICS, true);
		ioSettings.SetBoolProp(IMP_GEOMETRY, true);
		ioSettings.SetBoolProp(IMP_ANIMATION, true);
		ioSettings.SetBoolProp(IMP_LIGHT, false);
		ioSettings.SetBoolProp(IMP_ENVIRONMENT, false);
		ioSettings.SetBoolProp(IMP_CAMERA, false);
		ioSettings.SetBoolProp(IMP_VIEW_CUBE, false);
		ioSettings.SetBoolProp(IMP_ZOOMEXTENTS, false);
		ioSettings.SetBoolProp(IMP_GLOBAL_AMBIENT_COLOR, false);
		ioSettings.SetBoolProp(IMP_META_DATA, false);
		ioSettings.SetBoolProp(IMP_REMOVEBADPOLYSFROMMESH, false);

		// FBX
		ioSettings.SetBoolProp(IMP_FBX_TEMPLATE, true);
		ioSettings.SetBoolProp(IMP_FBX_PIVOT, true);
		ioSettings.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
		ioSettings.SetBoolProp(IMP_FBX_CHARACTER, false);
		ioSettings.SetBoolProp(IMP_FBX_CONSTRAINT, false);
		ioSettings.SetBoolProp(IMP_FBX_MERGE_LAYER_AND_TIMEWARP, false);
		ioSettings.SetBoolProp(IMP_FBX_GOBO, false);
		ioSettings.SetBoolProp(IMP_FBX_SHAPE, false);
		ioSettings.SetBoolProp(IMP_FBX_LINK, false);
		ioSettings.SetBoolProp(IMP_FBX_MATERIAL, true);
		ioSettings.SetBoolProp(IMP_FBX_TEXTURE, true);
		ioSettings.SetBoolProp(IMP_FBX_MODEL, true);
		ioSettings.SetBoolProp(IMP_FBX_ANIMATION, true);
		ioSettings.SetBoolProp(IMP_FBX_EXTRACT_EMBEDDED_DATA, true);

		ioSettings.SetStringProp(IMP_EXTRACT_FOLDER, filepath.parent_path().string().c_str());

		// 3DS
		ioSettings.SetBoolProp(IMP_3DS_REFERENCENODE, false);
		ioSettings.SetBoolProp(IMP_3DS_TEXTURE, true);
		ioSettings.SetBoolProp(IMP_3DS_MATERIAL, true);
		ioSettings.SetBoolProp(IMP_3DS_ANIMATION, false);
		ioSettings.SetBoolProp(IMP_3DS_MESH, true);
		ioSettings.SetBoolProp(IMP_3DS_LIGHT, false);
		ioSettings.SetBoolProp(IMP_3DS_CAMERA, false);
		ioSettings.SetBoolProp(IMP_3DS_AMBIENT_LIGHT, false);
		ioSettings.SetBoolProp(IMP_3DS_RESCALING, true);
		ioSettings.SetBoolProp(IMP_3DS_FILTER, false);
		ioSettings.SetBoolProp(IMP_3DS_SMOOTHGROUP, false);

		// OBJ
		ioSettings.SetBoolProp(IMP_OBJ_REFERENCE_NODE, false);

		// DXF
		ioSettings.SetBoolProp(IMP_DXF_WELD_VERTICES, false);
		ioSettings.SetBoolProp(IMP_DXF_OBJECT_DERIVATION, false);
		ioSettings.SetBoolProp(IMP_DXF_REFERENCE_NODE, false);
	}
}
