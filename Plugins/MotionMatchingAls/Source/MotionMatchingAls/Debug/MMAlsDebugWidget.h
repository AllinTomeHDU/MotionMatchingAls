// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MMAlsDebugWidget.generated.h"

class UMMAlsDebugComponent;

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsDebugWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetDebugComp(UMMAlsDebugComponent* Comp);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMMAlsDebugComponent* DebugComp;
	
};
