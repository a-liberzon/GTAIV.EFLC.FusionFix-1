module;

#include <common.hxx>
#include <cstdio>

export module shoulderswap;

import common;
import comvars;
import settings;
import natives;

// Same technique LibertyTweaks uses: there is no simple "aim camera side" variable
// to flip. Instead, attach an invisible, collidable object to the player's root
// bone near one shoulder - the game's own camera collision-avoidance pushes the
// aim camera away from it, indirectly producing the shoulder-swap effect.

constexpr uint32_t BONE_ROOT = 0;

static void ShoulderSwapLog(const char* msg)
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
                return;

            static bool everRan = false;
            if (!everRan)
            {
                everRan = true;
                ShoulderSwapLog("shoulderswap: feature enabled, using object-attach technique.");
            }

            Ped pedHandle = 0;
            Natives::GetPlayerChar(Natives::ConvertIntToPlayerIndex(Natives::GetPlayerId()), &pedHandle);
            if (!pedHandle)
                return;

            static bool isSwapped = false;
            static Object swapObj = 0;

            bool inCar = Natives::IsCharInAnyCar(pedHandle);
            auto pPed = CPlayer::getLocalPlayerPed();
            bool isAiming = !inCar && pPed && CPed::ComparePedTasks(pPed, TaskID::TaskSimpleAimGun);

            static auto oldKeyState = IsKeyboardKeyPressed('B');
            auto curKeyState = IsKeyboardKeyPressed('B');
            bool toggled = (curKeyState && !oldKeyState) || Natives::IsButtonJustPressed(0, BUTTON_BUMPER_LEFT);
            oldKeyState = curKeyState;

            if (toggled && isAiming && !Natives::IsPauseMenuActive())
            {
                isSwapped = !isSwapped;
                char buf[64];
                sprintf_s(buf, "toggled, isSwapped=%s", isSwapped ? "true" : "false");
                ShoulderSwapLog(buf);
            }

            if (isAiming)
            {
                if (isSwapped)
                {
                    if (swapObj == 0)
                    {
                        float px = 0, py = 0, pz = 0;
                        Natives::GetCharCoordinates(pedHandle, &px, &py, &pz);
                        uint32_t model = Natives::GetHashKey((char*)"bm_poolcue");
                        Natives::CreateObject(model, px, py, pz + 20.0f, &swapObj, true);
                        Natives::SetObjectDynamic(swapObj, false);
                        Natives::AttachObjectToPed(swapObj, pedHandle, BONE_ROOT, 0.45f, 0.1f, 0.2f, 0.0f, 0.0f, 0.0f, 0);
                        Natives::SetObjectVisible(swapObj, false);
                        ShoulderSwapLog("created swap object and attached to ped.");
                    }
                    else
                    {
                        Natives::SetObjectCollision(swapObj, true);
                        Natives::SetObjectAlpha(swapObj, 0);
                        float heading = 0;
                        Natives::GetCharHeading(pedHandle, &heading);
                        Natives::SetObjectHeading(swapObj, heading);
                    }
                }
                else if (swapObj != 0)
                {
                    Natives::DeleteObject(&swapObj);
                    ShoulderSwapLog("deleted swap object (un-swapped while aiming).");
                }
            }
            else
            {
                if (swapObj != 0)
                {
                    Natives::DeleteObject(&swapObj);
                    ShoulderSwapLog("deleted swap object (no longer aiming).");
                }
            }
        };
    }
} ShoulderSwap;
