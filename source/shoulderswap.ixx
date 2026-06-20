module;

#include <common.hxx>

export module shoulderswap;

import common;
import comvars;
import settings;
import natives;

export bool bShoulderSwapRight = true;
export bool bShoulderSwapActive = false;

class ShoulderSwap
{
public:
    ShoulderSwap()
    {
        FusionFix::onGameProcessEvent() += []()
        {
            static auto ss = FusionFixSettings.GetRef("PREF_SHOULDERSWAP");
            if (!ss->get())
            {
                bShoulderSwapActive = false;
                return;
            }

            static auto oldState = IsKeyboardKeyPressed('B');
            auto curState = IsKeyboardKeyPressed('B');
            if (((oldState == 0) && curState) || Natives::IsButtonJustPressed(0, BUTTON_BUMPER_LEFT))
            {
                bShoulderSwapRight = !bShoulderSwapRight;
            }
            oldState = curState;

            auto pPed = CPlayer::getLocalPlayerPed();
            bShoulderSwapActive = pPed && CPed::ComparePedTasks(pPed, TaskID::TaskSimpleAimGun);
        };
    }
} ShoulderSwap;
