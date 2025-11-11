// Developer: Luoo


#include "MMAlsRagdollSettings.h"


UAnimMontage* UMMAlsRagdollSettings::GetRagdollUpAnimation(const EMMAlsOverlayPose& OverlayPose, const bool bFaceUp)
{
    if (bFaceUp)
    {
        return *GetUpAnimationMap_FaceUp.Find(OverlayPose);
    }
    else
    {
        return *GetUpAnimationMap_FaceDown.Find(OverlayPose);
    }
}
