#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <chrono>
#include <cstdint>

namespace Flame
{

	// Enum for camera movements
	enum class CameraMovement : uint32_t {
		Forward = 1 << 0,
		Backward = 1 << 1,
		Left = 1 << 2,
		Right = 1 << 3,
		Up = 1 << 4,
		Down = 1 << 5
	};

	// Custom CameraMovementFlags class to handle bitwise operations
	class CameraMovementFlags {
	public:
		CameraMovementFlags() : mFlags(0) {}
		CameraMovementFlags(CameraMovement movement) : mFlags(static_cast<uint32_t>(movement)) {}

		void SetFlag(CameraMovement movement) {
			mFlags |= static_cast<uint32_t>(movement);
		}

		void ClearFlag(CameraMovement movement) {
			mFlags &= ~static_cast<uint32_t>(movement);
		}

		bool HasFlag(CameraMovement movement) const {
			return (mFlags & static_cast<uint32_t>(movement)) != 0;
		}

		void ClearAll() {
			mFlags = 0;
		}

	private:
		uint32_t mFlags;
	};

	// Struct for camera specifications
	struct EditorCameraSpecs {
		float Fov = glm::radians(45.0f);    // Field of view
		float NearClip = 0.1f;               // Near clip plane
		float FarClip = 100.0f;              // Far clip plane
		float AspectRatio = 16.0f / 9.0f;    // Aspect ratio
	};

	// EditorCamera class
	class EditorCamera {
	public:
		using NanoSeconds = std::chrono::nanoseconds;

		EditorCamera()
		{
			UpdateProjection();
			UpdateView();
		}

		// Set camera specifications
		void SetCameraSpec(const EditorCameraSpecs& specs)
		{
			mCameraSpecs = specs;
			UpdateProjection();
		}

		// Set position directly (updates view matrix)
		void SetPosition(const glm::vec3& position)
		{
			mPosition = position;
			UpdateView();
		}

		// Set orientation directly (takes orientation matrix and calculates yaw, pitch, roll)
		void SetOrientation(const glm::mat3& orientation)
		{
			mForwardDirection = glm::normalize(orientation[2]);  // Z-axis (forward)
			mRightDirection = glm::normalize(orientation[0]);    // X-axis (right)
			mUpDirection = glm::normalize(orientation[1]);       // Y-axis (up)

			// Manually calculate pitch, yaw, and roll from the orientation matrix
			mYaw = glm::degrees(atan2(orientation[2][0], orientation[2][2]));
			mPitch = glm::degrees(asin(-orientation[2][1]));
			mRoll = glm::degrees(atan2(orientation[0][1], orientation[1][1]));

			UpdateView();
		}

		// Updated OnUpdate method
		void OnUpdate(NanoSeconds deltaTime, const CameraMovementFlags& movementFlags,
			const glm::vec2& mousePos, bool rotate = true)
		{
			// Convert nanoseconds to seconds
			float dt = static_cast<float>(deltaTime.count()) / NanoSecondsTicksInOneSecond();

			glm::vec3 movement(0.0f);
			float speed = mMoveSpeed * dt;

			if (movementFlags.HasFlag(CameraMovement::Forward)) movement += mForwardDirection * speed;
			if (movementFlags.HasFlag(CameraMovement::Backward)) movement -= mForwardDirection * speed;
			if (movementFlags.HasFlag(CameraMovement::Left)) movement -= mRightDirection * speed;
			if (movementFlags.HasFlag(CameraMovement::Right)) movement += mRightDirection * speed;
			if (movementFlags.HasFlag(CameraMovement::Up)) movement += mUpDirection * speed;
			if (movementFlags.HasFlag(CameraMovement::Down)) movement -= mUpDirection * speed;

			mPosition += movement;

			if (rotate) {
				float dx = mousePos.x - mLastMouseX;
				float dy = mousePos.y - mLastMouseY;

				mYaw += dx * mRotationSpeed;
				mPitch -= dy * mRotationSpeed;

				if (mPitch > 89.0f) mPitch = 89.0f;
				if (mPitch < -89.0f) mPitch = -89.0f;

				mLastMouseX = mousePos.x;
				mLastMouseY = mousePos.y;

				UpdateView();
			}
		}

		// Get separate view and projection matrices
		const glm::mat4& GetViewMatrix() const { return mView; }
		const glm::mat4& GetProjectionMatrix() const { return mProjection; }

		// Set linear and rotation speeds
		void SetLinearSpeed(float speed) { mMoveSpeed = speed; }
		void SetRotationSpeed(float speed) { mRotationSpeed = speed; }

	private:
		// Camera specifications
		EditorCameraSpecs mCameraSpecs;

		// Camera properties
		glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 5.0f);
		glm::vec3 mForwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 mRightDirection = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 mUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);

		float mYaw = -90.0f;
		float mPitch = 0.0f;
		float mRoll = 0.0f;

		float mMoveSpeed = 5.0f;
		float mRotationSpeed = 0.1f;
		float mLastMouseX = 0.0f;
		float mLastMouseY = 0.0f;

		glm::mat4 mView = glm::mat4(1.0f);
		glm::mat4 mProjection = glm::mat4(1.0f);

	private:

		// Constant for converting nanoseconds to seconds
		static constexpr float NanoSecondsTicksInOneSecond()
		{
			// 1 second = 10^9 nanoseconds
			return 1e9f;
		}

		// Updates the projection matrix based on current camera specs
		void UpdateProjection()
		{
			mProjection = glm::perspective(glm::radians(mCameraSpecs.Fov),
				mCameraSpecs.AspectRatio, mCameraSpecs.NearClip, mCameraSpecs.FarClip);
		}

		// Updates the view matrix based on position, yaw, and pitch
		void UpdateView()
		{
			mForwardDirection = glm::normalize(glm::vec3(
				cos(glm::radians(mYaw)) * cos(glm::radians(mPitch)),
				sin(glm::radians(mPitch)),
				sin(glm::radians(mYaw)) * cos(glm::radians(mPitch))
			));

			mRightDirection = glm::normalize(glm::cross(mForwardDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
			mUpDirection = glm::normalize(glm::cross(mRightDirection, mForwardDirection));

			mView = glm::lookAt(mPosition, mPosition + mForwardDirection, mUpDirection);
		}
	};
}
