// The file with Unknown1347b00, th06's Stage, from 0x404fe0, as th06's
// stage.cpp.

#include <math.h>
#include <string.h>

#include "effect.h"
#include "sprite.h"
#include "unsorted.h"

// An inline of unsorted.h, probably, whose out-of-line copy the original
// keeps with this file's; defined here so that the other files' calls
// stay calls. unk_ac has to be unk_34[2] plus unk_b4[2] for each byte
// ComputeChecksum sums (0x84 past unk_ac), unk_b0 plus unk_8c[3] unk_9614.
int Unknown626270::ChecksumMismatch()
{
    return this->stats->unk_ac == this->stats->unk_34[2] + this->stats->unk_b4[2] * ((BYTE *)&this->stats->unk_ac - (BYTE *)this->stats->unk_34 + 0x84) &&
                   this->stats->unk_b0 + this->stats->unk_8c[3] == (int)this->unk_9614
               ? FALSE
               : TRUE;
}

Unknown1347b00::Unknown1347b00()
{
    memset(this, 0, sizeof(Unknown1347b00));
    this->camera.eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    this->camera.at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    this->camera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    this->camera.fov = 0.5235988f;
    this->unk_50f8 = this->camera;
    this->unk_5138 = this->camera;
}

float HermiteInterpolate(float p0, float p1, float m0, float m1, float t)
{
    float f00;
    float f10;
    float f01;
    float f11;

    f00 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    f01 = t * t * (3.0f - 2.0f * t);
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    return f00 * p0 + f01 * p1 + f10 * m0 + f11 * m1;
}

void UpdateEasedVector(Unknown1347b00 *stage, int i, D3DXVECTOR3 *out, D3DXVECTOR3 *from, D3DXVECTOR3 *to, D3DXVECTOR3 *m0, D3DXVECTOR3 *m1)
{
    float t;

    if (stage->unk_5248[i] < stage->unk_5238[i])
    {
        stage->unk_5248[i].Tick();
        t = stage->unk_5248[i].AsFloat() / stage->unk_5238[i];
    }
    else
    {
        stage->unk_5248[i].Set(stage->unk_5238[i]);
        t = 1.0f;
        stage->unk_5238[i] = 0;
    }
    switch (stage->unk_5278[i])
    {
    case 1:
        t = 1.0f - (t = 1.0f - t) * t;
        break;
    case 2:
        t = 1.0f - (t = 1.0f - t) * t * t;
        break;
    case 3:
        t = 1.0f - (t = 1.0f - t) * t * t * t;
        break;
    case 4:
        t = t * t;
        break;
    case 5:
        t = t * t * t;
        break;
    case 6:
        t = t * t * t * t;
        break;
    }
    if (stage->unk_5278[i] != 7)
    {
        *out = *to - *from;
        *out = t * *out + *from;
    }
    else
    {
        out->x = HermiteInterpolate(from->x, to->x, m0->x, m1->x, t);
        out->y = HermiteInterpolate(from->y, to->y, m0->y, m1->y, t);
        out->z = HermiteInterpolate(from->z, to->z, m0->z, m1->z, t);
    }
}

void Unknown1347b00::SetTintColor(D3DCOLOR color)
{
    ColorBytes c;

    if (this->unk_52ac.a == 0)
    {
        this->unk_52ac.color = color;
    }
    else
    {
        c.color = color;
        this->unk_52ac.r = (DWORD)(c.r + this->unk_52ac.r) >> 1;
        this->unk_52ac.g = (DWORD)(c.g + this->unk_52ac.g) >> 1;
        this->unk_52ac.b = (DWORD)(c.b + this->unk_52ac.b) >> 1;
        this->unk_52ac.a = (DWORD)(c.a + this->unk_52ac.a) >> 1;
    }
}

// Channel `c` scaled by `factor`, 0x80 for 1, up to 0xff.
inline DWORD ModulateChannel(BYTE c, BYTE factor)
{
    DWORD channel = (DWORD)c * factor >> 7;
    if (channel >= 0x100)
    {
        channel = 0xff;
    }
    return channel;
}

// Scales byte `i` of `color` by byte `i` of `factor`.
#define MODULATE_CHANNEL(color, factor, i) \
    ((BYTE *)&(color))[i] = ModulateChannel(((BYTE *)&(color))[i], ((BYTE *)&(factor))[i])

int Unknown1347b00::OnDraw(Unknown1347b00 *stage)
{
    D3DVIEWPORT8 viewport;
    D3DCOLOR fogColor;

    g_SpriteEngine->BeginQuadBatch();
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_SpriteEngine->ClearUnk8();
    g_SpriteEngine->ClearUnk2e4d4();
    if (!g_App.flags.noFog)
    {
        g_App.DisableFog();
    }
    g_SpriteEngine->FlushQuadBatch();
    if (stage->unk_51c != 0)
    {
        viewport.X = 32;
        viewport.Y = 16;
        viewport.Width = 384;
        viewport.Height = 448;
        g_App.d3dDevice->SetViewport(&viewport);
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
        stage->unk_51c = 0;
    }
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    if (stage->unk_52ac.a > 0)
    {
        g_SpriteEngine->SetUnk0(stage->unk_52ac.color);
    }
    stage->unk_52ac.a = 0;
    stage->unk_52ac.r = 0x80;
    stage->unk_52ac.g = 0x80;
    stage->unk_52ac.b = 0x80;
    if (stage->spellCardState <= 1 && !g_Unknown49fbf0.IsLoadingShown())
    {
        if (stage->unk_4.spriteIndex > 0)
        {
            g_SpriteEngine->DrawAndFlush(&stage->unk_4);
        }
        if (stage->unk_250.spriteIndex > 0)
        {
            g_SpriteEngine->DrawAndFlush(&stage->unk_250);
        }
    }
    if (stage->unk_4d8 != 0)
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, stage->unk_4d8, 1.0f, 0);
    }
    else
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->unk_4d8, 1.0f, 0);
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    if (g_SpriteEngine->unk_4 == 0)
    {
        g_App.SetRenderState(D3DRS_FOGCOLOR, stage->fog.color);
    }
    else
    {
        fogColor = stage->fog.color;
        MODULATE_CHANNEL(fogColor, g_SpriteEngine->unk_0, 2);
        MODULATE_CHANNEL(fogColor, g_SpriteEngine->unk_0, 1);
        MODULATE_CHANNEL(fogColor, g_SpriteEngine->unk_0, 0);
        g_App.SetRenderState(D3DRS_FOGCOLOR, fogColor);
    }
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->fog.nearPlane);
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->fog.farPlane);
    if (!g_App.flags.noFog)
    {
        g_App.EnableFog();
    }
    if (stage->spellCardState <= 1 && !g_Unknown49fbf0.IsLoadingShown())
    {
        stage->DrawLayer(0);
        stage->DrawLayer(1);
    }
    return 1;
}

int Unknown1347b00::OnDraw2(Unknown1347b00 *stage)
{
    float fog;
    int opacity;
    float area[4];
    int j;

    if (stage->spellCardState <= 1 && !g_Unknown49fbf0.IsLoadingShown())
    {
        stage->DrawLayer(2);
        stage->DrawLayer(3);
        if (!g_App.flags.noFog)
        {
            g_App.DisableFog();
        }
        g_EffectManager.OnDraw2();
        if (stage->spellCardState == 1)
        {
            area[0] = 32.0f;
            area[1] = 16.0f;
            area[2] = 416.0f;
            area[3] = 464.0f;
            opacity = stage->spellCardFrames * 0xff / 60;
            g_SpriteEngine->FlushQuadBatch();
            g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
            if (!g_App.flags.noFog)
            {
                g_App.SetRenderState(D3DRS_FOGENABLE, FALSE);
            }
            DrawColorRect(area, opacity << 24);
        }
    }
    g_SpriteEngine->FlushQuadBatch();
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_App.flags.noFog)
    {
        g_App.DisableFog();
    }
    if (stage->spellCardState >= 1)
    {
        for (j = 0; j < stage->unk_520; j++)
        {
            g_SpriteEngine->DrawAndFlush(&stage->unk_528[j]);
        }
    }
    g_SpriteEngine->SetUnk2e4d4(0);
    stage->SetupFixedCamera();
    g_App.d3dDevice->SetViewport(&g_App.viewport);
    fog = 1000.0f;
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog);
    fog = 2000.0f;
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog);
    if (stage->unk_52b0 == 0)
    {
        g_SpriteEngine->ResetUnk0();
    }
    stage->unk_52b0 = 0;
    return 1;
}

int Unknown1347b00::AddedCallback(Unknown1347b00 *stage)
{
    int i;

    stage->scriptTime.Set(0);
    stage->scriptIndex = 0;
    stage->cameraPos.x = 0.0f;
    stage->cameraPos.y = 0.0f;
    stage->cameraPos.z = 0.0f;
    stage->spellCardState = 0;
    stage->fogFadeFrames = 0;
    switch (g_Unknown626270.stage)
    {
    case 1:
        if (g_SpriteEngine->LoadAnm(5, "data/stg1bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    case 2:
        if (g_SpriteEngine->LoadAnm(5, "data/stg2bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    case 3:
        if (g_SpriteEngine->LoadAnm(5, "data/stg3bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    case 4:
        if (g_SpriteEngine->LoadAnm(5, "data/stg4bg.anm", 0x300) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(6, "data/stg4bg2.anm", 0x310) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(7, "data/stg4bg3.anm", 0x320) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(8, "data/stg4bg4.anm", 0x330) != 0)
        {
            return -1;
        }
        if (g_SpriteEngine->LoadAnm(9, "data/stg4bg5.anm", 0x340) != 0)
        {
            return -1;
        }
        break;
    case 5:
        if (g_SpriteEngine->LoadAnm(5, "data/stg5bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    case 6:
        if (g_SpriteEngine->LoadAnm(5, "data/stg6bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    case 7:
        if (g_SpriteEngine->LoadAnm(5, "data/stg7bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    case 8:
        if (g_SpriteEngine->LoadAnm(5, "data/stg8bg.anm", 0x300) != 0)
        {
            return -1;
        }
        break;
    }
    if (stage->Load(g_StageDataFiles[g_Unknown626270.stage]) != 0)
    {
        return -1;
    }
    stage->fog.color = 0xff000000;
    stage->fog.nearPlane = 200.0f;
    stage->fog.farPlane = 500.0f;
    stage->camera.eye = D3DXVECTOR3(0.0f, 0.0f, 1000.0f);
    stage->camera.at = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    stage->camera.up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    stage->camera.fov = 0.5235988f;
    stage->unk_50f8 = stage->camera;
    stage->unk_5138 = stage->camera;
    for (i = 0; i < 4; i++)
    {
        stage->unk_5238[i] = 0;
        stage->unk_5248[i].Set(0);
    }
    stage->unk_50f4 = 0;
    return 0;
}

int Unknown1347b00::DeletedCallback(Unknown1347b00 *stage)
{
    g_SpriteEngine->ReleaseAnm(5);
    g_SpriteEngine->ReleaseAnm(6);
    g_SpriteEngine->ReleaseAnm(7);
    g_SpriteEngine->ReleaseAnm(8);
    g_SpriteEngine->ReleaseAnm(9);
    if (stage->sprites != NULL)
    {
        MemFree(stage->sprites);
        stage->sprites = NULL;
    }
    if (stage->data != NULL)
    {
        MemFree(stage->data);
        stage->data = NULL;
    }
    return 0;
}

int Unknown1347b00::RegisterChain(int stage)
{
    Unknown1347b00 *s = &g_Unknown1347b00;

    memset(s, 0, sizeof(Unknown1347b00));
    s->data = NULL;
    s->unk_4c4 = 0;
    s->stage = stage;
    g_Unknown1347b00CalcChain.SetCallback((ChainCallback)Unknown1347b00::OnUpdate);
    g_Unknown1347b00CalcChain.addedCallback = (ChainCallback)Unknown1347b00::AddedCallback;
    g_Unknown1347b00CalcChain.deletedCallback = (ChainCallback)Unknown1347b00::DeletedCallback;
    g_Unknown1347b00CalcChain.arg = s;
    if (g_Chain.AddToCalcChain(&g_Unknown1347b00CalcChain, 7) != 0)
    {
        return -1;
    }
    g_Unknown1347b00DrawChain.SetCallback((ChainCallback)Unknown1347b00::OnDraw);
    g_Unknown1347b00DrawChain.arg = s;
    g_Chain.AddToDrawChain(&g_Unknown1347b00DrawChain, 3);
    g_Unknown1347b00DrawChain2.SetCallback((ChainCallback)Unknown1347b00::OnDraw2);
    g_Unknown1347b00DrawChain2.arg = s;
    g_Chain.AddToDrawChain(&g_Unknown1347b00DrawChain2, 4);
    return 0;
}

void Unknown1347b00::RemoveChain()
{
    g_Chain.Remove(&g_Unknown1347b00CalcChain);
    g_Chain.Remove(&g_Unknown1347b00DrawChain);
    g_Chain.Remove(&g_Unknown1347b00DrawChain2);
}

int Unknown1347b00::Load(char *dataPath)
{
    int count;
    int i;
    StageObject *ob;
    StageQuad *quad;

    this->data = (StageData *)LoadFileData(dataPath, 0);
    if (this->data == NULL)
    {
        LogPrintf(&g_Log, "ステージデータが見つかりません。データが壊れています\r\n");
        return -1;
    }
    this->objectCount = this->data->objectCount;
    this->quadCount = this->data->quadCount;
    this->instances = (StageInstance *)(this->data->instancesOffset + (int)this->data);
    this->script = (StageInstr *)(this->data->scriptOffset + (int)this->data);
    this->objects = (StageObject **)((char *)this->data + 0x490);
    for (i = 0; i < this->objectCount; i++)
    {
        this->objects[i] = (StageObject *)((int)this->objects[i] + (int)this->data);
    }
    this->sprites = (Sprite *)MemAlloc(this->quadCount * sizeof(Sprite));
    for (i = 0, count = 0; i < this->objectCount; i++)
    {
        ob = this->objects[i];
        ob->flags = 1;
        quad = ob->quads;
        while (quad->type >= 0)
        {
            g_SpriteEngine->StartScriptAtOrigin(&this->sprites[count], quad->script + 0x300);
            quad->spriteIndex = count;
            count++;
            quad = (StageQuad *)((char *)quad + quad->size);
        }
    }
    return 0;
}

int Unknown1347b00::StepObjects()
{
    StageQuad *altQuad;
    int busy;
    int i;
    Sprite *sprite;
    StageObject *obj;
    StageQuad *piece;

    for (i = 0; i < this->objectCount; i++)
    {
        obj = this->objects[i];
        if (obj->flags & 1)
        {
            busy = 0;
            piece = obj->quads;
            while (piece->type >= 0)
            {
                sprite = &this->sprites[piece->spriteIndex];
                switch (piece->type)
                {
                case 0:
                    g_SpriteEngine->Step(sprite);
                    break;
                case 1:
                    altQuad = piece;
                    g_SpriteEngine->Step(sprite);
                    break;
                }
                if (sprite->scriptPos != NULL)
                {
                    busy++;
                }
                piece = (StageQuad *)((char *)piece + piece->size);
            }
            if (busy == 0)
            {
                obj->flags &= ~1;
            }
        }
    }
    return 0;
}

void Unknown1347b00::SetupFixedCamera()
{
    float midX;
    float halfHeight;
    float aspectXY;
    float fovAngle;
    float zDist;

    midX = (float)g_App.viewport.Width / 2.0f;
    halfHeight = (float)g_App.viewport.Height / 2.0f;
    aspectXY = (float)g_App.viewport.Width / (float)g_App.viewport.Height;
    fovAngle = 0.31415927f;
    zDist = halfHeight / tanf(fovAngle / 2.0f);
    {
        D3DXVECTOR3 up(0.0f, -1.0f, 0.0f);
        D3DXVECTOR3 lookAt(midX, halfHeight, 0.0f);
        D3DXVECTOR3 from(midX, halfHeight, zDist);
        D3DXMatrixLookAtLH(&g_App.viewMatrix, &from, &lookAt, &up);
        D3DXMatrixPerspectiveFovLH(&g_App.projectionMatrix, fovAngle, aspectXY, 1.0f, 10000.0f);
        g_App.d3dDevice->SetTransform(D3DTS_VIEW, &g_App.viewMatrix);
        g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_App.projectionMatrix);
    }
}

void Unknown1347b00::SetupCamera()
{
    D3DXVECTOR3 at = this->camera.at + this->camera.eye;

    D3DXMatrixLookAtLH(&g_App.viewMatrix, &this->camera.eye, &at, &this->camera.up);
    D3DXMatrixPerspectiveFovLH(&g_App.projectionMatrix, this->camera.fov, (float)g_App.viewport.Width / (float)g_App.viewport.Height, 30.0f, 1800.0f);
    g_App.d3dDevice->SetTransform(D3DTS_VIEW, &g_App.viewMatrix);
    g_App.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_App.projectionMatrix);
    D3DXVec3Cross(&this->camera.unk_30, &this->camera.at, &this->camera.up);
    D3DXVec3Normalize(&this->camera.unk_30, &this->camera.unk_30);
}
