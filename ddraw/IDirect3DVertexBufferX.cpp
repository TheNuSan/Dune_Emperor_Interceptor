/**
* Copyright (C) 2020 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "ddraw.h"
#include <map>
#include <sstream>
#include <iomanip>

HRESULT m_IDirect3DVertexBufferX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DVertexBufferX::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DVertexBufferX::Release()
{
	ULONG ref = ProxyInterface->Release();

	if (ref == 0)
	{
		WrapperInterface->DeleteMe();
	}

	return ref;
}

LPVOID VertexBuffer_Lock_Data = nullptr;
DWORD VertexBuffer_Lock_Size = 0;
HRESULT m_IDirect3DVertexBufferX::Lock(DWORD dwFlags, LPVOID * lplpData, LPDWORD lpdwSize)
{
	HRESULT res = ProxyInterface->Lock(dwFlags, lplpData, lpdwSize);
	
	LPVOID data = lplpData ? *lplpData : nullptr;
	DWORD Size = lpdwSize ? *lpdwSize : 0;
	
	Log() << "Lock 3D Vertex Buffer " << this->ProxyInterface << " " << data << " " << Size;

	D3DVERTEXBUFFERDESC vertDesc;
	GetVertexBufferDesc(&vertDesc);
	Log() << "Vertex Buffer Desc " << std::hex << vertDesc.dwFVF << " " << vertDesc.dwNumVertices << " " << vertDesc.dwSize << " " << vertDesc.dwCaps;

	VertexBuffer_Lock_Data = data;
	VertexBuffer_Lock_Size = Size;

	return res;
}


std::map<LPVOID, LPDWORD> LockedBuffers;

struct VertexNormalTex1
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;

	static const DWORD format = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
};

struct VertexDiffuseTex2
{
	float x, y, z;
	D3DCOLOR diffuse;
	float u, v;
	float u2, v2;

	static const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2;
};

struct VertexDiffuseTex3
{
	float x, y, z;
	float rhw;
	D3DCOLOR diffuse;
	D3DCOLOR specular;
	float u, v;

	static const DWORD format = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1;
};


HRESULT m_IDirect3DVertexBufferX::Unlock()
{
	Log() << "Unlock 3D Vertex Buffer " << this->ProxyInterface << " " << VertexBuffer_Lock_Data << " " << VertexBuffer_Lock_Size;

	extern bool BackingMeshes;
	if (BackingMeshes) {
		Log() << "Bake Unlocked VB " << VertexBuffer_Lock_Data;
		D3DVERTEXBUFFERDESC vertDesc;
		GetVertexBufferDesc(&vertDesc);
		Log() << "Vertex Buffer Desc " << std::hex << vertDesc.dwFVF << " " << vertDesc.dwNumVertices << " " << vertDesc.dwSize << " " << vertDesc.dwCaps;
		if (vertDesc.dwFVF == VertexNormalTex1::format) {
			Log() << "Found VertexNormalTex1";
			// only store the first n frames of the mesh
			int maxFrameCapture = 40;
			bool savedFrame = false;
			for (int frame = 0; frame < maxFrameCapture; ++frame)
			{
				std::stringstream ss;
				ss << "DumpMeshes/" << VertexBuffer_Lock_Data << "_" << std::setfill('0') << std::setw(4) << frame << ".txt";
				if (FILE* fileTest = fopen(ss.str().c_str(), "r")) {
					fclose(fileTest);
				} else {
					Log() << "Dump vertex buffer frame " << frame << VertexBuffer_Lock_Data << " file " << ss.str();
					std::ofstream file(ss.str());
					VertexNormalTex1* Vertices = (VertexNormalTex1*)VertexBuffer_Lock_Data;
					for (int i = 0; i < vertDesc.dwNumVertices; ++i) {
						file << Vertices->x << "," << Vertices->y << "," << Vertices->z << ", " << Vertices->nx << "," << Vertices->ny << "," << Vertices->nz << ", " << Vertices->u << "," << Vertices->v << "\n";
						Vertices++;
					}
					savedFrame = true;
					break;
				}
			}
			if (!savedFrame) {
				Log() << "Max frame reached";
			}
		} else {
			Log() << "Unknown Vertex Format " << std::hex << vertDesc.dwFVF;
		}
	}

	HRESULT res = ProxyInterface->Unlock();

	return res;
}

HRESULT m_IDirect3DVertexBufferX::ProcessVertices(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPDIRECT3DVERTEXBUFFER7 lpSrcBuffer, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	if (lpSrcBuffer)
	{
		lpSrcBuffer = static_cast<m_IDirect3DVertexBuffer7 *>(lpSrcBuffer)->GetProxyInterface();
	}
	if (lpD3DDevice)
	{
		lpD3DDevice = static_cast<m_IDirect3DDevice7 *>(lpD3DDevice)->GetProxyInterface();
	}

	return ProxyInterface->ProcessVertices(dwVertexOp, dwDestIndex, dwCount, lpSrcBuffer, dwSrcIndex, lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC lpVBDesc)
{
	return ProxyInterface->GetVertexBufferDesc(lpVBDesc);
}

HRESULT m_IDirect3DVertexBufferX::Optimize(LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	if (lpD3DDevice)
	{
		lpD3DDevice = static_cast<m_IDirect3DDevice7 *>(lpD3DDevice)->GetProxyInterface();
	}

	return ProxyInterface->Optimize(lpD3DDevice, dwFlags);
}

HRESULT m_IDirect3DVertexBufferX::ProcessVerticesStrided(DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, LPD3DDRAWPRIMITIVESTRIDEDDATA lpVertexArray, DWORD dwSrcIndex, LPDIRECT3DDEVICE7 lpD3DDevice, DWORD dwFlags)
{
	if (lpD3DDevice)
	{
		lpD3DDevice = static_cast<m_IDirect3DDevice7 *>(lpD3DDevice)->GetProxyInterface();
	}

	return ProxyInterface->ProcessVerticesStrided(dwVertexOp, dwDestIndex, dwCount, lpVertexArray, dwSrcIndex, lpD3DDevice, dwFlags);
}
