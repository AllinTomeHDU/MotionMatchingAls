// Developer: Luoo


#include "MMAlsMovementSettings.h"

float FMMAlsSpeedSetting::GetSpeedForGait(const EMMAlsGait& Gait) const
{
    switch (Gait)
    {
    case EMMAlsGait::Walking:
        return WalkSpeed;
    case EMMAlsGait::Running:
        return RunSpeed;
    case EMMAlsGait::Sprinting:
        return SprintSpeed;
    }
    return 0.f;
}

FMMAlsSpeedSetting UMMAlsMovementSettings::GetCurrentSpeedSetting(const EMMAlsRotationMode& RotationMode) const
{
    return RotationMode == EMMAlsRotationMode::VelocityDirection ? VelocitySettings : LookingSettings;
}

float UMMAlsMovementSettings::GetCurrentMaxSpeed(const EMMAlsRotationMode& RotationMode, const EMMAlsStance& Stance, 
                                                 const EMMAlsGait& Gait, const bool& bAiming) const
{
    if (bAiming) return AimingSpeed;
    if (Stance == EMMAlsStance::Crouching) return CrouchingSpeed;

    return GetCurrentSpeedSetting(RotationMode).GetSpeedForGait(Gait);
}

