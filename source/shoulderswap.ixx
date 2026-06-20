module;

#include <common.hxx>
#include <cstdio>

export module shoulderswap;

import common;
import comvars;
import settings;
import natives;

export bool bShoulderSwapRight = true;
export bool bShoulderSwapActive = false;

export void ShoulderSwapLog(const char* msg)
{
    FILE* f = nullptr;
    fopen_s(&f, "ShoulderSwap.log", "a");
    if (f)
    {
        SYSTEMTIME st; GetLocalTime(&st);
        fprintf(f, "[%02d:%02d:%02d] %s\n", st.wHour, st.wMinute, st.wSecond, msg);
        fclose(f);
    }
}

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

            static bool everRan = false;
            if (!everRan)
            {
                everRan = true;
                ShoulderSwapLog("shoulderswap onGameProcessEvent: first tick, feature enabled.");
            }

            static auto oldState = IsKeyboardKeyPressed('B');
            auto curState = IsKeyboardKeyPressed('B');
            if (((oldState == 0) && curState) || Natives::IsButtonJustPressed(0, BUTTON_BUMPER_LEFT))
            {
                bShoulderSwapRight = !bShoulderSwapRight;
                char buf[64];
                sprintf_s(buf, "key B detected, toggled to %s", bShoulderSwapRight ? "RIGHT" : "LEFT");
                ShoulderSwapLog(buf);
            }
            oldState = curState;

            auto pPed = CPlayer::getLocalPlayerPed();
            bool newActive = pPed && CPed::ComparePedTasks(pPed, TaskID::TaskSimpleAimGun);
            if (newActive != bShoulderSwapActive)
            {
                char buf[96];
                sprintf_s(buf, "aim state changed to %s (pPed=0x%p)", newActive ? "AIMING" : "not aiming", (void*)pPed);
                ShoulderSwapLog(buf);
            }
            bShoulderSwapActive = newActive;
        };
    }
} ShoulderSwap;
