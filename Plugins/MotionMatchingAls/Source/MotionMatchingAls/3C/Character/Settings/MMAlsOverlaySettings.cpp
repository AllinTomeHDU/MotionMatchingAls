// Developer: Luoo


#include "MMAlsOverlaySettings.h"

TSubclassOf<UAnimInstance> UMMAlsOverlaySettings::GetOverlayBaseClass(EMMAlsOverlayBase OverlayBase)
{
	return OverlayBaseAnimClassMap.Contains(OverlayBase) ? *(OverlayBaseAnimClassMap.Find(OverlayBase)) : nullptr;
}

FMMAlsOverlayPoseSetting UMMAlsOverlaySettings::GetOverlayPoseSetting(EMMAlsOverlayPose OverlayPose)
{
	return *(OverlayPoseAnimClassMap.Find(OverlayPose));
}
