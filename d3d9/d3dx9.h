#pragma once

#include <d3d9.h>

#define D3DX_FILTER_NONE 1

struct D3DXMACRO
{
	LPCSTR Name;
	LPCSTR Definition;
};

typedef interface ID3DXBuffer *LPD3DXBUFFER;
typedef interface ID3DXInclude *LPD3DXINCLUDE;

DECLARE_INTERFACE_(ID3DXBuffer, IUnknown)
{
	// IUnknown
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	// ID3DXBuffer
	STDMETHOD_(LPVOID, GetBufferPointer)(THIS) PURE;
	STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;
};

void UnLoadd3dx9();

HRESULT D3DXAssembleShader(LPCSTR, UINT, const D3DXMACRO *, LPD3DXINCLUDE, DWORD, LPD3DXBUFFER *, LPD3DXBUFFER *);
HRESULT D3DXDisassembleShader(const DWORD *, BOOL, LPCSTR, LPD3DXBUFFER *);
HRESULT D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9, const PALETTEENTRY *, const RECT *, LPDIRECT3DSURFACE9, const PALETTEENTRY *, const RECT *, DWORD, D3DCOLOR);
