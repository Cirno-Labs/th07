// The file with ScreenEffect, from 0x44a460.

#include <string.h>

#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void ClearScreen(D3DCOLOR color)
{
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    if (FAILED(g_App.d3dDevice->Present(NULL, NULL, NULL, NULL)))
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
}

void ClearViewport(D3DCOLOR color)
{
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->Func44f5c0();
    }
    g_App.viewport.X = 0;
    g_App.viewport.Y = 0;
    g_App.viewport.Width = 640;
    g_App.viewport.Height = 480;
    g_App.viewport.MinZ = 0.0f;
    g_App.viewport.MaxZ = 1.0f;
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    ClearScreen(color);
}

int ScreenEffect::OnUpdateFadeIn(ScreenEffect *effect)
{
    if (effect->duration != 0)
    {
        effect->alpha = 255.0f - effect->timer.AsFloat() * 255.0f / effect->duration;
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    if (effect->timer >= effect->duration)
    {
        return 0;
    }
    effect->timer.Tick();
    return 1;
}

void DrawColorRect(float *rect, D3DCOLOR color)
{
    g_SpriteEngine->Func44f5c0();
    VertexDiffuseXyzrwh vertices[4];

    vertices[0].pos = D3DXVECTOR3(rect[0], rect[1], 0.0f);
    vertices[1].pos = D3DXVECTOR3(rect[2], rect[1], 0.0f);
    vertices[2].pos = D3DXVECTOR3(rect[0], rect[3], 0.0f);
    vertices[3].pos = D3DXVECTOR3(rect[2], rect[3], 0.0f);
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    vertices[0].diffuse = vertices[1].diffuse = vertices[2].diffuse = vertices[3].diffuse = color;
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_App.flags.noDepthTest)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

void DrawGradientRect(float *rect, D3DCOLOR topLeft, D3DCOLOR topRight, D3DCOLOR bottomLeft, D3DCOLOR bottomRight)
{
    g_SpriteEngine->Func44f5c0();
    VertexDiffuseXyzrwh vertices[4];

    vertices[0].pos = D3DXVECTOR3(rect[0], rect[1], 0.0f);
    vertices[1].pos = D3DXVECTOR3(rect[2], rect[1], 0.0f);
    vertices[2].pos = D3DXVECTOR3(rect[0], rect[3], 0.0f);
    vertices[3].pos = D3DXVECTOR3(rect[2], rect[3], 0.0f);
    vertices[0].w = vertices[1].w = vertices[2].w = vertices[3].w = 1.0f;
    vertices[0].diffuse = topLeft;
    vertices[1].diffuse = topRight;
    vertices[2].diffuse = bottomLeft;
    vertices[3].diffuse = bottomRight;
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_App.flags.noDepthTest)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    if (!g_App.flags.noColorBlending)
    {
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

int ScreenEffect::OnDrawScreen(ScreenEffect *effect)
{
    float area[4];

    area[0] = 0.0f;
    area[1] = 0.0f;
    area[2] = 640.0f;
    area[3] = 480.0f;
    g_SpriteEngine->Func44f5c0();
    g_App.viewport.X = 0;
    g_App.viewport.Y = 0;
    g_App.viewport.Width = 640;
    g_App.viewport.Height = 480;
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    DrawColorRect(area, effect->alpha << 24 | effect->color);
    return 1;
}

int ScreenEffect::OnUpdateFadeOut(ScreenEffect *effect)
{
    if (effect->duration != 0)
    {
        effect->alpha = effect->timer.AsFloat() * 255.0f / effect->duration;
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    if (effect->timer >= effect->duration)
    {
        return 0;
    }
    effect->timer.Tick();
    return 1;
}

int ScreenEffect::OnDrawPlayArea(ScreenEffect *effect)
{
    float area[4];

    area[0] = 32.0f;
    area[1] = 16.0f;
    area[2] = 416.0f;
    area[3] = 464.0f;
    DrawColorRect(area, effect->alpha << 24 | effect->color);
    return 1;
}

int ScreenEffect::OnUpdateFlash(ScreenEffect *effect)
{
    if (effect->timer < effect->duration)
    {
        effect->alpha = ((effect->flashColor >> 24) & 0xff) -
                        (int)(((effect->flashColor >> 24) & 0xff) * effect->timer.AsFloat() / effect->duration);
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    else
    {
        effect->alpha = 0;
        effect->flashes--;
        if (effect->flashes <= 0)
        {
            return 0;
        }
        effect->timer.Set(0);
    }
    effect->timer.Tick();
    return 1;
}
