#include "Camera.h"

namespace RaytracingDX12
{
	Camera::Camera(RenderDeviceD3D12* device, UINT width, UINT height) :
		m_Device(device),
		m_ViewDirty(true)
	{
		m_NearValue = 1.0f;
		m_FarValue = 1000.0f;

		XMVECTOR pos = XMVectorSet(0.0f, 80.0f, -200.0f, 0.0f);
		XMVECTOR dir = XMVector3Normalize(XMVectorSet(0.0f, -0.3f, 1.0f, 0.0f));
		XMVECTOR up = XMVectorSet(0, 1, 0, 0);

		XMMATRIX V = XMMatrixLookToLH(
			pos,
			dir,
			up);
		XMStoreFloat4x4(&m_ViewMatrix, (V));

		SetProjectionMatrix(width, height);

		XMStoreFloat3(&m_Look, dir);
		XMStoreFloat3(&m_Up, up);
		XMStoreFloat3(&m_Right, XMVector3Cross(up, dir));
		XMStoreFloat3(&m_Position, pos);

		m_RotateAround = false;
	}

	XMMATRIX Camera::GetViewProjMatrix() const
	{
		return DirectX::XMLoadFloat4x4(&m_ViewMatrix) * DirectX::XMLoadFloat4x4(&m_ProjectionMatrix);
	}

	void Camera::SetProjectionMatrix(UINT newWidth, UINT newHeight)
	{
		m_ScreenWidth = newWidth;
		m_ScreenHeight = newHeight;

		SetProjectionMatrix();
	}

	void Camera::SetProjectionMatrix(float* fov, float* nearView, float* farView)
	{
		if (fov) m_FovY = *fov;
		if (nearView) m_NearValue = *nearView;
		if (farView) m_FarValue = *farView;

		m_FovY = std::max(m_FovY, FLT_MIN);
		m_NearValue = std::max(m_NearValue, FLT_MIN);
		m_FarValue = std::max(m_FarValue, m_NearValue + 0.1f);

		auto aspectRatio = ((float)m_ScreenWidth) / ((float)m_ScreenHeight);
		if (aspectRatio == 0)
			aspectRatio = FLT_MAX;

		auto nearWindowHeight = 2.0f * m_NearValue * tanf(0.5f * m_FovY);
		float halfWidth = 0.5f * aspectRatio * nearWindowHeight;
		m_FovX = 2.0f * atan(halfWidth / m_NearValue);

		XMMATRIX P = XMMatrixPerspectiveFovLH(
			m_FovY,
			aspectRatio,
			m_NearValue,
			m_FarValue
		);
		XMStoreFloat4x4(&m_ProjectionMatrix, (P));
	}

	void Camera::Pitch(float angle)
	{
		if (angle == 0)
			return;

		XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_Right), angle);

		XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
		XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

		m_ViewDirty = true;
	}

	void Camera::RotateY(float angle)
	{
		if (angle == 0)
			return;

		XMMATRIX R = XMMatrixRotationY(angle);

		XMStoreFloat3(&m_Right, XMVector3TransformNormal(XMLoadFloat3(&m_Right), R));
		XMStoreFloat3(&m_Up, XMVector3TransformNormal(XMLoadFloat3(&m_Up), R));
		XMStoreFloat3(&m_Look, XMVector3TransformNormal(XMLoadFloat3(&m_Look), R));

		m_ViewDirty = true;
	}

	void Camera::Move(XMVECTOR deltaPos)
	{
		XMVECTOR pos = XMLoadFloat3(&m_Position);
		pos += deltaPos;
		XMStoreFloat3(&m_Position, pos);

		m_ViewDirty = true;
	}

	void Camera::Update(const Timer& timer)
	{
		if (m_RotateAround)
		{
			XMFLOAT3 target = { 0, 0, 0 };
			float radius = sqrtf(m_Position.x * m_Position.x + m_Position.y * m_Position.y + m_Position.z * m_Position.z);

			float theta = atan2f(m_Position.x, m_Position.z) + timer.GetDeltaTime();
			float phi = asinf(m_Position.y / radius);

			float x = target.x + radius * cosf(phi) * sinf(theta);
			float y = target.y + radius * sinf(phi);
			float z = target.z + radius * cosf(phi) * cosf(theta);

			m_Position = XMFLOAT3(x, y, z);

			XMVECTOR posVec = XMLoadFloat3(&m_Position);
			XMVECTOR targetVec = XMLoadFloat3(&target);
			XMVECTOR lookVec = XMVector3Normalize(XMVectorSubtract(targetVec, posVec));

			XMStoreFloat3(&m_Look, lookVec);

			XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(worldUp, lookVec));
			XMStoreFloat3(&m_Right, rightVec);

			XMVECTOR upVec = XMVector3Normalize(XMVector3Cross(lookVec, rightVec));
			XMStoreFloat3(&m_Up, upVec);

			m_ViewDirty = true;
		}

		if (m_ViewDirty)
		{
			ConstructViewMatrix(m_ViewMatrix, m_Right, m_Up, m_Look, m_Position);
			m_ViewDirty = false;
		}
	}

	void Camera::Setup(XMFLOAT3 pos, XMFLOAT3 look, XMFLOAT3 right, XMFLOAT3 up)
	{
		m_Position = pos;
		m_Look = look;
		m_Right = right;
		m_Up = up;
		m_ViewDirty = true;
	}

	void Camera::SetRotateAroundMode(bool enable)
	{
		m_RotateAround = enable;
	}

	void Camera::ConstructViewMatrix(XMFLOAT4X4& view, XMFLOAT3& right, XMFLOAT3& up, XMFLOAT3& look, XMFLOAT3& pos) const
	{
		XMVECTOR R = XMLoadFloat3(&right);
		XMVECTOR U = XMLoadFloat3(&up);
		XMVECTOR L = XMLoadFloat3(&look);
		XMVECTOR P = XMLoadFloat3(&pos);

		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		R = XMVector3Cross(U, L);

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&right, R);
		XMStoreFloat3(&up, U);
		XMStoreFloat3(&look, L);

		view(0, 0) = right.x;
		view(1, 0) = right.y;
		view(2, 0) = right.z;
		view(3, 0) = x;

		view(0, 1) = up.x;
		view(1, 1) = up.y;
		view(2, 1) = up.z;
		view(3, 1) = y;

		view(0, 2) = look.x;
		view(1, 2) = look.y;
		view(2, 2) = look.z;
		view(3, 2) = z;

		view(0, 3) = 0.0f;
		view(1, 3) = 0.0f;
		view(2, 3) = 0.0f;
		view(3, 3) = 1.0f;
	}
}