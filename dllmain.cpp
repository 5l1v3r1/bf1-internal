#include <Windows.h>
#include <stdio.h>
#include "frosbite.h"
#include "obfuscationmgr.h"
#include "DirectOverlay.h"
#include <iostream>

#define _CRT_SECURE_NO_WARNINGS

#define pasteColor(x) x.r, x.g, x.b
#define pasteColorA(x) x.r, x.g, x.b, x.a
#define checkSetting(teamval, enemyval) (((pLocalPlayer->teamId == pPlayer->teamId) && teamval) || ((pLocalPlayer->teamId != pPlayer->teamId) && enemyval))

D3DXCOLOR occludedEnemyColor = D3DXCOLOR(255, 0, 0, 255);
D3DXCOLOR visibleEnemyColor = D3DXCOLOR(255, 255, 0, 255);

bool Menu = false;
bool EnemyESP = true;
bool BoxESP = true;
bool LineESP = false;
bool NameEsp = false;

bool WorldToScreen(D3DXVECTOR3& vLocVec4)
{
    D3DXMATRIXA16 ViewProj = s_ViewProj;
    float mX = s_Width / 2;
    float mY = s_Height / 2;

    float w = ViewProj(0, 3) * vLocVec4.x + ViewProj(1, 3) * vLocVec4.y + ViewProj(2, 3) * vLocVec4.z + ViewProj(3, 3);

    if (w < 0.65f)
    {
        vLocVec4.z = w;
        return false;
    }
    float x = ViewProj(0, 0) * vLocVec4.x + ViewProj(1, 0) * vLocVec4.y + ViewProj(2, 0) * vLocVec4.z + ViewProj(3, 0);
    float y = ViewProj(0, 1) * vLocVec4.x + ViewProj(1, 1) * vLocVec4.y + ViewProj(2, 1) * vLocVec4.z + ViewProj(3, 1);
    vLocVec4.x = (mX + mX * x / w);
    vLocVec4.y = (mY - mY * y / w);
    vLocVec4.z = w;
    return true;
}

float Distance3D(D3DXVECTOR3  v1, D3DXVECTOR3 v2)
{
    float x_d = (v2.x - v1.x);
    float y_d = (v2.y - v1.y);
    float z_d = (v2.z - v1.z);
    return sqrt((x_d * x_d) + (y_d * y_d) + (z_d * z_d));
}

void menu()
{
    if (Menu)
    {
        DrawBox(5, 10, 150, 140, 0.0f, 0.f, 0.f, 0.f, 100, true);
        DrawString("Bf1 Skill V6.9", 17, 10, 8, 255.f, 255.f, 255.f, 255.f);

        if (EnemyESP)
            DrawString("ON", 13, 10 + 110, 10 + 20, 0.f, 255.f, 0.f, 255.f);
        else
            DrawString("OFF", 13, 10 + 110, 10 + 20, 255.f, 0.f, 0.f, 255.f);

        DrawString("[F1] Enemy ESP >>", 13, 10, 10 + 20, 255.f, 255.f, 255.f, 255.f);

        if (BoxESP)
            DrawString("ON", 13, 10 + 93, 10 + 40, 0.f, 255.f, 0.f, 255.f);
        else
            DrawString("OFF", 13, 10 + 93, 10 + 40, 255.f, 0.f, 0.f, 255.f);

        DrawString("[F2] Box ESP >>", 13, 10, 10 + 40, 255.f, 255.f, 255.f, 255.f);

        if (LineESP)
            DrawString("ON", 13, 10 + 95, 10 + 60, 0.f, 255.f, 0.f, 255.f);
        else
            DrawString("OFF", 13, 10 + 95, 10 + 60, 255.f, 0.f, 0.f, 255.f);

        DrawString("[F3] Line ESP >>", 13, 10, 10 + 60, 255.f, 255.f, 255.f, 255.f);
    }
}

void drawLoop(int width, int height) {
        menu();
    
        s_Width = width;
        s_Height = height;

        fb::GameRenderer* pGameRenderer = fb::GameRenderer::GetInstance();

        if (!ValidPointer(pGameRenderer) || !ValidPointer(pGameRenderer->renderView))
            return;

        s_ViewProj = pGameRenderer->renderView->viewProj;

        fb::ClientPlayer* pLocalPlayer = GetLocalPlayer();

        if (!ValidPointer(pLocalPlayer))
            return;

        fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->clientSoldierEntity;

        if (!ValidPointer(pLocalSoldier))
            return;

        for (int i = 0; i < 64; i++)
        {
            fb::ClientPlayer* pPlayer = GetPlayerById(i);

            if (!ValidPointer(pPlayer))
                continue;

            if (pPlayer == pLocalPlayer)
                continue;

            fb::ClientSoldierEntity* pSoldierEntity = pPlayer->clientSoldierEntity;

            if (!ValidPointer(pSoldierEntity))
                continue;

            // check that their health component is valid
            if (!ValidPointer(pSoldierEntity->healthcomponent))
                continue;

            // check that their health is above 0
            if (pSoldierEntity->healthcomponent->m_Health <= 0)
                continue;

            // set up our color to render with
            D3DXCOLOR chosenColor;
            // if the team IDs are the same
            if (pPlayer->teamId == pLocalPlayer->teamId) {
                continue;
            }
            else {
                // the soldier is an enemy, now check if they're occluded (not visible)
                if (pSoldierEntity->occluded) {
                    // if they aren't visible, set the color to their occluded color
                    chosenColor = occludedEnemyColor;
                }
                else {
                    // otherwise set it to Settings::visibleEnemyColor
                    chosenColor = visibleEnemyColor;
                }
            }

            // get the position of the players foot, and their head
            D3DXVECTOR3 footLocation = pSoldierEntity->location;
            D3DXVECTOR3 headLocation = footLocation;
            // hardcode a head height offset, based on pose

            if (pSoldierEntity->poseType == 0)
                headLocation.y += 1.6;

            if (pSoldierEntity->poseType == 1)
                headLocation.y += 1;

            if (pSoldierEntity->poseType == 2)
                headLocation.y += .5;

            if (WorldToScreen(footLocation) && WorldToScreen(headLocation)) {
                float w2sHeight = Distance3D(footLocation, headLocation);
                float w2sWidth = w2sHeight;

                // adjust the w2sWidth so the boxes aren't proportional
                if (pSoldierEntity->poseType == 0)
                    w2sWidth /= 2;

                if (pSoldierEntity->poseType == 1)
                    w2sWidth /= 1.5;

                // get the (world) distance between the localplayer and the enemy player
                float distanceToPlayer = Distance3D(pLocalSoldier->location, pSoldierEntity->location);

                if (EnemyESP)
                {
                    if (BoxESP)
                    {
                        DrawBox(headLocation.x - w2sWidth / 2, headLocation.y, w2sWidth, w2sHeight, 2, chosenColor.r, chosenColor.g, chosenColor.b, chosenColor.a, false);
                    }
                        
                    if (LineESP)
                    {
                        DrawLine(s_Width / 2, s_Height, footLocation.x, footLocation.y, 2, chosenColor.r, chosenColor.g, chosenColor.b, chosenColor.a);
                    }
                }
            }
        }
}

DWORD WINAPI DllThread(PVOID pThreadParameter)
{
    UNREFERENCED_PARAMETER(pThreadParameter);

    DirectOverlaySetOption(D2DOV_FONT_COURIER);
    DirectOverlaySetup(drawLoop);

    for (;;) 
    { 
        Sleep(100); 
    }
}

DWORD Menuthread(LPVOID in)
{
    while (1)
    {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Menu = !Menu;
        }

        if (Menu)
        {
            if (GetAsyncKeyState(VK_F1) & 1) {
                EnemyESP = !EnemyESP;
            }

            if (GetAsyncKeyState(VK_F2) & 1) {
                BoxESP = !BoxESP;
            }

            if (GetAsyncKeyState(VK_F3) & 1) {
                LineESP = !LineESP;
            }
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DllThread, lpReserved, 0, NULL);
        CreateThread(NULL, NULL, Menuthread, NULL, NULL, NULL);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

