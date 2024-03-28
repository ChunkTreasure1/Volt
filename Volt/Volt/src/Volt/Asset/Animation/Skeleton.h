#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/Animation/Animation.h"

#include <glm/glm.hpp>

namespace Volt
{
	class Skeleton : public Asset
	{
	public:
		struct Joint
		{
			std::string name;
			int32_t parentIndex = -1;

			static void Serialize(BinaryStreamWriter& streamWriter, const Joint& data)
			{
				streamWriter.Write(data.name);
				streamWriter.Write(data.parentIndex);
			}

			static void Deserialize(BinaryStreamReader& streamReader, Joint& outData)
			{
				streamReader.Read(outData.name);
				streamReader.Read(outData.parentIndex);
			}
		};

		struct JointAttachment
		{
			std::string name;
			int32_t jointIndex = -1;
			UUID id = 0;

			glm::vec3 positionOffset = 0.f;
			glm::quat rotationOffset = { 1.f, 0.f, 0.f, 0.f };

			inline const bool IsValid() const { return id != 0; }

			static void Serialize(BinaryStreamWriter& streamWriter, const JointAttachment& data)
			{
				streamWriter.Write(data.name);
				streamWriter.Write(data.jointIndex);
				streamWriter.Write(data.id);
				streamWriter.Write(data.positionOffset);
				streamWriter.Write(data.rotationOffset);
			}

			static void Deserialize(BinaryStreamReader& streamReader, JointAttachment& outData)
			{
				streamReader.Read(outData.name);
				streamReader.Read(outData.jointIndex);
				streamReader.Read(outData.id);
				streamReader.Read(outData.positionOffset);
				streamReader.Read(outData.rotationOffset);
			}
		};

		Skeleton() = default;
		~Skeleton() override
		{
			m_joints.clear();
			m_inverseBindPose.clear();
		}

		inline const size_t GetJointCount() const { return m_joints.size(); }
		inline const std::vector<glm::mat4>& GetInverseBindPose() const { return m_inverseBindPose; }
		inline const std::vector<Animation::TRS>& GetRestPose() const { return m_restPose; }
		inline const std::vector<Joint>& GetJoints() const { return m_joints; }
		inline const std::vector<JointAttachment>& GetJointAttachments() const { return m_jointAttachments; }

		const JointAttachment& GetJointAttachmentFromName(std::string_view name) const;
		const JointAttachment& GetJointAttachmentFromID(const UUID& id) const;
		bool HasJointAttachment(std::string_view name) const;

		const bool JointIsDecendantOf(int32_t jointIndex, int32_t parentIndex) const;

		const int32_t GetJointIndexFromName(const std::string& str);
		const std::string GetNameFromJointIndex(int32_t index);

		static AssetType GetStaticType() { return AssetType::Skeleton; }
		AssetType GetType() override { return GetStaticType(); };
		uint32_t GetVersion() const override { return 1; }

	private:
		friend class FbxImporter;
		friend class SkeletonImporter;
		friend class SkeletonSerializer;

		std::vector<Joint> m_joints;
		std::vector<JointAttachment> m_jointAttachments;
		std::vector<Animation::TRS> m_restPose;
		std::vector<glm::mat4> m_inverseBindPose;

		std::unordered_map<std::string, size_t> m_jointNameToIndex;

		std::string m_name = "Skeleton";
	};
}
