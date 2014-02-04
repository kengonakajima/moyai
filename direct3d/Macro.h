#pragma once

#define moyai_align(size) __declspec(align(size))
#define aligned_size(size, alignment) ((size + (alignment - 1)) & (~(alignment - 1)))
#define member_size(type, member) sizeof(((type*)0)->member)

#define SafeRelease(resource) \
	if (resource) { resource->Release(); resource = nullptr; }

#define SafeDelete(resource) \
	delete resource; resource = nullptr;

#define CheckFailure(hr, msg, ...)                  \
	if (FAILED(hr))                                 \
	{                                               \
		char errmsg[1024];							\
		sprintf_s(errmsg, msg, __VA_ARGS__);		\
		OutputDebugStringA(errmsg);					\
		assert(false);                              \
		return 0;                                   \
	}