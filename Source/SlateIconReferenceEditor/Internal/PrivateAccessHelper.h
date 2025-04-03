// Copyright 2025, Aquanox.

#pragma once

#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
#include "Misc/DefinePrivateMemberPtr.h"
#else

#include "HAL/PreprocessorHelpers.h"
#include "Templates/Identity.h"

namespace UE_Core_Private
{
	template <auto Storage, auto PtrToMember>
	struct TPrivateAccess
	{
		TPrivateAccess()
		{
			*Storage = PtrToMember;
		}

		static TPrivateAccess Instance;
	};

	template <auto Storage, auto PtrToMember>
	TPrivateAccess<Storage, PtrToMember> TPrivateAccess<Storage, PtrToMember>::Instance;
}

#define UE_DEFINE_PRIVATE_MEMBER_PTR(InType, Name, Class, Member) \
	TIdentity<PREPROCESSOR_REMOVE_OPTIONAL_PARENS(InType)>::Type PREPROCESSOR_REMOVE_OPTIONAL_PARENS(Class)::* Name; \
	template struct UE_Core_Private::TPrivateAccess<&Name, &PREPROCESSOR_REMOVE_OPTIONAL_PARENS(Class)::Member>

#endif
