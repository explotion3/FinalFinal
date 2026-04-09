#pragma once

#include "CoreMinimal.h"
#include "FinalIds.generated.h"

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalCharacterId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalCharacterId() = default;
	explicit FFinalCharacterId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalCharacterId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalCharacterId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalCardId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalCardId() = default;
	explicit FFinalCardId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalCardId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalCardId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalEnemyId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalEnemyId() = default;
	explicit FFinalEnemyId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalEnemyId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalEnemyId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalStatusId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalStatusId() = default;
	explicit FFinalStatusId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalStatusId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalStatusId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalPassiveId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalPassiveId() = default;
	explicit FFinalPassiveId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalPassiveId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalPassiveId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalRelicId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalRelicId() = default;
	explicit FFinalRelicId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalRelicId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalRelicId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalEventId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalEventId() = default;
	explicit FFinalEventId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalEventId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalEventId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalEncounterId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalEncounterId() = default;
	explicit FFinalEncounterId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalEncounterId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalEncounterId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalUltimateId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalUltimateId() = default;
	explicit FFinalUltimateId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalUltimateId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalUltimateId& Id) { return GetTypeHash(Id.Value); }
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalRuleConfigId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Ids")
	FName Value = NAME_None;

	FFinalRuleConfigId() = default;
	explicit FFinalRuleConfigId(const FName InValue)
		: Value(InValue)
	{
	}

	bool IsValid() const { return !Value.IsNone(); }
	FString ToString() const { return Value.ToString(); }
	bool operator==(const FFinalRuleConfigId& Other) const { return Value == Other.Value; }
	friend uint32 GetTypeHash(const FFinalRuleConfigId& Id) { return GetTypeHash(Id.Value); }
};
